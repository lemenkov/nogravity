<!--
SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Changelog

Versions are listed newest first. Within a version the changes are listed
in the order they were made, oldest first.

## 3.0.0 (unreleased, September 2026)

A modernization of the 2006 code base on top of SDL3, done by Peter
Lemenkov with the Claude Fable 5.1 model as co-author. The game logic,
the scene format and the art are untouched; everything between them and
the operating system was replaced or removed.

### Build and platform

- Added a Meson build (`meson setup build && ninja -C build`).
- Removed the Windows, BeOS, macOS, AmigaOS and DOS platform layers, the
  autotools, Visual C, XCode and BeIDE build files, and the software,
  Direct3D, Glide and per-platform OpenGL renderers.
- Ported the remaining platform layer to SDL3: window, keyboard with
  layout-independent scancodes, mouse, joystick and timers.
- Converted the last C++ files to C. The project is plain C17.
- Replaced the in-house mixer, the OpenAL and the SDL_mixer sound drivers
  with SDL3 audio streams and SDL3_sound for decoding.
- Locked the mouse only in a focused fullscreen window; the pointer stays
  free in a window so it can reach the window controls.
- Honoured the selected resolution in windowed mode.
- Replaced the OpenGL renderer with an SDL_GPU renderer (Vulkan, Metal,
  Direct3D 12 through SDL). Shaders are GLSL compiled to SPIR-V at build
  time, with the Metal source embedded for macOS.
- Added `NOGRAVITY_TEST_KEYS`, `NOGRAVITY_SCREENSHOT`, `NOGRAVITY_STAGE`,
  `NOGRAVITY_GPU_DEBUG` and `NOGRAVITY_TRACE_FILES` for automated runs.

### Data

- Imported the GPL game data from `NOGRAVITY.RMX` into the repository as
  plain files under `data/`, extracted by `tools/rmx_extract.py`; the
  game reads a directory tree instead of the archive.
- Stored the 57 scene files uncompressed and dropped zlib.
- Repaired the scene files' gzip trailers along the way (the 2005 writer
  stored a wrong CRC), before they were decompressed for good.

### Removed features

- Demo recording and replay.
- Add-on resource packs.
- The DirectPlay-only networking and multiplayer code. Multiplayer is to
  be redone on SDL3_net.
- Colour depths other than 32-bit, and the 8-bit palette rendering paths.

### Fixes

- Serialized music stream access between the audio thread and the game.
- Fixed the arena allocator's realloc copying overlapping ranges.
- The Z-buffer setting is honoured; it used to be overwritten by a
  capability bit that did not fit in a byte.
- Screenshots are written without the meaningless alpha channel.

### Code cleanup

- Bumped the version to 3.0.0, normalized line endings and whitespace,
  and made the tree REUSE compliant with SPDX headers and a `LICENSES`
  directory.
- Replaced the string, memory, byte order, random, thread, mutex and
  clock wrappers with SDL3 and C library calls.
- Collapsed the run time configuration record to the fields still read.
- Removed code nothing reaches, made file-local globals static, and
  trimmed the driver interface tables to the slots that are called.
- Folded the renderer capability flags that never change with one
  renderer, and the pixel depth branches with one colour depth.
- Removed the pass-through layers around file I/O, startup and colour
  conversion, unused macros, typedefs, enum constants and struct fields.
- Dropped the software render classes from the material system.
- Flattened the engine tree to `rlx32/include` and `rlx32/src`, merged
  the base headers into `rlx32.h` and the startup files into `main.c`,
  and removed the DLL export and calling convention decorations.
- Replaced the debug output, the frame timer, the key name table and the
  settings parser's string handling with SDL3 and C library equivalents.

The engine and game sources went from about 65,000 lines to about 14,000.
Run time dependencies are SDL3, SDL3_sound and libpng; build time
dependencies are Meson, Ninja, a C17 compiler and glslc.

## 2.0 as packaged by Fedora (2006 to 2026)

Patches carried by the Fedora package, applied here as commits with their
original authorship.

- Fixed audio on PowerPC (Peter Lemenkov, 2006).
- Added a settings file option to enable or disable fullscreen mode
  (Peter Lemenkov, 2006).
- Added the proper path to the resource file (Peter Lemenkov, 2006).
- Added `GL_GLEXT_PROTOTYPES` (realtech, 2006).
- Fixed the joystick configuration key (realtech, D. Longbottom, 2006).
- Implemented `StreamSetVolume` (Matt Williams, 2006).
- Made the game work on 64-bit machines (Hans de Goede, 2007).
- Fixed the OpenAL sound effects volume control and other OpenAL issues
  (Hans de Goede, 2007).
- Noted in the README that the data is GPL licensed too (Hans de Goede,
  2007).
- Fixed several wrong memory uses and unterminated strings (Hans de
  Goede, 2007).
- Fixed a crash on startup on 64-bit Intel CPUs (Hans de Goede, 2009).
- Fixed the crash reported in rhbz#699274 (Hans de Goede, 2011).
- Fixed building with libpng 1.5 while keeping libpng 1.2 working (Hans
  de Goede, 2011).
- Removed unused scene code and the `MaxExtentableObjet` field (Hans de
  Goede, 2012).
- Fixed build failures with newer compilers and libraries (Hans de Goede,
  2015, 2016, 2024 and 2026; rhbz#1307807, rhbz#2261407, rhbz#2340938,
  rhbz#2385217).

## 2.0 (2006-02-15)

- New input controls for joystick, mouse and keyboard, bug fixes in the
  input configuration, and configuration of the third and fourth joystick
  axes.
- Universal binaries for macOS 10.4 on x86 and 10.2 on PowerPC (XCode 2.2
  project).

## 1.98 (2005)

Development releases r6 to r9 on SourceForge after the code went GPL.

- Release 6: BeOS bug fixes, the Linux/SDL port, a BWindowScreen driver
  and a new mouse driver; the Zeta project and unused files removed.
- Video mode selection, a fix for a memory access violation, and a fix
  for windowed mode.
- OpenAL sound support, SDL joystick fixes, and an SDL_mixer sound driver.
- autoconf/automake build, RPM packaging, later autopackage.
- Release 8: the OpenGL initialisation on Win32 rewritten, multisampling
  support, crashes in the missions "Missile" and "Space wars" fixed,
  OpenAL as the default on macOS.
- Release 9: screen resolution independence, a smaller sound buffer, a
  non-power-of-two texture option, trilinear filtering no longer forced,
  a texture filter fix, HID mouse support on macOS, a mouse fix for the
  pointer leaving the viewport, OpenGL 1.1 compatibility checks.
- Software renderer support, including a Linux software renderer.
- GCC 4.0 compatibility, macOS 10.4 OpenGL support, XCode 2.0 project.
- Joystick calibration menu for BeOS and macOS, calibration of the third
  and fourth axes.
- C++ dependencies removed; the code compiles as C.
- SDL threads instead of pthreads; endianness fixes for the software
  renderer; big-endian support for the Intel Macs; AmigaOS 4 merge.

## 1.9 (2005-02-24)

- First public release of the source code under the GNU General Public
  License, version 2 or later, on SourceForge. The game data followed
  under the same license.
