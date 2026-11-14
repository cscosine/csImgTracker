# csimgtracker

## Deployment

The install (`cmake --workflow ...`, or `cmake --build --preset install-...`) also makes a deployment of the
application (`cs_deployment` in `csImgTracker/CMakeLists.txt`, csCMake): a folder that runs on a machine without the
build, in `workspace/deploy/<variant>/csImgTracker/app-Release/`, with the shared libraries of OpenCV and Qt and the Qt
plugins it needs. On Linux, `system-requirements.txt` in it lists what the machine needs from the system (the
`apt install` line). CI archives it and attaches it to the releases.

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
