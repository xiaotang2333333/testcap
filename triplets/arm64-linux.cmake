# Overlay triplet: identical to the official triplets/arm64-linux.cmake, plus one
# variable. Kept as an overlay so the official triplet definitions stay untouched.
#
# Why X_VCPKG_FORCE_VCPKG_X_LIBRARIES is needed here:
# vcpkg's X11 ports (libx11, libxrender, libxrandr, libxi, ...) are EMPTY packages on
# non-Windows by default -- they expect the system to provide the libraries. That is
# fine for a native build, but under cross-compilation cairo's meson link test for
# XRenderCreateConicalGradient links against the host (x86_64) libXrender, which fails
# for an arm64 target. HAVE_XRENDERCREATECONICALGRADIENT then stays unset, cairo
# re-defines XLinearGradient/XCircle/XRadialGradient/XConicalGradient in
# cairo-xlib-xrender-private.h, and those collide with the system Xrender.h:
#
#   cairo-xlib-xrender-private.h:109: error: conflicting types for 'XLinearGradient'
#   /usr/include/X11/extensions/Xrender.h:186: note: originally defined here
#
# Forcing vcpkg to build the X11 libraries for the target keeps headers and libraries
# architecture-consistent, so the link test succeeds and cairo uses the system types.

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(X_VCPKG_FORCE_VCPKG_X_LIBRARIES ON)
