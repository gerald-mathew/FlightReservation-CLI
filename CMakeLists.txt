cmake_minimum_required(VERSION 3.31)

# The sources use `import std;`. CMake gates its built-in std-module support
# behind an experimental feature flag that MUST be set before the CXX language is
# enabled. CMake only implements this with the Ninja generator (see
# CMakeCommonCompilerMacros.cmake), so every platform must configure with
# `-G Ninja`. The activation value is a UUID that CMake bumps on every minor
# release, so select the one matching the running CMake. Override with
# -DCMAKE_EXPERIMENTAL_CXX_IMPORT_STD=<uuid> (see CMake's Help/dev/experimental)
# when building with a release newer than this list knows about.
if(NOT DEFINED CMAKE_EXPERIMENTAL_CXX_IMPORT_STD)
	if(CMAKE_VERSION VERSION_LESS "4.3")
		set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444") # CMake 4.2
	elseif(CMAKE_VERSION VERSION_LESS "4.4")
		set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b") # CMake 4.3
	elseif(CMAKE_VERSION VERSION_LESS "4.5")
		set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d") # CMake 4.4
	else()
		message(FATAL_ERROR
			"This project has no `import std;` activation UUID recorded for CMake "
			"${CMAKE_VERSION}. Set -DCMAKE_EXPERIMENTAL_CXX_IMPORT_STD=<uuid> using "
			"the value documented in that CMake's Help/dev/experimental.rst.")
	endif()
endif()

