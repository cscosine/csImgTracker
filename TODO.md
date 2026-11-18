# TODO

## Windows: a GUI application in Release, at csCMake level

`csImgTracker.exe` is a GUI application in Release (no console window next to it, `WIN32_EXECUTABLE
$<CONFIG:Release>` in `csImgTracker/CMakeLists.txt`) and a console application in the other configurations. The
deployments and the release archives are Release; the smoke run in CI is unaffected (started by Python
`subprocess.run`, it waits for the process and reads its output through the inherited pipes).

- Move it to csCMake: a `GUI` keyword of `cs_add_executable` (Release only) instead of the line in every project; then
  the same for skiTrackerApp and the csVisOpenGL sample.
- `--smoke` started by hand from a terminal on a Release build prints nothing (no console attached) and the terminal
  does not wait for a GUI application: if needed, the application attaches to the console of its parent
  (`AttachConsole(ATTACH_PARENT_PROCESS)` at start, when its standard output is not redirected).

Alternatives considered (2026-11-17):

- A console application with `consoleAllocationPolicy` `detached` in its manifest (Windows 11 24H2+: no console on
  double-click, the terminal waits; older Windows: the console as today), the application attaching to the console of
  its parent; on older Windows it can release a console created only for it (a brief flash).
- A `--console` option (a console window for the logs on demand), and logs to a file (a Qt message handler), whatever
  the choice above.
