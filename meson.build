project('nogravity', 'c',
  version : '2.00',
  license : 'GPL-2.0-or-later',
  meson_version : '>= 1.0.0',
  default_options : ['c_std=gnu17,c17', 'warning_level=1', 'buildtype=debugoptimized'])

cc = meson.get_compiler('c')

sdl_dep    = dependency('sdl3')
gl_dep     = dependency('gl')
png_dep    = dependency('libpng')
zlib_dep   = dependency('zlib')
sound_dep  = dependency('sdl3-sound')
m_dep      = cc.find_library('m', required : false)
thread_dep = dependency('threads')

deps = [sdl_dep, sound_dep, gl_dep, png_dep, zlib_dep, m_dep, thread_dep]

datadir = get_option('nogravity_datadir')
if datadir == ''
  datadir = get_option('prefix') / get_option('datadir') / 'nogravity'
endif

cflags = ['-DNOGRAVITY_DATADIR="@0@"'.format(datadir)]

if get_option('debug_mode')
  cflags += '-D_DEBUG'
endif

incdirs = include_directories(
  'rlx32/include',
  'rlx32/src',
  'rlx32/src/sdl',
  'rlx32/src/renderers/opengl')

engine_src = files(
  'rlx32/src/gui_os.c',
  'rlx32/src/gx_alloc.c',
  'rlx32/src/gx_csp.c',
  'rlx32/src/gx_flc.c',
  'rlx32/src/gx_form.c',
  'rlx32/src/gx_init.c',
  'rlx32/src/gx_png.c',
  'rlx32/src/gx_rgb.c',
  'rlx32/src/gx_tool.c',
  'rlx32/src/sysgi.c',
  'rlx32/src/sysini.c',
  'rlx32/src/syslist.c',
  'rlx32/src/sysresmx.c',
  'rlx32/src/systools.c',
  'rlx32/src/syszlib.c',
  'rlx32/src/v3x_1.c',
  'rlx32/src/v3x_2.c',
  'rlx32/src/v3x_3.c',
  'rlx32/src/v3xcoll.c',
  'rlx32/src/v3xdata.c',
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
  'rlx32/src/sdl/sysnet.c',
  'rlx32/src/sdl/sysreg.c',
  'rlx32/src/sdl/systime.c',
  'rlx32/src/sdl/systool2.c',
)

renderer_src = files(
  'rlx32/src/renderers/opengl/gl_gx.c',
  'rlx32/src/renderers/opengl/gl_v3x.c',
  'rlx32/src/renderers/opengl/sdl_gl.c',
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
  'src/lt_reso.c',
  'src/lt_save.c',
  'src/lt_sound.c',
  'src/lt_st2.c',
  'src/lt_start.c',
  'src/lt_video.c',
  'src/lt_weap.c',
  'src/sdl/preferences.c',
)

executable('nogravity',
  engine_src + platform_src + renderer_src + game_src,
  include_directories : incdirs,
  c_args : cflags,
  dependencies : deps,
  install : true)
