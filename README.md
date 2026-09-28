# vcpkg overlay ports — opencv5 5.0.0 and opencv4 4.14.0

This repository carries **overlay ports** for vcpkg plus GitHub Actions workflows that
validate them by building and installing them for the official vcpkg triplets:

* `overlay-ports/opencv5` — OpenCV **5.0.0** (new; see below)
* `overlay-ports/opencv4` — upgrades vcpkg's `opencv4` port from `4.12.0#9` to `4.14.0`
* `overlay-ports/opencv` — alias port forwarding to `opencv4`

No branch or commit is made inside the vcpkg checkout; everything lives here and is
consumed through `overlay-ports`.

## OpenCV 5.0.0 (`overlay-ports/opencv5`, CI: `ci-opencv5.yml`)

The port is derived from the opencv4 port and revalidated against the pristine
`5.0.0` sources (main repo + `opencv_contrib`). Its manifest is
`ci/opencv5/vcpkg.json` so both ports can be CI'd from one repository.

### OpenCV 5 build-system findings (why the patch set changed)

* **Module restructuring**: `calib3d` was split into `geometry`/`calib`/`stereo`
  (+ new `ptcloud`), `features2d` → `features`, and **`ml` + `gapi` moved to
  `opencv_contrib`**. The feature list follows OpenCV 5's own structure instead of
  staying compatible with the opencv4 port: `calib3d` is gone and replaced by
  `calib`, `stereo` and `ptcloud` (all default features; `calib` pulls `stereo`,
  and `contrib` pulls all three because ccalib/structured_light/rgbd need them).
  `geometry` is deliberately **not** a feature — `imgproc`, `features`, `photo` and
  `objdetect` depend on it, so it is always built. `gapi` (and `ade`, `freetype`)
  depend on the `contrib` feature, and `gapi` is no longer a default feature.
* **`quirc` is gone** from the sources entirely (QR decoding is built into
  `objdetect`), so the port dropped the `quirc` feature, its dep and its patch.
* **TFLite**: 5.0 ships a pre-generated `misc/tflite/schema_generated.h`, so the
  port no longer runs `flatc` at build time; `0017-fix-flatbuffers.patch` still
  redirects flatbuffers detection to the vcpkg `flatbuffers` config package.
* **OpenEXR**: `FindOpenEXR` now exists in-tree with a `find_package(OpenEXR 3 …)`
  call; `0012-miss-openexr.patch` became redundant (its include already happens
  under `WITH_OPENEXR`) and the REQUIRED forcing folded into
  `0003-force-package-requirements.patch`.
* **New configure-time download**: `imgproc` embeds the WenQuanYi Micro Hei font
  (`WITH_UNIFONT` defaults ON), so the port pre-seeds that cache entry; the
  tiny-dnn pre-seed was dropped (no longer referenced).
* **ippicv 2026.0.0**: x64 Windows/Linux now fetch `ippicv_2026.0.0_*` from commit
  `406d398c…`; the pre-seeds were updated (the `ipp` feature is not default, but
  the seeds are correct for `--cmake-args=-DVCPKG_OPENCV4_UPDATE=1` style refreshes).

Patch disposition on 5.0.0 (13 main + 6 contrib, all verified to apply cleanly in
portfile order on pristine trees):

| Status | Patches |
| --- | --- |
| Kept byte-for-byte | `0001` `0004` `0009` `0010` `0017` `0021` `0022` `0025` `0026` `0028` + contrib `0007` `0013` `0016` `0018` `0019` |
| Refreshed | `0002-install-options` (dropped the `data/CMakeLists.txt` hunk — the directory is gone), `0003-force-package-requirements` (rebased onto 5.0 `OpenCVFindLibsGrfmt`, JPEG/OpenEXR hunks rewritten), `0005-vulkan.diff` (dnn hunk rebased) |
| Retargeted | `0015-fix-freetype` now patches **contrib** `modules/gapi/cmake/init.cmake` (gapi moved out of the main repo) |
| Dropped | `0008-devendor-quirc` (no quirc in 5.0), `0012-miss-openexr` (upstream now includes the find under `WITH_OPENEXR`) |

### CI

