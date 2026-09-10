// SPDX-FileCopyrightText: 1996-2005 realtech VR
// SPDX-License-Identifier: GPL-2.0-or-later
//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2005 - realtech VR

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
*/
//-------------------------------------------------------------------------
#ifndef __INI_H
#define __INI_H

enum ConfigDataType {
      T_BOOL,
      T_LONG,
      T_STR,
      T_HEX
};
typedef union {
   u_int8_t   i_bool;
   int32_t    i_long;
   char    *i_str;
} ConfigItemData;

typedef struct c_item {
   char            name[32];
   char            *data;
   struct c_item   *nextItem;
} ConfigItem;

typedef struct c_class {
   char            name[32];
   ConfigItem      *firstItem;
   struct c_class  *nextClass;
} ConfigClass;

typedef struct {
   char            *fileName;
   ConfigClass     *currentClass;
   ConfigClass     *firstClass;
} ConfigFile;


int ReadConfig(char const *fileName, ConfigFile *c_file);
void DestroyConfig(ConfigFile *q);
int SelectConfigClass(char const *className, ConfigFile *c_file);
int32_t GetCF_long(char *s, ConfigFile *ini);
int32_t GetCF_bool(char *s, ConfigFile *ini);
char *GetCF_str(char *s, ConfigFile *ini);
char *GetCF_str2(char *s, ConfigFile *ini);
int GetCF_long2(char *s, ConfigFile *ini, int32_t *val);
int GetCF_bool2(char *s, ConfigFile *ini, u_int8_t *val);


#endif
