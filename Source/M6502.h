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

#include "Asm.h"
#include "Ident.h"
#include "DebugInfo.h"
#include "OutputMem.h"


class CLeksem : public CAsm
{
public:
/*
  enum IdentInfo	// info o identyfikatorze
  {
    I_UNDEF,		// identyfikator niezdefiniowany
    I_ADDRESS,		// identyfikator zawiera adres
    I_VALUE		// identyfikator zawiera wartoœæ liczbow¹
  };

  struct Ident
  {
    IdentInfo info;
    int value;
    CString *str;
  };
*/

  struct Code
  {
    OpCode code;
    CodeAdr adr;
  };


  enum InstrArg		// rodzaj argumentów dyrektywy
  {
    A_BYTE,
    A_NUM,
    A_LIST,
    A_STR,
    A_2NUM
  };

  struct Instr
  {
    InstrType type;
    InstrArg arg;
  };


  enum NumType		// rodzaj liczby
  {
    N_DEC,
    N_HEX,
    N_BIN,
    N_CHR,	// znak
    N_CHR2	// dwa znaki, np. 'ab'
  };

  struct Num
  {
    NumType type;
    SINT32 value;
  };

  enum Error
  {
    ERR_NUM_HEX,
    ERR_NUM_DEC,
    ERR_NUM_BIN,
    ERR_NUM_CHR,
    ERR_NUM_BIG,	// b³¹d przekroczenia zakresu
    ERR_BAD_CHR,
    ERR_STR_UNLIM	// niezamkniêty ³añcuch znaków
  };

  //...........................................................................


  enum LeksType
  {
    L_UNKNOWN,		// nierozpoznany znak

    L_NUM,		// liczba (dec, hex, bin, lub znak)
    L_STR,		// ci¹g znaków w apostrofach lub cudzys³owach
    L_IDENT,		// identyfikator
    L_IDENT_N,		// numerowany identyfikator (zakoñczony przez '#' i numer)
    L_SPACE,		// odstêp
    L_OPER,		// operator
    L_BRACKET_L,	// lewy nawias '('
    L_BRACKET_R,	// prawy nawias ')'
    L_EXPR_BRACKET_L,	// lewy nawias dla wyra¿eñ '['
    L_EXPR_BRACKET_R,	// prawy nawias dla wyra¿eñ ']'
    L_COMMENT,		// znak komentarza ';'
    L_LABEL,		// znak etykiety ':'
    L_COMMA,		// znak przecinka ','
    L_STR_ARG,		// znak dolara '$', koñczy parametr typu tekstowego
    L_MULTI,		// znak wielokropka '...'
    L_INV_COMMAS,	// znak cudzys³owu
    L_HASH,		// znak '#'
    L_EQUAL,		// znak przypisania '='
    L_PROC_INSTR,	// instrukcja procesora
    L_ASM_INSTR,	// dyrektywa asemblera
    L_CR,		// koniec wiersza

    L_FIN,		// koniec danych
    L_ERROR		// b³¹d
  };

  const LeksType type;

  class CLString : public CString
  {
    static char* s_ptr_1;
    static char* s_ptr_2;
    static char s_buf_1[];
    static char s_buf_2[];
    static const size_t s_cnMAX;
  public:
    CLString() : CString()
    {}
    CLString(const CString& str) : CString(str)
    {}
    CLString(const TCHAR* ptr, int size) : CString(ptr,size)
    {}
#ifdef _DEBUG
    void* operator new(size_t size, LPCSTR /*lpszFileName*/, int /*nLine*/)
#else
    void* operator new(size_t size)
#endif
    {
      if (size > s_cnMAX)
	return CObject::operator new(size);
      else if (s_ptr_1 == NULL)
	return s_ptr_1 = s_buf_1;
      else if (s_ptr_2 == NULL)
	return  s_ptr_2 = s_buf_2;
      else
	return CObject::operator new(size);
    }
    void operator delete(void* ptr)
    {
      if (ptr == s_ptr_1)
	s_ptr_1 = NULL;
      else if (ptr == s_ptr_2)
	s_ptr_2 = NULL;
      else
	CObject::operator delete(ptr);
    }
  };

private:

  union
  {
//    Ident id;		// identyfikator
    OperType op;	// operator binarny lub unarny
    OpCode code;	// mnomonik
    InstrType instr;	// dyrektywa
//    int val;		// sta³a liczbowa lub znakowa
    Num num;		// sta³a liczbowa lub znakowa
    CLString *str;	// identyfikator lub ci¹g znaków
    Error err;
  };

public:
  CLeksem(const CLeksem &leks);

