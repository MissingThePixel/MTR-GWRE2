package com.missingthepixel.gwre2;

import android.Manifest;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.view.Gravity;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import java.io.File;

public class StorageActivity extends Activity {
    private TextView instructions;
    private Button accessButton;
    private boolean launching;

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        int padding = Math.round(24 * getResources().getDisplayMetrics().density);
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setPadding(padding, padding, padding, padding);
        layout.setBackgroundColor(Color.rgb(16, 18, 30));
        TextView title = new TextView(this);
        title.setText("Geometry Wars: Retro Evolved 2");
        title.setTextSize(24);
        title.setTextColor(Color.WHITE);
        title.setGravity(Gravity.CENTER);
        layout.addView(title);
        instructions = new TextView(this);
        instructions.setTextSize(17);
        instructions.setTextColor(Color.WHITE);
        instructions.setGravity(Gravity.CENTER);
        instructions.setPadding(0, padding, 0, padding);
        layout.addView(instructions);
        accessButton = new Button(this);
        accessButton.setText("Allow file access");
        accessButton.setOnClickListener(view -> requestFileAccess());
        layout.addView(accessButton);
        Button retry = new Button(this);
        retry.setText("Open game");
        retry.setOnClickListener(view -> refresh());
        layout.addView(retry);
        setContentView(layout);
    }

    @Override protected void onResume() {
        super.onResume();
        CrashReports.exportSaved(this);
        refresh();
    }

    private void refresh() {
        if (launching || instructions == null) return;
        File game = GameStorage.findGame(this);
        if (game != null) {
            launching = true;
            Intent launch = new Intent(this, GameActivity.class);
            if (getIntent().getExtras() != null) launch.putExtras(getIntent().getExtras());
            startActivity(launch);
            finish();
            return;
        }
        boolean allowed = GameStorage.hasSharedAccess(this);
        accessButton.setVisibility(allowed ? android.view.View.GONE : android.view.View.VISIBLE);
        File folder = GameStorage.sharedFolder();
        if (allowed && !folder.isDirectory() && !folder.mkdirs()) {
            instructions.setText("Could not create the GWRE2 folder in internal storage.\n"
                    + "Create it with your file manager, then try again.");
            return;
        }
        instructions.setText((allowed ? "" : "Allow file access, then ")
                + "copy your extracted game files into the GWRE2 folder in internal storage.\n\n"
                + "Put default.xex and the other game files directly inside GWRE2.\n"
                + "GWRE2/Game is also supported.\n\n"
                + "Return here and choose Open game.");
    }

    private void requestFileAccess() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE }, 1);
            return;
        }
        try {
            startActivity(new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                    Uri.parse("package:" + getPackageName())));
        } catch (ActivityNotFoundException unavailable) {
            try {
                startActivity(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION));
            } catch (ActivityNotFoundException unsupported) {
                instructions.setText("Open Settings, find this app under All files access, "
                        + "and allow access. Then return here.");
            }
        }
    }

    @Override public void onRequestPermissionsResult(int request, String[] permissions,
                                                      int[] results) {
        super.onRequestPermissionsResult(request, permissions, results);
        refresh();
    }
}
