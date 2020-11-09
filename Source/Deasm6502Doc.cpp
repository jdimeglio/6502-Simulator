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

// Deasm6502Doc.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "Deasm6502Doc.h"
#include "DeasmSaveOptions.h"
#include <locale.h>
#include "Deasm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc

IMPLEMENT_DYNCREATE(CDeasm6502Doc, CDocument)

CDeasm6502Doc::CDeasm6502Doc()
{
  m_pCtx = NULL;
  m_nPointerAddr = -1;

  m_uStart  = 0x0000;
  m_uEnd    = 0xFFFF;
  m_uLength = 0x10000;
  m_bSaveAsData = false;
}


BOOL CDeasm6502Doc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;

  SetContext( theApp.m_global.GetSimulator()->GetContext() );
  SetStart( theApp.m_global.GetStartAddr() );

  return TRUE;
}

CDeasm6502Doc::~CDeasm6502Doc()
{
}


BEGIN_MESSAGE_MAP(CDeasm6502Doc, CDocument)
  //{{AFX_MSG_MAP(CDeasm6502Doc)
  // NOTE - the ClassWizard will add and remove mapping macros here.
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc diagnostics

#ifdef _DEBUG
void CDeasm6502Doc::AssertValid() const
{
  CDocument::AssertValid();
}

void CDeasm6502Doc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc serialization

void CDeasm6502Doc::Serialize(CArchive& ar)
{
  if (ar.IsStoring())
  {
    CWaitCursor wait;
    DeassembleSave(ar, *m_pCtx, m_uStart, m_uEnd, 0);
  }
  else
  {
    // TODO: add loading code here
  }
}

/////////////////////////////////////////////////////////////////////////////
// Zapisanie deasemblatu

