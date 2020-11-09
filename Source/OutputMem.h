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

#ifndef _output_mem_h_
#define _output_mem_h_

class COutputMem : private CByteArray
{
  UINT m_uMask;

public:
  COutputMem()
  { SetSize(0x10000); m_uMask = 0xFFFF; }

  void ClearMem()	// wyzerowanie pamiêci
  { memset(m_pData, 0, m_nSize*sizeof(BYTE)); }

	void ClearMem(int nByte)
	{ memset(m_pData, nByte, m_nSize*sizeof(BYTE)); }

  COutputMem &operator= (const COutputMem &src)
  {
    ASSERT(m_nSize == src.m_nSize);		// wymiary musz¹ byæ identyczne
    memcpy(m_pData, src.m_pData, m_nSize * sizeof(BYTE));
    m_uMask = src.m_uMask;
    return *this;
  }

  void Save(CArchive &archive, UINT16 start, UINT16 end)
  {
    ASSERT(end >= start);
    archive.Write(m_pData + start, int(end) - start + 1);
  }

  void Load(CArchive &archive, UINT16 start, UINT16 end)
  {
    ASSERT(end >= start);
    archive.Read(m_pData + start, int(end) - start + 1);
  }

  const UINT8 *Mem()
  { return m_pData; }

  void SetMask(UINT16 uMask)  { m_uMask = uMask; }

  UINT8& operator[] (UINT16 uAddr)        { ASSERT(m_uMask > 0);  return m_pData[uAddr & m_uMask]; } // ElementAt(uAddr & m_uMask); }
  UINT8  operator[] (UINT16 uAddr) const  { ASSERT(m_uMask > 0);  return m_pData[uAddr & m_uMask]; } // GetAt(uAddr & m_uMask); }

  UINT16 GetWord(UINT16 uLo, UINT16 uHi) const
  {
    ASSERT(m_uMask > 0);
    return GetAt(uLo & m_uMask) + (UINT16(GetAt(uHi & m_uMask)) << 8);
  }
  UINT16 GetWord(UINT16 uAddr) const
  {
    ASSERT(m_uMask > 0);
    return GetAt(uAddr & m_uMask) + (UINT16(GetAt((uAddr + 1) & m_uMask)) << 8);
  }
  UINT16 GetWordInd(UINT8 uZpAddr) const
  {
    ASSERT(m_uMask > 0);
    return GetAt(uZpAddr) + (GetAt(uZpAddr + UINT8(1)) << 8);
//    UINT8 uAddr= GetAt(uZpAddr);
//    return GetAt(uZpAddr) + UINT16(GetAt(uZpAddr + UINT8(1)) << 8);
  }
};

#endif
