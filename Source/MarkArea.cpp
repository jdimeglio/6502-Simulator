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
#include "MarkArea.h"


void CMarkArea::SetEnd(int end)
{
  ASSERT(start >= 0);		// przed 'SetEnd' musi byæ wywo³ane 'SetStart'
  ASSERT(end >= start);		// b³êdne krañce przedzia³ów
  if (end < start)		// b³êdne krañce przedzia³ów?
    return;

  Pair pair= {start,end};
  for (UINT i=0; i<n; i++)
  {
    if (arr[i].a > end || arr[i].b < start)
      continue;			// przedzia³y roz³¹czne
    if (arr[i].a <= start)
      if (arr[i].b >= end)
	return;			// nowa para mieœci siê w przedziale
      else
      {
	arr[i].b = end;		// przesuniêcie koñca przedzia³u
	return;
      }
    else if (arr[i].b <= end)
    {
      arr[i].a = start;		// przesuniêcie pocz¹tku przedzia³u
      return;
    }
    else
    {
      arr[i].a = start;		// poszerzenie ca³ego przedzia³u
      arr[i].b = end;
      return;
    }
  }
  arr.SetAtGrow(n,pair);
  n++;
}


bool CMarkArea::GetPartition(UINT no, int &a, int &b)
{
  if (no >= n)
    return FALSE;
  a = arr[no].a;
  b = arr[no].b;
  return true;
}
