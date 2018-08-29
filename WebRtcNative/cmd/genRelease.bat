d:

cd D:\webrtc-checkout\src

set DEPOT_TOOLS_WIN_TOOLCHAIN=0

gn gen out/ReleaseX86 --ide="vs2017" --args="is_debug=false target_cpu=\"x86\" rtc_use_h264=true symbol_level=0 is_clang=false target_winuwp_family=\"desktop\" ffmpeg_branding=\"Chrome\""

pause