  ~CLeksem();

  CLeksem(LeksType type) : type(type)
  { }
  CLeksem(OperType oper) : type(L_OPER), op(oper)
  { }
  CLeksem(OpCode code) : type(L_PROC_INSTR), code(code)
  { }
  CLeksem(InstrType it) : type(L_ASM_INSTR), instr(it)
  { }
  CLeksem(Error err) : type(L_ERROR), err(err)
  { }
  CLeksem(NumType type, SINT32 val) : type(L_NUM)
  { num.type=type;  num.value=val; }
  CLeksem(CLString *str) : type(L_STR), str(str)
  { }
  CLeksem(CLString *str, int dummy) : type(L_IDENT), str(str)
  { }
  CLeksem(CLString *str, long dummy) : type(L_IDENT_N), str(str)
  { }
//  CLeksem(const CLeksem &leks, long dummy) : type(L_IDENT_N)
//  { ASSERT(leks.type == L_IDENT);  str = new CString(*leks.str); }

  CLeksem & operator=(const CLeksem &);

  const CString *GetIdent() const
  {
    ASSERT(type==L_IDENT);
    return str;
  }

  OperType GetOper()
  {
    ASSERT(type==L_OPER);
    return op;
  }

  OpCode GetCode()
  {
    ASSERT(type==L_PROC_INSTR);
    return code;
  }

  InstrType GetInstr()
  {
    ASSERT(type==L_ASM_INSTR);
    return instr;
  }

  int GetValue()
  {
    ASSERT(type==L_NUM);
    return num.value;
  }

  const CString *GetString() const
  {
    ASSERT(type==L_STR || type==L_IDENT || type==L_IDENT_N);
    return str;
  }

  void Format(SINT32 val)	// znormalizowanie postaci etykiety numerycznej
  {
    ASSERT(type==L_IDENT_N);
    CString num(' ',9);
    num.Format("#%08X",(int)val);
    *str += num;		// do³¹czenie numeru
  }
};

//-----------------------------------------------------------------------------

// tablica asocjacyjna identyfikatorów
class CIdentTable : public CMap<CString, LPCTSTR, CIdent, CIdent>
{

public:
  CIdentTable(int nSize= 500) : CMap<CString, LPCTSTR, CIdent, CIdent>(nSize)
  { InitHashTable(1021); }
  ~CIdentTable()
  { }

  bool insert(const CString &str, CIdent &ident);
  bool replace(const CString &str, const CIdent &ident);

  bool lookup(const CString &str, CIdent &ident) const
  { return Lookup(str,ident); }

  void clr_table()
  { RemoveAll(); }

};

//=============================================================================

class CInputBase	// klasa bazowa dla klas odczytu danych Ÿród³owych
{
protected:
  int m_nLine;
  CString m_strFileName;
  bool m_bOpened;

public:
  CInputBase(const TCHAR *str= NULL) : m_strFileName(str)
  { m_nLine = 0;  m_bOpened = false; }

  virtual ~CInputBase()
  { ASSERT(m_bOpened==false); }

  virtual void open()
  { m_bOpened = true; }

  virtual void close()
  { m_bOpened = false; }

  virtual void seek_to_begin()
  { m_nLine = 0; }

  virtual LPTSTR read_line(LPTSTR str, UINT max_len)= 0;

  virtual int get_line_no()
  { return m_nLine-1; }		// numeracja wierszy od 0

  virtual const CString &get_file_name()
  { return CInputBase::m_strFileName; }
};

//-----------------------------------------------------------------------------

class CInputFile : public CInputBase, CStdioFile
{
public:
  CInputFile(const CString &str) : CInputBase(str)
  { }

  ~CInputFile()
  { }

  virtual void open()
  {
    ASSERT(m_bOpened==false);	    // plik nie mo¿e byæ jeszcze otwarty
    CFileException *ex= new CFileException;
    if (!Open(CInputBase::m_strFileName, CFile::modeRead|CFile::shareDenyWrite|CFile::typeText, ex))
      throw (ex);
    m_bOpened = true;
    /*delete*/ ex->Delete();
  }

  virtual void close()
  {
    ASSERT(m_bOpened==true);	    // plik musi byæ otworzony
    Close();
    m_bOpened = false;
  }

