<!--
SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Modernization plan and future ideas

This document describes where the 3.0 modernization of No Gravity stands,
the principles behind it, and what could come next. The changelog has the
detailed list of changes.

## Goals

- One modern, cross-platform stack with as few dependencies as possible:
  SDL3 for window, input, audio and GPU, SDL3_sound for decoding, libpng
  for the art. No per-platform code in the game or the engine.
- Plain C17, not a mixed C and C++ project.
- The game data as plain files in the repository, readable and editable
  without tools, under the same license as the code.
- Keep the game playable after every step, one logical change per commit,
  and keep the history of every file.
- Keep the engine (`rlx32`) and the game (`src`) separate: `rlx32/include`
  is the interface the game may use, `rlx32/src` the engine and its private
  headers.

## Where it stands

Done, as of September 2026:

- Meson build; SDL3 platform layer; SDL3 audio and SDL3_sound; SDL_GPU
  renderer with SPIR-V and Metal shaders.
- Game data imported and read from a directory tree; scenes stored plain.
- Demo replay, add-on packs, the DirectPlay networking and every dead
  platform, renderer and colour depth removed.
- The engine cut down to what the SDL_GPU path uses: about 14,000 lines of
  engine and game sources instead of about 65,000.
- REUSE compliant tree, version 3.0.0.

Verified on Linux with Vulkan. Windows and macOS are supported by the same
sources through SDL3 but have not been built yet; that is the first item
below.

## Plan

In order of priority.

1. **Build on Windows and macOS.** Nothing platform specific remains, but
   the Metal shader path and the Direct3D 12 path have only been reasoned
   about. A GitHub Actions workflow building all three platforms would
   keep it that way.
2. **Play-test combat.** The automated runs cover menus, mission loading
   and free flight. Weapons, explosions, shields, the warp effect and the
   end-of-mission screens deserve a human play-through on the new
   renderer.
3. **Fedora package.** Update the spec to Meson, SDL3-devel,
   SDL3_sound-devel, libpng-devel and glslc; drop the separate data
   package, the wrapper script and the patches, which are all upstream
   now.
4. **Tag 3.0.0** once the above is done, with the changelog as release
   notes.
5. **Settings clean-up.** The settings file still carries options that do
   nothing on the GPU renderer (dithering, texture power-of-two, the
   detail level for the sky). Remove them from the file and the options
   menu, or make them do something.
6. **Material system.** The material struct still mirrors the 1997 scene
   file layout for its first 56 bytes and keeps a near/far property pair
   for the level-of-detail switch. A loader that fills a lean in-memory
   struct from the file layout would finish the separation between file
   format and engine.

## Future ideas

Not planned, listed so they are not forgotten.

- **Multiplayer on SDL3_net.** The original DirectPlay code was removed
  rather than ported. The game logic still has the hooks (the chat key,
  the player structures) for a fresh implementation.
- **Gamepads through SDL_Gamepad.** Joysticks are read through the raw
  joystick API with user-configured axes. SDL_Gamepad would give
  standardized mappings for modern controllers out of the box.
- **Higher resolution art.** The renderer already draws at native
  resolution and scales the 640x480 (or 768x480 wide) HUD layout. The
  256x256 8-bit textures and the palette-based sprites are the limit now;
  the PNG loader accepts true colour, so replacement art needs no code.
- **Data-driven episodes.** Missions are scripts (`data/cmx`) with scenes
  and art per episode directory. A manifest per episode would let new
  campaigns be added as directories without touching the source.
- **Text and localization.** The mission and menu text lives in the
  script files in five languages, in a Latin-1 encoding with a custom
  bitmap font. Moving to UTF-8 and a TrueType font through SDL3_ttf would
  open the door to more languages.
- **Save game format.** Pilots and high scores are written as raw C
  structs. A text format would survive struct changes and be portable
  between architectures.
- **Level editor and tools.** The scene format is documented only by the
  loader. A small Python reader in `tools/`, like the archive extractor,
  would make the 57 scenes inspectable and editable.
