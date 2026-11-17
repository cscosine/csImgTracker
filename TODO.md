# TODO

## Windows: no console window for the users (to evaluate)

Today `csImgTracker.exe` is a console application: started from the deployment (double-click), Windows opens a console
window next to the application.

Preferred (to evaluate): a GUI application in **Release only**, a console application in the other configurations:

```cmake
set_target_properties(csImgTracker PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
```

- The deployments and the release archives are Release: what the users download has no console; Debug /
  RelWithDebInfo keep the console in development. No switch in CI: the configuration decides.
- Qt adds the entry point (`Qt6::EntryPoint`: `WinMain` calls `main()`), no code change.
- To check: the smoke run in CI (`--smoke`: the output through the pipes of CI, the exit code to the parent);
  `--smoke` started by hand from a terminal on a Release build prints nothing unless the application attaches to the
  console of its parent (`AttachConsole(ATTACH_PARENT_PROCESS)` at start, a few lines), and the terminal does not wait
  for a GUI application.
- If it fits, the same for skiTrackerApp and the csVisOpenGL sample; then a `GUI` keyword of csCMake
  `cs_add_executable` instead of the line in every project.

Alternatives considered (2026-11-17):

- A console application with `consoleAllocationPolicy` `detached` in its manifest (Windows 11 24H2+: no console on
  double-click, the terminal waits; older Windows: the console as today), the application attaching to the console of
  its parent; on older Windows it can release a console created only for it (a brief flash).
- A `--console` option (a console window for the logs on demand), and logs to a file (a Qt message handler), whatever
  the choice above.
