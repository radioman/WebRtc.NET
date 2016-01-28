
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
        public const string Falid = "falid";
        public const string Success = "success";
        public const string Exist = "exist";
    }
    public class Command
    {
        public const string closeClient = "closeClient";
        public const string setMaster = "setMaster";
        public const string Conn = "conn";
        public const string startStream = "startStream";
    }

    class WebRTCServer : IDisposable
    {
        public IWebSocketConnection Master;
        public ConcurrentDictionary<Guid, IWebSocketConnection> UserList = new ConcurrentDictionary<Guid, IWebSocketConnection>();
        public ConcurrentDictionary<Guid, IWebSocketConnection> Streams = new ConcurrentDictionary<Guid, IWebSocketConnection>();

        private WebSocketServer server;
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
            if (UserList.Count == 0 && Master == null)
            {
                // no masters around, become the master!
                Master = context;
                context.Send(JsonHelper.GetJsonStr(Command.setMaster, null, Str.Success));
                return;
            }

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
                if (ctx != null)
                {
                    if (Master != null)
                    {
                        Master.Send(JsonHelper.GetJsonStr(Command.closeClient, context.ConnectionInfo.Id.ToString(), null));
                    }
                }
            }
        }
        private void OnReceive(IWebSocketConnection context, string msg)
        {
            Debug.WriteLine($"OnReceive {context.ConnectionInfo.Id}: {msg}");

            if (!msg.Contains("command")) return; 

            if(UserList.ContainsKey(context.ConnectionInfo.Id) || context == Master)
            {
                JsonData jd = JsonMapper.ToObject(msg);
                string command = jd["command"].ToString();

                switch (command) 
                {
                    case Command.setMaster:
                    {
                        lock (this)
                        {
                            if (Master == null) // there can be only one ;}
                            {
                                Master = context;

                                IWebSocketConnection ctx;
                                UserList.TryRemove(Master.ConnectionInfo.Id, out ctx);

                                context.Send(JsonHelper.GetJsonStr(Command.setMaster, null, Str.Success));
                            }
                            else
                            {
                                context.Send(JsonHelper.GetJsonStr(Command.setMaster, null, Str.Falid));
                            }
                        }
                    }                         
                    break;

                    case Command.startStream:
                    {
                        if (context != Master &&
                            UserList.Count <= ClientLimit &&
                            !Streams.ContainsKey(context.ConnectionInfo.Id))
                        {
                            Streams[context.ConnectionInfo.Id] = context;

                            JsonData send = JsonHelper.GetJson("conn", "main");
                            lock (this)
                            {
                                send["to"] = Master.ConnectionInfo.Id.ToString();
                            }
                            send["from"] = context.ConnectionInfo.Id.ToString();
                            send["type"] = "start";
                            context.Send(send.ToJson());
                        }
                        else
                        {
                            context.Send(JsonHelper.GetJsonStr(Command.startStream, null, Str.Falid));
                        }
                    }
                    break;

                    case Command.Conn: 
                    {
                        Guid id = Guid.Parse(jd["to"].ToString());
                        if (id == Master.ConnectionInfo.Id)
                        {
                            Master.Send(msg);
                        }
                        else
                        {
                            UserList[id].Send(msg);
                        }
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
