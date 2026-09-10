===================================================================
No Gravity 3.0.0 - Source Code and Data Release
===================================================================

 ===========================================================
1. LEGAL STUFF
-----------

"No Gravity" is a registered trademark of RealTech VR, Ltd.

"No Gravity" copyright 1996 - 2005 RealTech. All trademarks and copyrights reserved.

This is the complete source code and data for No Gravity 3.0.0, the community edition of the 2005 GPL release, buildable as detailed in the next section.

The code and data are licensed under the terms of the GPL (gnu public license).  You should read the entire license (filename "GNU.TXT" in this archive), so you understand your rights and what you can and cannot do with the source code from this release.

Please note that this is being released without any kind of support from RealTech.

 We cannot help in getting this running and we do not guarantee that you will be able to get it to work, nor do we guarantee that it won't blow up your computer if you do try and use it.  Use at your own risk.


 ===========================================================
2. Installation


Building from source (Linux, macOS, Windows):

The game is plain C17 and builds with Meson. It needs SDL3, SDL3_sound,
libpng and zlib, plus glslc (from shaderc) to compile the shaders for
the Vulkan backend; on macOS the Metal shader is compiled at run time.

  meson setup build
  ninja -C build
  ./build/nogravity

The game data lives in the data/ directory of the source tree (it was
extracted from the original NOGRAVITY.RMX archive with
tools/rmx_extract.py) and is installed to <prefix>/share/nogravity.
At run time the game looks for it in $NOGRAVITY_DATA, then in the
install location, then in the source tree and finally in ./data.
Settings are kept in the per-user SDL preferences directory (for
example ~/.local/share/realtech/nogravity on Linux).

Environment variables useful for development:

  NOGRAVITY_SCREENSHOT=file.png     write frame NOGRAVITY_SCREENSHOT_FRAME
                                    (default 120) to file.png; a %d in the
                                    name writes every that many frames
  NOGRAVITY_TEST_KEYS=ms:key,...    press keys at the given times, e.g.
                                    "3000:Return,8000:Down" (SDL key names)
  NOGRAVITY_GPU_DEBUG=1             enable the SDL_GPU validation layers
  NOGRAVITY_TRACE_FILES=1           report data files that fail to open




 ===========================================================
4.  Credits

Programming : Stephane Denis
Graphics : Romuald Genevois
Music: Philippe Meisburger
Linux / SDL Port : Matt Williams
Amiga OS4 Port : Juha Niemimaki


 ===========================================================
5.  Project page
http://www.realtech-vr.com/nogravity
http://sourceforge.net/projects/nogravity


 ===========================================================
6. DEPENDENCIES
  OGG / Vorbis
  zLib
  LibPNG
  SDL3
  SDL3_sound
  libpng
  zlib


 ===========================================================
 7. General Gameplay Information
  
   7.1 Keyboard Controls

     Help can be available during the game by pressing F1

     7.1.1  Control 

      L                 : Lock Target
      Tab               : AfterBurner, give a boost toward
      BackSpace         : RetroBooster, give a boost backward
      Space             : Use selected item, use - and = near the backspace key
      A                 : Go to next NAV point ( Q on french keyboard ) when
                          requested. It will warp one on the next battle theatre
      0 and .           : Rolling space ship.
      + and -           : Select default speed. 
      Right Shift       : Speed up
      /                 : Full Speed
      Alt Gr            : Fire with select weapons, can be select with 1 to 5
                          when ammo is exhausted, it will change a other weapons

     7.1.2  Communications
    
      Press C for communicate with your wingmen. Select an order. For
      Attack this Target, lock a target before. For Request status, lock
      one of your wingman.  
      During network play, use T key to send messages.
 
     7.1.3  Views

      F2   : Cockpit view          F3   : Cycle view clockwise
      F4   : Cycle View            F5   : MiniCam (Rear view window)
      F6   : NAV Point view        F7   : Missile view window

     7.1.4  Details Level

     F9   : Sky On/Low/Off        
     F11  : Filtering On/Off      


     7.1.5  Miscellanous

     F1                   : Help 
     PrtScrn              : Dump a .PCX or .TGA (on c:\rt)
     I                    : HUD Display On/Off (faster for 3D acceleration)          
     T                    : Talk (Netchat)

     7.1.6   Weapon
 
     TwinLaser            : Small energy shoot
     QuadLaser            : Medium energy shoot. Used for dog fight
     OctoLaser            : Large energy shoot. Slower, but make a lot of damage
     Freezer              : Freezes any objects for few seconds
     Missile              : Missile Friend-Or-Foe. Need to lock the target
                            Excellent for dog fight
     Torpedo              : Super Missile . Quite long to lock . Not recommanded
                            for dog fight but for large enemy base.

     7.1.7  Bonus

     Ammo                 : Give more shoots for weapons
     Cell                 : Add +25%
     MegaCell             : Restore all energy
     Stealth              : Invisibility. Enemy will not attack you for 20 seconds
     SystemShield         : Invicibility !
     Cloak Device         : Warp near to the nav point
     BlackHole            : Destroy everything around you                                        

  *END OF FILE*
@