  virtual void seek_to_begin()
  {
    SeekToBegin();
    m_nLine = 0;
  }

  virtual LPTSTR read_line(LPTSTR str, UINT max_len);

//  virtual int get_line_no()

//  virtual const CString &get_file_name()
};

//-----------------------------------------------------------------------------

class CInputWin : public CInputBase, public CAsm	// odczyt danych z okna dokumentu
{
  CWnd *m_pWnd;
public:
  CInputWin(CWnd *pWnd) : m_pWnd(pWnd)
  { }
  virtual LPTSTR read_line(LPTSTR str, UINT max_len);
  virtual const CString &get_file_name();
  virtual void seek_to_begin();
};

//-----------------------------------------------------------------------------

class CInput : CList <CInputBase *, CInputBase *>, CAsm
{
  CInputBase *tail;
  FileUID fuid;

  int calc_index(POSITION pos);

public:
  void open_file(const CString &fname);
  void open_file(CWnd *pWin);
  void close_file();

  CInput(const CString &fname) : fuid(0)
  { open_file(fname); }
  CInput(CWnd *pWin) : fuid(0)
  { open_file(pWin); }
  CInput() : fuid(0), tail(NULL)
  { }
  ~CInput();

  LPTSTR read_line(LPTSTR str, UINT max_len)
  { return tail->read_line(str,max_len); }

  void seek_to_begin()
  { tail->seek_to_begin(); }

  int get_line_no()
  { return tail->get_line_no(); }

  int get_count()
  { return GetCount(); }

  const CString &get_file_name()
  { return tail->get_file_name(); }

  FileUID get_file_UID()
  { return fuid; }

  void set_file_UID(FileUID fuid)
  { this->fuid = fuid; }

  bool is_present()
  { return tail != NULL; }
};

//-----------------------------------------------------------------------------

struct Expr		 // klasa do opisu wyra¿enia arytmetycznego/logicznego/tekstowego
{
  SINT32 value;
  CString string;
  enum
  {
    EX_UNDEF,		// wartoœæ nieznana
    EX_BYTE,		// bajt, tj. od -255 do 255 (sic!)
    EX_WORD,		// s³owo, od -65535 do 65535
    EX_LONG,		// poza powy¿szym zakresem
    EX_STRING		// ci¹g znaków
  } inf;

  Expr() : inf(EX_UNDEF)
  { }
  Expr(SINT32 value) : inf(EX_LONG), value(value)
  { }
};

//-----------------------------------------------------------------------------

  // elementy wspólne odczytu wierszy dla .MACRO, .REPEAT i normalnego odczytu
class CSource : public CObject, public CAsm
{
  FileUID m_fuid;

public:
  CSource() : m_fuid(0)
  { }
  virtual ~CSource()
  { }

  virtual void Start()			// rozpoczêcie odczytu wierszy
  { }
  virtual void Fin()			// zakoñczenie odczytu wierszy
  { }
  virtual const TCHAR* GetCurrLine(CString &str)= 0;	// odczyt bie¿¹cego wiersza
  virtual int GetLineNo()= 0;		// odczyt numeru wiersza
  virtual FileUID GetFileUID()		// odczyt ID pliku
  { return m_fuid; }
  void SetFileUID(FileUID fuid)		// ustawienie ID pliku
  { m_fuid = fuid; }
  virtual const CString &GetFileName()	= 0;	// nazwa aktualnego pliku

//  virtual bool IsMacro()		// czy Ÿród³em danych jest rozwijane makro?
//  { return false; }
//  virtual bool IsRepeat()		// czy Ÿród³em danych jest powtórzenie (.REPEAT)?
//  { return false; }

  static const CString s_strEmpty;
};

//.............................................................................

struct CStrLines : public CStringArray
{
  CStrLines(int nInitSize, int nGrowBy)
  { SetSize(nInitSize,nGrowBy); }
};


  // elementy wymagane do zapamiêtywania i odtwarzania wierszy Ÿród³owych programu
class CRecorder
{
  CStrLines m_strarrLines;
  CDWordArray m_narrLineNums;
  int m_nLine;
public:
  CRecorder(int nInitSize=10, int nGrowBy=10) :	// pocz¹tkowe rozmiary tablic
    m_strarrLines(nInitSize,nGrowBy), m_nLine(0)
  {
    m_narrLineNums.SetSize(nInitSize,nGrowBy);
  }
  virtual ~CRecorder()
  { }

