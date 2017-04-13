using System;
using System.Diagnostics;
using System.IO;

namespace WebRtc.NET
{
    public class Util
    {
        //static Random r = new Random();
        //static byte[] ImageBytes;
        //static byte[] yuv;
        //static Bitmap m_Bitmap;

        static UInt64 i = 0;

        public unsafe static void OnFillBuffer(byte* pData, long lDataLen, int part_idx, bool keyFrame)
        {
            Trace.WriteLine($"{i++}: Encode[{keyFrame}|{part_idx}]: {lDataLen}");

            using (var f = File.Open("dump.bin", FileMode.OpenOrCreate, FileAccess.ReadWrite))
            {
                f.Seek(0, SeekOrigin.End);

                using (var b = new BinaryWriter(f))
                {
                    //b.Write((int)lDataLen);

                    using (UnmanagedMemoryStream ms = new UnmanagedMemoryStream(pData, lDataLen))
                    {
                        //ms.CopyTo(f);
                    }
                }
            }

            //if (m_Bitmap == null)
            //{
            //    m_Bitmap = new Bitmap(@"D:\Dev\comDemo\Filters\logo.bmp");

            //    Rectangle bounds = new Rectangle(0, 0, m_Bitmap.Width, m_Bitmap.Height);
            //    var m_BitmapData = m_Bitmap.LockBits(bounds, ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);
            //    var RowSizeBytes = m_BitmapData.Stride;

            //    int total_size = m_BitmapData.Stride * m_BitmapData.Height;
            //    if (ImageBytes == null || ImageBytes.Length != total_size)
            //    {
            //        ImageBytes = new byte[total_size];
            //    }
            //    // Copy the data into the ImageBytes array.
            //    Marshal.Copy(m_BitmapData.Scan0, ImageBytes, 0, total_size);

            //    encoder.EncodeRGBtoI420(ImageBytes, m_Bitmap.Width, m_Bitmap.Height, ref yuv, true);
            //}
            //else
            //if(f != null)
            {
                //f.OnFillBuffer(pData, lDataLen);
            }
        }
    }
}
