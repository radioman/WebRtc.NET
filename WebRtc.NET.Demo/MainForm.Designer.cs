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
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.pictureBoxRemote = new System.Windows.Forms.PictureBox();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.pictureBoxLocal = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.comboBoxVideo = new System.Windows.Forms.ComboBox();
            this.checkBoxVirtualCam = new System.Windows.Forms.CheckBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.textBoxExtIP = new System.Windows.Forms.TextBox();
            this.checkBoxTurn = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.numericMaxClients = new System.Windows.Forms.NumericUpDown();
            this.checkBoxWebsocket = new System.Windows.Forms.CheckBox();
            this.numericWebSocket = new System.Windows.Forms.NumericUpDown();
            this.checkBoxScreen = new System.Windows.Forms.CheckBox();
            this.tabControl1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxRemote)).BeginInit();
            this.tabPage3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLocal)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericMaxClients)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericWebSocket)).BeginInit();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Location = new System.Drawing.Point(12, 123);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(543, 342);
            this.tabControl1.TabIndex = 7;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.pictureBoxRemote);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(535, 316);
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
            this.pictureBoxRemote.Size = new System.Drawing.Size(529, 310);
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
            this.tabPage3.Size = new System.Drawing.Size(535, 316);
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
            this.pictureBoxLocal.Size = new System.Drawing.Size(529, 310);
            this.pictureBoxLocal.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBoxLocal.TabIndex = 3;
            this.pictureBoxLocal.TabStop = false;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.groupBox5);
            this.groupBox1.Controls.Add(this.groupBox4);
            this.groupBox1.Controls.Add(this.groupBox3);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(543, 105);
            this.groupBox1.TabIndex = 8;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Control";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.checkBoxScreen);
            this.groupBox5.Controls.Add(this.comboBoxVideo);
            this.groupBox5.Controls.Add(this.checkBoxVirtualCam);
            this.groupBox5.Location = new System.Drawing.Point(172, 19);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(192, 74);
            this.groupBox5.TabIndex = 22;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Video capture";
            // 
            // comboBoxVideo
            // 
            this.comboBoxVideo.Dock = System.Windows.Forms.DockStyle.Top;
            this.comboBoxVideo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxVideo.FormattingEnabled = true;
            this.comboBoxVideo.Location = new System.Drawing.Point(3, 16);
            this.comboBoxVideo.Name = "comboBoxVideo";
            this.comboBoxVideo.Size = new System.Drawing.Size(186, 21);
            this.comboBoxVideo.TabIndex = 21;
            this.comboBoxVideo.SelectedIndexChanged += new System.EventHandler(this.comboBoxVideo_SelectedIndexChanged);
            // 
            // checkBoxVirtualCam
            // 
            this.checkBoxVirtualCam.AutoSize = true;
            this.checkBoxVirtualCam.Checked = true;
            this.checkBoxVirtualCam.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxVirtualCam.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxVirtualCam.Location = new System.Drawing.Point(6, 46);
            this.checkBoxVirtualCam.Name = "checkBoxVirtualCam";
            this.checkBoxVirtualCam.Size = new System.Drawing.Size(88, 17);
            this.checkBoxVirtualCam.TabIndex = 18;
            this.checkBoxVirtualCam.Text = "virtual cam";
            this.checkBoxVirtualCam.UseVisualStyleBackColor = true;
            this.checkBoxVirtualCam.CheckedChanged += new System.EventHandler(this.checkBoxVirtualCam_CheckedChanged);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.textBoxExtIP);
            this.groupBox4.Controls.Add(this.checkBoxTurn);
            this.groupBox4.Location = new System.Drawing.Point(370, 19);
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
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.label1);
            this.groupBox3.Controls.Add(this.numericMaxClients);
            this.groupBox3.Controls.Add(this.checkBoxWebsocket);
            this.groupBox3.Controls.Add(this.numericWebSocket);
            this.groupBox3.Location = new System.Drawing.Point(7, 19);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(159, 74);
            this.groupBox3.TabIndex = 11;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "WebSocket";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(77, 47);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 19;
            this.label1.Text = "- max client";
            // 
            // numericMaxClients
            // 
            this.numericMaxClients.Location = new System.Drawing.Point(6, 45);
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
            // checkBoxScreen
            // 
            this.checkBoxScreen.AutoSize = true;
            this.checkBoxScreen.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(186)));
            this.checkBoxScreen.Location = new System.Drawing.Point(98, 46);
            this.checkBoxScreen.Name = "checkBoxScreen";
            this.checkBoxScreen.Size = new System.Drawing.Size(58, 17);
            this.checkBoxScreen.TabIndex = 22;
            this.checkBoxScreen.Text = "screen";
            this.checkBoxScreen.UseVisualStyleBackColor = true;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(567, 477);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.tabControl1);
            this.Name = "MainForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "WebRtc.NET demo";
            this.tabControl1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxRemote)).EndInit();
            this.tabPage3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLocal)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericMaxClients)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericWebSocket)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.NumericUpDown numericMaxClients;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.NumericUpDown numericWebSocket;
        private System.Windows.Forms.CheckBox checkBoxWebsocket;
        private System.Windows.Forms.CheckBox checkBoxTurn;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.TextBox textBoxExtIP;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.PictureBox pictureBoxRemote;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.PictureBox pictureBoxLocal;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.Label label1;
        internal System.Windows.Forms.ComboBox comboBoxVideo;
        internal System.Windows.Forms.CheckBox checkBoxVirtualCam;
        internal System.Windows.Forms.CheckBox checkBoxScreen;
    }
}