# third_party

Vendored third-party headers used **only by the g++/clang fallback builds** in
`build/build.bat`. The MSVC build does not use these — it consumes the upstream
headers from vcpkg via the `.vcxproj`.

## nlohmann (JSON for Modern C++) 3.12.0 — locally patched

Copied from `C:\vcpkg\installed\x64-windows-static\include\nlohmann`.

GCC and Clang enforce `[basic.link]`: a C++20 module **interface** unit may not
expose a translation-unit-local (internal-linkage) entity. nlohmann 3.12.0
declares two namespace-scope helpers `static` (internal linkage), and they are
reachable from our exported module interfaces (`seats`, `crypto`, `flights`, …),
so g++/clang reject the build. MSVC does not enforce this, which is why the
upstream vcpkg headers are fine for the MSVC path.

Patch applied (`static` → `inline`, matching their already-`inline` siblings):

```diff
# detail/string_escape.hpp:65
-static void unescape(StringType& s)
+inline void unescape(StringType& s)

# detail/input/binary_reader.hpp:56
-static inline bool little_endianness(int num = 1) noexcept
+inline bool little_endianness(int num = 1) noexcept
```

### Refreshing this copy after a vcpkg upgrade

```sh
cp -r C:/vcpkg/installed/x64-windows-static/include/nlohmann third_party/nlohmann
# then re-apply the two edits above (only if upstream is still 3.12.0 / unfixed)
```

If a future nlohmann release makes these `inline` upstream, this vendored copy
can be deleted and `build.bat`'s `FALLBACK_INCLUDE` repointed at vcpkg.
