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

            textBoxExtIP.Text = "192.168.0.100";

            comboBoxVideo.DataSource = ManagedConductor.GetVideoDevices();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (encoderRemote != null)
            {
                encoderRemote.Dispose();
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

        readonly TurboJpegEncoder encoderRemote = TurboJpegEncoder.CreateEncoder();

        byte[] bgrBuffremote;
        Bitmap remoteImg;
        public unsafe void OnRenderRemote(byte* yuv, uint w, uint h)
        {
            lock (pictureBoxRemote)
            {
                if (0 == encoderRemote.EncodeI420toBGR24(yuv, w, h, ref bgrBuffremote, true))
                {
                    if (remoteImg == null)
                    {
                        var bufHandle = GCHandle.Alloc(bgrBuffremote, GCHandleType.Pinned);
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

                if (f.tabControl1.SelectedTab == f.tabPage2)
                {
                    f.pictureBoxRemote.Invalidate();
                }
            }
        });

        readonly TurboJpegEncoder encoderLocal = TurboJpegEncoder.CreateEncoder();
        byte[] bgrBufflocal;
        Bitmap localImg;
        public unsafe void OnRenderLocal(byte* yuv, uint w, uint h)
        {
            lock (pictureBoxLocal)
            {
                if (0 == encoderLocal.EncodeI420toBGR24(yuv, w, h, ref bgrBufflocal, true))
                {
                    if (remoteImg == null)
                    {
                        var bufHandle = GCHandle.Alloc(bgrBufflocal, GCHandleType.Pinned);
                        localImg = new Bitmap((int)w, (int)h, (int)w * 3, PixelFormat.Format24bppRgb, bufHandle.AddrOfPinnedObject());
                    }
                }
            }

            try
            {
                Invoke(renderLocal, this);
            }
            catch // don't throw on form exit
            {
            }
        }

        readonly Action<MainForm> renderLocal = new Action<MainForm>(delegate (MainForm f)
        {
            lock (f.pictureBoxLocal)
            {
                if (f.pictureBoxLocal.Image == null)
                {
                    f.pictureBoxLocal.Image = f.localImg;

                    if (f.pictureBoxRemote.Image == null)
                    {
                        f.tabControl1.SelectTab(f.tabPage3);
                    }
                }

                if (f.tabControl1.SelectedTab == f.tabPage3)
                {
                    f.pictureBoxLocal.Invalidate();
                }
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
                    webSocketServer.Form = this;
                    unsafe
                    {
                        webSocketServer.OnRenderRemote = OnRenderRemote;
                        webSocketServer.OnRenderLocal = OnRenderLocal;
                    }
                    numericMaxClients_ValueChanged(null, null);

                    if (checkBoxVirtualCam.Checked)
                    {
                        checkBoxVirtualCam_CheckedChanged(null, null);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("checkBoxWebsocket_CheckedChanged: " + ex.Message);
            }
        }

        private System.Windows.Forms.Timer timerVirtualCam;

        public const bool audio = true;
        public const int screenWidth = 640;
        public const int screenHeight = 360;
        public const int captureFps = 5;
        public const bool barCodeScreen = false;

        readonly byte[] imgBuf = new byte[screenWidth * 3 * screenHeight];
        IntPtr imgBufPtr = IntPtr.Zero;
        Bitmap img;
        Graphics g;

        public int desktopWidth;
        public int desktopHeight;

        private void timerVirtualCam_Tick(object sender, EventArgs e)
        {
            try
            {
                if (img == null)
                {
                    var bufHandle = GCHandle.Alloc(imgBuf, GCHandleType.Pinned);
                    imgBufPtr = bufHandle.AddrOfPinnedObject();
                    img = new Bitmap(screenWidth, screenHeight, screenWidth * 3, PixelFormat.Format24bppRgb, imgBufPtr);
                    g = Graphics.FromImage(img);
                }

                {
                    // render
                    {
                        g.Clear(Color.DarkBlue);

                        if (checkBoxScreen.Checked)
                        {
                            g.CopyFromScreen(Cursor.Position, new Point(), new Size(screenWidth, screenHeight));
                        }

                        if (checkBoxScreen.Checked)
                        {
                            #region -- TODO: native desktop capture --
                            //foreach (var s in webSocketServer.Streams)
                            //{
                            //    s.Value.WebRtc.CaptureFrame();

                            //    if (imgDesktop == null)
                            //    {
                            //        var bufHandle = GCHandle.Alloc(imgBufDesktop, GCHandleType.Pinned);
                            //        varimgBufDesktopPtr = bufHandle.AddrOfPinnedObject();
                            //        img = new Bitmap(screenWidth, screenHeight, screenWidth * 3, PixelFormat.Format24bppRgb, imgBufPtr);
                            //        g = Graphics.FromImage(img);
                            //    }
                            //    if (desktopWidth == 0)
                            //    {
                            //        s.Value.WebRtc.DesktopCapturerSize(ref desktopWidth, ref desktopHeight);
                            //    }
                            //    unsafe
                            //    {
                            //        var rgba = s.Value.WebRtc.DesktopCapturerRGBAbuffer();
                            //        if (rgba != null)
                            //        {
                            //            //encoderRemote.EncodeBGRAtoI420((byte*)imgBufPtr.ToPointer(), screenWidth, screenHeight, yuv, 0, true);
                            //        }
                            //    }
                            //} 
                            #endregion
                        }

                        var rc = RectangleF.FromLTRB(0, 0, img.Width, img.Height);
                        g.DrawString(string.Format("{0}", DateTime.Now.ToString("hh:mm:ss.fff")), fBig, Brushes.LimeGreen, rc, sfTopRight);
                    }

                    foreach (var s in webSocketServer.Streams)
                    {
                        if (!barCodeScreen)
                        {
                            unsafe
                            {
                                var yuv = s.Value.WebRtc.VideoCapturerI420Buffer();
                                if (yuv != null)
                                {
                                    encoderRemote.EncodeBGR24toI420((byte*)imgBufPtr.ToPointer(), screenWidth, screenHeight, yuv, 0, true);
                                }
                            }
                        }
                        s.Value.WebRtc.PushFrame();
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("timerDemo_Tick: " + ex);
            }
        }

        private void checkBoxVirtualCam_CheckedChanged(object sender, EventArgs e)
        {
            if (timerVirtualCam == null)
            {
                timerVirtualCam = new System.Windows.Forms.Timer();
                timerVirtualCam.Interval = 1000 / captureFps;
                timerVirtualCam.Tick += timerVirtualCam_Tick;
            }
            timerVirtualCam.Enabled = checkBoxVirtualCam.Checked;
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

        internal string videoDevice = string.Empty;
        private void comboBoxVideo_SelectedIndexChanged(object sender, EventArgs e)
        {
            videoDevice = comboBoxVideo.SelectedItem as string;
        }
    }
}
