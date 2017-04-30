z:
cd Z:\Coding\WebRtc.NET\WebRtcNative\lib_x64

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" 

del link\protobuf_full.lib

lib /out:webrtc-all.lib link\*.lib

pause