  void AddLine(const CString &strLine, int num)	// zapamiêtanie kolejnego wiersza
  { m_strarrLines.SetAtGrow(m_nLine,strLine); m_narrLineNums.SetAtGrow(m_nLine,num); m_nLine++; }

  const CString& GetLine(int nLineNo)	// odczyt wiersza 'nLineNo'
  { return m_strarrLines[nLineNo]; }

  int GetLineNo(int nLineNo)	// odczyt numeru wiersza w pliku Ÿród³owym
  { return m_narrLineNums[nLineNo]; }

  int GetSize()			// odczyt iloœci wierszy w tablicy
  {
//    ASSERT( m_narrLineNums.GetSize() == m_narrLineNums.GetSize() );
    return m_nLine;
  }

};

//.............................................................................
/*
  // obs³uga tablicy identyfikatorów etykiet lokalnych
class CLabels : CIdentTable
{

};
*/

//-----------------------------------------------------------------------------

class CAsm6502;

class CMacroDef : public CSource, public CRecorder
{
  CIdentTable param_names;	// tablica nazw parametrów makra
  int m_nParams;		// iloœæ parametrów wymagana
  int m_nParamCount;		// iloœæ parametrów w wywo³aniu makra
  CStringArray m_strarrArgs;	// kolejne argumenty wywo³ania - tylko ³añcuchy znaków
  CDWordArray m_narrArgs;	// kolejne argumenty wywo³ania - tylko wartoœci wyra¿eñ
  enum ArgType { NUM, STR, UNDEF_EXPR };
  CByteArray m_arrArgType;	// typy argumentów (NUM - liczba, STR - ³añcuch znaków)
  int m_nLineNo;		// nr aktualnego wiersza (przy odczycie)
  int m_nFirstLineNo;		// numer wiersza, z którego wywo³ywane jest makro
  FileUID m_nFirstLineFuid;	// ID pliku, z którego wywo³ywane jest makro
public:
  CString m_strName;		// nazwa makra
  bool m_bFirstCodeLine;	// flaga odczytu pierwszego wiersza makra zawieraj¹cego instr. 6502

  CMacroDef() : param_names(2), m_nParams(0), m_nLineNo(0), m_nFirstLineNo(-1),
    m_nFirstLineFuid(FileUID(-1)), m_bFirstCodeLine(true)
  { }
  ~CMacroDef()
  { }

  int AddParam(const CString &strParam)	// dopisanie nazwy kolejnego parametru
  {
    if (strParam.Compare(MULTIPARAM) == 0)
    {
      m_nParams = -(m_nParams + 1);
      return 1;		// koniec listy parametrów
    }
    if (!param_names.insert(strParam,CIdent(CIdent::I_VALUE,m_nParams)))
      return -1;	// powtórzona nazwa parametru!
    m_nParams++;
    return 0;
  }

  int GetParamsFormat()			// iloœæ przekazywanych parametrów
  { return m_nParams; }

  virtual const TCHAR* GetCurrLine(CString &str);	// odczyt aktualnego wiersza makra

  virtual int GetLineNo()
  { return CRecorder::GetLineNo(m_nLineNo-1); }

  int GetFirstLineNo()
  { return m_nFirstLineNo; }

  FileUID GetFirstLineFileUID()
  { return m_nFirstLineFuid; }

  void Start(int line, FileUID file)	// przygotowanie do odczytu
  { m_nLineNo = 0; m_nFirstLineNo = line; m_nFirstLineFuid = file; }
  virtual void Fin()
  { }					// zakoñczenie rozwijania bie¿¹cego makra

    // wczytanie argumentów wywo³ania
  CAsm::Stat ParseArguments(CLeksem &leks, CAsm6502 &asmb);

  CAsm::Stat ParamLookup(CLeksem &leks, const CString param_name, Expr &expr, bool &found, CAsm6502 &asmb);
  CAsm::Stat ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb);
  CAsm::Stat AnyParamLookup(CLeksem &leks, CAsm6502 &asmb);

//  virtual bool IsMacro()		// Ÿród³em danych jest rozwijane makro
//  { return true; }

  CMacroDef &operator= (const CMacroDef &src)
  {
    ASSERT(false);	// nie wolno przypisywaæ obiektów typu CMacroDef
    return *this;
  }

  virtual const CString &GetFileName()	// nazwa aktualnego pliku
  { return s_strEmpty; }
};


