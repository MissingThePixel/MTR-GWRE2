using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using System.Windows.Forms;

sealed class LauncherSettings {
    public string GameFolder { get; set; }
    public int ResolutionScale { get; set; }
    public bool Fullscreen { get; set; }
}

sealed class Gw2Launcher : Form {
    readonly string root = AppDomain.CurrentDomain.BaseDirectory;
    readonly TextBox folder = new TextBox();
    readonly ComboBox resolution = new ComboBox();
    readonly CheckBox fullscreen = new CheckBox();
    readonly Button play = new Button();
    readonly Label status = new Label();
    Process game;

    [STAThread]
    static void Main() {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new Gw2Launcher());
    }

    public Gw2Launcher() {
        Text = "MTR-GWRE2";
        ClientSize = new Size(780, 420);
        FormBorderStyle = FormBorderStyle.FixedSingle;
        MaximizeBox = false;
        StartPosition = FormStartPosition.CenterScreen;
        AutoScaleMode = AutoScaleMode.Dpi;
        BackColor = Color.FromArgb(18, 18, 27);
        ForeColor = Color.FromArgb(235, 236, 244);
        Font = new Font("Segoe UI", 10);
        Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath);
        var picture = new PictureBox { Location = new Point(0, 0), Size = new Size(330, 420), SizeMode = PictureBoxSizeMode.Zoom };
        using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("launchericon.png")) {
            if (stream != null) using (var image = Image.FromStream(stream)) picture.Image = new Bitmap(image);
        }
        Controls.Add(picture);
        AddLabel("GEOMETRY WARS", 354, 26, 400, 35, 21, FontStyle.Bold);
        AddLabel("Retro Evolved 2", 356, 63, 400, 30, 16, FontStyle.Regular);
        AddLabel("Game folder", 356, 118, 400, 25, 10, FontStyle.Regular);
        folder.SetBounds(356, 148, 302, 30);
        folder.BackColor = Color.FromArgb(35, 35, 47);
        folder.ForeColor = ForeColor;
        Controls.Add(folder);
        var browse = MakeButton("Browse", 668, 146, 87, 32);
        browse.Click += delegate {
            using (var dialog = new FolderBrowserDialog { Description = "Choose the extracted game folder containing default.xex", SelectedPath = Directory.Exists(folder.Text) ? folder.Text : root, ShowNewFolderButton = false }) {
                if (dialog.ShowDialog(this) == DialogResult.OK) folder.Text = dialog.SelectedPath;
            }
        };
        AddLabel("Resolution", 356, 195, 180, 25, 10, FontStyle.Regular);
        resolution.DropDownStyle = ComboBoxStyle.DropDownList;
        resolution.Items.AddRange(new object[] { "4K (3840 x 2160)", "1080p (1920 x 1080)" });
        resolution.SetBounds(356, 225, 230, 30);
        Controls.Add(resolution);
        fullscreen.Text = "Fullscreen";
        fullscreen.SetBounds(611, 225, 145, 30);
        Controls.Add(fullscreen);
        play = MakeButton("Play", 356, 286, 400, 48);
        play.BackColor = Color.FromArgb(126, 47, 224);
        play.Font = new Font("Segoe UI", 13, FontStyle.Bold);
        play.Click += delegate { StartGame(); };
        AcceptButton = play;
        status.SetBounds(356, 350, 400, 50);
        status.ForeColor = Color.FromArgb(172, 175, 192);
        status.Text = "Choose your game folder, then press Play.";
        Controls.Add(status);
        LoadSettings();
    }

    void AddLabel(string text, int x, int y, int w, int h, float size, FontStyle style) {
        Controls.Add(new Label { Text = text, Location = new Point(x, y), Size = new Size(w, h), Font = new Font("Segoe UI", size, style) });
    }

    Button MakeButton(string text, int x, int y, int w, int h) {
        var button = new Button { Text = text, Location = new Point(x, y), Size = new Size(w, h), FlatStyle = FlatStyle.Flat, BackColor = Color.FromArgb(44, 44, 59), ForeColor = ForeColor };
        button.FlatAppearance.BorderSize = 0;
        Controls.Add(button);
        return button;
    }

    void LoadSettings() {
        folder.Text = Path.Combine(root, "Game");
        resolution.SelectedIndex = 0;
        fullscreen.Checked = true;
        try {
            var path = Path.Combine(root, "launcher-settings.json");
            if (!File.Exists(path)) return;
            var saved = new JavaScriptSerializer().Deserialize<LauncherSettings>(File.ReadAllText(path));
            if (saved == null) return;
            if (!String.IsNullOrWhiteSpace(saved.GameFolder)) folder.Text = saved.GameFolder;
            resolution.SelectedIndex = saved.ResolutionScale == 1 ? 1 : 0;
            fullscreen.Checked = saved.Fullscreen;
        } catch { status.Text = "Choose your game folder to save new settings."; }
    }

    // Windows command-line quoting, including trailing backslashes in folder paths.
    internal static string Quote(string value) {
        var result = new StringBuilder("\"");
        int slashes = 0;
        foreach (char character in value) {
            if (character == '\\') { slashes++; continue; }
            if (character == '"') { result.Append('\\', slashes * 2 + 1); result.Append('"'); }
            else { result.Append('\\', slashes); result.Append(character); }
            slashes = 0;
        }
        result.Append('\\', slashes * 2);
        result.Append('"');
        return result.ToString();
    }

    void StartGame() {
        if (game != null) return;
        try {
            string gameFolder = Path.GetFullPath(folder.Text.Trim());
            if (!File.Exists(Path.Combine(gameFolder, "default.xex"))) {
                MessageBox.Show(this, "Choose the complete extracted game folder containing default.xex.", "Game folder needed", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }
            string executable = Path.Combine(root, "gw2_recompiled.exe");
            if (!File.Exists(executable)) throw new FileNotFoundException("Extract the complete release ZIP beside this launcher. The game executable is missing.");
            var saved = new LauncherSettings { GameFolder = gameFolder, ResolutionScale = resolution.SelectedIndex == 1 ? 1 : 2, Fullscreen = fullscreen.Checked };
            string settingsFile = Path.Combine(root, "launcher-settings.json");
            string temporaryFile = settingsFile + ".tmp";
            File.WriteAllText(temporaryFile, new JavaScriptSerializer().Serialize(saved));
            if (File.Exists(settingsFile)) File.Replace(temporaryFile, settingsFile, null);
            else File.Move(temporaryFile, settingsFile);
            string logDirectory = Path.Combine(root, "logs");
            Directory.CreateDirectory(logDirectory);
            string arguments = "--game_data_root " + Quote(gameFolder)
                + " --user_data_root " + Quote(Path.Combine(root, "userdata"))
                + " --cache_root " + Quote(Path.Combine(root, "cache"))
                + " --execute_unclipped_draw_vs_on_cpu --gpu_plugin=xenos --license_mask=1"
                + " --d3d12_present_vsync --resolution=1080p --resolution_scale=" + saved.ResolutionScale
                + (saved.Fullscreen ? " --fullscreen" : " --fullscreen=false")
                + " --log_level=warn --log_verbose=false --guest_frame_stats=false"
                + " --gpu_command_stats=false --gpu_slow_frame_stats=false --d3d12_fence_stats=false"
                + " --log_file " + Quote(Path.Combine(logDirectory, "geometry-wars-2.log"));
            game = new Process { StartInfo = new ProcessStartInfo(executable, arguments) { WorkingDirectory = root, UseShellExecute = false }, EnableRaisingEvents = true };
            game.Exited += delegate {
                if (!IsDisposed && IsHandleCreated) BeginInvoke((MethodInvoker)delegate {
                    int code = game.ExitCode;
                    game.Dispose(); game = null;
                    play.Enabled = true;
                    status.Text = code == 0 ? "Ready to play." : "The game closed unexpectedly. See the log in the logs folder.";
                    Show(); Activate();
                });
            };
            if (!game.Start()) throw new InvalidOperationException("The game could not be started.");
            play.Enabled = false;
            Hide();
        } catch (Exception error) {
            if (game != null) { game.Dispose(); game = null; }
            MessageBox.Show(this, error.Message, "Unable to launch", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
}
