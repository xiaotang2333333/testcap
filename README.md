# opencv4 vcpkg overlay port — 4.12.0#9 → 4.14.0

This repository carries an **overlay port** that upgrades vcpkg's `opencv4` port from
`4.12.0#9` to `4.14.0`, plus a GitHub Actions workflow that validates the port by
building and installing it for the official vcpkg triplets.

No branch or commit is made inside the vcpkg checkout; everything lives here and is
consumed through `overlay-ports`.

## Layout

```
.
├── overlay-ports/opencv4/       # the upgraded port (portfile + patches + manifest)
├── vcpkg.json                   # manifest: depends on opencv4, baselines + overlay config
├── .github/workflows/ci-opencv4.yml
├── CMakeLists.txt / main.cpp    # tiny consumer used as a link-time sanity check
└── CMakePresets.json
```

## What changed versus the 4.12.0#9 port

Version metadata:

| Item | Before | After |
| --- | --- | --- |
| `vcpkg.json` `version` | `4.12.0` | `4.14.0` |
| `vcpkg.json` `port-version` | `9` | `0` |
| `documentation` | `docs.opencv.org/4.12.0/` | `docs.opencv.org/4.14.0/` |
| `vcpkg_from_github(opencv)` SHA512 | `8ac63ddd…` | `4f1e606c…` |
| `vcpkg_from_github(opencv_contrib)` SHA512 | `574121ca…` | `7fa7ecaa…` |

The refs stay on `REF "${VERSION}"`, so only the hashes had to move.

### Patch disposition

`4.14.0` was extracted, every patch was dry-run applied with the exact command vcpkg
uses (`git apply --ignore-whitespace --whitespace=nowarn`), and each failure was
classified.

**Dropped — already upstream in 4.14.0** (verified by reverse-apply + source inspection):

| Patch | Why |
| --- | --- |
| `0020-fix-narrow-filesystem.diff` | upstream now uses `GetTempPathW` + `WideCharToMultiByte(CP_UTF8, …)`, a superset of the old `GetTempPathA` fix |
| `0023-ffmpeg8-support.patch` | upstream ships the same `LIBAVCODEC_BUILD` guards and `coded_side_data` path |
| `0024-openvino-const-tensor-data.patch` | upstream already has `const_cast<void*>(blob.data())`, `ov::element::dynamic` and the `const uint8_t*` copy |
| `0027-contrib-cuda-tuple.patch` | upstream `opencv_contrib` already carries the cudev tuple fix |
| downloaded patch `468de9b3…` (`PATCH1_FILE`, Eigen version) | upstreamed in 4.14.0 |
| downloaded patch `f0888a10…` (`CUDA_13_SUPPORT_PATCH`) | upstreamed in 4.14.0 |
| downloaded patch `f2854f4f…` (`CONTRIB_CUDA_NAMESPACE_FIX`) | upstreamed in 4.14.0 |
| downloaded patch `f49f0aef…` (`CONTRIB_CUDA_NOT1_FIX`) | upstreamed in 4.14.0 |

Removing them also removes four network downloads from the port.

**Refreshed — still required, context rebased onto 4.14.0:**

| Patch | What was stale |
| --- | --- |
| `0002-install-options.patch` | upstream added `include(cmake/OpenCVDetectDLPack.cmake)` next to the Python detection, so the `WITH_PYTHON` hunk was rebased (hunk body grew by one context line) |
| `0003-force-package-requirements.patch` | upstream added `PNG_PNG_INCLUDE_DIR` to `ocv_clear_internal_cache_vars(...)` in `OpenCVFindLibsGrfmt.cmake`, which broke the `find_package(PNG REQUIRED)` hunk |
| `0017-fix-flatbuffers.patch` | vendored flatbuffers version string moved `23.5.9` → `25.9.23` |
| `0021-fix-qt-gen-def.patch` | upstream replaced `add_definitions(${Qt6*_DEFINITIONS})` with `link_libraries(${Qt6*})`; the patch now only drops the global `include_directories(${Qt6*_INCLUDE_DIRS})` |

**Kept unchanged** (they still apply byte-for-byte): `0001`, `0004`, `0005`, `0007`,
`0008`, `0009`, `0010`, `0012`, `0013`, `0015`, `0016`, `0018`, `0019`, `0022`,
`0025`, `0026`, `0028`.

All 16 main-tree patches and all 5 contrib patches were re-verified to apply cleanly,
in portfile order, on a pristine `4.14.0` tree.

## Running the CI

`.github/workflows/ci-opencv4.yml`

* Runs automatically on push / pull request with the **core** triplet set.
* `workflow_dispatch` adds a `scope` input:

| scope | triplets |
| --- | --- |
| `core` (default) | `x64-windows`, `x64-windows-static`, `x64-windows-static-md`, `x64-windows-release`, `x86-windows`, `x64-linux` |
| `all` | core + `arm64-windows`, `arm64-windows-static-md`, `arm64-osx`, `arm64-linux` (cross, gcc-aarch64-linux-gnu), `x64-android`, `arm64-android`, `arm-neon-android` |

Each job:

1. clones vcpkg and checks out the pinned baseline `ee6a47da…` (the same commit as the
   local vcpkg checkout, so official triplet definitions and dependency versions match);
2. resolves the manifest with `--dry-run` to fail fast on JSON/port errors;
3. runs `vcpkg install --triplet <t> --clean-after-build`;
4. asserts `include/opencv4/opencv2` and an `opencv_core` library exist;
5. on `x64-windows`, configures the sample project with
   `find_package(OpenCV CONFIG REQUIRED)` and runs it, proving the port is consumable.

`fail-fast: false`, so one triplet failing does not hide the others. Expect multi-hour
runtimes for the `all` scope: OpenCV plus its default-feature dependency tree is big,
and there is no binary cache.

## Running it locally

The local vcpkg is `C:\msys64\home\xiaotang\vcpkg`. Nothing in it was modified.

```powershell
cd c:\Users\65717\Desktop\testcap
C:\msys64\home\xiaotang\vcpkg\vcpkg.exe install --triplet x64-windows
```

`vcpkg.json` already points `overlay-ports` at `./overlay-ports` and pins
`builtin-baseline`, so the overlay `opencv4` `4.14.0` is what gets built. A full OpenCV
build is memory hungry; the CI matrix is the intended validation path.
