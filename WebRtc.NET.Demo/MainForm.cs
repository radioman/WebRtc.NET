﻿using System;
using System.Collections.Generic;
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

            comboBoxVideo.DataSource = WebRtcNative.VideoDevices();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
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

        internal void ResetRemote()
        {
            pictureBoxRemote.Image = null;
            if (remoteImg != null)
            {
                remoteImg.Dispose();
                remoteImg = null;
            }
        }

        Bitmap remoteImg;
        public void OnRenderRemote(IntPtr BGR24, uint w, uint h)
        {
            if (remoteImg == null)
            {
                lock (pictureBoxRemote)
                {
                    remoteImg = new Bitmap((int)w, (int)h, (int)w * 3, PixelFormat.Format24bppRgb, BGR24);
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

        internal void ResetLocal()
        {
            pictureBoxLocal.Image = null;
            if (localImg != null)
            {
                localImg.Dispose();
                localImg = null;
            }
        }

        Bitmap localImg;
        public void OnRenderLocal(IntPtr BGR24, uint w, uint h)
        {
            if (localImg == null)
            {
                lock (pictureBoxLocal)
                {
                    localImg = new Bitmap((int)w, (int)h, (int)w * 3, PixelFormat.Format24bppRgb, BGR24);
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

        readonly byte[] imgBuf = new byte[screenWidth * 3 * screenHeight];
        IntPtr imgBufPtr = IntPtr.Zero;
        Bitmap img;
        Graphics g;

        int captureWidth;
        int captureHeight;
        readonly Dictionary<IntPtr, Bitmap> imgCapture = new Dictionary<IntPtr, Bitmap>();

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
                    if (!checkBoxTest.Checked)
                    {
                        g.Clear(Color.DarkBlue);

                        if (checkBoxScreen.Checked)
                        {
                            g.CopyFromScreen(Cursor.Position, new Point(), new Size(screenWidth, screenHeight));
                        }

                        if (checkBoxInternalScreen.Checked)
                        {
                            #region -- native webrtc desktop capture --

                            //set internals.h #define DESKTOP_CAPTURE 1

                            foreach (var s in webSocketServer.Streams)
                            {
                                // if no editing is needed & size match
                                //s.Value.WebRtc.CaptureFrameAndPush();
                                //return;

                                //if (false)
                                {
                                    int x = -1, y = -1;
                                    var rgbaPtr = s.Value.WebRtc.CaptureFrameBGRX(ref x, ref y);
                                    if (rgbaPtr != IntPtr.Zero)
                                    {
                                        Bitmap captureImg = null;

                                        if (x != captureWidth || y != captureHeight
                                            || !imgCapture.TryGetValue(rgbaPtr, out captureImg))
                                        {
                                            if (captureImg != null)
                                                captureImg.Dispose();

                                            imgCapture[rgbaPtr] = captureImg = new Bitmap(x, y, x * 4, PixelFormat.Format32bppRgb, rgbaPtr);

                                            captureWidth = x;
                                            captureHeight = y;
                                        }
                                        g.DrawImage(captureImg, 0, 0, new Rectangle(Cursor.Position, new Size(screenWidth, screenHeight)), GraphicsUnit.Pixel);
                                    }
                                }
                                break;
                            }

                            #endregion
                        }

                        var rc = RectangleF.FromLTRB(0, 0, img.Width, img.Height);
                        g.DrawString(string.Format("{0}", DateTime.Now.ToString("hh:mm:ss.fff")), fBig, Brushes.LimeGreen, rc, sfTopRight);
                    }

                    foreach (var s in webSocketServer.Streams)
                    {
                        s.Value.WebRtc.PushFrame(!checkBoxTest.Checked ? imgBufPtr : IntPtr.Zero);
                    }
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine("timerDemo_Tick: " + ex);
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
                    using (var mc = new WebRtcNative())
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
