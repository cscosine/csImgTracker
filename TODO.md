# TODO: Verify csImgTracker bundled library resolution

## What we spotted

The `csImgTracker` bundle contains shared libraries with absolute RUNPATH entries pointing to build environments that do not exist on the target machine.

In particular:
- Bundled OpenCV libraries reference an absolute CI installation path, preventing reliable resolution of sibling OpenCV dependencies.
- The application executable contains absolute paths from the developer's workspace, making runtime behavior dependent on the original build location.
- Missing library dependencies can currently be worked around by setting `LD_LIBRARY_PATH` manually.

The underlying library-path issues should be addressed in the respective OpenCV and `csVisOpenGL` builds.

## Verification

After fixing and rebuilding those dependencies, verify the application without `LD_LIBRARY_PATH`:

```bash
env -u LD_LIBRARY_PATH ldd path/to/csImgTracker-executable \
    | grep 'not found'
```

Inspect the executable's embedded paths:

```bash
readelf -d path/to/csImgTracker-executable \
    | grep -E 'RPATH|RUNPATH'
```

Launch the application with `LD_LIBRARY_PATH` unset and verify that it starts successfully.

## Acceptance criteria

- No unresolved shared-library dependencies.
- No dependency on absolute developer-workspace or CI paths.
- Application launches successfully without manually setting `LD_LIBRARY_PATH`.
