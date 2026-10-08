package com.missingthepixel.gwre2;

import java.io.File;
import org.libsdl.app.SDLActivity;

public class GameActivity extends SDLActivity {
    @Override public void loadLibraries() {
        CrashReports.note(this, "Loading native libraries");
        try { super.loadLibraries(); }
        catch (UnsatisfiedLinkError | RuntimeException failure) {
            CrashReports.record(this, "library-load", "Native library loading failed", failure);
            throw failure;
        }
        CrashReports.note(this, "Native libraries loaded");
    }
    @Override protected void onCreate(android.os.Bundle state) {
        CrashReports.note(this, "Game activity starting");
        super.onCreate(state);
        if (SDLActivity.mBrokenLibraries) {
            CrashReports.record(this, "sdl-startup", "SDL initialization failed; inspect library-load reports and session logs", null);
        }
    }

    public int openContentFileDescriptor(String uri, String mode) {
        try {
            android.os.ParcelFileDescriptor fd = getContentResolver().openFileDescriptor(
                android.net.Uri.parse(uri), mode);
            if (fd == null) return -1;
            try { return fd.detachFd(); } finally { fd.close(); }
        } catch (java.io.IOException | SecurityException error) { return -1; }
    }
    @Override protected String[] getLibraries() {
        return new String[] { "c++_shared", "SDL3", "rexruntime", "rexgpu-xenos", "main" };
    }
    @Override protected String[] getArguments() {
        boolean diagnostic = BuildConfig.DEBUG && getIntent().getBooleanExtra("diagnostic", false);
        File game = GameStorage.findGame(this);
        if (game == null) game = GameStorage.sharedFolder();
        File saves = new File(getFilesDir(), "userdata");
        File cache = new File(getCacheDir(), "shaders-texbias-v1");
        saves.mkdirs(); cache.mkdirs();
        CrashReports.note(this, "Starting Vulkan game; game files=" + game.getAbsolutePath());
        CrashReports.exportSaved(this);
        return new String[] {
            "--game_data_root", game.getAbsolutePath(),
            "--user_data_root", saves.getAbsolutePath(),
            "--cache_root", cache.getAbsolutePath(),
            "--log_file", (BuildConfig.CRASH_LOGS || diagnostic) ? CrashReports.runtimeLog(this).getAbsolutePath() : "",
            "--android_gpu_poll_backoff_us=" + getIntent().getIntExtra("gpu_poll_backoff_us", 50),
            "--android_app=gw2_recompiled", "--gpu_plugin=xenos", "--license_mask=1", "--execute_unclipped_draw_vs_on_cpu",
            getIntent().getBooleanExtra("low_resolution", false) ? "--resolution=720p" : "--resolution=1080p", "--resolution_scale=1", "--fullscreen=true",
            "--clear_memory_page_state=" + !getIntent().getBooleanExtra("retain_cpu_pages", true),
            "--vulkan_submit_on_primary_buffer_end=" + !getIntent().getBooleanExtra("batch", false),
            "--vulkan_dynamic_rendering=" + !getIntent().getBooleanExtra("legacy_render_pass", false),
            "--gamma_render_target_as_unorm16=" + !getIntent().getBooleanExtra("compact_gamma", false),
            "--vulkan_require_vertex_pipeline_stores_and_atomics=" + !BuildConfig.MALI_COMPAT,
            "--vulkan_allow_present_mode_immediate=false",
            "--vulkan_allow_present_mode_mailbox=false",
            "--vulkan_allow_present_mode_fifo_relaxed=false",
            (BuildConfig.CRASH_LOGS || diagnostic) ? "--log_level=info" : "--log_level=off", "--log_verbose=false",
            diagnostic ? "--guest_frame_stats=true" : "--guest_frame_stats=false",
            diagnostic ? "--gpu_command_stats=true" : "--gpu_command_stats=false",
            diagnostic ? "--gpu_vblank_stats=true" : "--gpu_vblank_stats=false", "--gpu_slow_frame_stats=false"
        };
    }
}
