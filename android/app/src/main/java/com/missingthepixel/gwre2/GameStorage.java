package com.missingthepixel.gwre2;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import java.io.File;

final class GameStorage {
    private GameStorage() {}

    static File sharedFolder() {
        return new File(Environment.getExternalStorageDirectory(), "GWRE2");
    }

    static boolean hasSharedAccess(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        }
        return context.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED;
    }

    private static boolean containsGame(File directory) {
        if (directory == null) return false;
        File executable = new File(directory, "default.xex");
        return executable.isFile() && executable.canRead() && executable.length() > 0;
    }

    static File findGame(Context context) {
        File shared = sharedFolder();
        if (containsGame(shared)) return shared;
        File nested = new File(shared, "Game");
        if (containsGame(nested)) return nested;
        // Preserve existing test installations and their game-data placement.
        File external = context.getExternalFilesDir(null);
        if (external != null) {
            File legacy = new File(external, "Game");
            if (containsGame(legacy)) return legacy;
        }
        File internal = new File(context.getFilesDir(), "Game");
        return containsGame(internal) ? internal : null;
    }
}
