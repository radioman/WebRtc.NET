d:

cd D:\webrtc-checkout

set DEPOT_TOOLS_WIN_TOOLCHAIN=0

gclient sync --force 
gclient sync --force

gclient runhooks --force

pause