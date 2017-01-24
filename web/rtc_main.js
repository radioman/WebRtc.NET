
var socket = null;
var localstream = null;
var remotestream = null;
var remoteIce = [];
var remoteAnswer = null;
var localIce = [];

var dataChannel = null;
var testmsgcount = 0;
var feedbackmsg = "";
var feedbackmsgrecv = "";
var feedbackmsgsend = "";

var pcOptions = {
    optional: [
        { DtlsSrtpKeyAgreement: true }
    ]
}

var servers = {
    //iceTransportPolicy: 'relay', // force turn
    iceServers:
             [
                 { url: 'stun:stun.l.google.com:19302' },
                 { url: 'stun:stun.stunprotocol.org:3478' },
                 { url: 'stun:stun.anyfirewall.com:3478' },
                 { url: 'turn:192.168.0.100:3478', username: 'test', credential: 'test' }
             ]
};

var offerOptions = {
    offerToReceiveAudio: 1,
    offerToReceiveVideo: 1,
    voiceActivityDetection: false,
    iceRestart: true
};

var vgaConstraints = {
    video: true,
    //audio: true
};

var dataChannelOptions = {
    ordered: false,     // do not guarantee order

    maxRetransmits: 1,  // The maximum number of times to try and retransmit
                        // a failed message (forces unreliable mode)

    negotiated: false   // If set to true, it removes the automatic
                        // setting up of a data channel on the other peer,
                        // meaning that you are provided your own way to
                        // create a data channel with the same id on the other side
                        // aka: session.WebRtc.CreateDataChannel("msgDataChannel");
};

window.onload = function () {
    document.getElementById('btnconnect').disabled = false;
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
    
    // optional data channel
    dataChannel = remotestream.createDataChannel("msgDataChannel", dataChannelOptions);
    setDataChannel(dataChannel);

    remotestream.onaddstream = function (e) {
        try {
            console.log("remote media connection success!");

            var vid2 = document.getElementById('vid2');
            vid2.srcObject = e.stream;
            vid2.onloadedmetadata = function (e) {
                vid2.play();
            };

            // send some test feedback
            var tmsg = setInterval(function () {
                if (!remotestream) {
                    clearInterval(tmsg);
                }
                else if(dataChannel && dataChannel.readyState == "open") {
                    dataChannel.send("TEST_" + testmsgcount++ + ", feedback: " + feedbackmsg);
                }
            }, 1000);

            var t = setInterval(function () {
                if (!remotestream) {
                    clearInterval(t);
                }
                else {
                    Promise.all([
                        remotestream.getStats(null).then(function (o) {

                            var rcv = null;
                            var snd = null;

                            o.forEach(function (s) {
                                if ((s.type == "inbound-rtp" && s.mediaType == "video" && !s.isRemote) ||
                                   (s.type == "ssrc" && s.mediaType == "video" && s.id.indexOf("recv") >= 0))
                                {
                                    rcv = s;
                                }
                                else if((s.type == "outbound-rtp" && s.mediaType == "video" && !s.isRemote) ||
                                        (s.type == "ssrc" && s.mediaType == "video" && s.id.indexOf("send") >= 0))
                                {
                                    snd = s;
                                }
                            });                            

                            return dumpStat(rcv, snd);                           
                        })
                       
                    ]).then(function (s) {
                        statsdiv.innerHTML = "<small>" + s + "</small>";
                    });
                }
            }, 100);

        } catch (ex) {
            console.log("Failed to connect to remote media!", ex);
            socket.close();
        }
    };

    remotestream.onicecandidate = function (event) {
        if (event.candidate) {

            var ice = parseIce(event.candidate.candidate);
            if (ice && ice.component_id == 1           // skip RTCP 
                    //&& ice.type == 'relay'           // force turn
                    && ice.localIP.indexOf(":") < 0) { // skip IP6

                console.log('onicecandidate[local]: ' + event.candidate.candidate);
                var obj = JSON.stringify({
                    "command": "onicecandidate",
                    "candidate": event.candidate
                });
                send(obj);
                localIce.push(ice);
            }
            else {
                console.log('onicecandidate[local skip]: ' + event.candidate.candidate);
            }
        }
        else {
            console.log('onicecandidate: complete.')

            if (remoteAnswer) {                

                // fill empty pairs using last remote ice
                //for (var i = 0, lenl = localIce.length; i < lenl; i++) {
                //    if (i >= remoteIce.length) {
                //        var c = remoteIce[remoteIce.length - 1];

                //        var ice = parseIce(c.candidate);
                //        ice.foundation += i;
                //        c.candidate = stringifyIce(ice);

                //        remotestream.addIceCandidate(c);
                //    }
                //}
            }
        }
    };

    // can't manage to get trigger from other side ;/ wtf?
    remotestream.ondatachannel = function (event) {
        dataChannel = event.channel;
        setDataChannel(dataChannel);
    }

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

    socket.onopen = function () {
        console.log("Socket connected!");

        startStream();
    };

    socket.onclose = function () {
        console.log("Socket connection has been disconnected!");

        if (dataChannel)
        {
            dataChannel.close();
            dataChannel = null;
        }

        if (remotestream) {
            remotestream.close();
            remotestream = null;
        }
        remoteAnswer = null;
        remoteIce = [];
        localIce = [];

        document.getElementById('btnconnect').disabled = false;
    };

    socket.onmessage = function (Message) {
        var obj = JSON.parse(Message.data);
        var command = obj.command;
        switch (command) {
            case "OnSuccessAnswer": {
                if (remotestream) {
                    console.log("OnSuccessAnswer[remote]: " + obj.sdp);

                    remoteAnswer = obj.sdp;

                    remotestream.setRemoteDescription(
                    new RTCSessionDescription({ type: "answer", sdp: remoteAnswer }),
                    function () { },
                    function (errorInformation) {
                        console.log('setRemoteDescription error: ' + errorInformation);
                        socket.close();
                    });
                }
            }
                break;

            case "OnIceCandidate": {
                if (remotestream) {
                    console.log("OnIceCandidate[remote]: " + obj.sdp);

                    var c = new RTCIceCandidate({
                        sdpMLineIndex: obj.sdp_mline_index,
                        candidate: obj.sdp
                    });
                    remoteIce.push(c);

                    remotestream.addIceCandidate(c);
                }
            }
                break;

            default: {
                console.log(Message.data);
            }
        }
    };
}

