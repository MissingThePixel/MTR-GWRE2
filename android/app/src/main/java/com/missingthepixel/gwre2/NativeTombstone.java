package com.missingthepixel.gwre2;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

// Small, bounded reader for Android's debuggerd tombstone protobuf. Unknown
// fields are skipped; the untouched protobuf is also saved for offline analysis.
final class NativeTombstone {
    private static final class Field {
        int number; long value; byte[] data;
        String text() { return data == null ? "" : new String(data, StandardCharsets.UTF_8); }
    }
    private static List<Field> fields(byte[] bytes) throws IOException {
        List<Field> result = new ArrayList<>();
        int[] position = {0};
        while (position[0] < bytes.length) {
            long tag = varint(bytes, position);
            Field field = new Field(); field.number = (int)(tag >>> 3);
            if (field.number == 0) throw new IOException("Invalid protobuf field");
            int wire = (int)(tag & 7);
            if (wire == 0) field.value = varint(bytes, position);
            else if (wire == 2) {
                long length = varint(bytes, position);
                if (length < 0 || length > bytes.length - position[0]) throw new IOException("Truncated protobuf");
                field.data = java.util.Arrays.copyOfRange(bytes, position[0], position[0] + (int)length);
                position[0] += (int)length;
            } else if (wire == 1 || wire == 5) {
                int length = wire == 1 ? 8 : 4;
                if (length > bytes.length - position[0]) throw new IOException("Truncated fixed field");
                position[0] += length;
            } else throw new IOException("Unsupported protobuf wire type " + wire);
            result.add(field);
        }
        return result;
    }
    private static long varint(byte[] bytes, int[] position) throws IOException {
        long value = 0;
        for (int shift = 0; shift < 64; shift += 7) {
            if (position[0] >= bytes.length) throw new IOException("Truncated varint");
            int next = bytes[position[0]++] & 255;
            if (shift == 63 && (next & 254) != 0) throw new IOException("Overflowing varint");
            value |= (long)(next & 127) << shift;
            if ((next & 128) == 0) return value;
        }
        throw new IOException("Invalid varint");
    }
    private static Field get(List<Field> fields, int number) {
        for (Field field : fields) if (field.number == number) return field;
        Field empty = new Field(); empty.number = number; return empty;
    }
    static String describe(byte[] bytes) throws IOException {
        List<Field> tombstone = fields(bytes);
        long crashingTid = get(tombstone, 6).value;
        StringBuilder report = new StringBuilder("\nAndroid native crash dump\n");
        report.append("Timestamp: ").append(get(tombstone, 4).text()).append('\n');
        report.append("Crashing thread: ").append(crashingTid).append('\n');
        report.append("Abort message: ").append(get(tombstone, 14).text()).append('\n');
        Field signal = get(tombstone, 10);
        if (signal.data != null) {
            List<Field> info = fields(signal.data);
            report.append("Signal: ").append(get(info, 2).text()).append(" (").append(get(info, 1).value)
                    .append(") ").append(get(info, 4).text()).append("; fault address 0x")
                    .append(Long.toHexString(get(info, 9).value)).append('\n');
        }
        for (Field entry : tombstone) {
            if (entry.number == 15 && entry.data != null) {
                report.append("Cause: ").append(get(fields(entry.data), 1).text()).append('\n');
            }
            if (entry.number != 16 || entry.data == null) continue;
            List<Field> map = fields(entry.data);
            Field thread = get(map, 2);
            if (thread.data == null) continue;
            List<Field> threadFields = fields(thread.data);
            long tid = get(threadFields, 1).value;
            if (tid == 0) tid = get(map, 1).value;
            report.append("\nThread ").append(tid).append(" ").append(get(threadFields, 2).text());
            if (tid == crashingTid) report.append(" [CRASHED]");
            report.append('\n'); int index = 0;
            for (Field frame : threadFields) {
                if (frame.number != 4 || frame.data == null) continue;
                List<Field> details = fields(frame.data);
                report.append(String.format(Locale.ROOT, "#%02d pc %016x ", index++, get(details, 1).value))
                        .append(get(details, 6).text()).append(" (").append(get(details, 4).text())
                        .append('+').append(get(details, 5).value).append(") BuildId: ")
                        .append(get(details, 8).text()).append('\n');
            }
        }
        return report.toString();
    }
}