`.github/workflows/ci-opencv5.yml` mirrors `ci-opencv4.yml` (same 13 official
triplets, `scope=core|all`, system-package prep, arm64 cross fix, failure
annotations) but runs `vcpkg install` from `ci/opencv5/`. The manifest pins
`builtin-baseline` to the local vcpkg `master` commit the port was developed
against.

## opencv4 4.12.0#9 → 4.14.0

### Layout

```
.
├── overlay-ports/opencv5/       # OpenCV 5.0.0 port (portfile + patches + manifest)
├── overlay-ports/opencv4/       # the upgraded port (portfile + patches + manifest)
├── ci/opencv5/vcpkg.json        # manifest for the opencv5 CI (depends on opencv5)
├── vcpkg.json                   # manifest: depends on opencv4, baseline + overlay config
├── .github/workflows/ci-opencv5.yml
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

## The `opencv` alias port

`opencv` is a pure alias: its portfile is an empty package that forwards every feature to
`opencv4`, and its `vcpkg.json` only depends on `opencv4`. It needs no build changes, but
upstream keeps its version metadata in step with `opencv4`, so `overlay-ports/opencv/`
mirrors that. Only `vcpkg.json` differs from upstream (`4.12.0` -> `4.14.0` plus the
documentation link); `portfile.cmake` and `vcpkg-cmake-wrapper.cmake.in` are byte
identical.

## Verification status

All **13 official triplets** in `triplets/` pass, with zero failures:

| triplet | time | triplet | time |
| --- | --- | --- | --- |
| `x64-windows` | 52:41 | `arm64-windows` | 60:11 |
| `x64-windows-static` | 51:52 | `arm64-windows-static-md` | 49:05 |
| `x64-windows-static-md` | 50:32 | `arm64-osx` | 16:52 |
| `x64-windows-release` | 36:13 | `arm64-linux` | 47:27 |
| `x86-windows` | 50:55 | `x64-android` | 33:18 |
| `x64-linux` | 48:27 | `arm64-android` | 34:41 |
| | | `arm-neon-android` | 31:43 |

`x64-windows` additionally configures, links and runs a CMake consumer via
`find_package(OpenCV CONFIG REQUIRED)`, so the port is proven consumable and not merely
installable.

### How this compares to upstream CI

Upstream tests opencv on **12** triplets — the 13 above minus `arm64-linux`
(`scripts/ci.baseline.txt`). Upstream also tests with `default-features: false` and a
feature list that omits `gtk` (`scripts/test_ports/vcpkg-ci-opencv/vcpkg.json`), which
sidesteps the whole gtk3 dependency tree.

This CI is therefore **stricter**: it uses opencv4's default features (including `gtk` on
Linux) and covers `arm64-linux` as well. The gtk3 tree is what needs the extra system
packages listed in the workflow.

### arm64-linux cross-compilation

`arm64-linux` is cross-compiled, matching upstream's own `arm64_linux` job
(`scripts/azure-pipelines/linux-arm64/Dockerfile` uses `gcc-13-aarch64-linux-gnu`). The
official triplets are used **unmodified**.

vcpkg's X11 ports are empty packages on non-Windows
(`ports/libxrender/portfile.cmake` sets `VCPKG_POLICY_EMPTY_PACKAGE`), so the system must
supply `libX11`/`libXrender`/... . Under cross-compilation only the host x86_64 libraries
exist, cairo's meson link test for `XRenderCreateConicalGradient` fails for an arm64
target, `HAVE_XRENDERCREATECONICALGRADIENT` stays unset, and cairo re-defines
`XLinearGradient`/`XCircle`/`XRadialGradient`/`XConicalGradient` in
`cairo-xlib-xrender-private.h`, colliding with the system `Xrender.h`.

The workflow therefore installs the **arm64 variants** of the X11 development packages and
points `PKG_CONFIG_LIBDIR` at `/usr/lib/aarch64-linux-gnu/pkgconfig`. Note that only the
X11 layer comes from the system: `cairo`, `pango`, `gdk-pixbuf`, `glib`, `atk`,
`at-spi2-core` and `libepoxy` are all built by vcpkg for the target, exactly as upstream's
Dockerfile does (it installs no gtk/cairo/pango packages either).

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
