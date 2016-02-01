
namespace WebRtc.NET.AppLib
{
    using Fleck;
    using LitJson;
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Diagnostics;

    public class Str
    {
        public const string Failed = "failed";
        public const string Success = "success";
        public const string Exist = "exist";
    }
    public class Command
    {
        public const string offer = "offer";
        public const string onicecandidate = "onicecandidate";
    }

    class WebRTCServer : IDisposable
    {
        public ConcurrentDictionary<Guid, IWebSocketConnection> UserList = new ConcurrentDictionary<Guid, IWebSocketConnection>();
        public ConcurrentDictionary<Guid, IWebSocketConnection> Streams = new ConcurrentDictionary<Guid, IWebSocketConnection>();

        internal WebRtc.NET.ManagedConductor mc;

        WebSocketServer server;
        public WebRTCServer(int port) : this("ws://0.0.0.0:" + port)
        {            
        }

        public WebRTCServer(string URL)
        {
            server = new WebSocketServer(URL);
            server.Start(socket =>
            {
                socket.OnOpen = () =>
                {
                    try
                    {
                        OnConnected(socket);
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine($"OnConnected: {ex}");
                    }
                };
                socket.OnMessage = message =>
                {
                    try
                    {
                        OnReceive(socket, message);
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine($"OnReceive: {ex}");
                    }
                };
                socket.OnClose = () =>
                {
                    try
                    {
                        OnDisconnect(socket);
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine($"OnDisconnect: {ex}");
                    }
                };
                socket.OnError = (e) =>
                {
                    try
                    {
                        OnDisconnect(socket);
                        socket.Close();
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine($"OnError: {ex}");
                    }
                };
            });
        }

        private void OnConnected(IWebSocketConnection context)
        {
            if (UserList.Count < ClientLimit)
            {
                Debug.WriteLine($"OnConnected: {context.ConnectionInfo.Id}, {context.ConnectionInfo.ClientIpAddress}");

                UserList[context.ConnectionInfo.Id] = context;
            }
            else
            {
                Debug.WriteLine($"OverLimit, Closed: {context.ConnectionInfo.Id}, {context.ConnectionInfo.ClientIpAddress}");
                context.Close();
            }
        }

        private int clientLimit = 5; 
        public int ClientLimit
        {
            get
            {
                lock(this)
                {
                    return clientLimit;
                }
            }
            set
            {
                lock (this)
                {
                    clientLimit = value;
                }
            }
        }

        public int ClientCount
        {
            get
            {
                return UserList.Count;
            }
        }

        public int StreamsCount
        {
            get
            {
                return Streams.Count;
            }
        }

        private void OnDisconnect(IWebSocketConnection context)
        {
            Debug.WriteLine($"OnDisconnect: {context.ConnectionInfo.Id}, {context.ConnectionInfo.ClientIpAddress}");
            {
                IWebSocketConnection ctx;                
                UserList.TryRemove(context.ConnectionInfo.Id, out ctx);
                Streams.TryRemove(context.ConnectionInfo.Id, out ctx);
            }
        }

        private void OnReceive(IWebSocketConnection context, string msg)
        {
            Debug.WriteLine($"OnReceive {context.ConnectionInfo.Id}: {msg}");

            if (!msg.Contains("command") || mc == null) return; 

            if(UserList.ContainsKey(context.ConnectionInfo.Id))
            {
                JsonData jd = JsonMapper.ToObject(msg);
                string command = jd["command"].ToString();

                switch (command) 
                {
                    case Command.offer:
                    {
                        if (UserList.Count <= ClientLimit && !Streams.ContainsKey(context.ConnectionInfo.Id))
                        {
                            Streams[context.ConnectionInfo.Id] = context;

                            var desc = jd["desc"];
                            var sdp = desc["sdp"];

                            mc.OnOfferRequest(sdp.ToString());
                        }
                        else
                        {
                            context.Send(JsonHelper.GetJsonStr(Command.offer, null, Str.Failed));
                        }
                    }
                    break;

                    case Command.onicecandidate:
                    {
                        var c = jd["candidate"];

                        var sdpMLineIndex = c["sdpMLineIndex"];
                        var sdpMid = c["sdpMid"];
                        var candidate = c["candidate"];

                        mc.AddIceCandidate(sdpMid.ToString(), (int)sdpMLineIndex, candidate.ToString());
                    }
                    break;
                }
            }
        }

        public void Dispose()
        {
            try
            {
                foreach (IWebSocketConnection i in UserList.Values)
                {
                    i.Close();
                }
                server.Dispose();
                UserList.Clear();
            }
            catch { }
        }
    }

    public class JsonHelper
    {
        public static JsonData GetJson(string command, string ret)
        {
            JsonData jd = new JsonData();
            jd["command"] = command;
            jd["ret"] = ret;
            return jd;
        }

        public static string GetJsonStr(string command, string data, string ret)
        {
            JsonData jd = new JsonData();
            jd["command"] = command;
            jd["data"] = data;
            jd["ret"] = ret;
            return jd.ToJson();
        }
    }
}
