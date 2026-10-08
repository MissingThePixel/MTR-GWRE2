using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
static class IconBuilder {
    static void Main(string[] args) {
        using (Image source = Image.FromFile(args[0]))
        using (Bitmap bitmap = new Bitmap(256, 256))
        using (Graphics graphics = Graphics.FromImage(bitmap))
        using (MemoryStream png = new MemoryStream()) {
            graphics.InterpolationMode = InterpolationMode.HighQualityBicubic;
            graphics.DrawImage(source, 0, 0, 256, 256);
            bitmap.Save(png, ImageFormat.Png);
            byte[] data = png.ToArray();
            using (BinaryWriter writer = new BinaryWriter(File.Create(args[1]))) {
                writer.Write((ushort)0); writer.Write((ushort)1); writer.Write((ushort)1);
                writer.Write((byte)0); writer.Write((byte)0); writer.Write((byte)0); writer.Write((byte)0);
                writer.Write((ushort)1); writer.Write((ushort)32); writer.Write((uint)data.Length);
                writer.Write((uint)22); writer.Write(data);
            }
        }
    }
}
