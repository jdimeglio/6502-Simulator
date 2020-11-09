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

#include "Broadcast.h"
#include "M6502.h"
#include "Sym6502.h"
#include "MarkArea.h"
#include "IntGenerator.h"


class CGlobal : public CObject, CAsm, virtual CBroadcast
{
	UINT m_uAddrBusWidth;			// szerokoœæ szyny adresowej
	bool m_bCodePresent;			// true -> po udanej asemblacji
	COutputMem m_ProgMem;			// pamiêæ zapisywana w procesie asemblacji
	CDebugInfo m_Debug;				// informacja uruchomieniowa dla symulatora
	UINT16 m_uOrigin;				// pocz¹tek programu 6502
	CSym6502 *m_pSym6502;			// symulator
	Finish m_SymFinish;				// sposób koñczenia programu przez symulator
	CMarkArea m_MarkArea;			// oznaczenie fragmentów pamiêci zawieraj¹cej kod wynikowy
public:
	bool m_bProc6502;				// typ procesora
	COutputMem m_Mem;				// pamiêæ dla kodu wynikowego i symulatora
	bool m_bGenerateListing;		// generowaæ listing przy asemblacji?
	CString m_strListingFile;		// plik z listingiem
	CIntGenerator m_IntGenerator;	// interrupt request generator data

	CGlobal() : m_pSym6502(NULL), m_bCodePresent(false)
	{ SetAddrBusWidth(16); }

	~CGlobal()
	{ if (m_pSym6502) delete m_pSym6502; }

	void SetAddrBusWidth(UINT w)
	{
		m_uAddrBusWidth = w;
		if (m_pSym6502)
			m_pSym6502->set_addr_bus_width(w);
	}

	CDebugInfo *GetDebug()
	{ return &m_Debug; }

	COutputMem *GetMemForAsm()	// pamiêæ na kod wynikowy (asemblacja)
	{ 
		m_ProgMem.ClearMem();
		return &m_ProgMem;
	}

	COutputMem *GetMemForSym()	// pamiêæ z kodem wynikowym (symulator)
	{
		if (m_bCodePresent)
		{
			m_Mem = m_ProgMem;
			return &m_Mem;
		}
		return NULL;
	}

	CMarkArea *GetMarkArea()
	{ return &m_MarkArea; }

	COutputMem *GetMem()		// pamiêæ z kodem wynikowym
	{ return &m_Mem; }
	UINT16 GetStartAddr()		// pocz¹tek programu
	{ return m_uOrigin; }

	bool IsCodePresent()
	{ return m_bCodePresent; }

	bool IsDebugInfoPresent()
	{ return m_bCodePresent; }	// do poprawienia

	bool IsDebugger()
	{ return m_pSym6502 != NULL; }

	bool IsProgramRunning()
	{ return m_pSym6502 ? m_pSym6502->IsRunning() : false; }

	bool IsProgramFinished()
	{ return m_pSym6502 ? m_pSym6502->IsFinished() : false; }

	void SetCodePresence(bool present)
	{
		if (present)
			m_Mem = m_ProgMem;
		m_bCodePresent = present;
	}

	void StartDebug();

	void RestartProgram()
	{ StartDebug(); }

	void ExitDebugger();

	void SetStart(UINT16 prog_start)
	{ m_uOrigin = prog_start; }

	CSym6502 *GetSimulator()
	{ return m_pSym6502; }

	CString GetStatMsg()
	{ return m_pSym6502->GetLastStatMsg(); }

	Finish GetSymFinish()
	{ ASSERT(m_pSym6502==NULL || m_pSym6502->finish==m_SymFinish); return m_SymFinish; }

	void SetSymFinish(Finish fin)
	{ m_SymFinish = fin;  if (m_pSym6502) m_pSym6502->finish = fin; }

	Breakpoint SetBreakpoint(int line, CString doc_title);
	Breakpoint GetBreakpoint(int line, CString doc_title);
	Breakpoint ModifyBreakpoint(int line, CString doc_title, Breakpoint bp);
	void ClrBreakpoint(int line, CString doc_title);
	DbgFlag GetLineDebugFlags(int line, CString doc_title);
	UINT16 GetLineCodeAddr(int line, CString doc_title);
	bool SetTempExecBreakpoint(int line, CString doc_title);

	void AbortProg()
	{
		if (m_pSym6502 != NULL)
			m_pSym6502->AbortProg();
	}

	//---------------------------------------------------------------------------

	bool GetProcType()
	{ return m_bProc6502; }

	void SetProcType(bool b6502)
	{ m_bProc6502 = b6502; }

	//---------------------------------------------------------------------------

	bool CreateDeasm();			// nowe okno deasemblera

	Breakpoint GetBreakpoint(UINT16 addr)	// pobranie przerwania pod danym adresem
	{ return m_Debug.GetBreakpoint(addr); }

	//---------------------------------------------------------------------------

	void SaveCode(CArchive &archive, UINT16 start, UINT16 end, int info);
	void LoadCode(CArchive &archive, UINT16 start, UINT16 end, int info, int nClear= 0);

};
