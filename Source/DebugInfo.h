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

#include "MapFile.h"
#include "Asm.h"
#include "Ident.h"

#ifndef _debug_info_h_
#define _debug_info_h_


struct CLine : CAsm
{
  int ln;		// nr wiersza w pliku Ÿród³owym
  FileUID file;		// identyfikator pliku
  CLine(int ln, FileUID file) : ln(ln), file(file)
  { }
  CLine() : ln(0), file(0)
  { }
  int operator==(const CLine &arg) const
  { return ln==arg.ln && file==arg.file; }
  operator DWORD()	// konwersja dla funkcji mieszaj¹cej (CMap<>::HashKey())
  { return DWORD( (ln<<4) ^ (file<<8) ); }
};


struct CDebugLine : CAsm	// info dla symulatora o jednym wierszu Ÿród³owym programu
{
  UINT8 flags;		// flagi opisuj¹ce wiersz (DbgFlag)
  UINT16 addr;		// adres programu 6502
  CLine line;
  CDebugLine() : flags(CAsm::DBG_EMPTY), addr(0)
  { }
  CDebugLine(int ln, FileUID uid, UINT16 addr, int flg) : flags((UINT8)flg), addr(addr),
    line(ln,uid)
  { }
  CDebugLine(const CDebugLine &src)
  { memcpy(this,&src,sizeof(*this)); }

  const CDebugLine &operator=(const CDebugLine &src)
  {
    memcpy(this,&src,sizeof(*this));
    return *this;
  }
};



class CDebugLines : CArray<CDebugLine,CDebugLine&>, public CAsm
{

  CMap<UINT16, UINT16, int, int> addr_to_idx;	// tablice asocjacyjne do szybkiego
  CMap<CLine, CLine&, int, int> line_to_idx;	// odszukiwania adresu lub wiersza

public:
  CDebugLines() : addr_to_idx(50), line_to_idx(50)
  { SetSize(50,50); }

	// znalezienie wiersza odpowiadaj¹cego adresowi
  void GetLine(CDebugLine &ret, UINT16 addr)
  {
    static const CDebugLine empty;	// pusty obiekt - do oznaczenia "nie znaleziony wiersz"
    int idx;
    if (addr_to_idx.Lookup(addr,idx))
      ret = GetAt(idx);
    else
      ret = empty;
  }

	// znalezienie adresu odp. wierszowi
  void GetAddress(CDebugLine &ret, int ln, FileUID file)
  {
    static const CDebugLine empty;	// pusty obiekt - do oznaczenia "nie znaleziony adres"
    int idx;
    if (line_to_idx.Lookup(CLine(ln,file),idx))
      ret = GetAt(idx);
    else
      ret = empty;
  }

  void AddLine(CDebugLine &dl)
  {
    ASSERT(dl.flags != DBG_EMPTY);	// niewype³niony opis wiersza
    int idx = Add(dl);			// dopisanie info o wierszu, zapamiêtanie indeksu
    addr_to_idx.SetAt(dl.addr,idx);	// zapisanie indeksu
    line_to_idx.SetAt(dl.line,idx);	// j.w.
  }

  void Empty()
  {
    RemoveAll();
    addr_to_idx.RemoveAll();
    line_to_idx.RemoveAll();
  }
};


class CDebugBreakpoints : CAsm, CByteArray	// informacja o miejscach przerwañ
{
  UINT16 temp_bp_index;
public:
  CDebugBreakpoints() : temp_bp_index(0)
  { SetSize(0x10000); }

  Breakpoint Set(UINT16 addr, int bp= BPT_EXECUTE)	// ustawienie przerwania
  {
    ASSERT( (bp & ~BPT_MASK) == 0 );	// niedozwolona kombinacja bitów okreœlaj¹cych przerwanie
    return Breakpoint( (*this)[addr] |= bp );
  }
  Breakpoint Clr(UINT16 addr, int bp= BPT_MASK)		// skasowanie przerwania
  {
    ASSERT( (bp & ~BPT_MASK) == 0 );	// niedozwolona kombinacja bitów okreœlaj¹cych przerwanie
    return Breakpoint( (*this)[addr] &= ~bp );
  }
  Breakpoint Toggle(UINT16 addr, int bp)
  {
    ASSERT( (bp & ~BPT_MASK) == 0 );	// niedozwolona kombinacja bitów okreœlaj¹cych przerwanie
    return Breakpoint( (*this)[addr] &= ~bp );
  }
  Breakpoint Get(UINT16 addr)
  { return Breakpoint( (*this)[addr] ); }

