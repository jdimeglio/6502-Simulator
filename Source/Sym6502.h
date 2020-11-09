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

#ifndef _sym_6502_h_
#define _sym_6502_h_

#include "DebugInfo.h"
#include "OutputMem.h"
#include "LogBuffer.h"
class CSrc6502View;
#undef OVERFLOW

class CContext
{
	void reset()
	{
		pc = 0;
		a = x = y = s = 0;
		io = false;
		negative = overflow = zero = carry = false;
		break_bit = decimal = interrupt = false;
		reserved = true;
		uCycles = 0;
	}

public:
	UINT8 a, x, y, s;
	UINT16 pc;
	ULONG uCycles;

	bool negative, overflow, zero, carry;
	bool reserved, break_bit, decimal, interrupt;

	enum Flags
	{
		NEGATIVE  = 0x80,
		OVERFLOW  = 0x40,
		RESERVED  = 0x20,
		BREAK     = 0x10,
		DECIMAL   = 0x08,
		INTERRUPT = 0x04,
		ZERO      = 0x02,
		CARRY     = 0x01,
		NONE      = 0x00,
		ALL       = 0xFF,
		// numery bitów
		N_NEGATIVE  = 7,
		N_OVERFLOW  = 6,
		N_RESERVED  = 5,
		N_BREAK     = 4,
		N_DECIMAL   = 3,
		N_INTERRUPT = 2,
		N_ZERO      = 1,
		N_CARRY     = 0
	};

	COutputMem &mem;

	UINT16 mem_mask;		// maska pamiêci - zale¿na od szerokoœci szyny adresowej,
							// zawiera jedynki w miejscu wa¿nych bitów adresów
	bool io;
	UINT16 io_from, io_to;

	// funkcje zmieniaj¹ce zawartoœæ rejestru flagowego
	void set_status_reg_VZNC(bool v, bool z, bool n, bool c)
	{ negative=!!n; overflow=!!v; zero=!!z; carry=!!c; }
	void set_status_reg_ZNC(bool z, bool n, bool c)
	{ negative=!!n; zero=!!z; carry=!!c; }
	void set_status_reg_VZN(bool v, bool z, bool n)
	{ negative=!!n; overflow=!!v; zero=!!z; }
	void set_status_reg_ZN(bool z, bool n)
	{ negative=!!n; zero=!!z; }
	void set_status_reg(UINT8 val)
	{ zero = val==0;  negative = !!(val & 0x80); }

	UINT8 get_status_reg() const;
	void set_status_reg_bits(UINT8 reg);
	void set_addr_bus_width(UINT16 w)
	{
		mem_mask = UINT16((1 << w) - 1);
		mem.SetMask(mem_mask);
		ASSERT(w >= 10 && w <= 16);
	}

	CContext(COutputMem &mem, int addr_bus_width) : mem(mem)
	{
		set_addr_bus_width(addr_bus_width);
		reset();
	}

	CContext(const CContext &src) : mem(src.mem)
	{ *this = src; }

	CContext &operator= (const CContext &src)
	{ memcpy(this,&src,sizeof *this); return *this; }

	void Reset(const COutputMem &mem)
	{
		this->mem = mem;
		reset();
	}
};


struct CmdInfo	// single command info (for logging)
{
	CmdInfo(const CContext& ctx)
	{
		a = ctx.a;
		x = ctx.x;
		y = ctx.y;
		s = ctx.s;
		flags = ctx.get_status_reg();
		pc = ctx.pc;
		cmd = ctx.mem[ctx.pc];
		arg1 = ctx.mem[ctx.pc + 1];
		arg2 = ctx.mem[ctx.pc + 2];
		argVal = 0;
	}

	CmdInfo(UINT8 a, UINT8 x, UINT8 y, UINT8 s, UINT8 flags, UINT8 cmd, UINT8 arg1, UINT8 arg2, UINT16 pc)
		: a(a), x(x), y(y), s(s), flags(flags), pc(pc), cmd(cmd), arg1(arg1), arg2(arg2)
	{
		argVal = 0;
	}

	CmdInfo() {}

	CString Asm() const;

	UINT8 a;
	UINT8 x;
	UINT8 y;
	UINT8 s;
	UINT8 flags;
	UINT8 cmd;
	UINT8 arg1;
	UINT8 arg2;
	UINT16 pc;
	UINT16 argVal;
};

typedef CLogBuffer<CmdInfo> CommandLog;

//=============================================================================

class CSym6502 : public CAsm
{
	CContext ctx, old;
	CDebugInfo *debug;
	CommandLog m_Log;
public:
	static int bus_width;
	static UINT16 io_addr;	// pocz¹tek obszaru we/wy symulatora
	static bool io_enabled;
	static bool s_bWriteProtectArea;
	static UINT16 s_uProtectFromAddr;
	static UINT16 s_uProtectToAddr;

