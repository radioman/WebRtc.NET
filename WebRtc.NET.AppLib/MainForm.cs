using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace WebRtc.NET.AppLib
{
    public partial class MainForm : Form
    {
        WebRTCServer webSocketServer;

        public MainForm()
        {
            InitializeComponent();

            FormClosing += MainForm_FormClosing;
            Shown += MainForm_Shown;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (encoder != null)
            {
                encoder.Dispose();
            }

            if (webSocketServer != null)
            {
                webSocketServer.Dispose();
                webSocketServer = null;
            }
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            checkBoxWebsocket.Checked = true;
        }

        const int screenWidth = 640 * 1;
        const int screenHeight = 360 * 1;

        // stride: 4512
        readonly Bitmap img = new Bitmap(screenWidth, screenHeight, PixelFormat.Format24bppRgb);
        readonly Bitmap imgView = new Bitmap(screenWidth, screenHeight, PixelFormat.Format24bppRgb);
        readonly Rectangle bounds = new Rectangle(0, 0, screenWidth, screenHeight);
        readonly Rectangle boundsItem = new Rectangle(0, 0, 640, 360);

        readonly TurboJpegEncoder encoder = TurboJpegEncoder.CreateEncoder();
        public unsafe void OnFillBuffer(byte* yuv, long yuvSize)
        {
            lock (img)
            {
                if (SetEncode)
                {
                    var data = img.LockBits(bounds, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
                    {
                        encoder.EncodeRGB24toI420((byte*)data.Scan0.ToPointer(), data.Width, data.Height, yuv, yuvSize, false);
                    }
                    img.UnlockBits(data);
                }
            }
            //Thread.Sleep(200);
        }

        static readonly Font f = new Font("Tahoma", 14);
        static readonly Font fBig = new Font("Tahoma", 36);
        static readonly StringFormat sfTopLeft = new StringFormat()
        {
            Alignment = StringAlignment.Near,
            LineAlignment = StringAlignment.Near
        };

        static readonly StringFormat sfTopRight = new StringFormat()
        {
            Alignment = StringAlignment.Far,
            LineAlignment = StringAlignment.Near
        };

        static readonly StringFormat sfBottomLeft = new StringFormat()
        {
            Alignment = StringAlignment.Near,
            LineAlignment = StringAlignment.Far
        };

        // ...

        internal bool SetEncode = true;
        internal bool SetView = true;

        private void checkBoxEncode_CheckedChanged(object sender, EventArgs e)
        {
            lock (this)
            {
                SetEncode = checkBoxEncode.Checked;
                SetView = checkBoxView.Checked;
            }
        }

        private void numericMaxClients_ValueChanged(object sender, EventArgs e)
        {
            if (webSocketServer != null)
                webSocketServer.ClientLimit = (int)numericMaxClients.Value;
        }

        private void checkBoxWebsocket_CheckedChanged(object sender, EventArgs e)
        {
            if (webSocketServer != null)
            {
                webSocketServer.Dispose();
                webSocketServer = null;
            }

            if (checkBoxWebsocket.Checked)
            {
                webSocketServer = new WebRTCServer((int)numericWebSocket.Value);
                unsafe
                {
                    webSocketServer.OnFillBuffer = OnFillBuffer;
                }
                numericMaxClients_ValueChanged(null, null);

                if (checkBoxDemo.Checked)
                {
                    checkBoxDemo_CheckedChanged(null, null);
                }
            }
        }

        private Timer timerDemo;

        private void timerDemo_Tick(object sender, EventArgs e)
        {
            try
            {
                if (SetView)
                {
                    lock (imgView)
                    {
                        using (var g = Graphics.FromImage(imgView))
                        {
                            g.Clear(Color.DarkBlue);

                            var rc = RectangleF.FromLTRB(0, 0, img.Width, img.Height);
                            g.DrawString(string.Format("{0}", DateTime.Now.ToString("hh:mm:ss.fff")), fBig, Brushes.LimeGreen, rc, sfTopRight);
                        }

                        if (pictureBox1.Image == null)
                        {
                            pictureBox1.Image = imgView;
                        }
                        {
                            pictureBox1.Invalidate();
                        }
                    }
                }

                if (SetEncode)
                {
                    lock (img)
                    {
                        using (var g = Graphics.FromImage(img))
                        {
                            g.Clear(Color.DarkBlue);

                            var rc = RectangleF.FromLTRB(0, 0, img.Width, img.Height);
                            g.DrawString(string.Format("{0}", DateTime.Now.ToString("hh:mm:ss.fff")), fBig, Brushes.LimeGreen, rc, sfTopRight);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("timerDemo_Tick: " + ex);
            }
        }

        private void checkBoxDemo_CheckedChanged(object sender, EventArgs e)
        {
            if (timerDemo == null)
            {
                timerDemo = new Timer();
                timerDemo.Interval = 200;
                timerDemo.Tick += timerDemo_Tick;
            }
            timerDemo.Enabled = checkBoxDemo.Checked;
        }
    }
}