class CMacroDefs : public CArray<CMacroDef, CMacroDef&>
{
};

//-----------------------------------------------------------------------------

class CRepeatDef : public CSource, public CRecorder
{
  int m_nLineNo;		// nr aktualnego wiersza (przy odczycie)
  int m_nRepeat;		// iloœæ powtórzeñ wierszy
public:

  CRepeatDef(int nRept= 0) : m_nLineNo(0), m_nRepeat(nRept)
  { }
  ~CRepeatDef()
  { }

  virtual const TCHAR* GetCurrLine(CString &str);	// odczyt aktualnego wiersza

  virtual int GetLineNo()
  { return CRecorder::GetLineNo(m_nLineNo-1); }

  virtual void Start()
  { m_nLineNo = GetSize(); }		// licznik wierszy na koniec
  virtual void Fin()			// zakoñczenie powtórki wierszy
  { delete this; }

//  virtual bool IsRepeat()		// Ÿród³em danych jest powtórzenie (.REPEAT)
//  { return true; }

  CRepeatDef &operator= (const CRepeatDef &src)
  {
    ASSERT(false);	// nie wolno przypisywaæ obiektów typu CRepeatDef
    return *this;
  }

  virtual const CString &GetFileName()	// nazwa aktualnego pliku
  { return s_strEmpty; }
};


class CRepeatDefs : public CArray<CRepeatDef, CRepeatDef&>
{
};

//-----------------------------------------------------------------------------

class CSourceText : public CSource
{
  CInput input;
public:
  CSourceText(const CString &file_in_name) : input(file_in_name)
  { }
  CSourceText(CWnd *pWnd) : input(pWnd)
  { }
  CSourceText()
  { }

  virtual void Start()			// rozpoczêcie odczytu wierszy
  { input.seek_to_begin(); }

  virtual const TCHAR* GetCurrLine(CString &str)// odczyt bie¿¹cego wiersza
  {
    const TCHAR *ret= input.read_line(str.GetBuffer(1024+4), 1024+4);
    str.ReleaseBuffer(-1);
    return ret;
  }

  virtual int GetLineNo()		// odczyt numeru wiersza
  { return input.get_line_no(); }

  FileUID GetFileUID()			// odczyt ID pliku
  { return input.get_file_UID(); }

  void SetFileUID(CDebugInfo* pDebugInfo)
  {
    if (pDebugInfo)
      input.set_file_UID(pDebugInfo->GetFileUID(input.get_file_name()));
  }

  void Include(const CString &fname, CDebugInfo* pDebugInfo= NULL)	// w³¹czenie pliku
  {
    input.open_file(fname);
    if (pDebugInfo)
      input.set_file_UID(pDebugInfo->GetFileUID(fname));
  }

  bool TextFin()			// zakoñczony bie¿¹cy plik
  {
    if (input.get_count() > 1)	// zagnie¿d¿ony odczyt (.include) ?
    {
      input.close_file();
      return true;
    }
    else
      return false;	// koniec plików Ÿród³owych
  }
/*
  bool IsPresent()			// spr. czy jest jakiœ odczytywany plik
  { return input.is_present(); }
*/
  virtual const CString &GetFileName()	// nazwa aktualnego pliku
  { return input.get_file_name(); }
};


//-----------------------------------------------------------------------------


class CSourceStack : CTypedPtrArray<CObArray,CSource *>	// Stos obiektów bêd¹cych Ÿród³em wierszy
{
  int m_nIndex;
public:
  CSourceStack()
  { m_nIndex = -1; }
  ~CSourceStack()
  {
    for (int i=m_nIndex; i>=0; i--)
      GetAt(i)->Fin();
  }

  void Push(CSource *pSrc)		// Dodanie elementu na wierzcho³ku stosu
  { ++m_nIndex; SetAtGrow(m_nIndex,pSrc); }

  CSource *Peek()			// Sprawdzenie elementu na szczycie stosu
  { return m_nIndex < 0 ? NULL : GetAt(m_nIndex); }

  CSource *Pop()			// Zdjêcie elementu ze stosu
  { return GetAt(m_nIndex--); }

//  void RemoveAll()
//  { RemoveAll();  m_nIndex = -1; }

