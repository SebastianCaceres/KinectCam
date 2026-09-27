$csharpSource = @"
using System;
using System.IO;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;

public class IconGenerator
{
    public static Bitmap RenderKinect(int size)
    {
        Bitmap bmp = new Bitmap(size, size, PixelFormat.Format32bppArgb);
        using (Graphics g = Graphics.FromImage(bmp))
        {
            g.SmoothingMode = SmoothingMode.HighQuality;
            g.InterpolationMode = InterpolationMode.HighQualityBicubic;
            g.PixelOffsetMode = PixelOffsetMode.HighQuality;
            g.Clear(Color.Transparent);

            float s = size / 256.0f;

            // 1. Base Stand Oval
            using (var brush = new LinearGradientBrush(
                new PointF(0, 170 * s), new PointF(0, 215 * s),
                Color.FromArgb(255, 45, 45, 50), Color.FromArgb(255, 18, 18, 20)))
            {
                g.FillEllipse(brush, 68 * s, 175 * s, 120 * s, 38 * s);
            }

            // 2. Neck
            using (var brush = new SolidBrush(Color.FromArgb(255, 28, 28, 30)))
            {
                g.FillRectangle(brush, 114 * s, 145 * s, 28 * s, 38 * s);
            }

            // 3. Sensor Bar Outer Shell
            float barX = 14 * s, barY = 68 * s, barW = 228 * s, barH = 82 * s, r = 18 * s;
            using (GraphicsPath path = CreateRoundRect(barX, barY, barW, barH, r))
            {
                using (var brush = new LinearGradientBrush(
                    new PointF(0, barY), new PointF(0, barY + barH),
                    Color.FromArgb(255, 55, 58, 62), Color.FromArgb(255, 18, 18, 20)))
                {
                    g.FillPath(brush, path);
                }
                using (var pen = new Pen(Color.FromArgb(255, 80, 84, 90), Math.Max(1f, 2f * s)))
                {
                    g.DrawPath(pen, path);
                }
            }

            // 4. Front Glossy Glass Faceplate
            float faceX = 24 * s, faceY = 78 * s, faceW = 208 * s, faceH = 62 * s, fr = 10 * s;
            using (GraphicsPath path = CreateRoundRect(faceX, faceY, faceW, faceH, fr))
            {
                using (var brush = new LinearGradientBrush(
                    new PointF(0, faceY), new PointF(0, faceY + faceH),
                    Color.FromArgb(255, 12, 12, 14), Color.FromArgb(255, 24, 24, 28)))
                {
                    g.FillPath(brush, path);
                }
            }

            // 5. Left Lens: IR Projector (Maroon / Burgundy)
            float l1X = 50 * s, l1Y = 93 * s, lSize = 32 * s;
            using (var bOuter = new SolidBrush(Color.FromArgb(255, 16, 16, 18)))
            using (var bInner = new SolidBrush(Color.FromArgb(255, 55, 18, 30)))
            using (var pen = new Pen(Color.FromArgb(255, 65, 68, 75), 1.5f * s))
            {
                g.FillEllipse(bOuter, l1X, l1Y, lSize, lSize);
                g.FillEllipse(bInner, l1X + 4 * s, l1Y + 4 * s, lSize - 8 * s, lSize - 8 * s);
                g.DrawEllipse(pen, l1X, l1Y, lSize, lSize);
            }

            // 6. Center Lens: RGB Camera (Deep Blue & Iris reflection)
            float l2X = 112 * s, l2Y = 93 * s;
            using (var bOuter = new SolidBrush(Color.FromArgb(255, 16, 16, 18)))
            using (var bMid = new SolidBrush(Color.FromArgb(255, 18, 42, 70)))
            using (var bCore = new SolidBrush(Color.FromArgb(255, 30, 85, 140)))
            using (var bGlint = new SolidBrush(Color.FromArgb(200, 255, 255, 255)))
            using (var pen = new Pen(Color.FromArgb(255, 75, 80, 90), 1.5f * s))
            {
                g.FillEllipse(bOuter, l2X, l2Y, lSize, lSize);
                g.FillEllipse(bMid, l2X + 4 * s, l2Y + 4 * s, lSize - 8 * s, lSize - 8 * s);
                g.FillEllipse(bCore, l2X + 8 * s, l2Y + 8 * s, lSize - 16 * s, lSize - 16 * s);
                g.FillEllipse(bGlint, l2X + 9 * s, l2Y + 9 * s, 6 * s, 4 * s);
                g.DrawEllipse(pen, l2X, l2Y, lSize, lSize);
            }

            // 7. Glowing Green LED Indicator
            float ledX = 152 * s, ledY = 105 * s, ledSize = 8 * s;
            using (var bGlow = new SolidBrush(Color.FromArgb(80, 0, 255, 90)))
            using (var bCore = new SolidBrush(Color.FromArgb(255, 35, 255, 105)))
            using (var bCenter = new SolidBrush(Color.FromArgb(255, 220, 255, 230)))
            {
                g.FillEllipse(bGlow, ledX - 3 * s, ledY - 3 * s, ledSize + 6 * s, ledSize + 6 * s);
                g.FillEllipse(bCore, ledX, ledY, ledSize, ledSize);
                g.FillEllipse(bCenter, ledX + 1.5f * s, ledY + 1.5f * s, 3 * s, 3 * s);
            }

            // 8. Right Lens: IR Camera
            float l3X = 174 * s, l3Y = 93 * s;
            using (var bOuter = new SolidBrush(Color.FromArgb(255, 16, 16, 18)))
            using (var bInner = new SolidBrush(Color.FromArgb(255, 28, 30, 36)))
            using (var pen = new Pen(Color.FromArgb(255, 65, 68, 75), 1.5f * s))
            {
                g.FillEllipse(bOuter, l3X, l3Y, lSize, lSize);
                g.FillEllipse(bInner, l3X + 4 * s, l3Y + 4 * s, lSize - 8 * s, lSize - 8 * s);
                g.DrawEllipse(pen, l3X, l3Y, lSize, lSize);
            }

            // 9. Glass Specular Sheen (top half of faceplate)
            using (var brush = new LinearGradientBrush(
                new PointF(faceX, faceY), new PointF(faceX, faceY + 28 * s),
                Color.FromArgb(35, 255, 255, 255), Color.FromArgb(0, 255, 255, 255)))
            {
                g.FillRectangle(brush, faceX + 2 * s, faceY + 2 * s, faceW - 4 * s, 26 * s);
            }
        }
        return bmp;
    }

