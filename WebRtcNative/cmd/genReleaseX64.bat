d:

cd D:\webrtc-checkout\src

set DEPOT_TOOLS_WIN_TOOLCHAIN=0

gn gen out/ReleaseX64 --ide="vs2015" --args="is_debug=false target_cpu=\"x64\" is_component_build=false symbol_level=0"

pause