  CSource* FindMacro()			// odszukanie ostatniego makra
  {
    for (int i=m_nIndex; i>=0; i--)
      if (CMacroDef* pSrc= dynamic_cast<CMacroDef*>(GetAt(i)))
        return pSrc;
    return NULL;
  }
  CSource* FindRepeat()			// odszukanie ostatniego powtórzenia
  {
    for (int i=m_nIndex; i>=0; i--)
      if (CRepeatDef* pSrc= dynamic_cast<CRepeatDef*>(GetAt(i)))
	return pSrc;
//      if (GetAt(i)->IsRepeat())
//        return GetAt(i);
    return NULL;
  }
};


//-----------------------------------------------------------------------------
class CMarkArea;

class CAsm6502 : public CAsm, public CObject
{
	friend class CMacroDef;

	CString current_line;
	const TCHAR *ptr;				// do œledzenia aktualnego wiersza
	const TCHAR *err_start;
	const TCHAR *ident_start;		// po³o¿enie identyfikatora w wierszu
	const TCHAR *ident_fin;			// po³o¿enie koñca identyfikatora w wierszu

	bool check_line;				// flaga: true - analiza jednego wiersza, false - programu
	UINT32 origin;
	bool originWrapped;				// true - jeœli licznik rozkazów "przewin¹³ siê"
	UINT32 program_start;			// pocz¹tek programu
	UINT32 mem_mask;				// granica pamiêci procesora (maska), normalnie $FFFF
	int local_area;					// nr obszaru etykiet lokalnych
	int macro_local_area;			// nr obszaru etykiet lokalnych makrodefinicji
	int pass;						// numer przejœcia (1 lub 2)
	int conditional;				// asemblacja warunkowa - poziom zag³êbienia
	CString include_fname;
	CString user_error_text;		// tekst b³êdu u¿ytkownika (dyrektywy .ERROR)
	const TCHAR *instr_start;		// do zapamiêtania pocz¹tku
	const TCHAR *instr_fin;			// i koñca instrukcji w wierszu
	CMacroDefs macros;				// makrodefinicje

	CSource *text;					// bie¿¹cy tekst Ÿród³owy
	CSourceText entire_text;		// pierwszy (pocz¹tkowy) tekst Ÿród³owy
	CRepeatDef *pRept;

	// leksyka:
	CLeksem get_dec_num();			// interpretacja liczby dziesiêtnej
	CLeksem get_hex_num();			// interpretacja liczby szesnastkowej
	CLeksem get_bin_num();			// interpretacja liczby dwójkowej
	CLeksem get_char_num();			// interpretacja sta³ej znakowej
//  CLeksem get_ident();			// wyodrêbnienie napisu
	CLeksem::CLString* get_ident();	// wyodrêbnienie napisu
	CLeksem get_string(TCHAR lim);	// wyodrêbnienie ³añcucha znaków
	CLeksem eat_space();			// ominiêcie odstêpu
	bool proc_instr(const CString &str, OpCode &code);
	bool asm_instr(const CString &str, InstrType &it);
	CLeksem next_leks(bool nospace= true);	// pobranie kolejnego symbolu
	bool next_line();				// wczytanie kolejnego wiersza

	COutputMem *out;				// pamiêæ dla kodu wynikowego
	CMarkArea *markArea;			// do zaznaczania u¿ytych obszarów pamiêci z 'out'
	CIdentTable local_ident;		// tablica identyfikatorów lokalnych
	CIdentTable global_ident;		// tablica identyfikatorów globalnych
	CIdentTable macro_name;			// tablica nazw makrodefinicji
	CIdentTable macro_ident;		// tablica identyfikatorów w makrorozwiniêciach
	CDebugInfo *debug;				// informacja uruchomieniowa dla symulatora

//	CString err_file;				// nazwa pliku, którego odczyt spowodowa³ b³¹d
//	int err_line;					// nr wiersza, w którym napotkano b³¹d
	CString err_ident;				// nazwa etykiety, która spowodowa³a b³¹d

	bool temporary_out;				// flaga - 'out' zosta³o alokowane w konstruktorze
	bool abort_asm;					// zmienna do wymuszenia przerwania asemblacji z zewn¹trz
	bool is_aborted()
	{ return abort_asm ? abort_asm=false, true : false; }