    private static GraphicsPath CreateRoundRect(float x, float y, float w, float h, float r)
    {
        GraphicsPath path = new GraphicsPath();
        path.AddArc(x, y, r * 2, r * 2, 180, 90);
        path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
        path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
        path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
        path.CloseFigure();
        return path;
    }

    public static void GenerateIco(string outputPath)
    {
        int[] sizes = new int[] { 256, 48, 32, 16 };
        byte[][] pngBytes = new byte[sizes.Length][];

        for (int i = 0; i < sizes.Length; i++)
        {
            using (Bitmap b = RenderKinect(sizes[i]))
            using (MemoryStream ms = new MemoryStream())
            {
                b.Save(ms, ImageFormat.Png);
                pngBytes[i] = ms.ToArray();
            }
        }

        using (FileStream fs = File.Create(outputPath))
        using (BinaryWriter bw = new BinaryWriter(fs))
        {
            // ICONDIR
            bw.Write((ushort)0); // Reserved
            bw.Write((ushort)1); // Type = 1 (Icon)
            bw.Write((ushort)sizes.Length); // Image count

            int offset = 6 + (sizes.Length * 16);

            for (int i = 0; i < sizes.Length; i++)
            {
                int sz = sizes[i];
                byte w = (byte)(sz >= 256 ? 0 : sz);
                byte h = (byte)(sz >= 256 ? 0 : sz);
                uint len = (uint)pngBytes[i].Length;

                bw.Write(w);
                bw.Write(h);
                bw.Write((byte)0); // Color count
                bw.Write((byte)0); // Reserved
                bw.Write((ushort)1); // Planes
                bw.Write((ushort)32); // BPP
                bw.Write(len);
                bw.Write((uint)offset);

                offset += (int)len;
            }

            for (int i = 0; i < sizes.Length; i++)
            {
                bw.Write(pngBytes[i]);
            }
        }
    }
}
"@

Add-Type -TypeDefinition $csharpSource -ReferencedAssemblies "System.Drawing.dll"
New-Item -ItemType Directory -Path "assets" -Force | Out-Null
[IconGenerator]::GenerateIco("C:\Users\Seba\IdeaProjects\KinectCam\assets\kinect.ico")
Write-Output "Generated assets\kinect.ico successfully!"