project(NexoraAirways
	VERSION 1.0.0
	DESCRIPTION "Nexora Airways flight reservation CLI"
	LANGUAGES CXX
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(NEXORA_IS_TOP_LEVEL ON)
else()
	set(NEXORA_IS_TOP_LEVEL OFF)
endif()

# C++20 named modules (import std; import seats; ...): the build system must
# scan module interfaces and compile them in dependency order. CMake does that
# natively with the Ninja and Visual Studio generators, driven by the CXX_MODULES
# file set below. GCC 14+ / Clang 16+ / MSVC 19.28+ are required.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Extensions must stay ON: CMake compiles its synthesized std module in GNU mode
# (gnu++23). Compiling the target in strict mode (c++23) would hide
# __GLIBCXX_TYPE_INT_N_0 in this TU only, and GCC then reports a conflicting
# redeclaration of std::__is_integral_helper<unsigned __int128> between the
# textually-included <type_traits> and the imported std module.
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# nlohmann/json is header-only. The vendored, locally-patched copy under
# third_party/nlohmann is used on every platform: GCC/Clang reject the upstream
# 3.12.0 headers because two namespace-scope helpers are declared `static`
# (TU-local) and become visible through our module interfaces. Using the patched
# copy everywhere keeps the MSVC and GCC/Clang builds identical and removes the
# need for a vcpkg nlohmann-json package. See third_party/README.md.
set(NEXORA_VENDORED_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

find_package(PkgConfig QUIET)

# ---- argon2 (password hashing) ---------------------------------------------
find_package(unofficial-argon2 CONFIG QUIET)
if(TARGET unofficial-argon2::argon2)
	set(NEXORA_ARGON2 unofficial-argon2::argon2)
else()
	if(PkgConfig_FOUND)
		pkg_check_modules(ARGON2 IMPORTED_TARGET libargon2)
	endif()
	if(TARGET PkgConfig::ARGON2)
		set(NEXORA_ARGON2 PkgConfig::ARGON2)
	else()
		find_path(ARGON2_INCLUDE_DIR NAMES argon2.h)
		find_library(ARGON2_LIBRARY NAMES argon2)
		if(NOT ARGON2_INCLUDE_DIR OR NOT ARGON2_LIBRARY)
			message(FATAL_ERROR "argon2 not found. Install libargon2-dev (Linux), brew install argon2 (macOS), or vcpkg install argon2.")
		endif()
		add_library(NexoraArgon2 UNKNOWN IMPORTED)
		set_target_properties(NexoraArgon2 PROPERTIES
			IMPORTED_LOCATION "${ARGON2_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${ARGON2_INCLUDE_DIR}"
		)
		set(NEXORA_ARGON2 NexoraArgon2)
	endif()
endif()

# ---- sqlite3 ---------------------------------------------------------------
find_package(unofficial-sqlite3 CONFIG QUIET)
if(TARGET unofficial::sqlite3::sqlite3)
	set(NEXORA_SQLITE3 unofficial::sqlite3::sqlite3)
else()
	find_package(SQLite3 QUIET)
	if(TARGET SQLite::SQLite3)
		set(NEXORA_SQLITE3 SQLite::SQLite3)
	else()
		if(PkgConfig_FOUND)
			pkg_check_modules(SQLITE3 IMPORTED_TARGET sqlite3)
		endif()
		if(TARGET PkgConfig::SQLITE3)
			set(NEXORA_SQLITE3 PkgConfig::SQLITE3)
		else()
			message(FATAL_ERROR "sqlite3 not found. Install libsqlite3-dev (Linux), brew install sqlite (macOS), or vcpkg install sqlite3.")
		endif()
	endif()
endif()

# ---- libcurl (Paystack HTTPS) ----------------------------------------------
find_package(CURL QUIET)
if(TARGET CURL::libcurl)
	set(NEXORA_CURL CURL::libcurl)
else()
	if(PkgConfig_FOUND)
		pkg_check_modules(CURL IMPORTED_TARGET libcurl)
	endif()
	if(TARGET PkgConfig::CURL)
		set(NEXORA_CURL PkgConfig::CURL)
	else()
		message(FATAL_ERROR "libcurl not found. Install libcurl4-openssl-dev (Linux), brew install curl (macOS), or vcpkg install curl.")
	endif()
endif()

add_executable(NexoraAirways
	src/main.cpp
	src/crypto.cpp
	src/seats.cpp
	src/sqlite3db.cpp
	src/payment.cpp
	src/flights.cpp
	src/users.cpp
	src/menus.cpp
)

# Module interface units (export module X;). Order here is irrelevant: CMake
# scans imports and schedules compilation according to the actual dependency
# graph (seats/crypto/payment/sqlite3db -> flights -> users -> menus).
target_sources(NexoraAirways PRIVATE
	FILE_SET CXX_MODULES FILES
		src/crypto.ixx
		src/seats.ixx
		src/sqlite3db.ixx
		src/payment.ixx
		src/flights.ixx
		src/users.ixx
		src/menus.ixx
)

# The sources use `import std;` (the C++23 standard library module). This tells
# CMake to provide and build the `std` module for the target.
set_target_properties(NexoraAirways PROPERTIES CXX_MODULE_STD ON)

target_include_directories(NexoraAirways PRIVATE "${NEXORA_VENDORED_INCLUDE}")

target_link_libraries(NexoraAirways PRIVATE
	${NEXORA_ARGON2}
	${NEXORA_SQLITE3}
	${NEXORA_CURL}
)

if(WIN32)
	# curl/sqlite3/argon2 are consumed as static archives on Windows; the curl
	# headers need CURL_STATICLIB to avoid declaring dllimport symbols.
	target_compile_definitions(NexoraAirways PRIVATE CURL_STATICLIB)
	target_link_libraries(NexoraAirways PRIVATE
		crypt32 normaliz wldap32 ws2_32 secur32 iphlpapi psapi
	)
endif()

if(MSVC)
	target_compile_options(NexoraAirways PRIVATE /W3 /permissive-)
	target_compile_options(NexoraAirways PRIVATE /utf-8)
else()
	target_compile_options(NexoraAirways PRIVATE -Wall -Wextra)
endif()

# Run the program from the repository root so the relative data/ and config/
# paths resolve (data/flight_reservation.db, config/paystack_secret.txt).
if(NEXORA_IS_TOP_LEVEL)
	set_target_properties(NexoraAirways PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)
endif()

install(TARGETS NexoraAirways RUNTIME DESTINATION bin)
