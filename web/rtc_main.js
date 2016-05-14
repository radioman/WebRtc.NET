// M:\GoogleChrome\GoogleChromePortable.exe --allow-file-access-from-files --use-fake-ui-for-media-stream

var socket = null;
var localstream = null;
var remotestream = null;
var ice = [];
var answer = null;

var pcOptions = {
    optional: [
        { DtlsSrtpKeyAgreement: true }
    ]
}

var servers = {
    iceServers:
             [
                 { url: 'stun:stun.l.google.com:19302' },
                 { url: 'stun:stun.stunprotocol.org:3478' },
                 { url: 'stun:stun.anyfirewall.com:3478' }
             ]
};

var offerOptions = {
    offerToReceiveAudio: 0,
    offerToReceiveVideo: 1,
    voiceActivityDetection: false,
    iceRestart: true
};

var vgaConstraints = {
    video: true
};

function start() {

    if (!navigator.mediaDevices || !navigator.mediaDevices.enumerateDevices) {
        console.log("enumerateDevices() not supported.");
        return;
    }

    // List cameras and microphones.
    navigator.mediaDevices.enumerateDevices()
.then(function (devices) {
    devices.forEach(function (device) {
        console.log(device.kind + ": " + device.label +
                    " id = " + device.deviceId);
    });
})
.catch(function (err) {
    console.log(err.name + ": " + error.message);
});

    console.log('Requesting local stream');
    //startButton.disabled = true;
    navigator.mediaDevices.getUserMedia(vgaConstraints).then(function (stream) {
        console.log('Received local stream');

        var vid1 = document.getElementById('vid1');
        if (vid1) {
            vid1.srcObject = stream;
            vid1.onloadedmetadata = function (e) {
                vid1.play();
            };
        }

        localstream = stream;
        //callButton.disabled = false;

        connect();
    })
    .catch(function (err) {
        console.log(err.name + ": " + err.message);
        alert(err.name + ": " + err.message);
    });
}

window.onload = function () {
    start();
}

function send(data) {
    try {
        socket.send(data);
    }
    catch (ex) {
        console.log("Message sending failed!");
    }
}

function startStream() {
    console.log("startStream...");

    remotestream = new RTCPeerConnection(servers, pcOptions);

    if (localstream) {
        remotestream.addStream(localstream);
    }

    var isType = (stat, type) => stat.type == type && !stat.isRemote; // skip RTCP

    remotestream.onaddstream = function (e) {
        try {
            console.log("remote media connection success!");

            var vid2 = document.getElementById('vid2');
            vid2.srcObject = e.stream;
            vid2.onloadedmetadata = function (e) {
                vid2.play();
            };

            setInterval(() => Promise.all([
remotestream.getStats(null).then(o => dumpStat(o[Object.keys(o).find(key => isType(o[key], "inboundrtp"))]))
            ])
.then(strings => update(statsdiv, "<small>" + strings.join("") + "</small>")), 100);

        } catch (ex) {
            console.log("Failed to connect to remote media!", ex);
            socket.close();
        }
    };
    remotestream.onicecandidate = function (event) {
        if (event.candidate) {

            console.log('onicecandidate: ' + event.candidate.candidate);

            var obj = JSON.stringify({
                "command": "onicecandidate",
                "candidate": event.candidate
            });
            send(obj);
        }
        else {
            console.log('onicecandidate: complete.')

            if (answer) {

                remotestream.setRemoteDescription(
                new RTCSessionDescription({ type: "answer", sdp: answer }),
                function () { },
                function (errorInformation) {
                    console.log('setRemoteDescription error: ' + errorInformation);
                    socket.close();
                });

                for (var i = 0, len = ice.length; i < len; i++) {
                    var c = ice[i];
                    remotestream.addIceCandidate(c);
                }
            }
        }
    };

    remotestream.createOffer(function (desc) {
        console.log('createOffer: ' + desc.sdp);

        remotestream.setLocalDescription(desc, function () {
            var obj = JSON.stringify({
                "command": "offer",
                "desc": desc
            });
            send(obj);
        },
        function (errorInformation) {
            console.log('setLocalDescription error: ' + errorInformation);

            socket.close();
        });
    },
    function (error) {
        console.log(error);
        socket.close();
    },
    offerOptions);
}

function connect() {

    document.getElementById('btnconnect').disabled = true;

    socket = new WebSocket("ws://" + server.value);
    setSocketEvents(socket);

    function setSocketEvents(Socket) {
        Socket.onopen = function () {
            console.log("Socket connected!");

            startStream();
        };

        Socket.onclose = function () {
            console.log("Socket connection has been disconnected!");

            if (remotestream) {
                remotestream.close();
                remotestream = null;
            }
            answer = null;
            ice
            document.getElementById('btnconnect').disabled = false;
        }

        Socket.onmessage = function (Message) {
            var obj = JSON.parse(Message.data);
            var command = obj.command;
            switch (command) {
                case "OnSuccessAnswer": {
                    if (remotestream) {
                        console.log("OnSuccessAnswer: " + obj.sdp);

                        answer = obj.sdp;
                    }
                }
                    break;

                case "OnIceCandidate": {
                    if (remotestream) {
                        console.log("OnIceCandidate: " + obj.sdp);

                        ice.push(new RTCIceCandidate({
                            sdpMLineIndex: obj.sdp_mline_index,
                            candidate: obj.sdp
                        }));
                    }
                }
                    break;

                default: {
                    console.log(Message.data);
                }
            }
        };
    }
}

function dumpStat(o) {
    if (o != undefined) {
        var s = "Timestamp: " + new Date(o.timestamp).toTimeString() + " Type: " + o.type + "<br>";
        if (o.ssrc) s += "SSRC: " + o.ssrc + " ";
        if (o.packetsReceived !== undefined) {
            s += "Recvd: " + o.packetsReceived + " packets (" +
                 (o.bytesReceived / 1000000).toFixed(2) + " MB)" + " Lost: " + o.packetsLost;
        } else if (o.packetsSent !== undefined) {
            s += "Sent: " + o.packetsSent + " packets (" + (o.bytesSent / 1000000).toFixed(2) + " MB)";
        }
        if (o.bitrateMean !== undefined) {
            s += "<br>Avg. bitrate: " + (o.bitrateMean / 1000000).toFixed(2) + " Mbps (" +
                 (o.bitrateStdDev / 1000000).toFixed(2) + " StdDev)";
            if (o.discardedPackets !== undefined) {
                s += " Discarded packts: " + o.discardedPackets;
            }
        }
        if (o.framerateMean !== undefined) {
            s += "<br>Avg. framerate: " + (o.framerateMean).toFixed(2) + " fps (" +
                 o.framerateStdDev.toFixed(2) + " StdDev)";
            if (o.droppedFrames !== undefined) s += " Dropped frames: " + o.droppedFrames;
            if (o.jitter !== undefined) s += " Jitter: " + o.jitter;
        }
    }
    return s;
}

var log = msg => div.innerHTML += "<p>" + msg + "</p>";
var update = (div, msg) => div.innerHTML = msg;
var failed = e => log(e + ", line " + e.lineNumber);