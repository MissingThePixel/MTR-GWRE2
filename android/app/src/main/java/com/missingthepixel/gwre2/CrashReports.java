package com.missingthepixel.gwre2;

import android.app.ActivityManager;
import android.app.Application;
import android.app.ApplicationExitInfo;
import android.content.Context;
import android.os.Build;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.charset.StandardCharsets;
import java.util.List;

public class CrashReports extends Application {
    private static String session;
    private static final int TRACE_LIMIT = 4 * 1024 * 1024;
    @Override public void onCreate() {
        super.onCreate();
        if (!BuildConfig.CRASH_LOGS) return;
        session = android.os.Process.myPid() + "-" + System.currentTimeMillis();
        final Thread.UncaughtExceptionHandler previous = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler((thread, error) -> {
            record(this, "java-crash", "Thread: " + thread.getName(), error);
            if (previous != null) previous.uncaughtException(thread, error);
            else { android.os.Process.killProcess(android.os.Process.myPid()); System.exit(1); }
        });
        collectPreviousExits();
        note(this, device(this) + "Application started");
        exportSaved(this);
    }
    private static File directory(Context context) {
        File directory = new File(context.getFilesDir(), "crash-reports");
        directory.mkdirs(); return directory;
    }
    static File runtimeLog(Context context) {
        return new File(directory(context), "runtime-" + session + ".log");
    }
    private static String device(Context context) {
        String soc = Build.VERSION.SDK_INT >= 31 ? Build.SOC_MANUFACTURER + " " + Build.SOC_MODEL : Build.HARDWARE;
        return "MTR-GWRE2 Android " + BuildConfig.VERSION_NAME + "\n"
                + "Time: " + new java.util.Date() + "\n"
                + "Device: " + Build.MANUFACTURER + " " + Build.MODEL + "\n"
                + "Android: " + Build.VERSION.RELEASE + " / API " + Build.VERSION.SDK_INT + "\n"
                + "SoC: " + soc + "\nABIs: " + java.util.Arrays.toString(Build.SUPPORTED_ABIS) + "\n"
                + "Fingerprint: " + Build.FINGERPRINT + "\n"
                + "Native library folder: " + context.getApplicationInfo().nativeLibraryDir + "\n";
    }
    private static void write(File file, byte[] data) throws java.io.IOException {
        try (FileOutputStream output = new FileOutputStream(file)) {
            output.write(data); output.getFD().sync();
        }
    }
    static void note(Context context, String message) {
        if (!BuildConfig.CRASH_LOGS) return;
        try (FileOutputStream output = new FileOutputStream(new File(directory(context), "session-" + session + ".txt"), true)) {
            output.write((new java.util.Date() + " " + message + "\n").getBytes(StandardCharsets.UTF_8));
            output.getFD().sync();
        } catch (Exception ignored) {}
    }
    static void record(Context context, String type, String message, Throwable error) {
        if (!BuildConfig.CRASH_LOGS) return;
        try {
            StringWriter stack = new StringWriter();
            if (error != null) error.printStackTrace(new PrintWriter(stack));
            File file = new File(directory(context), type + "-" + System.currentTimeMillis() + ".txt");
            write(file, (device(context) + "\n" + message + "\n" + stack).getBytes(StandardCharsets.UTF_8));
            write(new File(directory(context), "latest-crash.txt"), (device(context) + "\n" + message + "\n" + stack).getBytes(StandardCharsets.UTF_8));
            exportSaved(context);
        } catch (Throwable ignored) { /* Never replace the original failure. */ }
    }
    private static byte[] readBounded(InputStream input, int limit) throws java.io.IOException {
        ByteArrayOutputStream output = new ByteArrayOutputStream(); byte[] buffer = new byte[8192];
        int read;
        while ((read = input.read(buffer)) != -1) {
            if (output.size() + read > limit) throw new java.io.IOException("Crash trace exceeds capture limit");
            output.write(buffer, 0, read);
        }
        return output.toByteArray();
    }
    private void collectPreviousExits() {
        if (Build.VERSION.SDK_INT < 30) return;
        try {
            ActivityManager manager = (ActivityManager)getSystemService(ACTIVITY_SERVICE);
            List<ApplicationExitInfo> exits = manager.getHistoricalProcessExitReasons(getPackageName(), 0, 5);
            for (int i = exits.size() - 1; i >= 0; --i) {
                ApplicationExitInfo exit = exits.get(i);
                int reason = exit.getReason();
                if (reason != ApplicationExitInfo.REASON_CRASH && reason != ApplicationExitInfo.REASON_CRASH_NATIVE
                        && reason != ApplicationExitInfo.REASON_ANR && reason != ApplicationExitInfo.REASON_LOW_MEMORY
                        && reason != ApplicationExitInfo.REASON_SIGNALED && reason != ApplicationExitInfo.REASON_INITIALIZATION_FAILURE) continue;
                String name = "exit-" + exit.getTimestamp() + "-" + exit.getPid();
                File report = new File(directory(this), name + ".txt");
                if (report.exists()) continue;
                StringBuilder text = new StringBuilder(device(this));
                text.append("\nPrevious process exit: ").append(reasonName(reason))
                        .append("\nExit time: ").append(new java.util.Date(exit.getTimestamp()))
                        .append("\nPID: ").append(exit.getPid()).append("\nStatus/signal: ").append(exit.getStatus())
                        .append("\nDescription: ").append(exit.getDescription()).append("\n");
                try (InputStream trace = exit.getTraceInputStream()) {
                    if (trace != null) {
                        byte[] data = readBounded(trace, TRACE_LIMIT);
                        boolean protobuf = reason == ApplicationExitInfo.REASON_CRASH_NATIVE && Build.VERSION.SDK_INT >= 31;
                        write(new File(directory(this), name + (protobuf ? ".pb" : "-trace.txt")), data);
                        if (protobuf) text.append(NativeTombstone.describe(data));
                        else text.append(new String(data, StandardCharsets.UTF_8));
                    } else text.append("Android did not provide a retained crash trace.\n");
                } catch (Exception unavailable) { text.append("Trace collection: ").append(unavailable).append('\n'); }
                byte[] data = text.toString().getBytes(StandardCharsets.UTF_8);
                write(report, data); write(new File(directory(this), "latest-crash.txt"), data);
            }
        } catch (Exception failure) { note(this, "Previous-exit collection: " + failure); }
    }
    private static String reasonName(int reason) {
        switch (reason) {
            case ApplicationExitInfo.REASON_CRASH: return "JAVA CRASH";
            case ApplicationExitInfo.REASON_CRASH_NATIVE: return "NATIVE CRASH";
            case ApplicationExitInfo.REASON_ANR: return "NOT RESPONDING";
            case ApplicationExitInfo.REASON_LOW_MEMORY: return "LOW MEMORY KILL";
            case ApplicationExitInfo.REASON_SIGNALED: return "SIGNAL TERMINATION";
            default: return "INITIALIZATION FAILURE";
        }
    }
    static void exportSaved(Context context) {
        if (!BuildConfig.CRASH_LOGS) return;
        try {
            if (!GameStorage.hasSharedAccess(context)) return;
            File destination = new File(GameStorage.sharedFolder(), "logs");
            if (!destination.isDirectory() && !destination.mkdirs()) return;
            File[] files = directory(context).listFiles();
            if (files == null) return;
            for (File file : files) {
                File target = new File(destination, file.getName());
                if (target.exists() && target.length() == file.length() && target.lastModified() >= file.lastModified()) continue;
                try (FileInputStream input = new FileInputStream(file); FileOutputStream output = new FileOutputStream(target)) {
                    byte[] buffer = new byte[8192]; int length;
                    while ((length = input.read(buffer)) != -1) output.write(buffer, 0, length);
                }
            }
        } catch (Exception ignored) { /* Private copies remain available for later export. */ }
    }
}
