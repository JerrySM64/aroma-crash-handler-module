# AromaCrashHandlerModule

A Wii U [Aroma](https://aroma.foryour.cafe) module (`.wms`) that catches application CPU
exceptions and shows a register / stack / code dump on screen, styled like the classic
TCPGecko / Geckiine crash screen.

## What it does

When an application crashes with a CPU exception (DSI, ISI, Program, ...), the module
shows a text screen on the TV and the GamePad with:

- the exception type,
- all 32 GPRs, LR, CTR, XER, SRR0, SRR1, DAR, DSISR,
- a short stack dump (return-address chain),
- a code dump (instruction words at the faulting PC).

The screen stays up (the system halts) until the console is powered off or rebooted. The
module does **not** fix or recover the crashed game, does not symbolicate, and adds no
work to normal frames.

## Building

Dependencies: [wut](https://github.com/devkitPro/wut) and
[WiiUModuleSystem](https://github.com/wiiu-env/WiiUModuleSystem) with the devkitPPC
toolchain. Pick one of the four routes below — all produce `CrashHandlerModule.wms`.

### Using the Dockerfile (recommended, works everywhere)

Nothing aside from Docker needs to be installed on the host:

```sh
docker build . -t crashhandlermodule-builder
docker run -it --rm -v ${PWD}:/project crashhandlermodule-builder make

# make clean works the same way
docker run -it --rm -v ${PWD}:/project crashhandlermodule-builder make clean
```

On Windows, `${PWD}` already works in PowerShell; in cmd.exe use `%cd%`, in Git Bash
use `$(pwd -W)`.

### Using Nix (NixOS or any system with Nix)

```sh
nix develop -c make
```

The flake pins devkitPPC, wut and WiiUModuleSystem by digest — nothing else to install.

### Native on Linux

1. Set up devkitPro's pacman and install the Wii U toolchain (provides devkitPPC and
   wut) — see the [devkitPro Getting Started guide](https://devkitpro.org/wiki/Getting_Started)
   for how to add the repositories on your distribution:

   ```sh
   sudo (dkp-)pacman -Syu --needed wiiu-dev
   ```

2. Set the environment variables (the devkitPro installer usually does this for you
   via `/etc/profile.d/devkit-env.sh`):

   ```sh
   export DEVKITPRO=/opt/devkitpro
   export DEVKITPPC=$DEVKITPRO/devkitPPC
   ```

3. WiiUModuleSystem is not available through the devkitPro repositories; build and
   install it into the devkitPro prefix:

   ```sh
   git clone https://github.com/wiiu-env/WiiUModuleSystem
   make -C WiiUModuleSystem install # installs into $DEVKITPRO/wums
   ```

4. Build:

   ```sh
   make
   ```

### Native on Windows (MSYS2)

Use the **MSYS2 UCRT64** environment.
Either run the [devkitPro graphical installer](https://github.com/devkitPro/installer/releases/latest)
(which sets up a matching MSYS2), or add the devkitPro repositories to an existing
MSYS2 following the [devkitPro pacman guide](https://devkitpro.org/wiki/devkitPro_pacman).

1. Open the **MSYS2 UCRT64** shell from the start menu.
2. Install the toolchain and build tools (package names match the Linux instructions):

   ```sh
   pacman -Syu --needed wiiu-dev make
   ```

3. Set the same environment variables as on Linux (inside MSYS2 the paths are `/opt/devkitpro`):

   ```sh
   export DEVKITPRO=/opt/devkitpro
   export DEVKITPPC=$DEVKITPRO/devkitPPC
   ```

4. Install WiiUModuleSystem as in the Linux instructions above (same `make -C ... install`, same `$DEVKITPRO/wums` target) and run `make`.

## Installing

Copy the built `CrashHandlerModule.wms` to the SD card:

```
sd:/wiiu/environments/[ENVIRONMENT]/modules
```

(`[ENVIRONMENT]` is a placeholder for the environment name you use, e.g. `aroma`.)
The module is loaded at boot by the [WUMSLoader](https://github.com/wiiu-env/WUMSLoader).

## Logging

- `make` logs errors only (via `OSReport`).
- `make DEBUG=1` enables information and error logging via the
  [LoggingModule](https://github.com/wiiu-env/LoggingModule).
- `make DEBUG=VERBOSE` enables verbose logging the same way.

If the LoggingModule is not present, the UDP fallback (port 4405) is used as it's used by other wiiu-env projects.

## Formatting

Match the code style of the other wiiu-env projects:

```sh
docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./source -i
```

## License

[GPL-3.0](LICENSE)
