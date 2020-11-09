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
#include "M6502.h"
#include "DialAsmStat.h"


class CInputWnd : CWnd
{
  void open(const CString &fname);
  void close();
  void seek_to_begin();
  LPTSTR read_line(LPTSTR str, UINT max_len);
  int get_line_no();
  const CString &get_file_name();
};


LPTSTR CInputWnd::read_line(LPTSTR str, UINT max_len)
{
  int ret= SendMessage(WM_USER_GET_NEXT_LINE, WPARAM(max_len), LPARAM(str));
  ASSERT(ret);
  return str;
}


int CInputWnd::get_line_no()
{
  int no;
  int ret= SendMessage(WM_USER_GET_LINE_NO, 0, LPARAM(&no));
  ASSERT(ret);
  return no;
}


const CString &CInputWnd::get_file_name()
{
  CString fname;
  int ret= SendMessage(WM_USER_GET_TITLE, WPARAM(_MAX_PATH), LPARAM(fname.GetBuffer(_MAX_PATH+1)));
  fname.ReleaseBuffer();
  ASSERT(ret);
  return fname;
}
