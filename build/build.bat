@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM Build script for FlightReservation-CLI (Nexora Airways)
REM
REM Build order:  1) MSVC (MSBuild)  ->  2) clang++  ->  3) g++
REM
REM The project now emits its executable directly into build\ (see the vcxproj
REM OutDir/TargetName), so no copy-out from x64\Debug or x64 cleanup is needed.
REM The C++20 named modules are compiled in dependency order for the g++/clang
REM fallbacks.
REM ============================================================================

REM --- Paths -------------------------------------------------------------------
set "BUILD_DIR=%~dp0"
set "PROJECT_DIR=%BUILD_DIR%.."
set "SRC_DIR=%PROJECT_DIR%\src"
set "TARGET_NAME=NexoraAirways"
set "TARGET_EXE=%BUILD_DIR%%TARGET_NAME%.exe"

REM Toolchain locations used by the fallbacks
set "MSYS2_ROOT=C:\msys64"
set "UCRT64=%MSYS2_ROOT%\ucrt64"
REM nlohmann/json is header-only. The fallback compilers (g++/clang) use a
REM vendored, locally-patched copy under third_party\ -- vcpkg's 3.12.0 declares
REM two namespace-scope helpers `static` (internal linkage), which g++/clang
REM reject as TU-local entities exposed by the module interfaces. MSVC ignores
REM this, so the MSVC build keeps using the vcpkg headers via the .vcxproj.
REM See third_party\README.md for the exact patch.
set "FALLBACK_INCLUDE=%PROJECT_DIR%\third_party"

REM Dependency-ordered module interface units. Interfaces MUST be compiled
REM before any unit that imports them:
REM   seats crypto payment sqlite3db  (depend only on `import std`)
REM   flights   -> seats
REM   users     -> flights, sqlite3db, crypto
REM   menus     -> payment, users, flights
set "MODULE_ORDER=seats crypto payment sqlite3db flights users menus"
REM Implementation units (.cpp). main.cpp is a plain TU that imports `menus`.
set "IMPL_UNITS=crypto seats sqlite3db payment flights users menus main"

cd /d "%PROJECT_DIR%"

