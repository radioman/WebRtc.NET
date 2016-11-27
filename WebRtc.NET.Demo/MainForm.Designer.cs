namespace WebRtc.NET.Demo
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.pictureBoxPreview = new System.Windows.Forms.PictureBox();
            this.checkBoxView = new System.Windows.Forms.CheckBox();
            this.checkBoxEncode = new System.Windows.Forms.CheckBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.pictureBoxRemote = new System.Windows.Forms.PictureBox();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.pictureBoxLocal = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.textBoxExtIP = new System.Windows.Forms.TextBox();
            this.checkBoxTurn = new System.Windows.Forms.CheckBox();
            this.checkBoxDemo = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.checkBoxWebsocket = new System.Windows.Forms.CheckBox();
            this.numericWebSocket = new System.Windows.Forms.NumericUpDown();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.numericMaxClients = new System.Windows.Forms.NumericUpDown();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPreview)).BeginInit();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxRemote)).BeginInit();
            this.tabPage3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLocal)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericWebSocket)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericMaxClients)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBoxPreview
            // 
            this.pictureBoxPreview.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.pictureBoxPreview.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBoxPreview.Location = new System.Drawing.Point(3, 3);
            this.pictureBoxPreview.Name = "pictureBoxPreview";
            this.pictureBoxPreview.Size = new System.Drawing.Size(347, 238);
            this.pictureBoxPreview.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxPreview.TabIndex = 1;
            this.pictureBoxPreview.TabStop = false;
            // 
            // checkBoxView
            // 
            this.checkBoxView.AutoSize = true;
            this.checkBoxView.Checked = true;
            this.checkBoxView.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxView.Location = new System.Drawing.Point(164, 83);
            this.checkBoxView.Name = "checkBoxView";
            this.checkBoxView.Size = new System.Drawing.Size(48, 17);
            this.checkBoxView.TabIndex = 2;
            this.checkBoxView.Text = "view";
            this.checkBoxView.UseVisualStyleBackColor = true;
            this.checkBoxView.CheckedChanged += new System.EventHandler(this.checkBoxEncode_CheckedChanged);
            // 
            // checkBoxEncode
            // 
            this.checkBoxEncode.AutoSize = true;
            this.checkBoxEncode.Checked = true;
            this.checkBoxEncode.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxEncode.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxEncode.Location = new System.Drawing.Point(218, 83);
            this.checkBoxEncode.Name = "checkBoxEncode";
            this.checkBoxEncode.Size = new System.Drawing.Size(68, 17);
            this.checkBoxEncode.TabIndex = 4;
            this.checkBoxEncode.Text = "encode";
            this.checkBoxEncode.UseVisualStyleBackColor = true;
            this.checkBoxEncode.CheckedChanged += new System.EventHandler(this.checkBoxEncode_CheckedChanged);
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Location = new System.Drawing.Point(12, 151);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(361, 270);
            this.tabControl1.TabIndex = 7;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.pictureBoxPreview);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(353, 244);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "DebugLocalVideo";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.pictureBoxRemote);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(353, 244);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "RemoteVideo";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // pictureBoxRemote
            // 
            this.pictureBoxRemote.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.pictureBoxRemote.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBoxRemote.Location = new System.Drawing.Point(3, 3);
            this.pictureBoxRemote.Name = "pictureBoxRemote";
            this.pictureBoxRemote.Size = new System.Drawing.Size(347, 238);
            this.pictureBoxRemote.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxRemote.TabIndex = 2;
            this.pictureBoxRemote.TabStop = false;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.pictureBoxLocal);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(353, 244);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "LocalVideo";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // pictureBoxLocal
            // 
            this.pictureBoxLocal.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.pictureBoxLocal.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBoxLocal.Location = new System.Drawing.Point(3, 3);
            this.pictureBoxLocal.Name = "pictureBoxLocal";
            this.pictureBoxLocal.Size = new System.Drawing.Size(347, 238);
            this.pictureBoxLocal.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxLocal.TabIndex = 3;
            this.pictureBoxLocal.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.groupBox4);
            this.groupBox1.Controls.Add(this.checkBoxDemo);
            this.groupBox1.Controls.Add(this.groupBox3);
            this.groupBox1.Controls.Add(this.groupBox2);
            this.groupBox1.Controls.Add(this.checkBoxView);
            this.groupBox1.Controls.Add(this.checkBoxEncode);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(361, 133);
            this.groupBox1.TabIndex = 8;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Control";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.textBoxExtIP);
            this.groupBox4.Controls.Add(this.checkBoxTurn);
            this.groupBox4.Location = new System.Drawing.Point(158, 19);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(160, 49);
            this.groupBox4.TabIndex = 20;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "TURN @ external IP";
            // 
            // textBoxExtIP
            // 
            this.textBoxExtIP.Location = new System.Drawing.Point(6, 17);
            this.textBoxExtIP.Name = "textBoxExtIP";
            this.textBoxExtIP.Size = new System.Drawing.Size(92, 20);
            this.textBoxExtIP.TabIndex = 20;
            this.textBoxExtIP.Text = "185.58.66.37";
            // 
            // checkBoxTurn
            // 
            this.checkBoxTurn.AutoSize = true;
            this.checkBoxTurn.Location = new System.Drawing.Point(103, 20);
            this.checkBoxTurn.Name = "checkBoxTurn";
            this.checkBoxTurn.Size = new System.Drawing.Size(53, 17);
            this.checkBoxTurn.TabIndex = 19;
            this.checkBoxTurn.Text = "RUN!";
            this.checkBoxTurn.UseVisualStyleBackColor = true;
            this.checkBoxTurn.CheckedChanged += new System.EventHandler(this.checkBoxTurn_CheckedChanged);
            // 
            // checkBoxDemo
            // 
            this.checkBoxDemo.AutoSize = true;
            this.checkBoxDemo.Checked = true;
            this.checkBoxDemo.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxDemo.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxDemo.Location = new System.Drawing.Point(164, 106);
            this.checkBoxDemo.Name = "checkBoxDemo";
            this.checkBoxDemo.Size = new System.Drawing.Size(88, 17);
            this.checkBoxDemo.TabIndex = 18;
            this.checkBoxDemo.Text = "virtual cam";
            this.checkBoxDemo.UseVisualStyleBackColor = true;
            this.checkBoxDemo.CheckedChanged += new System.EventHandler(this.checkBoxDemo_CheckedChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.checkBoxWebsocket);
            this.groupBox3.Controls.Add(this.numericWebSocket);
            this.groupBox3.Location = new System.Drawing.Point(7, 19);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(140, 49);
            this.groupBox3.TabIndex = 11;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "WebSocket";
            // 
            // checkBoxWebsocket
            // 
            this.checkBoxWebsocket.AutoSize = true;
            this.checkBoxWebsocket.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxWebsocket.Location = new System.Drawing.Point(75, 20);
            this.checkBoxWebsocket.Name = "checkBoxWebsocket";
            this.checkBoxWebsocket.Size = new System.Drawing.Size(57, 17);
            this.checkBoxWebsocket.TabIndex = 18;
            this.checkBoxWebsocket.Text = "RUN!";
            this.checkBoxWebsocket.UseVisualStyleBackColor = true;
            this.checkBoxWebsocket.CheckedChanged += new System.EventHandler(this.checkBoxWebsocket_CheckedChanged);
            // 
            // numericWebSocket
            // 
            this.numericWebSocket.Location = new System.Drawing.Point(6, 19);
            this.numericWebSocket.Maximum = new decimal(new int[] {
            65000,
            0,
            0,
            0});
            this.numericWebSocket.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericWebSocket.Name = "numericWebSocket";
            this.numericWebSocket.Size = new System.Drawing.Size(65, 20);
            this.numericWebSocket.TabIndex = 7;
            this.numericWebSocket.Value = new decimal(new int[] {
            9000,
            0,
            0,
            0});
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.numericMaxClients);
            this.groupBox2.Location = new System.Drawing.Point(7, 74);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(140, 49);
            this.groupBox2.TabIndex = 10;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "MaxStreams";
            // 
            // numericMaxClients
            // 
            this.numericMaxClients.Location = new System.Drawing.Point(6, 19);
            this.numericMaxClients.Name = "numericMaxClients";
            this.numericMaxClients.Size = new System.Drawing.Size(65, 20);
            this.numericMaxClients.TabIndex = 7;
            this.numericMaxClients.Value = new decimal(new int[] {
            5,
            0,
            0,
            0});
            this.numericMaxClients.ValueChanged += new System.EventHandler(this.numericMaxClients_ValueChanged);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(385, 433);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.tabControl1);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "WebRtc.NET demo";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPreview)).EndInit();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxRemote)).EndInit();
            this.tabPage3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLocal)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericWebSocket)).EndInit();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.numericMaxClients)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.PictureBox pictureBoxPreview;
        private System.Windows.Forms.CheckBox checkBoxView;
        private System.Windows.Forms.CheckBox checkBoxEncode;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.NumericUpDown numericMaxClients;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.NumericUpDown numericWebSocket;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox checkBoxWebsocket;
        private System.Windows.Forms.CheckBox checkBoxDemo;
        private System.Windows.Forms.CheckBox checkBoxTurn;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.TextBox textBoxExtIP;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.PictureBox pictureBoxRemote;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.PictureBox pictureBoxLocal;
    }
}