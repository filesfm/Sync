# CMake toolchain for cross-compiling the Linux client to aarch64 on an
# x86_64 Debian/Ubuntu host with multiarch (dpkg --add-architecture arm64).
#
# Used by the appimage-aarch64 job in .github/workflows/release.yml:
#   - compilers come from the g++-aarch64-linux-gnu package;
#   - target libraries/headers come from "<pkg>-dev:arm64" packages, which
#     land in /usr/lib/aarch64-linux-gnu and /usr/include, so no separate
#     sysroot is needed;
#   - dependencies built from source (KArchive, qtkeychain) are installed
#     into AARCH64_DEPS_PREFIX (default /opt/aarch64);
#   - Qt is a target (arm64) Qt in CMAKE_PREFIX_PATH plus an x86_64 host Qt
#     passed as -DQT_HOST_PATH=... for moc/rcc/uic/qmltyperegistrar etc.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Makes find_library()/find_package() search the multiarch directory
# /usr/lib/aarch64-linux-gnu (and never /usr/lib/x86_64-linux-gnu).
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

if(NOT DEFINED AARCH64_DEPS_PREFIX)
    if(DEFINED ENV{AARCH64_DEPS_PREFIX})
        set(AARCH64_DEPS_PREFIX "$ENV{AARCH64_DEPS_PREFIX}")
    else()
        set(AARCH64_DEPS_PREFIX /opt/aarch64)
    endif()
endif()
# Propagate into try_compile() projects, which re-read this file.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES AARCH64_DEPS_PREFIX)

# Build-time programs (pkg-config, rsvg-convert, Qt host tools, ...) are
# host binaries; libraries, headers and CMake packages are the target's.
# The target libraries live under /usr (multiarch), so lookups can't be
# restricted to a root path -- the host side simply has no x86_64 -dev
# packages of the project's dependencies installed to be found by mistake.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# pkg-config must only report aarch64 .pc files.
set(ENV{PKG_CONFIG_LIBDIR} "${AARCH64_DEPS_PREFIX}/lib/pkgconfig:/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")

# Safety net for anything that wants to run a freshly built target binary
# at build time (requires qemu-user binfmt or qemu-user-static on PATH).
find_program(QEMU_AARCH64_EXECUTABLE NAMES qemu-aarch64-static qemu-aarch64)
if(QEMU_AARCH64_EXECUTABLE)
    set(CMAKE_CROSSCOMPILING_EMULATOR "${QEMU_AARCH64_EXECUTABLE}")
endif()