REM ============================================================================
REM 1) MSVC (primary)
REM ============================================================================
set "VSINSTALL="
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -version "[18,19)" -property installationPath 2^>nul`) do (
	set "VSINSTALL=%%i"
	goto :found_vs
)

REM Fallback to hardcoded paths if vswhere fails
if not defined VSINSTALL (
	if exist "C:\Program Files\Microsoft Visual Studio\18\Community" (
		set "VSINSTALL=C:\Program Files\Microsoft Visual Studio\18\Community"
	) else if exist "C:\Program Files\Microsoft Visual Studio\2026\Community" (
		set "VSINSTALL=C:\Program Files\Microsoft Visual Studio\2026\Community"
	)
)

:found_vs
if not defined VSINSTALL (
	echo Visual Studio not found. Falling back to clang...
	goto :try_clang
)
if not exist "%VSINSTALL%" (
	echo Visual Studio path does not exist: "%VSINSTALL%"
	echo Falling back to clang...
	goto :try_clang
)

echo Setting up Visual Studio environment...
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if !errorlevel! neq 0 (
	echo Failed to set up Visual Studio environment. Falling back to clang...
	goto :try_clang
)

echo Building with MSBuild...
msbuild "FlightReservation-CLI.vcxproj" ^
	/p:Configuration=Debug ^
	/p:Platform=x64 ^
	/v:minimal
if !errorlevel! neq 0 (
	echo.
	echo MSBuild failed with error code !errorlevel!. Falling back to clang...
	goto :try_clang
)

if not exist "%TARGET_EXE%" (
	echo Build output not found at "%TARGET_EXE%". Falling back to clang...
	goto :try_clang
)

echo.
echo Build completed successfully (MSVC).
echo Executable path: %TARGET_EXE%
exit /b 0

REM ============================================================================
REM 2) clang++ (first fallback)
REM ============================================================================
:try_clang
echo.
echo Attempting clang++ compilation...

where clang++ >nul 2>&1
if !errorlevel! neq 0 (
	echo clang++ not found. Falling back to g++...
	goto :try_gpp
)

REM Locate libc++ std module source for `import std;` (best effort).
set "CLANG_STD_MODULE="
for %%P in (
	"%MSYS2_ROOT%\clang64\share\libc++\v1\std.cppm"
	"C:\Program Files\LLVM\share\libc++\v1\std.cppm"
) do (
	if exist %%P set "CLANG_STD_MODULE=%%~P"
)
if not defined CLANG_STD_MODULE (
	echo.
	echo [ERROR] Could not find libc++ 'std.cppm' for clang's `import std;`.
	echo   Looked in: %MSYS2_ROOT%\clang64\share\libc++\v1
	echo              C:\Program Files\LLVM\share\libc++\v1
	echo   Install LLVM/libc++ with the std module, or use the g++ fallback.
	echo Falling back to g++...
	goto :try_gpp
)

set "OBJ_DIR=%BUILD_DIR%clang-obj"
if exist "%OBJ_DIR%" rmdir /s /q "%OBJ_DIR%" 2>nul
mkdir "%OBJ_DIR%" 2>nul

set "CLANG_FLAGS=-std=c++23 -stdlib=libc++ -I"%FALLBACK_INCLUDE%" -fprebuilt-module-path="%OBJ_DIR%""

echo Building std module...
clang++ %CLANG_FLAGS% --precompile -x c++-module "%CLANG_STD_MODULE%" -o "%OBJ_DIR%\std.pcm" 2>"%BUILD_DIR%build_error.log"
if !errorlevel! neq 0 (
	echo [ERROR] Failed to precompile std module with clang.
	type "%BUILD_DIR%build_error.log"
	echo Falling back to g++...
	goto :try_gpp
)
clang++ %CLANG_FLAGS% -c "%OBJ_DIR%\std.pcm" -o "%OBJ_DIR%\std.o" 2>>"%BUILD_DIR%build_error.log"
set "CLANG_OBJS="%OBJ_DIR%\std.o""

echo Precompiling module interfaces (dependency order)...
for %%M in (%MODULE_ORDER%) do (
	echo   module: %%M
	clang++ %CLANG_FLAGS% --precompile -x c++-module "%SRC_DIR%\%%M.ixx" -o "%OBJ_DIR%\%%M.pcm" 2>>"%BUILD_DIR%build_error.log"
	if !errorlevel! neq 0 (
		echo [ERROR] clang failed to precompile module: %%M
		type "%BUILD_DIR%build_error.log"
		echo Falling back to g++...
		goto :try_gpp
	)
	clang++ %CLANG_FLAGS% -c "%OBJ_DIR%\%%M.pcm" -o "%OBJ_DIR%\%%M.pcm.o" 2>>"%BUILD_DIR%build_error.log"
	set "CLANG_OBJS=!CLANG_OBJS! "%OBJ_DIR%\%%M.pcm.o""
)

echo Compiling implementation units...
for %%U in (%IMPL_UNITS%) do (
	echo   unit: %%U
	clang++ %CLANG_FLAGS% -c "%SRC_DIR%\%%U.cpp" -o "%OBJ_DIR%\%%U.o" 2>>"%BUILD_DIR%build_error.log"
	if !errorlevel! neq 0 (
		echo [ERROR] clang failed to compile: %%U.cpp
		type "%BUILD_DIR%build_error.log"
		echo Falling back to g++...
		goto :try_gpp
	)
	set "CLANG_OBJS=!CLANG_OBJS! "%OBJ_DIR%\%%U.o""
)

echo Linking...
clang++ -std=c++23 -stdlib=libc++ !CLANG_OBJS! ^
	-L"%UCRT64%\lib" -largon2 -lcurl -lsqlite3 -lshell32 ^
	-o "%TARGET_EXE%" 2>>"%BUILD_DIR%build_error.log"
if !errorlevel! neq 0 (
	echo [ERROR] clang link step failed.
	type "%BUILD_DIR%build_error.log"
	echo Falling back to g++...
	goto :try_gpp
)
if not exist "%TARGET_EXE%" (
	echo [ERROR] clang produced no executable. Falling back to g++...
	goto :try_gpp
)

del /f /q "%BUILD_DIR%build_error.log" 2>nul
echo.
echo Build completed successfully (clang).
echo Executable path: %TARGET_EXE%
exit /b 0

REM ============================================================================
REM 3) g++ (final fallback) -- GCC named modules via -fmodules-ts
REM ============================================================================
:try_gpp
echo.
echo Attempting g++ compilation...

where g++ >nul 2>&1
if !errorlevel! neq 0 (
	echo.
	echo [ERROR] No compiler available -- MSVC, clang++, and g++ all failed/missing.
	echo Install one of:
	echo   1. Visual Studio 2026 Community with the C++ workload
	echo   2. MSYS2 + MinGW-w64 g++   ^(https://www.msys2.org/^)
	echo   3. LLVM/Clang with libc++  ^(https://releases.llvm.org/^)
	exit /b 1
)

if not exist "%UCRT64%\lib" (
	echo.
	echo [ERROR] MSYS2 UCRT64 library directory not found: "%UCRT64%\lib"
	echo Install MSYS2 ^(https://www.msys2.org/^) or fix MSYS2_ROOT in this script.
	exit /b 1
)

REM Locate the libstdc++ std module source shipped with this GCC.
set "STD_MODULE="
for /d %%V in ("%UCRT64%\include\c++\*") do (
	if exist "%%V\bits\std.cc" set "STD_MODULE=%%V\bits\std.cc"
)
if not defined STD_MODULE (
	echo.
	echo [ERROR] Could not find libstdc++ 'std.cc' under "%UCRT64%\include\c++".
	echo This GCC may be too old for `import std;` ^(needs GCC 15+^).
	exit /b 1
)

set "OBJ_DIR=%BUILD_DIR%gpp-obj"
if exist "%OBJ_DIR%" rmdir /s /q "%OBJ_DIR%" 2>nul
mkdir "%OBJ_DIR%" 2>nul

REM GCC writes/reads module BMIs in ./gcm.cache relative to the CWD, so run the
REM whole build from inside OBJ_DIR and reference sources by absolute path.
cd /d "%OBJ_DIR%"

set "GPP_FLAGS=-std=c++23 -fmodules-ts -I"%FALLBACK_INCLUDE%""

echo Building std module...
g++ %GPP_FLAGS% -c -x c++ "%STD_MODULE%" -o "std.o" 2>"%BUILD_DIR%build_error.log"
if !errorlevel! neq 0 (
	echo [ERROR] Failed to build std module with g++.
	type "%BUILD_DIR%build_error.log"
	exit /b 1
)
set "GPP_OBJS=std.o"

echo Compiling module interfaces (dependency order)...
for %%M in (%MODULE_ORDER%) do (
	echo   module: %%M
	g++ %GPP_FLAGS% -x c++ -c "%SRC_DIR%\%%M.ixx" -o "%%M.ixx.o" 2>"%BUILD_DIR%build_error.log"
	if !errorlevel! neq 0 (
		echo [ERROR] g++ failed to compile module interface: %%M
		type "%BUILD_DIR%build_error.log"
		exit /b 1
	)
	set "GPP_OBJS=!GPP_OBJS! %%M.ixx.o"
)

echo Compiling implementation units...
for %%U in (%IMPL_UNITS%) do (
	echo   unit: %%U
	g++ %GPP_FLAGS% -c "%SRC_DIR%\%%U.cpp" -o "%%U.o" 2>"%BUILD_DIR%build_error.log"
	if !errorlevel! neq 0 (
		echo [ERROR] g++ failed to compile: %%U.cpp
		type "%BUILD_DIR%build_error.log"
		exit /b 1
	)
	set "GPP_OBJS=!GPP_OBJS! %%U.o"
)

echo Linking...
g++ -std=c++23 !GPP_OBJS! ^
	-L"%UCRT64%\lib" -largon2 -lcurl -lsqlite3 -lshell32 ^
	-o "%TARGET_EXE%" 2>"%BUILD_DIR%build_error.log"
if !errorlevel! neq 0 (
	echo.
	echo [ERROR] g++ link step failed.
	echo Common causes:
	echo   - argon2 ^(password hashing^) or libcurl ^(HTTP^) not installed in UCRT64
	echo   - run:  pacman -S mingw-w64-ucrt-x86_64-argon2 mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-sqlite3
	echo.
	type "%BUILD_DIR%build_error.log"
	exit /b 1
)
if not exist "%TARGET_EXE%" (
	echo [ERROR] g++ produced no executable.
	exit /b 1
)

del /f /q "%BUILD_DIR%build_error.log" 2>nul
echo.
echo Build completed successfully (g++).
echo Executable path: %TARGET_EXE%
exit /b 0
