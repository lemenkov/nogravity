# SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later
project('nogravity', 'c',
  version : '3.0.0',
  license : 'GPL-2.0-or-later',
  meson_version : '>= 1.0.0',
  default_options : ['c_std=gnu17,c17', 'warning_level=1', 'buildtype=debugoptimized'])

cc = meson.get_compiler('c')

sdl_dep    = dependency('sdl3')
png_dep    = dependency('libpng')
zlib_dep   = dependency('zlib')
sound_dep  = dependency('sdl3-sound')
m_dep      = cc.find_library('m', required : false)
thread_dep = dependency('threads')

deps = [sdl_dep, sound_dep, png_dep, zlib_dep, m_dep, thread_dep]

datadir = get_option('nogravity_datadir')
if datadir == ''
  datadir = get_option('prefix') / get_option('datadir') / 'nogravity'
endif

cflags = ['-DNOGRAVITY_DATADIR="@0@"'.format(datadir),
          '-DNOGRAVITY_SOURCE_DATADIR="@0@"'.format(meson.project_source_root() / 'data')]

if get_option('debug_mode')
  cflags += '-D_DEBUG'
endif

incdirs = include_directories(
  'rlx32/include',
  'rlx32/src',
  'rlx32/src/sdl',
  'rlx32/src/renderers/gpu')

# Shaders: GLSL is compiled to SPIR-V for Vulkan with glslc; the Metal
# source is embedded as text (Metal compiles it at run time).
python3 = import('python').find_installation('python3')
embed = files('tools/embed.py')
shader_dir = 'rlx32/src/renderers/gpu/shaders'
shader_inputs = [shader_dir / 'nogravity.metal']
glslc = find_program('glslc', required : get_option('spirv'))
if glslc.found()
  foreach s : ['nogravity.vert', 'nogravity.frag']
    shader_inputs += custom_target(s + '.spv',
      input : shader_dir / s,
      output : s + '.spv',
      command : [glslc, '@INPUT@', '-o', '@OUTPUT@'])
  endforeach
  cflags += '-DHAVE_SPIRV'
endif
shaders_src = custom_target('gpu_shaders_data.c',
  input : shader_inputs,
  output : 'gpu_shaders_data.c',
  command : [python3, embed, '@OUTPUT@', '@INPUT@'])

engine_src = files(
  'rlx32/src/gui_os.c',
  'rlx32/src/gx_csp.c',
  'rlx32/src/gx_flc.c',
  'rlx32/src/gx_form.c',
  'rlx32/src/gx_init.c',
  'rlx32/src/gx_png.c',
  'rlx32/src/gx_rgb.c',
  'rlx32/src/gx_tool.c',
  'rlx32/src/sysini.c',
  'rlx32/src/sysresmx.c',
  'rlx32/src/systools.c',
  'rlx32/src/syszlib.c',
  'rlx32/src/v3x_1.c',
  'rlx32/src/v3x_2.c',
  'rlx32/src/v3x_3.c',
  'rlx32/src/v3xcoll.c',
  'rlx32/src/v3xmaps.c',
  'rlx32/src/v3xrend.c',
  'rlx32/src/v3xscen2.c',
  'rlx32/src/v3xscene.c',
  'rlx32/src/v3xsoft.c',
  'rlx32/src/_stub.c',
)

platform_src = files(
  'rlx32/src/sdl/_stubsdl.c',
  'rlx32/src/sdl/snd_sdl3.c',
  'rlx32/src/sdl/sysdbg.c',
  'rlx32/src/sdl/sysjoy.c',
  'rlx32/src/sdl/syskeyb.c',
  'rlx32/src/sdl/sysmouse.c',
  'rlx32/src/sdl/systime.c',
)

renderer_src = files(
  'rlx32/src/renderers/gpu/gpu_display.c',
  'rlx32/src/renderers/gpu/gpu_2d.c',
  'rlx32/src/renderers/gpu/gpu_3d.c',
)

game_src = files(
  'src/ccmx.c',
  'src/console.c',
  'src/lithium.c',
  'src/lt_data.c',
  'src/lt_fx.c',
  'src/lt_game.c',
  'src/lt_game2.c',
  'src/lt_info.c',
  'src/lt_input.c',
  'src/lt_menu.c',
  'src/lt_save.c',
  'src/lt_sound.c',
  'src/lt_st2.c',
  'src/lt_start.c',
  'src/lt_video.c',
  'src/lt_weap.c',
)

executable('nogravity',
  engine_src + platform_src + renderer_src + game_src + [shaders_src],
  include_directories : incdirs,
  c_args : cflags,
  dependencies : deps,
  install : true)

install_subdir('data', install_dir : get_option('datadir') / 'nogravity', strip_directory : true)
