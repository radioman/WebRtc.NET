d:

cd D:\webrtc-checkout\src

set DEPOT_TOOLS_WIN_TOOLCHAIN=0

gn gen out/DebugX86 --ide="vs2015" --args="is_debug=true target_cpu=\"x86\" is_component_build=false symbol_level=0"

pause