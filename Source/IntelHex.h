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

#ifndef _intel_hex_
#define _intel_hex_

class COutputMem;
class CMarkArea;

class CIntelHex
{
  UINT geth(const TCHAR *&ptr, UINT &sum);	// interpretacja dwucyfrowej liczby hex
  UINT row;

public:
  void SaveHexFormat(CArchive &archive, COutputMem &mem, CMarkArea &area, int prog_start= -1);

  void LoadHexFormat(CArchive &archive, COutputMem &mem, CMarkArea &area, int &prog_start);

  class CIntelHexException //: public CException
  {
  public:
    enum Err { E_NONE, E_BAD_FORMAT, E_CHKSUM, E_FORMAT };
  private:
    Err error;
    UINT row;
  public:
    CIntelHexException(Err err= E_NONE, UINT row= 0) : error(err), row(row)
    { }
    /*virtual*/ ~CIntelHexException()
    {}

    /*virtual*/ bool GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext = NULL);
  };
};


#endif