function formatStat(o) {
    var s = "";
    if (o != undefined) {
        s += o.type + ": " + new Date(o.timestamp).toISOString() + "<br>";
        if (o.ssrc) s += "SSRC: " + o.ssrc + " ";
        if (o.packetsReceived !== undefined) {
            s += "Recvd: " + o.packetsReceived + " packets (" +
                 (o.bytesReceived / 1000000).toFixed(2) + " MB)" + " Lost: " + o.packetsLost;

            feedbackmsgrecv = s;

        } else if (o.packetsSent !== undefined) {
            s += "Sent: " + o.packetsSent + " packets (" + (o.bytesSent / 1000000).toFixed(2) + " MB)";
            feedbackmsgsend = s;
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
        if (o.googFrameRateReceived !== undefined) {
            s += "<br>googFrameRateReceived: " + o.googFrameRateReceived + " fps";
            s += " googJitterBufferMs: " + o.googJitterBufferMs;
            s += "<br>googFrameReceived: " + o.googFrameWidthReceived + "x" + o.googFrameHeightReceived;
            s += "<br>googCurrentDelayMs: " + o.googCurrentDelayMs;
            s += " googDecodeMs: " + o.googDecodeMs;
        }

        if (o.googFrameRateSent !== undefined) {
            s += "<br>googFrameRateSent: " + o.googFrameRateSent + " fps";
            s += " googEncodeUsagePercent: " + o.googEncodeUsagePercent + "%";
            s += "<br>googFrameSent: " + o.googFrameWidthSent + "x" + o.googFrameHeightSent;
            s += " googAvgEncodeMs: " + o.googAvgEncodeMs;
        }
    }
    return s;
}

function dumpStat(o, b) {

    var s = "";

    s += formatStat(o);

    if (b != undefined) {
        s += "<br> <br>";
        s += formatStat(b);
    }

    feedbackmsg = feedbackmsgrecv + feedbackmsgsend;

    return s;
}

function parseIce(candidateString) {
    // token                  =  1*(alphanum / "-" / "." / "!" / "%" / "*"
    //                              / "_" / "+" / "`" / "'" / "~" )
    var token_re = '[0-9a-zA-Z\\-\\.!\\%\\*_\\+\\`\\\'\\~]+';

    // ice-char               = ALPHA / DIGIT / "+" / "/"
    var ice_char_re = '[a-zA-Z0-9\\+\\/]+';

    // foundation             = 1*32ice-char
    var foundation_re = ice_char_re;

    // component-id           = 1*5DIGIT
    var component_id_re = '[0-9]{1,5}';

    // transport             = "UDP" / transport-extension
    // transport-extension   = token      ; from RFC 3261
    var transport_re = token_re;

    // priority              = 1*10DIGIT
    var priority_re = '[0-9]{1,10}';

    // connection-address SP      ; from RFC 4566
    var connection_address_v4_re = '[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}';
    var connection_address_v6_re = '\\:?(?:[0-9a-fA-F]{0,4}\\:?)+'; // fde8:cd2d:634c:6b00:6deb:9894:734:f75f

    var connection_address_re = '(?:' + connection_address_v4_re + ')|(?:' + connection_address_v6_re + ')';

    // port                      ; port from RFC 4566
    var port_re = '[0-9]{1,5}';

    //  cand-type             = "typ" SP candidate-types
    //  candidate-types       = "host" / "srflx" / "prflx" / "relay" / token
    var cand_type_re = token_re;

    var ICE_RE = '(?:a=)?candidate:(' + foundation_re + ')' + // candidate:599991555 // 'a=' not passed for Firefox (and now for Chrome too)
      '\\s' + '(' + component_id_re + ')' +                 // 2
      '\\s' + '(' + transport_re + ')' +                 // udp
      '\\s' + '(' + priority_re + ')' +                 // 2122260222
      '\\s' + '(' + connection_address_re + ')' +                 // 192.168.1.32 || fde8:cd2d:634c:6b00:6deb:9894:734:f75f
      '\\s' + '(' + port_re + ')' +                 // 49827
      '\\s' + 'typ' +                       // typ
      '\\s' + '(' + cand_type_re + ')' +                 // host
      '(?:' +
      '\\s' + 'raddr' +
      '\\s' + '(' + connection_address_re + ')' +
      '\\s' + 'rport' +
      '\\s' + '(' + port_re + ')' +
      ')?' +
      '(?:' +
      '\\s' + 'generation' +                       // generation
      '\\s' + '(' + '\\d+' + ')' +                 // 0
      ')?' +
      '(?:' +
      '\\s' + 'ufrag' +                       // ufrag
      '\\s' + '(' + ice_char_re + ')' +      // WreAYwhmkiw6SPvs
      ')?';

    var pattern = new RegExp(ICE_RE);
    var parsed = candidateString.match(pattern);

    //console.log('parseIceCandidate(): candidateString:', candidateString);
    //console.log('parseIceCandidate(): pattern:', pattern);
    //console.log('parseIceCandidate(): parsed:', parsed);

    // Check if the string was successfully parsed
    if (!parsed) {
        console.warn('parseIceCandidate(): parsed is empty: \'' + parsed + '\'');
        return null;
    }

    var propNames = [
      'foundation',
      'component_id',
      'transport',
      'priority',
      'localIP',
      'localPort',
      'type',
      'remoteIP',
      'remotePort',
      'generation',
      'ufrag'
    ];

    var candObj = {};
    for (var i = 0; i < propNames.length; i++) {
        candObj[propNames[i]] = parsed[i + 1];
    }
    return candObj;
}

function stringifyIce(iceCandObj) {
    var s = 'candidate:' + iceCandObj.foundation + '' +
          ' ' + iceCandObj.component_id + '' +
          ' ' + iceCandObj.transport + '' +
          ' ' + iceCandObj.priority + '' +
          ' ' + iceCandObj.localIP + '' +
          ' ' + iceCandObj.localPort + '' +
          ' typ ' + iceCandObj.type + '' +
          (iceCandObj.remoteIP ? ' raddr ' + iceCandObj.remoteIP + '' : '') +
          (iceCandObj.remotePort ? ' rport ' + iceCandObj.remotePort + '' : '') +
          (iceCandObj.generation ? ' generation ' + iceCandObj.generation + '' : '') +
          (iceCandObj.ufrag ? ' ufrag ' + iceCandObj.ufrag + '' : '');
    return s;
}

//---------------------------------------

function getLocalStream() {

    if (!navigator.mediaDevices || !navigator.mediaDevices.enumerateDevices) {
        console.log("enumerateDevices() not supported.");
        return;
    }

    // List cameras and microphones.
    navigator.mediaDevices.enumerateDevices().then(function (devices) {
        devices.forEach(function (device) {
            console.log(device.kind + ": " + device.label +
                        " id = " + device.deviceId);
        });
    }).catch(function (err) {
        console.log(err.name + ": " + error.message);
    });

    console.log('Requesting local stream');

    navigator.mediaDevices.getUserMedia(vgaConstraints).then(function (stream) {
        console.log('Received local stream');

        var vid1 = document.getElementById('vidLocal');
        if (vid1) {
            vid1.srcObject = stream;
            vid1.onloadedmetadata = function (e) {
                vid1.play();
            };
        }

        localstream = stream;

        if (remotestream) {
            remotestream.addStream(localstream);
        }
    })
    .catch(function (err) {
        console.log(err.name + ": " + err.message);
        alert(err.name + ": " + err.message);
    });
}

function setDataChannel(dc) {

    console.log("setDataChannel[" + dc.id + "]: " + dc.label);

    dc.onerror = function (error) {
        console.log("DataChannel Error:", error);
    };
    dc.onmessage = function (event) {
        console.log("DataChannel Message:", event.data);
    };
    dc.onopen = function () {
        dataChannel.send("Hello World!");
    };
    dc.onclose = function () {
        console.log("DataChannel is Closed");
    };
}
