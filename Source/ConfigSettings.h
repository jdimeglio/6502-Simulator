/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#ifndef _config_settings_
#define _config_settings_


struct CConfigSettings
{
  static LOGFONT* const fonts[];	// struktury LOGFONT w ca³ym programie
  static CFont* const cfonts[];	// struktury CFont w ca³ym programie
  static COLORREF* text_color[];
  static COLORREF* bkgnd_color[];
  static COLORREF* color_syntax[];
  static BYTE* syntax_font_style[];
};


#endif
