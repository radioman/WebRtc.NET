d:

cd D:\webrtc-checkout\src

set DEPOT_TOOLS_WIN_TOOLCHAIN=0

gn gen out/DebugX64 --ide="vs2017" --args="is_debug=true target_cpu=\"x64\" rtc_use_h264=true is_clang=false target_winuwp_family=\"desktop\" ffmpeg_branding=\"Chrome\""

pause