	enum IOFunc			// funkcje kolejnych bajtów z obszaru we/wy symulatora
	{ 
		IO_NONE      = -1,
		TERMINAL_CLS = 0,
		TERMINAL_OUT,
		TERMINAL_OUT_CHR,
		TERMINAL_OUT_HEX,
		TERMINAL_IN,
		TERMINAL_GET_X_POS,
		TERMINAL_GET_Y_POS,
		TERMINAL_SET_X_POS,
		TERMINAL_SET_Y_POS,
		IO_LAST_FUNC= TERMINAL_SET_X_POS
	};

	// interrupt types
	enum IntType { NONE= 0, IRQ= 1, NMI= 2, RST= 4 };

private:
	IOFunc io_func;
	CWnd *io_window();		// odszukanie okna terminala
	CWnd *io_open_window();	// otwarcie okna terminala

	void inc_prog_counter(int step= 1)  { ctx.pc = UINT16(ctx.pc + step); }

	bool running;
	bool stop_prog;
	SymStat fin_stat;
	int m_nInterruptTrigger;

	SymStat perform_cmd();
	SymStat skip_cmd();		// ominiêcie bie¿¹cej instrukcji
	SymStat step_over();
	SymStat run_till_ret();
	SymStat run(bool animate= false);
	void interrupt(int& nInterrupt);	// interrupt requested: load pc
	SymStat perform_step(bool animate);
	SymStat perform_command();

	UINT16 get_argument_address(bool bWrite);	// get current cmd argument address
	UINT8 get_argument_value();					// get current cmd argument value

	void push_on_stack(UINT8 arg)
	{ ctx.mem[0x100 + ctx.s--] = arg; }

	void push_addr_on_stack(UINT16 arg)
	{
		ctx.mem[0x100 + ctx.s--] = (arg>>8) & 0xFF;
		ctx.mem[0x100 + ctx.s--] = arg & 0xFF;
	}

	UINT8 pull_from_stack()
	{ return ctx.mem[0x100 + ++ctx.s]; }

	UINT16 pull_addr_from_stack()
	{
		UINT16 tmp= ctx.mem[0x100 + ++ctx.s];
		tmp |= UINT16(ctx.mem[0x100 + ++ctx.s]) << UINT16(8);
		return tmp;
	}

	UINT16 get_irq_addr();

	static UINT start_step_over_thread(LPVOID ptr);
	static UINT start_run_thread(LPVOID ptr);
	static UINT start_animate_thread(LPVOID ptr);
	static UINT start_run_till_ret_thread(LPVOID ptr);
	HANDLE hThread;

	void SetPointer(const CLine &line, UINT16 addr);	// ustawienie strza³ki (->) przed aktualnym wierszem
	void SetPointer(CSrc6502View* pView, int nLine, bool bScroll); // helper fn
	void ResetPointer();			// schowanie strza³ki
	CSrc6502View *FindDocView(FileUID fuid);	// odszukanie okna dokumentu
	FileUID m_fuidLastView;			// zapamiêtanie okna, w którym narysowana jest strza³ka
	HWND m_hwndLastView;				// j.w.
	void AddBranchCycles(UINT8 arg);

	CEvent eventRedraw;			// synchronizacja odœwie¿ania okna przy animacji

	void init();
	void set_translation_tables();

	bool check_io_write(UINT16 addr);
	bool check_io_read(UINT16 addr);

	SymStat io_function(UINT8 arg);
	UINT8 io_function();

	const UINT8* m_vCodeToCommand;
	const UINT8* m_vCodeToCycles;
	const UINT8* m_vCodeToMode;

public:
	Finish finish;		// okreœlenie sposobu zakoñczenia wykonania programu
	UINT16 get_rst_addr();
	UINT16 get_nmi_addr();
	void Update(SymStat stat, bool no_ok= false);
	CString GetStatMsg(SymStat stat);
	CString GetLastStatMsg();
	SymStat SkipInstr();
	void SkipToAddr(UINT16 addr);
	void set_addr_bus_width(UINT w)
	{}
	UINT16 get_pc()
	{ return ctx.pc; }
	void set_pc(UINT16 pc)
	{ ctx.pc = pc; }

	void SetContext(CContext &context)
	{ ctx = context; }
	const CContext* GetContext()
	{ return &ctx; }

	CSym6502(COutputMem &mem, int addr_bus_width) : ctx(mem,addr_bus_width), old(ctx),
		eventRedraw(true,true)
	{ init(); }
	CSym6502(COutputMem &mem, CDebugInfo *debug, int addr_bus_width) : ctx(mem,addr_bus_width),
		old(ctx), eventRedraw(true), debug(debug)
	{ init(); }

	void Restart(const COutputMem &mem);
	void SymStart(UINT16 org);

	SymStat StepInto();
	SymStat StepOver();
	SymStat RunTillRet();
	SymStat Run();
	SymStat Animate();
	SymStat Interrupt(IntType eInt);

	bool IsFinished() const				{ return fin_stat==SYM_FIN; }
	bool IsRunning() const				{ return running; }
	void Break()						{ stop_prog = true; }
	bool IsBroken() const				{ return stop_prog; }
	void AbortProg();
	void ExitSym();

	void ClearCyclesCounter();

	const CommandLog& GetLog() const	{ return m_Log; }
};

#endif