BOOL CDeasm6502Doc::DoSave(LPCTSTR lpszPathName, BOOL bReplace/* = TRUE*/)
{
  CDeasmSaveOptions dlg;
  dlg.m_uStart  = m_uStart;
  dlg.m_uEnd    = m_uEnd;
  dlg.m_uLength = m_uLength;
  dlg.m_bSaveAsData = m_bSaveAsData;
  if (dlg.DoModal() == IDOK)		// opcje deasemblacji
  {
    m_uStart  = dlg.m_uStart;
    m_uEnd    = dlg.m_uEnd;
    m_uLength = dlg.m_uLength;
    m_bSaveAsData = dlg.m_bSaveAsData;
    return CDocument::DoSave(lpszPathName, bReplace);
  }
  return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc commands

void CDeasm6502Doc::SetStart(UINT16 addr, bool bDraw)
{
  m_uStartAddr = addr;
}


void CDeasm6502Doc::SetContext(const CContext *pCtx)
{
  ASSERT(m_pCtx == NULL);		// nie mo¿na zmieniaæ kontekstu
  m_pCtx = pCtx;
}


  // narysowanie/zmazanie strza³ki w wierszu
void CDeasm6502Doc::SetPointer(int addr, bool scroll/*= FALSE*/)
{
  if (scroll && addr != -1)
    UpdateAllViews(NULL,MAKELONG(2,addr));	// przesuniêcie zawartoœci aktywnego okna
//  if (m_nPointerAddr != -1)
  ASSERT(addr==-1 || addr>=0 && addr<=0xFFFF);
  m_nPointerAddr = addr;
//  RECT rect;
//  GetClientRect(&rect);
  UpdateAllViews(NULL,1);	// przerysowanie wskaŸników
}

//-----------------------------------------------------------------------------

void CDeasm6502Doc::DeassembleSave(CArchive &ar, const CContext &ctx, UINT16 start, UINT16 end, int opt)
{
  COutputMem info;
  info.ClearMem();
  enum Flag { NONE=0, ZPG=1, ABS=2, TXT=4, CODE=8 };

  UINT ptr;
  for (ptr= start; ptr < end; )
  {
    UINT8 cmd= ctx.mem[ptr];

    switch (CodeToMode()[cmd])	// faza zaznaczania wykorzystywanych adresów
    {
      case A_IMP:	// implied
      case A_ACC:	// accumulator
      case A_IMP2:	// implied dla BRK
      case A_ILL:	// illegal
	break;
      case A_IMM:	// immediate
	break;
      case A_ZPG:	// zero page
      case A_ZPG_X:	// zero page indexed X
      case A_ZPG_Y:	// zero page indexed Y
      case A_ZPGI:	// zero page indirect
      case A_ZPGI_X:	// zero page indirect, indexed X
      case A_ZPGI_Y:	// zero page indirect, indexed Y
      case A_ZPG2:	// zero page dla rozkazów RMB SMB z 6501
	info[ctx.mem[ptr+1 & 0xFFFF]] |= ZPG;
	break;
      case A_ABS:	// absolute
      case A_ABS_X:	// absolute indexed X
      case A_ABS_Y:	// absolute indexed Y
      case A_ABSI:	// absolute indirect
      case A_ABSI_X:	// absolute indirect: indexed X
      {
	UINT16 addr= (UINT16)ctx.mem[ptr+2 & 0xFFFF] << UINT16(8);
	addr += ctx.mem[ptr+1 & 0xFFFF];
        if (addr >= start && addr <= end)
	  info[addr] |= TXT;
        else
	  info[addr] |= ABS;
	break;
      }
      case A_REL:	// relative
      {
	UINT8 arg= ctx.mem[ptr+1 & 0xFFFF];
	UINT16 addr= arg & 0x80 ? ptr+2-(0x100-arg) : ptr+2+arg;
	info[addr] |= TXT;
	break;
      }
      case A_ZREL:	// zero page / relative -> BBS i BBR z 6501
      {
	UINT8 arg= ctx.mem[ptr+1 & 0xFFFF];
	info[ctx.mem[ptr+1 & 0xFFFF]] |= ZPG;
	info[ UINT16( (arg & 0x80) ? ptr+3-(0x100-arg) : ptr+3+arg) ] |= TXT;
	break;
      }
      default:
	ASSERT(FALSE);
	break;
    }

    info[ptr] |= CODE;
    ptr += cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];
  }

  setlocale(LC_ALL, "");
  CString str;
  CString strFormat;
  strFormat.LoadString(IDS_DISASM_FORMAT1);
  str.Format(strFormat, int(start), int(end), CTime::GetCurrentTime().Format("%x"));
//  str.Format("; Deasemblacja programu od $%04X do $%04X \t%s\r\n",(int)start,(int)end,CTime::GetCurrentTime().Format("%x"));
  ar.WriteString(str);
  if (!m_bSaveAsData)
  {
    strFormat.LoadString(IDS_DISASM_FORMAT2);
    ar.WriteString(strFormat);
    //  ar.WriteString("\r\n; oznaczenia etykiet:\r\n;   znn - adresy na str. zerowej\r\n"
    //    ";   annnn - adresy absolutne\r\n;   ennnn - adresy u¿ywane w rozkazach skoków wzglêdnych\r\n\r\n");
    str.Format("\t.ORG $%04X\r\n",(int)start);
    ar.WriteString(str);
  }

  if (!m_bSaveAsData)
    for (int i= 0; i < 0xFFFF; i++)
      if (info[i])
      {
        if (info[i] & ZPG)		// etykieta adresu na str. zerowej
        {
          ASSERT(i<256);
          str.Format("z%02X\t= $%02X\r\n", i, i);
          ar.WriteString(str);
        }
        if (info[i] & ABS)		// etykieta adresu absolutnego
        {
          str.Format("a%04X\t= $%04X\r\n", i, i);
          ar.WriteString(str);
        }
        if ( (info[i] & TXT) && (info[i] & CODE)==0 )	// skok wzglêdny
        {
          str.Format("e%04X\t= $%04X\r\n", i, i);
          ar.WriteString(str);
        }
      }

  ar.WriteString("\r\n");

  CDeasm deasm;

  for (ptr = start; ptr < end; )
  {
    if (m_bSaveAsData)
    {
      str.Format("e%04X:\t.DB $%02X, $%02X, $%02X, $%02X, $%02X, $%02X, $%02X, $%02X\r\n", int(ptr),
        int(ctx.mem[ptr+0]), int(ctx.mem[ptr+1]), int(ctx.mem[ptr+2]), int(ctx.mem[ptr+3]),
        int(ctx.mem[ptr+4]), int(ctx.mem[ptr+5]), int(ctx.mem[ptr+6]), int(ctx.mem[ptr+7]) );
      ar.WriteString(str);
      ptr += 8;
    }
    else
    {
      if (info[ptr] & TXT)
      {
        str.Format("e%04X:\r\n", (int)ptr);
        ar.WriteString(str);
      }

      if (CodeToCommand()[ctx.mem[ptr]] == C_ILL || ctx.mem[ptr] == 0)	// nielegalny rozkaz lub BRK?
      {
        str.Format("\t.DB $%02X\r\n", int(ctx.mem[ptr]));
        ar.WriteString(str);
      }
      else
      {
        int p= ptr;
        str = '\t';
        str += deasm.DeasmInstr(ctx,CAsm::DF_LABELS,p);
        ar.WriteString(str + "\r\n");
      }

      ptr += ctx.mem[ptr] == 0 ? 1 : mode_to_len[CodeToMode()[ ctx.mem[ptr] ]];
    }
  }

  ar.WriteString("\r\n");
  ar.WriteString("\t.END\r\n");
}
