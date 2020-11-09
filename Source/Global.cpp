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
#include "Deasm6502Doc.h"
#include "IntelHex.h"
#include "MotorolaSRecord.h"
#include "AtariBin.h"


CAsm::Breakpoint CGlobal::SetBreakpoint(int line, CString doc_title)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);
  return m_Debug.ToggleBreakpoint(line,fuid);	// ustawienie/skasowanie przerwania
}


CAsm::Breakpoint CGlobal::GetBreakpoint(int line, CString doc_title)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);
  return m_Debug.GetBreakpoint(line,fuid);	// ustawienie/skasowanie przerwania
}


CAsm::Breakpoint CGlobal::ModifyBreakpoint(int line, CString doc_title, Breakpoint bp)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);
  return m_Debug.ModifyBreakpoint(line,fuid,bp);// ustawienie przerwania
}


void CGlobal::ClrBreakpoint(int line, CString doc_title)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);
  m_Debug.ClrBreakpoint(line,fuid);	// skasowanie przerwania
}

CAsm::DbgFlag CGlobal::GetLineDebugFlags(int line, CString doc_title)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);	// ID pliku
  CDebugLine dl;
  m_Debug.GetAddress(dl,line,fuid);	// znalezienie adresu odpowiadaj¹cego wierszowi
  return (DbgFlag)dl.flags;		// flagi opisuj¹ce wiersz programu
}

UINT16 CGlobal::GetLineCodeAddr(int line, CString doc_title)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);	// ID pliku
  CDebugLine dl;
  m_Debug.GetAddress(dl,line,fuid);	// znalezienie adresu odpowiadaj¹cego wierszowi
  return dl.addr;
}

bool CGlobal::SetTempExecBreakpoint(int line, CString doc_title)
{
  FileUID fuid= m_Debug.GetFileUID(doc_title);	// ID pliku
  CDebugLine dl;
  m_Debug.GetAddress(dl,line,fuid);	// znalezienie adresu odpowiadaj¹cego wierszowi
  if (dl.flags == DBG_EMPTY || (dl.flags & DBG_MACRO))
    return FALSE;		// nie ma kodu w wierszu 'line'
  m_Debug.SetTemporaryExecBreakpoint(dl.addr);
  return TRUE;
}


bool CGlobal::CreateDeasm()
{
  ASSERT(m_pSym6502 != NULL);

  CDeasm6502Doc *pDoc= (CDeasm6502Doc*) theApp.m_pDocDeasmTemplate->OpenDocumentFile(NULL);
  if (pDoc == NULL)
    return FALSE;
/*
  pDoc->SetContext( m_pSym6502->GetContext() );
  pDoc->SetStart( m_pSym6502->get_pc() );
*/
  pDoc->SetPointer( m_pSym6502->get_pc() );

  return TRUE;
}

//-----------------------------------------------------------------------------

void CGlobal::StartDebug()
{
  GetMemForSym();
  bool restart;
  if (m_pSym6502 == NULL)
    restart=FALSE, m_pSym6502 = new CSym6502(m_Mem,&m_Debug,m_uAddrBusWidth);
  else
    restart=TRUE, m_pSym6502->Restart(m_Mem);
  m_pSym6502->finish = m_SymFinish;
  m_pSym6502->SymStart(m_uOrigin);
  m_pSym6502->Update(CAsm::SYM_OK,TRUE);
/*
  struct { const CString *pStr, const CContext *pCtx } data;
  CString str= m_pSym6502->GetStatMsg(stat);
  data.pStr = &str;
  data.pCtx = m_pSym6502->GetContext();
*/
  SendMessageToViews(WM_USER_START_DEBUGGER);
  SendMessageToPopups(WM_USER_START_DEBUGGER,(WPARAM)restart);
}


void CGlobal::ExitDebugger()
{
  if (m_pSym6502 == NULL)
    return;
  ASSERT(!m_pSym6502->IsRunning());
  SendMessageToViews(WM_USER_EXIT_DEBUGGER);
  SendMessageToPopups(WM_USER_EXIT_DEBUGGER);
  m_pSym6502->ExitSym();
  delete m_pSym6502;
  m_pSym6502 = NULL;
}


//-----------------------------------------------------------------------------

void CGlobal::SaveCode(CArchive &archive, UINT16 start, UINT16 end, int info)
{
//  ASSERT(m_bCodePresent);
  switch (info)
  {
    case 0:		// format Intel-HEX kodu wynikowego (*.65h/*.hex)
    {
      CIntelHex hex;
      hex.SaveHexFormat(archive, m_Mem, m_MarkArea, m_uOrigin);
      break;
    }
    case 1:		// format s-rekord Motoroli kodu wynikowego (*.65h/*.hex)
    {
      CMotorolaSRecord srec;
      srec.SaveHexFormat(archive, m_Mem, m_MarkArea, m_uOrigin);
      break;
    }
    case 2:		// obraz binarny kodu wynikowego (*.65b)
      m_Mem.Save(archive, start, end);
      break;
    case 3:		// program wynikowy (*.65p)
      m_Mem.Save(archive, start, end);
      break;
    default:
      ASSERT(FALSE);
  }
}


void CGlobal::LoadCode(CArchive &archive, UINT16 start, UINT16 end, int info, int nClear/*= 0*/)
{
	COutputMem mem;	// pamiêæ na ³adowany program
	int prog_start= -1;

	if (nClear != -1)
		mem.ClearMem(nClear);
	else
		mem = m_ProgMem;

	switch (info)
	{
	case 0:		// format Intel-HEX kodu wynikowego (*.65h/*.hex)
		{
			CIntelHex hex;
			hex.LoadHexFormat(archive, mem, m_MarkArea, prog_start);
			break;
		}

	case 1:		// format s-record Motoroli kodu wynikowego (*.65m/*.s9)
		{
			CMotorolaSRecord srec;
			srec.LoadHexFormat(archive, mem, m_MarkArea, prog_start);
			break;
		}

	case 2:		// obraz binarny kodu wynikowego (*.65b)
		mem.Load(archive, start, end);
		break;

	case 3:		// program wynikowy (*.65p)
		{
			WORD wTemp;
			archive >> wTemp;
			if (wTemp != 0xFFFF)
				throw new CFileException(CFileException::invalidFile);

			int nLen= archive.GetFile()->GetLength() - 2;
			do
			{
				WORD wFrom, wTo;
				archive >> wFrom;
				if (wFrom == 0xFFFF)
				{
					nLen -= 2;
					archive >> wFrom;
				}
				archive >> wTo;
				nLen -= 4;
				if (wTo < wFrom)
					throw new CFileException(CFileException::invalidFile);

				mem.Load(archive, wFrom, wTo);
				nLen -= wTo - wFrom + 1;
			}
			while (nLen > 0);
			//        while (archive.GetFile()->GetLength() > archive.GetFile()->GetPosition());
		}

	case 4:		// Atari binary program
		{
			CAtariBin bin;
			bin.LoadAtaBinFormat(archive, mem, m_MarkArea, prog_start);
		}
		break;

	default:
		ASSERT(false);
	}

	m_ProgMem = mem;	// uda³o siê za³adowaæ program
	SetCodePresence(TRUE);
	SetStart(prog_start != -1 ? prog_start : start);
	StartDebug();
	SendMessageToViews(WM_USER_PROG_MEM_CHANGED,(WPARAM)start);
	SendMessageToPopups(WM_USER_PROG_MEM_CHANGED,(WPARAM)start);
}