	bool add_ident(const CString &ident, CIdent &inf);
	Stat def_ident(const CString &ident, CIdent &inf);
	Stat chk_ident(const CString &ident, CIdent &inf);
	Stat chk_ident_def(const CString &ident, CIdent &inf);
	Stat def_macro_name(const CString &ident, CIdent &inf);
	Stat chk_macro_name(const CString &ident);
	CString format_local_label(const CString &ident, int area);
	// interpretacja instrukcji procesora
	Stat proc_instr_syntax(CLeksem &leks, CodeAdr &mode, Expr &expr, Expr &expr_bit, Expr &expr_zpg);
	// interpretacja dyrektywy
	Stat asm_instr_syntax_and_generate(CLeksem &leks, InstrType it, const CString *pLabel);
	CMacroDef *get_new_macro_entry()
	{ macros.SetSize(macros.GetSize()+1);  return &macros[macros.GetSize()-1]; }
	CMacroDef *get_last_macro_entry()
	{ ASSERT(macros.GetSize() > 0);  return &macros[macros.GetSize()-1]; }
	int get_last_macro_entry_index()
	{ ASSERT(macros.GetSize() > 0);  return macros.GetSize()-1; }
//  Stat (CLeksem &leks);
	int find_const(const CString& str);
	Stat predef_const(const CString &str, Expr &expr, bool &found);
	Stat predef_function(CLeksem &leks, Expr &expr, bool &fn);
	Stat constant_value(CLeksem &leks, Expr &expr, bool nospace);
	Stat factor(CLeksem &leks, Expr &expr, bool nospace= true);
	Stat mul_expr(CLeksem &leks, Expr &expr);
	Stat shift_expr(CLeksem &leks, Expr &expr);
	Stat add_expr(CLeksem &leks, Expr &expr);
	Stat bit_expr(CLeksem &leks, Expr &expr);
	Stat cmp_expr(CLeksem &leks, Expr &expr);
	Stat bool_expr_and(CLeksem &leks, Expr &expr);
	Stat bool_expr_or(CLeksem &leks, Expr &expr);
	Stat expression(CLeksem &leks, Expr &expr, bool str= false);	// interpretacja wyra¿enia
	bool is_expression(const CLeksem &leks);
	Stat assemble_line();			// interpretacja wiersza
	Stat assemble();
	const TCHAR *get_next_line();	// wczytanie kolejnego wiersza do asemblacji
	const TCHAR *play_macro();		// odczyt kolejnego wiersza makra
	const TCHAR *play_repeat();		// odczyt kolejnego wiersza powtórki
//  CPtrStack <CSource> source;		// stos obiektów zwracaj¹cych wiersze Ÿród³owe
	CSourceStack source;			// stos obiektów zwracaj¹cych wiersze Ÿród³owe
	void asm_start();				// rozpoczêcie asemblacji
	void asm_fin();					// zakoñczenie asemblacji
	void asm_start_pass();			// rozpoczêcie przejœcia asemblacji
	void asm_fin_pass();			// zakoñczenie przejœcia asemblacji
	Stat chk_instr_code(OpCode code, CodeAdr &mode, Expr expr, int &length);
	void generate_code(OpCode code, CodeAdr mode, Expr expr, Expr expr_bit, Expr expr_zpg);
	Stat inc_prog_counter(int dist);
	Stat look_for_endif();			// szukanie .ENDIF lub .ELSE
//  int get_line_no();				// numer wiersza (dla debug info)
//  FileUID get_file_UID();			// id pliku (dla debug info)
	void generate_debug(UINT16 addr, int line_no, FileUID file_UID);
	Stat generate_debug(InstrType it, int line_no, FileUID file_UID);
	void generate_debug();
	Stat look_for_endm();
	Stat record_macro();
//  void MacroExpandStart(CMacroDef *pMacro);	// przejœcie do trybu rozwijania makrodefinicji
//  void MacroExpandFin();			// zakoñczenie trybu rozwijania makrodefinicji
	CMacroDef *in_macro;			// aktualnie rejestrowane makro lub NULL
	CMacroDef *expanding_macro;		// aktualnie rozwijane makro lub NULL
//  CPtrStack<CMacroDef> expand_macros;	// lista rozwijanych makr
	CRepeatDef *repeating;			// aktualna powtórka (.REPEAT)
//  CPtrStack<CRepeatDef> repeats;	// lista powtórek
	Stat record_rept(CRepeatDef *pRept);
	Stat look_for_repeat();			// szukanie .ENDR lub .REPEAT
	int reptInit;					// wartoœæ do zainicjowania iloœci powtórzeñ
	int reptNested;					// licznik zagnie¿d¿eñ .REPEAT (przy rejestracji)
//  void RepeatStart(CRepeatDef *pRept);
//  void RepeatFin();
	bool b_listing;

