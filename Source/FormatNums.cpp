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

#include "stdafx.h"
#include "FormatNums.h"


void CFormatNums::IncEditField(CWnd *pCtrl, int iDelta, int iMin, int iMax)
{
  int num,old;
  NumFmt fmt;

  old = num = ReadNumber(pCtrl,fmt);

  num += iDelta;
  if (num > iMax)
    num = iMax;
  else if (num < iMin)
    num = iMin;
  if (num != old)
    SetNumber(pCtrl,num,fmt);
}


int CFormatNums::ReadNumber(CWnd *pCtrl, NumFmt &fmt)
{
  TCHAR buf[32];
  int num= 0;
  if (pCtrl==NULL)
    return num;

  pCtrl->GetWindowText(buf,sizeof(buf)/sizeof(buf[0]));
  if (buf[0]==_T('$'))
  {
    fmt = NUM_HEX_DOL;
    if (sscanf(buf+1, _T("%X"),&num) <= 0)
      ;
  }
  else if (buf[0]==_T('0') && (buf[1]==_T('x') || buf[1]==_T('X')))
  {
    fmt = NUM_HEX_0X;
    if (sscanf(buf+2, _T("%X"),&num) <= 0)
      ;
  }
  else if (buf[0]>=_T('0') && buf[0]<=_T('9'))
  {
    fmt = NUM_DEC;
    if (sscanf(buf, _T("%d"),&num) <= 0)
      ;
  }
  else
    fmt = NUM_ERR;

  return num;
}


void CFormatNums::SetNumber(CWnd *pCtrl, int num, NumFmt fmt)
{
  TCHAR buf[32];

  buf[0] = 0;

  switch (fmt)
  {
    case NUM_ERR:
    case NUM_HEX_0X:
      wsprintf(buf,_T("0x%04X"),num);
      break;
    case NUM_HEX_DOL:
      wsprintf(buf,_T("$%04X"),num);
      break;
    case NUM_DEC:
      wsprintf(buf,_T("%d"),num);
      break;
    default:
      ASSERT(FALSE);
  }

  if (pCtrl)
  {
    pCtrl->SetWindowText(buf);
    pCtrl->UpdateWindow();
  }
}
