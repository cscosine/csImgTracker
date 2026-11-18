# csImgTracker: licenses and third-party notices

**csImgTracker** — <https://github.com/cscosine/csImgTracker> — is released under the MIT License (`LICENSE` in this
folder). Its deployments contain, or are built with, the components below. This folder (`licenses/` of a deployment)
holds their notices and license texts (`texts/`); the About dialog of the application shows the same list.

| Component | Version | License | In the deployment | Notices |
|---|---|---|---|---|
| [Qt](https://www.qt.io) | 6.12.0 | LGPL-3.0-only | shared libraries and plugins | `Qt-third-party-notices.md`, `texts/LGPL-3.0-only.txt`, `texts/GPL-3.0-only.txt` |
| [OpenCV](https://opencv.org) | 5.0.0 | Apache-2.0 | shared libraries | `texts/Apache-2.0.txt`, `OpenCV/` (its third-party code) |
| [libassert](https://github.com/jeremy-rifkin/libassert) | 2.2.1 | MIT | in the executable | `libassert-LICENSE.txt` |
| [cpptrace](https://github.com/jeremy-rifkin/cpptrace) | 1.0.4 | MIT | in the executable | `cpptrace-LICENSE.txt` |
| [libdwarf](https://github.com/davea42/libdwarf-code) (Linux, macOS) | 2.1.0 | LGPL-2.1-only | in the executable (with cpptrace) | `libdwarf-COPYING.txt`, `libdwarf-LIBDWARFCOPYRIGHT.txt`, `texts/LGPL-2.1-only.txt` |
| [zstd](https://github.com/facebook/zstd) (Linux, macOS) | 1.5.7 | BSD-3-Clause | in the executable (with libdwarf) | `zstd-LICENSE.txt` |
| [Eigen](https://eigen.tuxfamily.org) | 5.0.1 | MPL-2.0 | in the executable (header-only) | `texts/MPL-2.0.txt` |
| [csfoundation](https://github.com/cscosine/csfoundation): csCore, csVisOpenGL | 0.1.0 | MIT | in the executable | `csfoundation-LICENSE.txt` |
| Visual C++ runtime (Windows) | MSVC | Microsoft Visual Studio redistribution terms | shared libraries | — |
| Big Buck Bunny (sample video) | — | CC BY 3.0, © Blender Foundation | `data/` | `data/CREDITS.md` |

On Linux the deployment also needs libraries of the system, not shipped (`system-requirements.txt`), among them FFmpeg,
used by OpenCV to read the videos; on Windows the videos are read by Media Foundation, part of the system.

## Qt (LGPL-3.0)

csImgTracker uses Qt under the GNU Lesser General Public License version 3 (`texts/LGPL-3.0-only.txt`, which
supplements the GNU General Public License version 3, `texts/GPL-3.0-only.txt`). Qt is shipped as separate shared
libraries and plugins, unmodified, so they can be replaced: see "Replacing the Qt libraries" in the
[README](https://github.com/cscosine/csImgTracker#replacing-the-qt-libraries).

- **Source**: Qt 6.12.0, the tag `v6.12.0` of <https://code.qt.io/qt/qt5.git> (also
  <https://download.qt.io/official_releases/qt/6.12/6.12.0/>), built without changes by
  [csqt6](https://github.com/cscosine/csqt6), whose README lists the modules and the configure options.
- **Third-party code inside Qt**: `Qt-third-party-notices.md`, generated from the SBOM of the Qt packages
  (`generate_qt_notices.py`).

## libdwarf (LGPL-2.1, Linux and macOS)

libdwarf is linked statically into the executable (by cpptrace, for the stack traces of libassert). The GNU Lesser
General Public License version 2.1 (`texts/LGPL-2.1-only.txt`) allows it with the source of the application: the source
of csImgTracker is at <https://github.com/cscosine/csImgTracker> (MIT), with the instructions to build it (README), so
it can be rebuilt and relinked with a modified libdwarf. The source of libdwarf 2.1.0:
<https://github.com/davea42/libdwarf-code/releases/tag/v2.1.0>.

## OpenCV (Apache-2.0)

OpenCV 5.0.0, from the fork <https://github.com/cscosine/opencv> built by
[csopencvcpu](https://github.com/cscosine/csopencvcpu), under the Apache License 2.0 (`texts/Apache-2.0.txt`).
`OpenCV/` holds the licenses of the third-party code it contains, as installed by OpenCV (on Windows also libjpeg-turbo:
this software is based in part on the work of the Independent JPEG Group; libpng, libtiff, OpenJPEG, libwebp, zlib).

## Eigen (MPL-2.0)

Eigen 5.0.1, unmodified, header-only, under the Mozilla Public License 2.0 (`texts/MPL-2.0.txt`); its source:
<https://gitlab.com/libeigen/eigen/-/tree/5.0.1>.
