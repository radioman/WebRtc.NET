using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WebRtc.NET.Demo
{
    public partial class MainForm : Form
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }

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

        const int screenWidth = 640;
        const int screenHeight = 360;

        readonly byte[] imgBuf = new byte[screenWidth * 3 * screenHeight];
        IntPtr imgBufPtr = IntPtr.Zero;
        Bitmap img;
        readonly Bitmap imgView = new Bitmap(screenWidth, screenHeight, PixelFormat.Format24bppRgb);

        readonly TurboJpegEncoder encoder = TurboJpegEncoder.CreateEncoder();
        public unsafe void OnFillBuffer(byte* yuv, uint yuvSize)
        {
            if (SetEncode && imgBufPtr != IntPtr.Zero)
            {
                lock (img)
                {
                    encoder.EncodeBGR24toI420((byte*)imgBufPtr.ToPointer(), screenWidth, screenHeight, yuv, yuvSize, true);
                }
            }
        }

        byte[] bgrBuff;
        Bitmap remoteImg;
        public unsafe void OnRenderRemote(byte* yuv, uint w, uint h)
        {
            lock (pictureBoxRemote)
            {
                if (0 == encoder.EncodeI420toBGR24(yuv, w, h, ref bgrBuff, true))
                {
                    if (remoteImg == null)
                    {
                        var bufHandle = GCHandle.Alloc(bgrBuff, GCHandleType.Pinned);
                        remoteImg = new Bitmap((int)w, (int)h, (int)w * 3, PixelFormat.Format24bppRgb, bufHandle.AddrOfPinnedObject());
                    }
                }
            }

            try
            {
                Invoke(renderRemote, this);
            }
            catch // don't throw on form exit
            {
            }
        }

        readonly Action<MainForm> renderRemote = new Action<MainForm>(delegate (MainForm f)
        {
            lock (f.pictureBoxRemote)
            {
                if (f.pictureBoxRemote.Image == null)
                {
                    f.pictureBoxRemote.Image = f.remoteImg;
                    f.tabControl1.SelectTab(f.tabPage2);
                }
                f.pictureBoxRemote.Invalidate();
            }
        });

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
            try
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
                        webSocketServer.OnRenderRemote = OnRenderRemote;
                    }
                    numericMaxClients_ValueChanged(null, null);

                    if (checkBoxDemo.Checked)
                    {
                        checkBoxDemo_CheckedChanged(null, null);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("checkBoxWebsocket_CheckedChanged: " + ex.Message);
            }
        }

        private System.Windows.Forms.Timer timerDemo;

        private void timerDemo_Tick(object sender, EventArgs e)
        {
            try
            {
                if (img == null)
                {
                    var bufHandle = GCHandle.Alloc(imgBuf, GCHandleType.Pinned);
                    imgBufPtr = bufHandle.AddrOfPinnedObject();
                    img = new Bitmap(screenWidth, screenHeight, screenWidth * 3, PixelFormat.Format24bppRgb, imgBufPtr);
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

                        if (pictureBoxPreview.Image == null)
                        {
                            pictureBoxPreview.Image = imgView;
                        }
                        {
                            pictureBoxPreview.Invalidate();
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
                timerDemo = new System.Windows.Forms.Timer();
                timerDemo.Interval = 200;
                timerDemo.Tick += timerDemo_Tick;
            }
            timerDemo.Enabled = checkBoxDemo.Checked;
        }

        CancellationTokenSource turnCancel;
        private void checkBoxTurn_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxTurn.Checked)
            {
                var ui = TaskScheduler.FromCurrentSynchronizationContext();

                Task.Factory.StartNew(delegate ()
                {
                    using (var mc = new ManagedConductor())
                    {
                        var ok = mc.RunTurnServer("0.0.0.0:3478", textBoxExtIP.Text, "test", "auth.txt");
                        if (!ok)
                        {
                            Task.Factory.StartNew(delegate ()
                            {
                                MessageBox.Show("TURN server start failed ;/");
                            }, CancellationToken.None, TaskCreationOptions.None, ui);
                        }
                        else
                        {
                            using (turnCancel = new CancellationTokenSource())
                            {
                                var stop = turnCancel.Token; 
                                while (!stop.IsCancellationRequested && mc.ProcessMessages(1000))
                                {
                                    Debug.WriteLine(".");
                                }                                

                                Task.Factory.StartNew(delegate ()
                                {
                                    MessageBox.Show("TURN server stoped.");
                                }, CancellationToken.None, TaskCreationOptions.None, ui);
                            }
                        }
                    }
                }, TaskCreationOptions.LongRunning);
            }
            else
            {
                if (turnCancel != null && !turnCancel.IsCancellationRequested)
                {
                    turnCancel.Cancel();
                    turnCancel = null;
                    checkBoxTurn.Enabled = false; // after dispose it fails to start again wtf?.. ;/
                }
            }
        }
    }
}
