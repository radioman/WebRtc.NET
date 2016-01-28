using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace WebRtc.NET.AppLib
{
    public class Util
    {
        public static bool AutoConnect;
        static Form f;

        public static void OnMainForm(bool param)
        {
            if (param)
            {
                if (f == null)
                {
                    AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
                }
                else
                {
                    CloseForm();
                }

                //using (var p = Process.GetCurrentProcess())
                //{
                //    if (p.MainWindowTitle.Contains("Chrome"))
                //    {
                //        MainWindowHandle = p.MainWindowHandle;
                //        p.PriorityClass = ProcessPriorityClass.Idle;
                //    }
                //}
                
                var thread = new Thread(delegate ()
                {
                    f = new MainForm();
                    Application.Run(f);
                });
                thread.SetApartmentState(ApartmentState.STA);
                thread.Start();
            }
            else
            {
                CloseForm();
            }
        }

        static void CloseForm()
        {
            if (f != null && !f.IsDisposed)
            {
                var t = f;
                t.BeginInvoke((Action)(() =>
                {
                    t.Close();
                }));
            }
            f = null;
        }

        private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            MessageBox.Show("CurrentDomain_UnhandledException: " + e.ExceptionObject);
        }

        //static Random r = new Random();
        //static byte[] ImageBytes;
        //static byte[] yuv;
        //static Bitmap m_Bitmap;

        public unsafe static void OnFillBuffer(byte * pData, long lDataLen)
        {
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
            if(f != null)
            {
                //f.OnFillBuffer(pData, lDataLen);
            }
        }

        public static unsafe int IndexOf(int startIndex, byte[] Haystack, long HaystackLength, byte[] Needle)
        {
            fixed (byte* H = Haystack)
            fixed (byte* N = Needle)
            {
                int i = 0, c = 0;
                bool Found = false;
                for (byte* hNext = H + startIndex, hEnd = H + HaystackLength; hNext < hEnd; i++, hNext++)
                {
                    Found = true;
                    c = 0;

                    for (byte* hInc = hNext, nInc = N, nEnd = N + Needle.LongLength;
                        Found && nInc < nEnd && hInc < hEnd;
                        Found = *nInc == *hInc, nInc++, hInc++)
                    {
                        if (Found) c++;
                    }

                    if (Found && c == Needle.LongLength)
                        return i + startIndex;
                }
                return -1;
            }
        }

        public static unsafe List<int> IndexesOf(byte[] Haystack, int HaystackSize, byte[] Needle)
        {
            List<int> Indexes = new List<int>();
            fixed (byte* H = Haystack)
            fixed (byte* N = Needle)
            {
                int i = 0, c = 0;
                for (byte* hNext = H, hEnd = H + HaystackSize; hNext < hEnd; i++, hNext++)
                {
                    bool Found = true;
                    c = 0;

                    for (byte* hInc = hNext, nInc = N, nEnd = N + Needle.LongLength; Found && nInc < nEnd && hInc < hEnd; Found = *nInc == *hInc, nInc++, hInc++)
                    {
                        if (Found) c++;
                    }

                    if (Found && c == Needle.LongLength)
                        Indexes.Add(i);
                }
                return Indexes;
            }
        }
    }
}
