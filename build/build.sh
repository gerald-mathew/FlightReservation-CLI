#!/usr/bin/env bash
# Cross-platform build for FlightReservation-CLI (Nexora Airways).
#
# Works on Linux, macOS, and Windows (MSYS2/Git Bash/WSL) with GCC 15+, Clang
# 16+ (with libc++), or MSVC. Ninja is required because CMake only implements
# the C++23 `import std;` support with that generator.
#
# Usage:
#   ./build/build.sh [release|debug] [--clean]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

PRESET="release"
CLEAN=0

for arg in "$@"; do
	case "$arg" in
		release|Release) PRESET="release" ;;
		debug|Debug) PRESET="debug" ;;
		--clean) CLEAN=1 ;;
		-h|--help)
			echo "Usage: $0 [release|debug] [--clean]"
			exit 0
			;;
		*)
			echo "Unknown argument: $arg" >&2
			echo "Usage: $0 [release|debug] [--clean]" >&2
			exit 2
			;;
	esac
done

if ! command -v cmake >/dev/null 2>&1; then
	echo "ERROR: cmake not found (need 3.28 or newer)." >&2
	echo "  Debian/Ubuntu: sudo apt install cmake ninja-build" >&2
	echo "  macOS:         brew install cmake ninja" >&2
	exit 1
fi

if ! command -v ninja >/dev/null 2>&1; then
	echo "ERROR: ninja not found. CMake requires it for 'import std;'." >&2
	echo "  Debian/Ubuntu: sudo apt install ninja-build" >&2
	echo "  macOS:         brew install ninja" >&2
	exit 1
fi

if [ "$CLEAN" -eq 1 ]; then
	rm -rf "${PROJECT_DIR}/build/cmake/${PRESET}"
fi

cd "${PROJECT_DIR}"

echo "Configuring (${PRESET})..."
cmake --preset "${PRESET}" -Wno-dev

echo "Building (${PRESET})..."
cmake --build --preset "${PRESET}"

EXE="${PROJECT_DIR}/build/cmake/${PRESET}/NexoraAirways"
[ -f "${EXE}.exe" ] && EXE="${EXE}.exe"

if [ -f "${EXE}" ]; then
	echo
	echo "Build completed successfully."
	echo "Executable: ${EXE}"
else
	echo "ERROR: build finished but no executable was produced." >&2
	exit 1
fi
