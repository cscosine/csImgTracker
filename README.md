# csimgtracker

Image tracking application

<!-- cs-template:begin (this block is owned by cs-project-template: do not edit; the rest of the file is the project's) -->
## Getting started

Requires Python 3.11 or newer and Git.

```bash
./setup.sh                    # Windows: .\setup.ps1
source .venv/bin/activate     # Windows: .\.venv\Scripts\Activate.ps1
```

`setup.sh` creates `.venv`, installs the project with its development tools and csorchestrator, and installs the
pre-commit hooks.

csorchestrator: `setup.sh` installs the pinned version (v0.1.0). Another one:
`./setup.sh --csorchestrator=<tag or branch>`; a local clone, editable: `./setup.sh --csorchestrator=../csorchestrator`
(Windows: `.\setup.ps1 -CsOrchestrator <...>`).

Run the recipe locally, regenerate the GitHub workflow after changing it:

```bash
python csImgTracker_project.py
python csImgTracker_project.py generate-github-workflow
```

Generated files, never edited by hand:

| Path | Made by | |
|---|---|---|
| `.github/workflows/csImgTracker.yml` | `csImgTracker_project.py generate-github-workflow` | Edit the recipe, then regenerate |
| `csorchestratorsdk/` | csorchestrator, at the same command | The release-manifest helpers the CI runs, without installing csorchestrator |
| `dependencies/` | csorchestrator, at the same command | The manifests and recipes of the releases this project builds on |
| `workspace/` *(git-ignored)* | a local run of `csImgTracker_project.py` | Sources, builds, installs, deploy archives; delete it to start clean |

Checks and tests:

```bash
pre-commit run --all-files
pytest                        # --run-all: every test, as CI; --run=<marker>: an opt-in group (tests/conftest.py)
```

The shared files (setup, checks, CI, agent rules) come from
[cs-project-template](https://github.com/cscosine/cs-project-template); to bring them to its latest version, on a
branch: `copier update --defaults`.
<!-- cs-template:end -->

## Download

The latest release, ready to run: extract the archive, then run `bin/csImgTracker` (`bin\csImgTracker.exe` on Windows).

### Default release

Built with MSVC 2022 (Windows), gcc (Ubuntu), Apple Clang with Ninja Multi-Config (macOS).

| Platform | Download |
|---|---|
| Windows 10 / 11 | [x64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-windows-v10-x64-generic-msvc-default-msvc2022-app-Release.zip) |
| Ubuntu 22.04 | [x64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu22.04-x64-generic-gcc-default-ninjamulticonfig-app-Release.tar.gz) · [arm64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu22.04-arm64-generic-gcc-default-ninja-app-Release.tar.gz) |
| Ubuntu 24.04 | [x64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu24.04-x64-generic-gcc-default-ninjamulticonfig-app-Release.tar.gz) · [arm64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu24.04-arm64-generic-gcc-default-ninja-app-Release.tar.gz) |
| Ubuntu 26.04 | [x64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu26.04-x64-generic-gcc-default-ninjamulticonfig-app-Release.tar.gz) · [arm64](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu26.04-arm64-generic-gcc-default-ninja-app-Release.tar.gz) |
| macOS 14.4 or newer | [Apple Silicon](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-macos-macos15-arm64-generic-appleclang-default-ninjamulticonfig-app-Release.tar.gz) |

Linux: the archive lists the system packages the application needs in `system-requirements.txt` (one `apt install`
line).

The builds are not signed yet, so Windows and macOS warn the first time:

- **Windows** (SmartScreen, "Windows protected your PC"): before extracting, right-click the `.zip` → Properties →
  *Unblock*; or, at the warning, *More info* → *Run anyway*.
- **macOS** (Gatekeeper): after extracting, in Terminal `xattr -dr com.apple.quarantine <extracted folder>`; or run it
  once, then System Settings → Privacy & Security → *Open Anyway*.

The deployment includes a sample video in `data/` (the open dialog starts there): an excerpt of
[Big Buck Bunny](https://peach.blender.org/) © copyright 2008, Blender Foundation, licensed under
[CC BY 3.0](https://creativecommons.org/licenses/by/3.0/); credits and changes in
[datasets/CREDITS.md](datasets/CREDITS.md).

### Alternative versions

The same application built with the other compilers and generators of CI; the builds of a platform are
equivalent.

| Windows x64 | MSVC | clang-cl |
|---|---|---|
| Visual Studio 2022 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-windows-v10-x64-generic-msvc-default-msvc2022-app-Release.zip) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-windows-v10-x64-generic-msvcclang-default-msvc2022-app-Release.zip) |
| Visual Studio 2026 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-windows-v10-x64-generic-msvc-default-msvc2026-app-Release.zip) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-windows-v10-x64-generic-msvcclang-default-msvc2026-app-Release.zip) |

