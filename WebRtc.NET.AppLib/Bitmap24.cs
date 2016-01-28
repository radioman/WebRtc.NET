using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace WebRtc.NET.AppLib
{
    public class Bitmap24
    {
        // Provide public access to the picture's byte data.
        public byte[] ImageBytes;
        public int RowSizeBytes;
        public const int PixelDataSize = 24;

        // A reference to the Bitmap.
        private Bitmap m_Bitmap;

        // Save a reference to the bitmap.
        public Bitmap24(Bitmap bm)
        {
            m_Bitmap = bm;
        }

        // Bitmap data.
        private BitmapData m_BitmapData;

        // Lock the bitmap's data.
        public void LockBitmap()
        {
            // Lock the bitmap data.
            Rectangle bounds = new Rectangle(
                0, 0, m_Bitmap.Width, m_Bitmap.Height);
            m_BitmapData = m_Bitmap.LockBits(bounds,
                ImageLockMode.ReadWrite,
                PixelFormat.Format24bppRgb);
            RowSizeBytes = m_BitmapData.Stride;

            // Allocate room for the data.
            int total_size = m_BitmapData.Stride * m_BitmapData.Height;
            ImageBytes = new byte[total_size];

            // Copy the data into the ImageBytes array.
            Marshal.Copy(m_BitmapData.Scan0, ImageBytes, 0, total_size);
        }

        // Copy the data back into the Bitmap
        // and release resources.
        public void UnlockBitmap()
        {
            // Copy the data back into the bitmap.
            int total_size = m_BitmapData.Stride * m_BitmapData.Height;
            Marshal.Copy(ImageBytes, 0, m_BitmapData.Scan0, total_size);

            // Unlock the bitmap.
            m_Bitmap.UnlockBits(m_BitmapData);

            // Release resources.
            ImageBytes = null;
            m_BitmapData = null;
        }
    }
}
