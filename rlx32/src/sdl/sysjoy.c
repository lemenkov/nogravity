//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2004 - realtech VR

This file is part of No Gravity 1.9

No Gravity is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Stephane Denis
Prepared for public release: 02/24/2004 - Stephane Denis, realtech VR
Linux/SDL Port: 2005 - Matt Williams
*/
//-------------------------------------------------------------------------
#include <SDL3/SDL.h>
#include "_rlx32.h"
#include "_rlx.h"
#include "sysctrl.h"

static int JoystickOpen(void *hnd, int force_feedback)
{
  int ok = FALSE;
  int idx, count = 0;
  SDL_JoystickID *ids;
  UNUSED(hnd);
  UNUSED(force_feedback);

  if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK))
    return FALSE;
  SDL_SetJoystickEventsEnabled(true);

  ids = SDL_GetJoysticks(&count);
  for (idx = 0; ids && (idx < count); idx ++)
  {
    SDL_Joystick *joy = SDL_OpenJoystick(ids[idx]);
    if (joy == NULL)
      continue;

    // Fewer than 3 axes is not enough for flying.
    if (SDL_GetNumJoystickAxes(joy) < 3)
    {
      SDL_CloseJoystick(joy);
      continue;
    }

    sJOY->numControllers++;

    // Keep the device with the most buttons.
    if (SDL_GetNumJoystickButtons(joy) > sJOY->numButtons)
    {
      if (sJOY->device != NULL)
        SDL_CloseJoystick((SDL_Joystick *)sJOY->device);
      sJOY->device = joy;
      sJOY->numButtons = SDL_GetNumJoystickButtons(joy);
      sJOY->numAxes = SDL_GetNumJoystickAxes(joy);
      sJOY->numPOVs = SDL_GetNumJoystickHats(joy);
      ok = TRUE;
    }
    else
    {
      SDL_CloseJoystick(joy);
    }
  }
  SDL_free(ids);

  if (sJOY->numButtons > 128) sJOY->numButtons = 128;
  if (sJOY->numPOVs > 4) sJOY->numPOVs = 4;

  if (!ok)
  {
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  }
  return ok;
}

static void JoystickRelease(void)
{
  SDL_Joystick *joy = (SDL_Joystick *)sJOY->device;
  if (joy != NULL)
  {
    SDL_CloseJoystick(joy);
    sJOY->device = NULL;
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  }
}

static int AxisValue(SDL_Joystick *joy, int axis)
{
  if (axis >= sJOY->numAxes)
    return 32768;
  return (int)SDL_GetJoystickAxis(joy, axis) + 32768;
}

static unsigned long JoystickUpdate(void *dev)
{
  SDL_Joystick *joy = (SDL_Joystick *)sJOY->device;
  int idx;
  UNUSED(dev);

  if (joy == NULL)
    return FALSE;

  SDL_UpdateJoysticks();

  // Copy the current button state to be the old button state.
  memcpy(sJOY->steButtons, sJOY->rgbButtons, sJOY->numButtons);

  for (idx = 0; idx < sJOY->numButtons; idx ++)
  {
    sJOY->rgbButtons[idx] = (u_int8_t)(SDL_GetJoystickButton(joy, idx) ? 1 : 0);
  }

  for (idx = 0; idx < sJOY->numPOVs; idx ++)
  {
    switch (SDL_GetJoystickHat(joy, idx))
    {
      case SDL_HAT_CENTERED:  sJOY->rgdwPOV[idx] =     1; break;
      case SDL_HAT_UP:        sJOY->rgdwPOV[idx] =     0; break;
      case SDL_HAT_RIGHTUP:   sJOY->rgdwPOV[idx] =  4500; break;
      case SDL_HAT_RIGHT:     sJOY->rgdwPOV[idx] =  9000; break;
      case SDL_HAT_RIGHTDOWN: sJOY->rgdwPOV[idx] = 13500; break;
      case SDL_HAT_DOWN:      sJOY->rgdwPOV[idx] = 18000; break;
      case SDL_HAT_LEFTDOWN:  sJOY->rgdwPOV[idx] = 22500; break;
      case SDL_HAT_LEFT:      sJOY->rgdwPOV[idx] = 27000; break;
      case SDL_HAT_LEFTUP:    sJOY->rgdwPOV[idx] = 31500; break;
      default: break;
    }
  }

  // Axes 0..5 map onto X, Y, Z and the three rotation axes; the game
  // lets the player choose which of them drive throttle and rudder.
  sJOY->lX  = AxisValue(joy, 0);
  sJOY->lY  = AxisValue(joy, 1);
  sJOY->lZ  = AxisValue(joy, 2);
  sJOY->lRx = AxisValue(joy, 3);
  sJOY->lRy = AxisValue(joy, 4);
  sJOY->lRz = AxisValue(joy, 5);
  return TRUE;
}

_RLXEXPORTFUNC JOY_ClientDriver *JOY_SystemGetInterface_STD(void)
{
  static JOY_ClientDriver driver =
  {
    JoystickOpen,
    JoystickRelease,
    JoystickUpdate
  };
  // Set the driver.
  sJOY = &driver;
  return sJOY;
}