  void Enable(UINT16 addr, bool enable= true)
  {
    ASSERT( (*this)[addr] & BPT_MASK );	// pod danym adresem nie ma przerwania
    if (enable)
      (*this)[addr] &= ~BPT_DISABLED;
    else
      (*this)[addr] |= BPT_DISABLED;
  }
  void ClrBrkp(UINT16 addr)		// skasowanie przerwania
  { (*this)[addr] = BPT_NONE; }

  void SetTemporaryExec(UINT16 addr)
  {
    temp_bp_index = addr;
    (*this)[addr] |= BPT_TEMP_EXEC;
  }
  void RemoveTemporaryExec()
  {
    (*this)[temp_bp_index] &= ~BPT_TEMP_EXEC;
  }

  void ClearAll()		// usuniêcie wszystkich przerwañ
  { memset(m_pData,BPT_NONE,m_nSize*sizeof(BYTE)); }
};



class CDebugIdents				// informacja o identyfikatorach
{
  CStringArray m_name;
  CArray<CIdent, const CIdent&> m_info;

public:
  void SetArrSize(int size)
  {
    m_name.RemoveAll();
    m_info.RemoveAll();
    m_name.SetSize(size);
    m_info.SetSize(size);
  }
  void SetIdent(int index, const CString &name, const CIdent &info)
  {
    m_name.SetAt(index,name);
    m_info.SetAt(index,info);
  }
  void GetIdent(int index, CString &name, CIdent &info)
  {
    name = m_name[index];	// m_name.ElementAt(index);
    info = m_info[index];	// m_info.ElementAt(index);
  }
  int GetCount()
  {
    ASSERT(m_name.GetSize() == m_info.GetSize());
    return m_name.GetSize();
  }
  void Empty()
  {
    m_name.RemoveAll();
    m_info.RemoveAll();
  }
};


class CDebugInfo : CAsm
{
  CDebugLines m_lines;			// informacja o wierszach
  CDebugIdents m_idents;		// informacja o identyfikatorach
  CDebugBreakpoints m_breakpoints;	// informacja o miejscach przerwañ
  CMapFile m_map_file;			// odwzorowania fazwy pliku Ÿród³owego na 'fuid' i odwrotnie

public:

  void Empty()
  { m_lines.Empty(); m_idents.Empty(); }

  void AddLine(CDebugLine &dl)
  { m_lines.AddLine(dl); }

  void GetLine(CDebugLine &ret, UINT16 addr)	// znalezienie wiersza odpowiadaj¹cego adresowi
  { m_lines.GetLine(ret,addr); }

  void GetAddress(CDebugLine &ret, int ln, FileUID file)	// znalezienie adresu odp. wierszowi
  { m_lines.GetAddress(ret,ln,file); }

  Breakpoint SetBreakpoint(int line, FileUID file, int bp= BPT_NONE);// ustawienie przerwania
  Breakpoint ToggleBreakpoint(int line, FileUID file);
  Breakpoint GetBreakpoint(int line, FileUID file);
  Breakpoint ModifyBreakpoint(int line, FileUID file, int bp);
  void ClrBreakpoint(int line, FileUID file);
  Breakpoint GetBreakpoint(UINT16 addr)
  { return m_breakpoints.Get(addr); }

  void SetTemporaryExecBreakpoint(UINT16 addr)
  { m_breakpoints.SetTemporaryExec(addr); }

  void RemoveTemporaryExecBreakpoint()
  { m_breakpoints.RemoveTemporaryExec(); }

  FileUID GetFileUID(const CString &doc_title)
  { return m_map_file.GetFileUID(doc_title); }		// ID pliku
  LPCTSTR GetFilePath(FileUID fuid)
  { return fuid ? m_map_file.GetPath(fuid) : NULL; }	// nazwa (œcie¿ka do) pliku
  void ResetFileMap()
  { m_map_file.Reset(); }

  void SetIdentArrSize(int size)
  { m_idents.SetArrSize(size); }
  void SetIdent(int index, const CString &name, const CIdent &info)
  { m_idents.SetIdent(index,name,info); }
  void GetIdent(int index, CString &name, CIdent &info)
  { m_idents.GetIdent(index,name,info); }
  int GetIdentCount()
  { return m_idents.GetCount(); }
};


#endif