| Ubuntu | gcc | clang |
|---|---|---|
| 22.04 x64 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu22.04-x64-generic-gcc-default-ninjamulticonfig-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu22.04-x64-generic-clang-default-ninjamulticonfig-app-Release.tar.gz) |
| 22.04 arm64 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu22.04-arm64-generic-gcc-default-ninja-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu22.04-arm64-generic-clang-default-ninja-app-Release.tar.gz) |
| 24.04 x64 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu24.04-x64-generic-gcc-default-ninjamulticonfig-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu24.04-x64-generic-clang-default-ninjamulticonfig-app-Release.tar.gz) |
| 24.04 arm64 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu24.04-arm64-generic-gcc-default-ninja-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu24.04-arm64-generic-clang-default-ninja-app-Release.tar.gz) |
| 26.04 x64 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu26.04-x64-generic-gcc-default-ninjamulticonfig-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu26.04-x64-generic-clang-default-ninjamulticonfig-app-Release.tar.gz) |
| 26.04 arm64 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu26.04-arm64-generic-gcc-default-ninja-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-linux-ubuntu26.04-arm64-generic-clang-default-ninja-app-Release.tar.gz) |

| macOS arm64 (runs from 14.4) | Ninja Multi-Config | Xcode |
|---|---|---|
| built on macOS 15 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-macos-macos15-arm64-generic-appleclang-default-ninjamulticonfig-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-macos-macos15-arm64-generic-appleclang-default-xcode-app-Release.tar.gz) |
| built on macOS 26 | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-macos-macos26-arm64-generic-appleclang-default-ninjamulticonfig-app-Release.tar.gz) | [⬇](https://github.com/cscosine/csImgTracker/releases/latest/download/csImgTracker-0.2.1-csv1-macos-macos26-arm64-generic-appleclang-default-xcode-app-Release.tar.gz) |

## Deployment

The install (`cmake --workflow ...`, or `cmake --build --preset install-...`) also makes a deployment of the
application (`cs_deployment` in `csImgTracker/CMakeLists.txt`, csCMake): a folder that runs on a machine without the
build, in `workspace/deploy/<variant>/csImgTracker/app-Release/`, with the shared libraries of OpenCV and Qt and the Qt
plugins it needs, the sample video (`data/`) and the licenses (`licenses/`, see [Licenses](#licenses)). On Linux,
`system-requirements.txt` in it lists what the machine needs from the system (the `apt install` line). CI archives it
and attaches it to the releases.

## Smoke run

```bash
<deployment>/bin/csImgTracker --smoke
```

Starts the application and, after 2 s, checks that every OpenGL view got its OpenGL context: prints
`smoke: ok, platform <platform>, <n> OpenGL view(s), OpenGL <version>` and ends with exit code 0, else exit code 1. The
process ends right after the check (`std::_Exit`, the output flushed), without the usual teardown: nothing is left to
check, and the teardown can hang with the software OpenGL of Mesa on Windows (its threads, at the unload of its DLL).

CI runs it on the deployment extracted from its archive, with a clean environment (`StepRunDeployment`,
csorchestrator): on Linux under `xvfb-run` (OpenGL from Mesa), on Windows with the software OpenGL of Mesa installed on
the runner as `opengl32sw.dll` and `QT_OPENGL=software` (the runners have no GPU; a real machine gets OpenGL from its
GPU driver). Without OpenGL (e.g. `QT_QPA_PLATFORM=offscreen` alone), csImgTracker has no OpenGL context and crashes.


## Licenses

csImgTracker is released under the [MIT License](LICENSE). Every deployment has a `licenses/` folder (from
[licenses/](licenses/), plus the licenses of the third-party code of OpenCV from its package) with the notices and the
license texts of what it contains; [licenses/THIRD-PARTY-NOTICES.md](licenses/THIRD-PARTY-NOTICES.md) is the list. The
About dialog of the application (*About csImgTracker*, at the bottom of the side panel) shows it too, with the version
of Qt running and the *licenses* folder.

| Component | License | In the deployment |
|---|---|---|
| **Qt 6.12** ([source](https://download.qt.io/official_releases/qt/6.12/), built unmodified by [csqt6](https://github.com/cscosine/csqt6) without the GPL-only modules) | [LGPL v3](https://www.gnu.org/licenses/lgpl-3.0.html) | separate shared libraries and plugins, which can be replaced: [Replacing the Qt libraries](#replacing-the-qt-libraries) |
| **OpenCV 5** ([fork](https://github.com/cscosine/opencv), built by [csopencvcpu](https://github.com/cscosine/csopencvcpu)) | [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0); its third-party code (on Windows also libjpeg-turbo: this software is based in part on the work of the Independent JPEG Group; libpng, libtiff, OpenJPEG, libwebp, zlib; Intel IPP for x64) under their licenses | shared libraries |
| **libassert**, **cpptrace** | MIT | in the executable |
| **libdwarf** (Linux, macOS; with cpptrace) | [LGPL v2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html): linked statically, the source of csImgTracker (this repository) lets you rebuild it with a modified libdwarf | in the executable |
| **zstd** (Linux, macOS; with libdwarf) | BSD-3-Clause | in the executable |
| **Eigen** ([source](https://gitlab.com/libeigen/eigen)) | [MPL 2.0](https://www.mozilla.org/MPL/2.0/), unmodified, header-only | in the executable |
| The libraries of [csfoundation](https://github.com/cscosine/csfoundation) | MIT | in the executable |
| Visual C++ runtime (Windows) | Microsoft Visual Studio redistribution terms | shared libraries |
| **Big Buck Bunny** (the sample video in `data/`) | [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/), © Blender Foundation; credits and changes in [datasets/CREDITS.md](datasets/CREDITS.md) | `data/` |

On Linux the deployment also needs libraries of the system, not shipped (`system-requirements.txt`), among them FFmpeg,
used by OpenCV to read the videos; on Windows the videos are read by Media Foundation, part of the system.

The notices of the third-party code inside Qt ([licenses/Qt-third-party-notices.md](licenses/Qt-third-party-notices.md))
are generated from the SBOM of the Qt packages of each platform by
[licenses/generate_qt_notices.py](licenses/generate_qt_notices.py) (Qt bundles different libraries per platform). When a
dependency changes (a new release of Qt, OpenCV, ...), update `licenses/` and the About dialog (`AboutDialog.cpp`).

## Replacing the Qt libraries

Qt is used under the LGPL v3: you may replace the Qt libraries of a deployment with other builds of Qt, e.g. a modified
one. The deployment ships Qt 6.12.0 as separate shared libraries and plugins:

| Platform | Qt libraries | Qt plugins | How the application finds them |
|---|---|---|---|
| Linux | `lib/libQt6*.so.6*` | `plugins/` (`platforms/`, `xcbglintegrations/`, `wayland-*/`) | the RUNPATH of the executable (`$ORIGIN/../lib`); Qt looks for its plugins relative to `libQt6Core` (`../plugins`) |
| Windows | `bin/Qt6*.dll` | `bin/platforms/`, `bin/styles/` | next to `csImgTracker.exe` |
| macOS | `lib/Qt*.framework` (the frameworks, without their headers) | `plugins/` | as on Linux: the RPATH (`@loader_path`) finds `lib/` |

The replacement must be:

- **Qt 6.12.0 or a later Qt 6 release**: Qt keeps binary compatibility within Qt 6, a newer Qt runs an application
  built with an older one.
- **Built for the same platform**: on Windows with MSVC 2022 or later, x64; on Linux for the same or an older Ubuntu
  (glibc), x64 or arm64 as the deployment; on macOS arm64, macOS 14.4 or later.
- **Complete**: every Qt library and plugin of the deployment from the same Qt build (the plugins must match the
  libraries), with the modules Widgets, OpenGL, OpenGLWidgets (and on Linux the xcb and Wayland platform plugins).

Then copy the libraries and the plugins over the ones of the deployment. The About dialog shows the version of Qt
running. On macOS the replaced files break the code signature of the application: sign it again, e.g. ad hoc on your
machine, `codesign --force --deep --sign - <deployment>`.

To build such a Qt: [csqt6](https://github.com/cscosine/csqt6) builds the Qt of the deployments, its README lists the
source, the modules and the configure options ("Rebuilding Qt"); or use the official Qt installers. To rebuild
csImgTracker itself against another Qt (or with modified libraries linked into the executable), build it from this
repository (`./csImgTracker_project.py`).

## Code signing

The deployments of the releases are not signed: Windows (SmartScreen) and macOS (Gatekeeper) warn the first time they
run (see [Download](#download) for what to do). Signing them, in CI when the release is made (csorchestrator), needs:

- **macOS**: the Apple Developer Program (US$99 a year): a *Developer ID Application* certificate; every binary signed
  (`codesign --options runtime --timestamp`: the executable, the dylibs, the Qt plugins), then notarized
  (`xcrun notarytool submit --wait`, an App Store Connect API key). The notarization ticket can be stapled to an `.app`,
  a `.dmg` or a `.pkg`, not to a `.tar.gz` of plain binaries: the deployment as an `.app` in a `.dmg` gives the
  smoothest first run.
- **Windows**: an Authenticode signature (`signtool`, or the GitHub Action of the provider), from one of:
  - Azure Artifact Signing (formerly Trusted Signing, about US$10 a month): individuals in the US or Canada only;
    organizations also in the EU, the UK and other countries;
  - SignPath Foundation (free for open source: an OSI licence, public repositories, builds on GitHub Actions; the
    certificate is issued to SignPath Foundation, the publisher shown);
  - an OV certificate (about US$200–400 a year, the key in a cloud HSM); a new signed executable may still get a
    SmartScreen warning until it has a download reputation.

The secrets (certificates, API keys) go in the GitHub secrets of the organization; the plan is in the TODO of
csorchestrator (signing and notarization of the release archives).
