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

#ifndef _draw_marks_h_
#define _draw_marks_h_

class CMarks
{

public:
  static COLORREF m_rgbPointer;
  static COLORREF m_rgbBreakpoint;
  static COLORREF m_rgbError;

  enum MarkType
  { MT_ERASE, MT_POINTER, MT_BREAKPOINT, MT_DISBRKP, MT_ERROR };

  static void draw_breakpoint(CDC &dc, int x, int y, int h, bool active);
  static void draw_pointer(CDC &dc, int x, int y, int h);
  static void draw_mark(CDC &dc, int x, int y, int h);

//  virtual int ScrollToLine(int line, int &height, BOOL scroll)= 0;
//  void DrawMark(CDC &dc, int line, MarkType type, BOOL scroll);

};

#endif