	static int __cdecl asm_str_key_cmp(const void *elem1, const void *elem2);
	struct ASM_STR_KEY
	{
		const TCHAR *str;
		CAsm::InstrType it;
	};

	class CConditionalAsm : public CAsm	// asemblacja warunkowa (automat ze stosem)
	{
	public:
		enum State
		{ BEFORE_ELSE, AFTER_ELSE };

	private:
		CByteArray stack;
		int level;

		State get_state()
		{ ASSERT(level>=0);  return stack.GetAt(level)&1 ? BEFORE_ELSE : AFTER_ELSE; }
		bool get_assemble()
		{ ASSERT(level>=0);  return stack.GetAt(level)&2 ? true : false; }
		bool get_prev_assemble()
		{ ASSERT(level>0);  return stack.GetAt(level-1)&2 ? true : false; }
		void set_state(State state, bool assemble)
		{ stack.SetAtGrow(level,BYTE((state==BEFORE_ELSE ? 1 : 0) + (assemble ? 2 : 0))); }
	public:
		CConditionalAsm() : level(-1)
		{ stack.SetSize(16,16); }
		Stat instr_if_found(Stat condition);
		Stat instr_else_found();
		Stat instr_endif_found();
		bool in_cond()
		{ return level>=0; }
	} conditional_asm;


	class CListing
	{
		CStdioFile m_File;	// wsk. do pliku z listingiem
		CString m_Str;		// bie¿¹cy wiersz listingu
		int m_nLine;		// bie¿¹cy wiersz

		void Open(const TCHAR *fname)
		{ m_File.Open(fname,CFile::modeCreate|CFile::modeWrite|CFile::typeText); }
		void Close()
		{ m_File.Close(); }
	public:
		CListing()
		{ m_nLine = -1; }
		CListing(const TCHAR *fname);
		~CListing()
		{ if (m_nLine != -1) Close(); }

		void Remove();
		void NextLine();
		void AddCodeBytes(UINT16 addr, int code1= -1, int code2= -1, int code3= -1);
		void AddValue(UINT16 val);
		void AddBytes(UINT16 addr, UINT16 mask, const UINT8 mem[], int len);
		void AddSourceLine(const TCHAR *line);

		bool IsOpen()
		{ return m_nLine != -1; }
	} listing;

	void init();
	void init_members();

public:
	bool bProc6502;						// true -> 6502, false -> 65C02
	static bool case_insensitive;		// true -> ma³e i du¿e litery w etykietach nie s¹ rozró¿niane
	static bool generateBRKExtraByte;	// generowaæ dodatkowy bajt za instrukcj¹ BRK?
	static UINT8 BRKExtraByte;			// wartoœæ dodatkowego bajtu za instrukcj¹ BRK

	CAsm6502(const CString &file_in_name, COutputMem *out= NULL, CDebugInfo *debug= NULL,
		CMarkArea *area= NULL, bool proc6502= false, const TCHAR *listing_file= NULL) :
		entire_text(file_in_name), out(out), debug(debug), markArea(area), bProc6502(proc6502), listing(listing_file)
	{ init(); }
	CAsm6502(CWnd *pWnd, COutputMem *out= NULL, CDebugInfo *debug= NULL, CMarkArea *area= NULL,
		bool proc6502= false, const TCHAR *listing_file= NULL) :
		entire_text(pWnd), out(out), debug(debug), markArea(area), bProc6502(proc6502), listing(listing_file)
	{ init(); }
	CAsm6502()
	{
		bProc6502 = false;
		init_members();
		temporary_out = false;
		check_line = true;
		text = NULL;
		pRept = NULL;
	}
	~CAsm6502()
	{
		if (temporary_out)
			delete out;
		if (text)
			text->Fin();
		if (pRept)
			pRept->Fin();
	}

	// sprawdzenie sk³adni w wierszu 'str'
	// w 'instr_idx_start' zwracane po³o¿enie instrukcji w wierszu lub 0
	// w 'instr_idx_fin' zwracane po³o¿enie koñca instrukcji w wierszu lub 0
	Stat CheckLine(const TCHAR *str, int &instr_idx_start, int &instr_idx_fin);

	void Abort()
	{ abort_asm = true; }

	CString GetErrMsg(Stat stat);		// opis b³êdu

	Stat Assemble()						// asemblacja
	{ return assemble(); }

	UINT16 GetProgramStart()			// pocz¹tek programu
	{ return program_start; }
};
