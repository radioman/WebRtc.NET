using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using LitJson;
using Microsoft.Win32;
using TurboJpeg;

namespace WebRtc.NET.AppLib
{
    public partial class MainForm : Form
    {
        class CamStream
        {
            internal BackgroundWorker Worker;
            internal string Ip;
            internal int Id;

            internal byte[] rgb;
            internal Bitmap rgbBits;
            internal bool SetEncode;
            internal bool SetView;
            internal TurboJpegEncoder encoder;
        }

        WebRTCServer webSocketServer;
        WebRtc.NET.ManagedConductor mc;

        BackgroundWorker webrtc;

        public MainForm()
        {
            InitializeComponent();

            FormClosing += MainForm_FormClosing;
            Shown += MainForm_Shown;

            webrtc = new BackgroundWorker();
            webrtc.WorkerSupportsCancellation = true;
            webrtc.DoWork += WebRtc_DoWork;
            webrtc.RunWorkerAsync();
        }

        volatile bool exit;
        private void WebRtc_DoWork(object sender, DoWorkEventArgs e)
        {
            try
            {
                WebRtc.NET.ManagedConductor.InitializeSSL();

                using (mc = new ManagedConductor())
                {
                    mc.OnSuccessAnswer += Mc_OnSuccessAnswer;
                    mc.OnIceCandidate += Mc_OnIceCandidate; ;
                    mc.OnFailure += Mc_OnFailure;
                    mc.OnError += Mc_OnError;
                    var r = mc.InitializePeerConnection();
                    if (r)
                    {
                        var bg = sender as BackgroundWorker;
                        while (!bg.CancellationPending && mc.ProcessMessages(1000))
                        {
                            Debug.Write(".");
                        }
                        mc.Quit();
                    }
                    else
                    {
                        Debug.WriteLine("InitializePeerConnection failed");
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"DoWork: {ex}");
            }
            WebRtc.NET.ManagedConductor.CleanupSSL();
            exit = true;
        }

        private void Mc_OnError()
        {
            Debug.WriteLine("Mc_OnError");  
        }

        private void Mc_OnFailure(string error)
        {
            Debug.WriteLine($"Mc_OnFailure: {error}");
        }

        private void Mc_OnIceCandidate(string sdp_mid, int sdp_mline_index, string sdp)
        {
            if (webSocketServer != null)
            {
                // send ice
                var c = webSocketServer.Streams.LastOrDefault();
                if (c.Value.IsAvailable)
                {
                    JsonData jd = new JsonData();
                    jd["command"] = "OnIceCandidate";
                    jd["sdp_mid"] = sdp_mid;
                    jd["sdp_mline_index"] = sdp_mline_index;
                    jd["sdp"] = sdp;
                    c.Value.Send(jd.ToJson());
                }
            }
        }

        private void Mc_OnSuccessAnswer(string sdp)
        {
            if (webSocketServer != null)
            {
                // send anwer
                var c = webSocketServer.Streams.LastOrDefault();
                if (c.Value.IsAvailable)
                {
                    JsonData jd = new JsonData();
                    jd["command"] = "OnSuccessAnswer";
                    jd["sdp"] = sdp;
                    c.Value.Send(jd.ToJson());
                }
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            foreach (var c in GetStreams())
            {
                if (c.Worker.IsBusy)
                {
                    c.Worker.CancelAsync();
                    while (c.Worker.IsBusy)
                    {
                        Application.DoEvents();
                        Thread.Sleep(100);                        
                    }
                }

                if (c.encoder != null)
                {
                    c.encoder.Dispose();
                    c.encoder = null;
                }

                if (c.rgbBits != null)
                {
                    c.rgbBits.Dispose();
                    c.rgbBits = null;
                }
            }

            if (encoder != null)
            {
                encoder.Dispose();
            }

            if (webSocketServer != null)
            {
                webSocketServer.Dispose();
                webSocketServer = null;
            }

            webrtc.CancelAsync();
            while (!exit)
            {
                Thread.Sleep(100);
            }           
        }

        const string config = @"SOFTWARE\WebRtc.NET\App";

        private void MainForm_Shown(object sender, EventArgs e)
        {
            try
            {
                //if (false)
                {
                    ReloadConfig();

                    if (Util.AutoConnect)
                    {
                        //button1.PerformClick();
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "MainForm_Shown", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void ReloadConfig()
        {
            using (var r = Registry.LocalMachine.CreateSubKey(config))
            {
                var configs = r.GetSubKeyNames();
                foreach (var c in configs)
                {
                    comboBoxCameraId.Items.Add(c);
                }

                string[] args = Environment.GetCommandLineArgs();
                //MessageBox.Show(string.Join("|", args));

                bool ok = false;
                var f = args.FirstOrDefault(p => p.StartsWith("--user-data-dir="));
                if (f != null && f.Contains("CamSessions"))
                {
                    var camId = Path.GetFileNameWithoutExtension(f.Split('=')[1]);
                    if (comboBoxCameraId.Items.Contains(camId))
                    {
                        comboBoxCameraId.SelectedItem = camId;
                        ok = true;
                    }
                }

                if (!ok && comboBoxCameraId.Items.Count > 0)
                {
                    var cfg = r.GetValue("CurrentConfig", null);
                    if (cfg != null)
                    {
                        comboBoxCameraId.SelectedItem = cfg;
                    }
                }
            }
        }

        private static void SetChromePriority()
        {
            var c = Process.GetProcessesByName("chrome");
            {
                foreach (var cp in c)
                {
                    using (cp)
                    {
                        cp.PriorityClass = ProcessPriorityClass.Idle;
                    }
                }
            }
        }

        const int screenWidth = 752 * 1;
        const int screenHeight = 480 * 1;

        // stride: 4512
        readonly Bitmap img = new Bitmap(screenWidth, screenHeight, PixelFormat.Format24bppRgb);
        readonly Bitmap imgView = new Bitmap(screenWidth, screenHeight, PixelFormat.Format24bppRgb);
        readonly Rectangle bounds = new Rectangle(0, 0, screenWidth, screenHeight);
        readonly Rectangle boundsItem = new Rectangle(0, 0, 752, 480);

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
            Thread.Sleep(200);
        }

        void GetCameraFrames_DoWork(object sender, DoWorkEventArgs e)
        {
            byte[] jpegMagic = new byte[] { 0xFF, 0xD8, 0xFF };
            int jpegMagicLength = jpegMagic.Length;
            const int bufferSize = 200 * 1024;
            byte[] buffer = new byte[bufferSize];
            MemoryStream ms = new MemoryStream();

            var cam = e.Argument as CamStream;
            Debug.WriteLine($"starting cam[{cam.Id}] stream: {cam.Ip}");

            int port = 0;
            var cip = cam.Ip.Split(':');
            if (cip.Length == 2)
            {
                if (!int.TryParse(cip[1], out port))
                {
                    //...
                }
            }

            var bg = sender as BackgroundWorker;
            while (!bg.CancellationPending)
            {
                try
                {
                    using (TcpClient t = new TcpClient())
                    {
                        t.ReceiveBufferSize = buffer.Length;
                        if (!t.Connected)
                        {
                            t.Connect(cip[0], port);
                        }

                        using (var responseStream = t.GetStream())
                        {
                            int count = 0;
                            int f = -1;
                            int start = 0;
                            int last = -1;
                            int frames = 0;

                            while (t.Connected && !bg.CancellationPending)
                            {
                                if (t.Available > 0 && (count = responseStream.Read(buffer, 0, buffer.Length)) > 0)
                                {
                                    //var x =  Util.IndexesOf(buffer, count, jpegMagic);
                                    //Debug.WriteLine($"c: {count}, buff: {string.Join("|", x)}");

                                    last = -1;
                                    start = 0;
                                    frames = 0;

                                    while ((f = Util.IndexOf(start, buffer, count, jpegMagic)) != -1)
                                    {
                                        if (last != -1)
                                        {
                                            ms.Position = 0;
                                            ms.Write(buffer, last, f - last);
                                            ProcessJpegFrame(ms.GetBuffer(), (int)ms.Position, cam);

                                            //Debug.WriteLine($"image: {ms.Position / 1024}KB");
                                            ms.Position = 0;
                                        }
                                        else
                                        {
                                            if (ms.Position > 0)
                                            {
                                                if (f != 0)
                                                {
                                                    ms.Write(buffer, 0, f);
                                                }
                                                ProcessJpegFrame(ms.GetBuffer(), (int)ms.Position, cam);
                                                ms.Position = 0;
                                            }
                                        }

                                        last = f;
                                        start = f + jpegMagic.Length;
                                        frames++;
                                    }

                                    if (last != -1)
                                    {
                                        ms.Write(buffer, last, count - last);
                                    }
                                    else
                                    {
                                        ms.Write(buffer, 0, count);
                                    }

                                    if (frames < 2)
                                    {
                                        Thread.Sleep(100);
                                        Debug.Write(".");
                                    }
                                }
                                else
                                {
                                    Thread.Sleep(300);
                                    Debug.Write(";");
                                }
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Bg_DoWork: " + ex);
                }
            }
            Debug.WriteLine($"stoped cam[{cam.Id}] stream: {cam.Ip}");
        }

        int fwidth = 752;
        int fheight = 480;
        //int fwidth = 640;

        int ProcessJpegFrame(byte[] buff, int size, CamStream c)
        {
            {
                if (buff[0] != 0xFF || buff[1] != 0xD8 ||
                    buff[size - 2] != 0xFF || buff[size - 1] != 0xD9)
                {
                    Debug.WriteLine("Bad jpeg...");
                    return -1;
                }

                c.encoder.EncodeJpegToRGB24(buff, size, ref c.rgb, ref fwidth, ref fheight);
                if (c.rgbBits == null)
                {
                    var p = Marshal.UnsafeAddrOfPinnedArrayElement(c.rgb, 0);
                    int bitsPerPixel = ((int)PixelFormat.Format24bppRgb & 0xff00) >> 8;
                    int bytesPerPixel = (bitsPerPixel + 7) / 8;
                    int stride = 4 * ((fwidth * bytesPerPixel + 3) / 4);
                    c.rgbBits = new Bitmap(fwidth, fheight, stride, PixelFormat.Format24bppRgb, p);
                    //t.Save("test2.bmp");

                    //var data = c.rgbBits.LockBits(boundsItem, ImageLockMode.ReadOnly, PixelFormat.Format24bppRgb);
                    //c.rgbBits.UnlockBits(data);
                }

                if (SetEncode)
                {
                    lock (img)
                    {
                        using (var g = Graphics.FromImage(img))
                        {
                            DrawFrame(c, g);
                        }
                    }
                }

                if (SetView && c.rgbBits != null)
                {
                    lock (imgView)
                    {
                        using (var gw = Graphics.FromImage(imgView))
                        {
                            DrawFrame(c, gw);
                        }
                    }
                }
            }

            return 0;
        }

        static readonly Font f = new Font("Tahoma", 14);
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

        // 0 1
        // 2 3
        void DrawFrame(CamStream c, Graphics g)
        {
            {
                {
                    int x = 0;
                    int y = 0;

                    switch (c.Id)
                    {
                        case 0:
                        x = 0;
                        y = 0;
                        break;

                        case 1:
                        x = 1;
                        y = 0;
                        break;

                        case 2:
                        x = 0;
                        y = 1;
                        break;

                        case 3:
                        x = 1;
                        y = 1;
                        break;
                    }

                    var image = c.rgbBits;

                    var maximumWidth = boundsItem.Width;
                    var maximumHeight = boundsItem.Height;

                    var newImageWidth = maximumWidth;
                    var newImageHeight = maximumHeight;

                    var ratioX = maximumWidth / (double)image.Width;
                    var ratioY = maximumHeight / (double)image.Height;
                    var ratio = ratioX < ratioY ? ratioX : ratioY;
                    newImageHeight = (int)(image.Height * ratio);
                    newImageWidth = (int)(image.Width * ratio);
                    {
                        int div = 1;

                        Point pv = new Point(
                            (int)((maximumWidth - (image.Width * ratio)) / div),
                            (int)((maximumHeight - (image.Height * ratio)) / div));

                        //pv.Offset(x * maximumWidth, y * maximumHeight);

                        g.DrawImage(image, pv.X, pv.Y, newImageWidth, newImageHeight);

                        var rc = RectangleF.FromLTRB(0, 0, newImageWidth, newImageHeight);
                        rc.Offset(pv);
                        g.DrawString(c.Ip, f, Brushes.LimeGreen, rc, sfTopLeft);

                        //if (c.Id == 1)
                        {
                            lock(this)
                            {
                                g.DrawString($"{DateTime.Now.ToLongTimeString()}", f, Brushes.LimeGreen, rc, sfTopRight);
                            }
                        }
                    }
                }
            }
        }

        IEnumerable<CamStream> GetStreams()
        {
            foreach (var c in cams)
            {
               yield return c;
            }
        }

        // ...
        System.Windows.Forms.Timer timerUpdate;

        readonly List<CamStream> cams = new List<CamStream>();

        private void button1_Click(object sender, EventArgs e)
        {
            Queue<string> camStack = new Queue<string>(textBoxCams.Lines.Where(p => p.Contains(':')));

            if (cams.Count == 0)
            {
                timerUpdate = new System.Windows.Forms.Timer();
                timerUpdate.Interval = 1000;
                timerUpdate.Tick += new System.EventHandler(this.timerUpdate_Tick);
                timerUpdate.Start();
            }

            Stack<CamStream> camTmp = new Stack<CamStream>(cams);

            int i = 0;
            while(camStack.Count > 0)
            {
               var cIp = camStack.Dequeue();

                CamStream c;
                if (camTmp.Count > 0)
                {
                    c = camTmp.Pop();
                }
                else
                {
                    c =  new CamStream()
                    {
                        Ip = cIp,
                        Worker = new BackgroundWorker(),
                        Id = i++,
                        SetEncode = true,
                        SetView = true,
                        encoder = TurboJpegEncoder.CreateEncoder()
                    };
                    c.Worker.WorkerSupportsCancellation = true;
                    c.Worker.DoWork += GetCameraFrames_DoWork;

                    cams.Add(c);
                }
                c.Worker.RunWorkerAsync(c);
               //break;               
            }

            button1.Enabled = false;
            buttonStop.Enabled = true;
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            foreach (var c in GetStreams())
            {
                if (c.Worker.IsBusy)
                {
                    c.Worker.CancelAsync();
                }
            }

            button1.Enabled = true;
            buttonStop.Enabled = false;
        }

        internal bool SetEncode = true;
        internal bool SetRandom = false;
        internal bool SetView = false;

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

        DateTime last = DateTime.MinValue;
        private void timerUpdate_Tick(object sender, EventArgs e)
        {
            if (webSocketServer != null)
            {
                Text = $"VideoBridge: {webSocketServer.StreamsCount} streams of {webSocketServer.ClientCount} sessions";
            }

            if (SetView)
            {
                try
                {
                    lock (imgView)
                    {
                        if (pictureBox1.Image == null)
                        {
                            pictureBox1.Image = imgView;
                        }
                        {
                            pictureBox1.Invalidate();
                        }
                    }
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("SetView: " + ex);
                }
            }

            if (DateTime.Now - last > TimeSpan.FromSeconds(10))
            {
                last = DateTime.Now;
            }
        }

        private void comboBoxCameraId_SelectedValueChanged(object sender, EventArgs e)
        {
            using (var r = Registry.LocalMachine.CreateSubKey(config))
            {
                using (var key = r.OpenSubKey(comboBoxCameraId.SelectedItem.ToString()))
                {
                    numericWebSocket.Value = (int)key.GetValue("WebSocket", 9000);
                    numericMaxClients.Value = (int)key.GetValue("MaxStreams", 5);
                    textBoxCams.Lines = (string[])key.GetValue("Config", new string[] {
                        "10.10.10.12:9911",
                        "10.10.10.12:9912"
                    });
                }
            }
            textBoxId.Text = comboBoxCameraId.SelectedItem.ToString();
        }

        private void buttonAdd_Click(object sender, EventArgs e)
        {
            using (var r = Registry.LocalMachine.CreateSubKey(config))
            {
                var id = Guid.NewGuid().ToString();
                using (var rid = r.CreateSubKey(id))
                {
                    comboBoxCameraId.Items.Add(id);
                    comboBoxCameraId.SelectedItem = id;
                }
            }
        }

        private void buttonRemove_Click(object sender, EventArgs e)
        {
            if (comboBoxCameraId.SelectedItem != null)
            {
                using (var r = Registry.LocalMachine.OpenSubKey(config, true))
                {
                    r.DeleteSubKeyTree(comboBoxCameraId.SelectedItem.ToString());
                    comboBoxCameraId.Items.Remove(comboBoxCameraId.SelectedItem);
                }

                if (comboBoxCameraId.Items.Count > 0)
                {
                    comboBoxCameraId.SelectedItem = comboBoxCameraId.Items[0];
                }
            }
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            if (comboBoxCameraId.SelectedItem != null)
            {
                using (var r = Registry.LocalMachine.CreateSubKey(config))
                {
                    r.SetValue("CurrentConfig", comboBoxCameraId.SelectedItem.ToString());

                    using (var key = r.CreateSubKey(comboBoxCameraId.SelectedItem.ToString()))
                    {
                        key.SetValue("WebSocket", (int)numericWebSocket.Value);
                        key.SetValue("MaxStreams", (int)numericMaxClients.Value);
                        key.SetValue("Config", textBoxCams.Lines);
                    }
                }
            }
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
                webSocketServer.mc = mc;
                numericMaxClients_ValueChanged(null, null);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            mc.CreateOffer();
        }
    }
}
