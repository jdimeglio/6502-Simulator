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

#include "StdAfx.h"
#include "DrawMarks.h"

// Funkcje kreœl¹ce strza³kê, miejsce przerwania i b³êdu
// Wykorzystane w klasach ...View


COLORREF CMarks::m_rgbPointer= RGB(255,255,0);
COLORREF CMarks::m_rgbBreakpoint= RGB(0,0,255);
COLORREF CMarks::m_rgbError= RGB(255,0,0);

//-----------------------------------------------------------------------------

void CMarks::draw_pointer(CDC &dc, int x, int y, int h)
{
  static const POINT shape[]=
  { {-4,-3}, {0,-3}, {0,-7}, {7,0}, {0,7}, {0,3}, {-4,3}, {-4,-3} };
  const int size= sizeof shape / sizeof(POINT);
  POINT coords[size];

  x += (6 * h) / 15;
  y += (7 * h) / 15;
  for (int i=0; i<size; i++)
  {
    coords[i].x = x + (shape[i].x * h) / 15;	// przeskalowanie i przesuniêcie
    coords[i].y = y + (shape[i].y * h) / 15;
  }

  CPen *pOldPen, pen(PS_SOLID,0,::GetSysColor(COLOR_WINDOWTEXT));
  pOldPen = dc.SelectObject(&pen);
  if (pOldPen==NULL)
    return;
  CBrush brush(m_rgbPointer);
  CBrush *pOldBrush= dc.SelectObject(&brush);
  if (pOldBrush==NULL)
  {
    dc.SelectObject(pOldPen);
    return;
  }

  dc.SetPolyFillMode(ALTERNATE);
  dc.Polygon(coords,size);

  dc.SelectObject(pOldPen);
  dc.SelectObject(pOldBrush);
}

//-----------------------------------------------------------------------------

void CMarks::draw_breakpoint(CDC &dc, int x, int y, int h, bool active)
{
  CPen *pOldPen, pen(PS_SOLID,0,::GetSysColor(COLOR_WINDOWTEXT));
  pOldPen = dc.SelectObject(&pen);
  if (pOldPen==NULL)
    return;
  CBrush brush(active ? m_rgbBreakpoint : dc.GetBkColor());
  CBrush *pOldBrush= dc.SelectObject(&brush);
  if (pOldBrush==NULL)
  {
    dc.SelectObject(pOldPen);
    return;
  }

  dc.Ellipse(x,y,x+h,y+h);

  dc.SelectObject(pOldPen);
  dc.SelectObject(pOldBrush);
}

//-----------------------------------------------------------------------------

void CMarks::draw_mark(CDC &dc, int x, int y, int h)
{
  static const POINT shape[]=
  { {-1,-7}, {0,-7}, {7,0}, {0,7}, {-1,7}, {-1,-7} };
//  { {0,0}, {1,0}, {8,7}, {1,14}, {0,14}, {0,0} };
  const int size= sizeof shape / sizeof(POINT);
  POINT coords[size];

  x += (3 * h) / 15;
  y += (7 * h) / 15;
  for (int i=0; i<size; i++)
  {
    coords[i].x = x + (shape[i].x * h) / 15;
    coords[i].y = y + (shape[i].y * h) / 15;
  }

  CPen *pOldPen, pen(PS_SOLID,0,::GetSysColor(COLOR_WINDOWTEXT));
  pOldPen = dc.SelectObject(&pen);
  if (pOldPen==NULL)
    return;
  CBrush brush(m_rgbError), *pOldBrush= dc.SelectObject(&brush);
  if (pOldBrush==NULL)
  {
    dc.SelectObject(pOldPen);
    return;
  }

  dc.SetPolyFillMode(ALTERNATE);
  dc.Polygon(coords,size);

  dc.SelectObject(pOldPen);
  dc.SelectObject(pOldBrush);
}

//-----------------------------------------------------------------------------
