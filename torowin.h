/*-----------------------------------------------------------------------------
	Windows 用定義集
-----------------------------------------------------------------------------*/
#define TOROsWEB "http://toro.d.dooo.jp"
#define TOROsWEBPAGE "http://toro.d.dooo.jp/index.html"
#define TOROsMAIL "toroid.jp@gmail.com"
#define REPORTURL T("http://toro.d.dooo.jp/report/receive.cgi")
#define CHECKVERSIONURL T("http://toro.d.dooo.jp/checkver/checkver.cgi")

#ifdef UNICODE
	#define CODETYPESTRING "UNICODE"
#else
	#define CODETYPESTRING "MultiByte"
#endif
#ifdef _WIN64
	#define ValueX3264(value32, value64) value64
	#ifdef _M_ARM64
		#define BITSTRING "ARM64"
	#else
		#define BITSTRING "x64"
	#endif
#else
	#define ValueX3264(value32, value64) value32
	#ifdef _M_ARM
		#define BITSTRING "ARM32"
	#else
		#define BITSTRING "x86"
	#endif
#endif

//--------------------------------------------------------- 定数
#define MAKE_WINTYPE(OSversion) (((OSversion).dwMajorVersion << 16) | (WORD)(OSversion).dwMinorVersion)
//#define WINTYPE_95	0x40000
//#define WINTYPE_98	0x4000a
//#define WINTYPE_98SE	0x4000a
//#define WINTYPE_me	0x4005a
#define WINTYPE_NT4		0x40000
#define WINTYPE_2000	0x50000
#define WINTYPE_XP		0x50001
#define WINTYPE_2003	0x50002
#define WINTYPE_VISTA	0x60000 // 2008
#define WINTYPE_7		0x60001 // 2008R2
#define WINTYPE_8		0x60002 // 2012
#define WINTYPE_81		0x60003 // 2012R2
#define WINTYPE_10		0xa0000 // 10 / 11 / 2022
#define WINTYPE_10_BUILD_TH1	10240 // 1507 LTS
#define WINTYPE_10_BUILD_TH2	10586 // 1511
#define WINTYPE_10_BUILD_RS1	14393 // 1607 LTS Server 2016
#define WINTYPE_10_BUILD_RS2	15063 // 1703
#define WINTYPE_10_BUILD_RS3	16299 // 1709
#define WINTYPE_10_BUILD_RS4	17134 // 1803
#define WINTYPE_10_BUILD_RS5	17763 // 1809 LTS Server 2019
#define WINTYPE_10_BUILD_19H1	18362 // 1903
#define WINTYPE_10_BUILD_19H2	18363 // 1909
#define WINTYPE_10_BUILD_20H1	19041 // 2004
#define WINTYPE_10_BUILD_20H2	19042 // 2009
#define WINTYPE_10_BUILD_21H1	19043 // 2009
#define WINTYPE_10_BUILD_21H2	19044 // 2009 LTS
#define WINTYPE_2022			20348 // Server 2022
#define WINTYPE_11_BUILD_21H1	22000

#define KB		0x400		// 1KiB
#define KBd		1000		// 1KB
#define MB		0x100000	// 1MiB
#define MBd		1000000		// 1MB
#define GB		0x40000000	// 1GiB
#define GBd		1000000000	// 1GB
#define TBdL	0xd4a51000	// 1TB(low dword)
#define TBdH	0xe8		// 1TB(high dword)
#define TBiL	0x00000000	// 1TiB(low dword)
#define TBiH	0x100		// 1TiB(high dword)
#define PBdL	0xa4c68000	// 1PB(low dword)
#define PBdH	0x38d7e		// 1PB(high dword)
#define PBiL	0x00000000	// 1PiB(low dword)
#define PBiH	0x40000		// 1PiB(high dword)
#define MAX16	0xffff
#define MAX32	0xffffFFFF
#ifdef _WIN64
	#define MAX64	0xffffFFFFffffFFFF
#endif
#define BADPTR ((void *)(DWORD_PTR)-1)

#define FFD		0xffffffff
//#define FFW		0xffff
#define NL		"\r\n"

#define EC_LAST 0x7fffffff // Edit control でカーソルを末尾にする定数
#define KEYSTATE_PUSH B15 // GetKeyState,GetAsyncKeyState でキーを押している状態
#define KEYSTATE_FULLPUSH (B15 | B0) // GetKeyState,GetAsyncKeyState でキーを押している状態(別フォーカスの場合でも有効)
#define BADATTR MAX32

#define LCID_ENGLISH	0x409
#define LCID_JAPANESE	0x411
#define CP__US		437 // IBM437 常に使用可能。※WinNT4 JP では列挙できない
#define CP__SJIS	932 // shift_jis 2000未満, Win9xではJP版のみ使用可
#define CP__UTF16L	1200 // utf-16 little endian .net のみ
#define CP__UTF16B	1201 // unicodeFFFE big endian .net のみ
#define CP__LATIN1	1252 // iso-8859-1 常に使用可
#define CP__JIS		50220 // ISO-2022 JIS X 0202-1984 .net のみ
#define CP__JISSI	50222 // iso-2022-jp JIS X 0202-1984 (SI/SO有り) .net のみ
#define CP__EUCJP	51932 // euc-jp .net のみ
#define CP__NOPREC	50220 // CP__JIS, 50221, CP__JISSI, 50225, 50227, 50229 ,570002-11, UTF-7, 42 は MB_PRECOMPOSED が使用できない
// CP_UTF7/CP_UTF8 は NT4 SP?/98SE? から使用可
#define INVALID_VALUE(type) ((type)(LONG_PTR)-1)

#define T_DEFAULTFONTSIZE -13 // 約10point

#define WNDCLASS_POPUPMENU "#32768"
#define WNDCLASS_DESKTOP "#32769"
#define WNDCLASS_DIALOG "#32770"
#define WNDCLASS_TASKSWITCH "#32771"
#define WNDCLASS_ICONTITLES "#32772"

//--------------------------------------------------------- ビット演算用
#define LSBIT	1 // 最下位ビット
#define B0		1
#define B1		2
#define B2		4
#define B3		8
#define B4		0x10
#define B5		0x20
#define B6		0x40
#define B7		0x80
#define B8		0x100
#define B9		0x200
#define B10		0x400
#define B11		0x800
#define B12		0x1000
#define B13		0x2000
#define B14		0x4000
#define B15		0x8000
#define B16		0x10000
#define B17		0x20000
#define B18		0x40000
#define B19		0x80000
#define B20		0x100000
#define B21		0x200000
#define B22		0x400000
#define B23		0x800000
#define B24		0x1000000
#define B25		0x2000000
#define B26		0x4000000
#define B27		0x8000000
#define B28		0x10000000
#define B29		0x20000000
#define B30		0x40000000
#define B31		0x80000000

//--------------------------------------------------------- 基本16+4色
#define C_BLACK		((COLORREF)0x000000)	// black
#define C_RED		((COLORREF)0x0000ff)	// red
#define C_GREEN		((COLORREF)0x00ff00)	// green
#define C_BLUE		((COLORREF)0xFF0000)	// blue
#define C_YELLOW	((COLORREF)0x00FFFF)	// yellow
#define C_CYAN		((COLORREF)0xFFFF00)	// cyan
#define C_MAGENTA	((COLORREF)0xFF00FF)	// magenta
#define C_WHITE		((COLORREF)0xFFFFFF)	// white

#define C_DBLACK	((COLORREF)0x808080)	// medium gray
#define C_DRED		((COLORREF)0x000080)	// dark red
#define C_DGREEN	((COLORREF)0x008000)	// dark green
#define C_DBLUE		((COLORREF)0x800000)	// dark blue
#define C_DYELLOW	((COLORREF)0x008080)	// dark yellow
#define C_DCYAN		((COLORREF)0x808000)	// dark cyan
#define C_DMAGENTA	((COLORREF)0x800080)	// dark megenta
#define C_DWHITE	((COLORREF)0xc0c0c0)	// light gray

#define C_MGREEN	((COLORREF)0xc0dcc0)	// money green
#define C_SBLUE		((COLORREF)0xf0caa6)	// sky blue
#define C_CREAM		((COLORREF)0xf0fbff)	// cream
#define C_GRAY		((COLORREF)0xa4a0a0)	// gray

#define C_AUTO		((COLORREF)MAX32)			// 色自動指定用

// 明るさを計算。算出式はかなり大雑把。
// 0.3 R + 0.6 G + 0.1 B に近似 範囲:0～0x9f6(0xff*10)
#define GetColorBright(color) ((((color) & 0xff) * 3) + ((((color) >> 8) & 0xff) * 6) + (((color) >> 16) & 0xff))

//--------------------------------------------------------- メニュー登録用
					// 普段使う TrackPopupMenu の設定
#define TPM_TDEFAULT (TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON)

#define MF_ES		(MF_ENABLED | MF_STRING)	//	文字列表示/通常
#define MF_GS		(MF_GRAYED | MF_STRING)		//	文字列表示/灰色
#define MF_EPOP		(MF_ENABLED | MF_POPUP)		//	下位階層表示/通常
#define MF_CHKES(func) ((func) ? (MF_ENABLED | MF_STRING | MF_CHECKED) : (MF_ENABLED | MF_STRING))	// チェック付き文字列

#define MB_QYES		(MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1)
#define MB_QNO		(MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON2)

#define GUI_TOUCH_MINSIZE 60 // 表示アイテムのタッチ可能な最低サイズ(6.0mm)
#define GUI_BAR_BUTTON_MINSIZE 85 // ツールバーボタンのタッチ可能な最低サイズ(8.5mm)
#define GUI_BUTTON_MINSIZE 120 // ボタンのタッチ可能な最低サイズ(12.0mm)
// iOS:7mm Android:9mm(7-10mm)
#define GUI_TEXT_MINSIZE 50 // 文字の視認可能な最低サイズ(50 = 5.0mm)
#define CalcMinDpiSize(dpi, size) (((dpi) * (size)) >> 8) // タッチ可能な最低サイズ（dpi * size / 254 の概算）
//-------------------------------------------------------- ANSI/UNICODE 処理用
#define UCF2HEADER "\xff\xfe"
#define UCF2BEHEADER "\xfe\xff"
#define UCF2HEADERSIZE 2
#define UTF8HEADER "\xef\xbb\xbf"
#define UTF8HEADERSIZE 3
// マクロ定義の文字列をUNICODE化
#define U__STR(string)	L##string
#define UNICODESTR(string)	U__STR(string)

// 格納可能な文字数をsizeofから算出
#define TSIZEOFW(size)   (sizeof(size) / sizeof(WCHAR))
#define TSIZEOFA(size)   sizeof(size)
#define SIZEOFTSTR(size) (sizeof(size) - sizeof(TCHAR))
#define TSIZEOFSTR(size) ((sizeof(size) / sizeof(TCHAR)) - 1)

#define TinyCharUpperA(chr) ((char)(unsigned char)((unsigned char)(chr) & 0xdf))
#define TinyCharUpperW(chr) ((WCHAR)((WCHAR)(chr) & 0xffdf))
#define TinyCharLowerA(chr) ((char)(unsigned char)((unsigned char)(chr) | 0x20))
#define TinyCharLowerW(chr) ((WCHAR)((WCHAR)(chr) | 0x20))

#ifdef __cplusplus
	extern "C" {
#endif
#ifndef WINEGCC
	#define strlenW(ptr) wcslen(ptr)
	#define strcpyW(dst, src) wcscpy((dst), (src))
	#ifdef __BORLANDC__		// Boland 用定義
		#define stpcpyA(dst, src) stpcpy((dst), (src))
	#else
		extern char *stpcpyA(char *dst, const char *src);
	#endif
	extern WCHAR *stpcpyW(WCHAR *dst, const WCHAR *src);
	#define strcatW(dst, src) wcscat((dst), (src))
	#define strcmpW(str1, str2) wcscmp((str1), (str2))
	#define stricmpW(str1, str2) _wcsicmp((str1), (str2))
	#define strnicmpW(str1, str2, size) _wcsnicmp((str1), (str2), (size))
	#define strchrW wcschr
	#define strrchrW wcsrchr
	#define strstrW wcsstr
	#define struprW _wcsupr
#else // Wine on Linux
	#define strlenW(ptr) (size_t)lstrlenW(ptr)
	#define strcpyW(dst, src) lstrcpyW((dst), (src))
	extern char *stpcpyA(char *dst, const char *src);
	extern WCHAR *stpcpyW(WCHAR *dst, const WCHAR *src);
	#define strcatW(dst, src) lstrcatW((dst), (src))
	#define strcmpW(str1, str2) lstrcmpW((str1), (str2))
	#define stricmpW(str1, str2) lstrcmpiW((str1), (str2))
	#define strnicmpW(str1, str2, size) (CompareStringW(GetUserDefaultLCID(), NORM_IGNORECASE, (str1), (size), (str2), (size)) - 2)
	extern WCHAR *strchrW(const WCHAR *text, WCHAR findchar);
	extern WCHAR *strrchrW(const WCHAR *text, WCHAR findchar);
	extern WCHAR *strstrW(const WCHAR *text1, const WCHAR *text2);
	#define struprW CharUpperW
#endif
#ifdef __cplusplus
	}
#endif

#ifdef UNICODE
	#define T(string)	U__STR(string)
	typedef WCHAR		UTCHAR;
	typedef const WCHAR	CTCHAR;
	#define TAPITAIL	"W"

	#define tWinMain	wWinMain
	#define tstrlen(ptr)	strlenW(ptr)
	#define tstrcpy(dst, src)	strcpyW((dst), (src))
	#define tstrcpypart(str1, str2, len)	{memcpy((char *)(WCHAR *)(str1), (const char *)(const WCHAR *)(str2), (len) * sizeof(WCHAR));*((WCHAR *)(str1) + len) = '\0';}
	#define tstpcpy(dst, src)	stpcpyW((dst), (src))
	#define tstrcat(dst, src)	strcatW((dst), (src))
	#define tstrcmp(str1, str2)	strcmpW((str1), (str2))
	#define tstricmp(str1, str2)	stricmpW((str1), (str2))
	#define tstrnicmp	strnicmpW
	#define tstrcmppart(str1, str2, len)	memcmp((const char *)(const WCHAR *)(str1), (const char *)(const WCHAR *)(str2), (len) * sizeof(WCHAR))
	#define tstrchr	strchrW
	#define tstrrchr	strrchrW
	#define tstrstr	strstrW
	#define tstrupr(str)	struprW(str)
	#define tstrgap(str, len)	memmove((char *)(WCHAR *)((WCHAR *)(str) + (len)), (char *)(WCHAR *)(str), (strlenW(str) + 1) * sizeof(WCHAR))
	#define TSTRSIZE(string)	((tstrlen(string) + 1) * sizeof(WCHAR))
	#define TSTRLENGTH(string)	(tstrlen(string) * sizeof(WCHAR))
	#define TSTROFF(size)	((size) * sizeof(WCHAR))
	#define TSIZEOF(size)	TSIZEOFW(size)
	#define CF_TTEXT	CF_UNICODETEXT
	#ifndef CFSTR_FILENAME
		#define CFSTR_FILENAME	T("FileNameW")
	#endif
	#define strcpyToA(dest, src, size) UnicodeToAnsi((src), (dest), (size))
	#define strcpyToW(dest, src, size) strcpyW((dest), (src))
	#define strcpyAToT(dest, src, size) AnsiToUnicode((src), (dest), (size))
	#define strcpyWToT(dest, src, size) strcpyW((dest), (src))
	#define TinyCharUpper(chr) TinyCharUpperW(chr)
	#define TinyCharLower(chr) TinyCharLowerW(chr)
#else
	#define T(string)	string
	typedef unsigned char	UTCHAR;
	typedef const char	CTCHAR;
	#define TAPITAIL	"A"

	#define tWinMain	WinMain
	#define tstrlen(ptr)	strlen(ptr)
	#define tstrcpy(dst, src)	strcpy((dst), (src))
	#define tstrcpypart(str1, str2, len)	{memcpy((str1), (str2), (len));*((char *)(str1) + len) = '\0';}
	#define tstpcpy(dst, src)	stpcpyA((dst), (src))
	#define tstrcat(dst, src)	strcat((dst), (src))
	#define tstrcmp(str1, str2)	strcmp((str1), (str2))
	#define tstricmp(str1, str2)	stricmp((str1), (str2))
	#define tstrnicmp	strnicmp
	#define tstrcmppart(str1, str2, len)	memcmp((str1), (str2), (len))
	#define tstrchr	strchr
	#define tstrrchr	strrchr
	#define tstrstr	strstr
	#define tstrupr(str)	strupr(str)
	#define tstrgap(str, len)	memmove((char *)((char *)(str) + (len)), (char *)(str), (strlen(str) + 1) * sizeof(char))
	#define TSTRSIZE(string)	(strlen(string) + 1)
	#define TSTRLENGTH(string)	strlen(string)
	#define TSTROFF(size)	(size)
	#define TSIZEOF(size)	TSIZEOFA(size)
	#define CF_TTEXT	CF_TEXT
	#ifndef CFSTR_FILENAME
		#define CFSTR_FILENAME	T("FileName")
	#endif
	#define strcpyToA(dest, src, size) strcpy((dest), (src))
	#define strcpyToW(dest, src, size) AnsiToUnicode((src), (dest), (size))
	#define strcpyAToT(dest, src, size) strcpy((dest), (src))
	#define strcpyWToT(dest, src, size) UnicodeToAnsi((src), (dest), (size))
	#define TinyCharUpper(chr) TinyCharUpperA(chr)
	#define TinyCharLower(chr) TinyCharLowerA(chr)
#endif
#ifdef _WIN64
	#define ALIGNMENT_BITS(value) ((DWORD)(DWORD_PTR)(value)) // PtrToUlong の代わり(アラインメント処理向け)
	#define ToSIZE32_T(value) ((DWORD)(DWORD_PTR)(value)) // PtrToUlong の代わり
	#define PtrToValue(value) ((int)(DWORD_PTR)(value)) // PtrToLong の代わり
	#define PtrToUValue(value) ((DWORD)(DWORD_PTR)(value)) // PtrToUlong の代わり
#else
	#define ALIGNMENT_BITS(value) ((DWORD)(value))
	#define ToSIZE32_T(value) ((DWORD)(value))
	#define PtrToValue(value) ((int)(value))
	#define PtrToUValue(value) ((DWORD)(value))
#endif
#define SIZE32_T DWORD
#define strlen32(ptr) ToSIZE32_T(strlen(ptr))
#define strlenW32(ptr) ToSIZE32_T(strlenW(ptr))
#define wcslen32(ptr) ToSIZE32_T(wcslen(ptr))
#define tstrlen32(ptr) ToSIZE32_T(tstrlen(ptr))
#define TSTRSIZE32(ptr) ToSIZE32_T(TSTRSIZE(ptr))
#define TSTROFF32(ptr) ToSIZE32_T(TSTROFF(ptr))
#define TSTRLENGTH32(ptr) ToSIZE32_T(TSTRLENGTH(ptr))

// ※いずれも、戻り値は、'\0'付きの文字長、destsizeに収まらないときは、入る分だけ(\0無)入って、0 を返す
#define AnsiToUnicode(ansi, unicode, destsize) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (ansi), -1, (unicode), (destsize))
#define UnicodeToAnsi(unicode, ansi, destsize) WideCharToMultiByte(CP_ACP, 0, (unicode), -1, (ansi), (destsize), NULL, NULL)
#define Utf8ToUnicode(utf8, unicode, destsize) MultiByteToWideChar(CP_UTF8, 0, (utf8), -1, (unicode), (destsize))
#define UnicodeToUtf8(unicode, utf8, destsize) WideCharToMultiByte(CP_UTF8, 0, (unicode), -1, (utf8), (destsize), NULL, NULL)

#define TNL		T("\r\n")
#define DefineToStr(def) DefineToStr2(def) // 定数定義を文字列にする
#define DefineToStr2(def) #def

//-------------------------------------------------------- BCC/CL
#ifdef __BORLANDC__		// Boland 用定義
	#define USEFASTCALL _fastcall
	#define DLLEntry WINAPI DllEntryPoint
	#define UnUsedParam(param)
	#define UNIONNAME(name, type) name.DUMMYUNIONNAME.type
	#define UNIONNAME2(name, type) name.DUMMYUNIONNAME2.type
	#define UNIONNAME3(name, type) name.DUMMYUNIONNAME3.type
	#define UNIONNAME0(name, type) name.u.type
#else
	#ifdef __GNUC__		// gcc 用定義
		#define UnUsedParam(param)
	#else
				   // Microsoft 用定義
	#define UnUsedParam(param) UNREFERENCED_PARAMETER(param)
#endif
#define USEFASTCALL __fastcall
#define DLLEntry APIENTRY DllMain
#define UNIONNAME(name, type) name.type
#define UNIONNAME2(name, type) name.type
#define UNIONNAME3(name, type) name.type
#define UNIONNAME0(name, type) name.type
#endif
#define USECDECL __cdecl
//-------------------------------------------------------- MinGW 対策用
// SDK と異なる定義があるヘッダの対策
#if (defined(_SHLOBJ_H) || defined(__WINE_SHLOBJ_H)) && !defined(_SHLOBJ_H_)
	#define _SHLOBJ_H_
#endif
#if (defined(_COMMCTRL_H) || defined(__WINE_COMMCTRL_H)) && !defined(_INC_COMMCTRL)
	#define _INC_COMMCTRL
#endif
//--------------------------------------------------------- その他
typedef DWORD ERRORCODE;	// Win32API のエラーコード(GetLastError)
#define IsTrue(form)	((form) != FALSE)
#define IsFalse(form)	((form) == FALSE)
#define setflag(flag, bit)	flag |= (bit)			//	特定フラグを立てる
#define resetflag(flag, bit)	flag &= ~(bit)			//	特定フラグを下げる
#ifndef max
	#define max(a, b) (((a) > (b)) ? (a) : (b))	//	大きいほうを返す
	#define min(a, b) (((a) < (b)) ? (a) : (b))	//	小さいほうを返す
#endif
#define LPARAMtoPOINT(pos, lParam) {(pos).x=(int)(short)LOWORD(lParam);(pos).y=(int)(short)HIWORD(lParam);}
#define GetMessagePosPoint(pos) {DWORD dpos = GetMessagePos();LPARAMtoPOINT((pos), (dpos));}
#define LOSHORTINT(lp) ((int)(short)LOWORD(lp))
#define HISHORTINT(lp) ((int)(short)HIWORD(lp))
#define CHILDWNDID(id) ((HMENU)(DWORD_PTR)(UINT)(id))

#define Message(mes)	MessageBox(GetActiveWindow(), (mes), NULL, MB_OK)
#define MessageA(mes)	MessageBoxA(GetActiveWindow(), (mes), NULL, MB_OK)
#define MessageW(mes)	MessageBoxW(GetActiveWindow(), (mes), NULL, MB_OK)
#define Messagef(fmt, mes)	{TCHAR T__[999];wsprintf(T__, T(fmt), (mes));Message(T__);}
#define TMAKEWPARAM(low, high) ((WORD)(low) | ((WORD)(high) << 16))
#define TMAKELPARAM(low, high) ((WORD)(low) | ((WORD)(high) << 16))
#define SkipSPC(ptr)	{while ((*(ptr) == ' ') || (*(ptr) == '\t')) (ptr)++;}
#define CharUPR(chr)	{if (Islower(chr)) (chr) -= (BYTE)0x20;}

typedef union {
	struct { // エンディアン注意
		DWORD LowPart;
		LONG HighPart;
	} u;
	struct { // エンディアン注意
		DWORD L;
		LONG H;
	} s;
#ifdef _WIN64
	__int64 QuadPart;
	__int64 HL;
#endif
	LARGE_INTEGER LI;
} INTHL;

typedef union {
	struct { // エンディアン注意
		DWORD LowPart;
		DWORD HighPart;
	} u;
	struct { // エンディアン注意
		DWORD L;
		DWORD H;
	} s;
#ifdef _WIN64
	unsigned __int64 QuadPart;
	unsigned __int64 HL;
#endif
	ULARGE_INTEGER ULI;
} UINTHL;

#define AddDD(dstL, dstH, srcL, srcH)\
	{(dstH) += (srcH); (dstL) += (srcL); if ( (dstL) < (srcL) ) (dstH)++;}

#define SubDD(dstL, dstH, srcL, srcH)\
	{DWORD I_;(dstH) -= (srcH);I_ = (dstL);(dstL) -= (srcL);if ( (dstL) > I_ ) (dstH)--;}
// ↑ dstL -= srcL;dstL + srcL;if ( C flag ) dstH--; と最適化される
// #  dstL -= srcL;if ( C flag ) dstH--; と最適化されると嬉しいのだが(^^;

#ifdef _WIN64
	#define LetHL_0(dstIntHL) {(dstIntHL).QuadPart = 0;}
	#define LetHLHL(dstIntHL, srcIntHL) {(dstIntHL).QuadPart = (srcIntHL).QuadPart;}
	#define AddHLHL(dstIntHL, srcIntHL) {(dstIntHL).QuadPart += (srcIntHL).QuadPart;}
	#define SubUHLHL(dstIntHL, srcIntHL) {(dstIntHL).QuadPart -= (srcIntHL).QuadPart;}
	#define AddHLI(dstIntHL, srcInt) {(dstIntHL).QuadPart += (srcInt);}
	#define SubUHLI(dstIntHL, srcInt) {(dstIntHL).QuadPart -= (srcInt);}

	// WIN32_FIND_DATA / WIN32_FILE_ATTRIBUTE_DATA / BY_HANDLE_FILE_INFORMATION の nFileSize 操作用
	// ※ nFileSize は nFileSizeHigh , nFileSizeLow の順に配置されている
	#define AddMaskedFilesize(dstIntHL, Fsize, andL, addL)\
		{(dstIntHL).HL += ((size_t)((Fsize).nFileSizeLow) + ((size_t)(Fsize).nFileSizeHigh << 32) + (size_t)(addL)) & (size_t)(andL);}

	#define AddHLFilesize(dstIntHL, Fsize)\
		{(dstIntHL).QuadPart += (size_t)(Fsize).nFileSizeLow + ((size_t)(Fsize).nFileSizeHigh << 32);}

	#define SubHLFilesize(dstIntHL, Fsize)\
		{(dstIntHL).QuadPart -= (size_t)(Fsize).nFileSizeLow + ((size_t)(Fsize).nFileSizeHigh << 32);}
#else
	#define LetHL_0(dstIntHL) {(dstIntHL).u.LowPart = (dstIntHL).u.HighPart = 0;}
	#define LetHLHL(dstIntHL, srcIntHL) {(dstIntHL) = (srcIntHL);}
	#define AddHLHL(dstIntHL, srcIntHL)\
		AddDD(((dstIntHL).s.L), ((dstIntHL).s.H), ((srcIntHL).s.L), ((srcIntHL).s.H))
	#define SubUHLHL(dstIntHL, srcIntHL)\
		SubDD(((dstIntHL).s.L), ((dstIntHL).s.H), ((srcIntHL).s.L), ((srcIntHL).s.H))
	#define AddHLI(dstIntHL, srcInt)\
		{(dstIntHL).u.LowPart += (srcInt); if ( (dstIntHL).u.LowPart < (srcInt) ) (dstIntHL).u.HighPart++;}
	#define SubUHLI(dstIntHL, srcInt)\
		{DWORD I_; I_ = (dstIntHL).u.LowPart; (dstIntHL).u.LowPart -= (srcInt); if ( (dstIntHL).u.LowPart > I_ ) (dstIntHL).u.HighPart--;}

	// WIN32_FIND_DATA / WIN32_FILE_ATTRIBUTE_DATA / BY_HANDLE_FILE_INFORMATION の nFileSize 操作用
	#define AddMaskedFilesize(dstIntHL, Fsize, andL, addL)\
		{DWORD I; \
		I = (((Fsize).nFileSizeLow) + (addL)) & (andL); \
		if (I < ((Fsize).nFileSizeLow)) ((dstIntHL).s.H)++; \
		AddDD(((dstIntHL).s.L), ((dstIntHL).s.H), I, ((Fsize).nFileSizeHigh));}

	#define AddHLFilesize(dstIntHL, Fsize)\
		{(dstIntHL.u.HighPart) += ((Fsize).nFileSizeHigh); (dstIntHL.u.LowPart) += ((Fsize).nFileSizeLow); if ( (dstIntHL.u.LowPart) < ((Fsize).nFileSizeLow) ) ((dstIntHL).u.HighPart)++;}

	#define SubHLFilesize(dstIntHL, Fsize)\
		SubDD(((dstIntHL).u.LowPart), ((dstIntHL).u.HighPart), (Fsize).nFileSizeLow, (Fsize).nFileSizeHigh)
#endif
#define LetFilesizeHL(Fsize, srcIntHL) {(Fsize).nFileSizeLow = (srcIntHL).u.LowPart; (Fsize).nFileSizeHigh = (srcIntHL).u.HighPart;}
#define LetHLFilesize(dstIntHL, Fsize) {(dstIntHL).u.LowPart = (Fsize).nFileSizeLow; (dstIntHL).u.HighPart = (Fsize).nFileSizeHigh;}

#define ShiftDivDD(srcL, srcH, rshift)\
	((DWORD)( ((DWORD)(srcL) >> (rshift)) | ((DWORD)(srcH) << (32 - (rshift))) ))

// 指定したメモリをセキュリティ目的で消去する
#define ClearMemory(ptr, size) memset((ptr), 0, (size));*(volatile char *)(ptr) = *(volatile char *)(ptr);

#define DwordAlignment(value) (((value) + 3) & 0xfffffffc)

// 指定したbit数における DWORD アライメントに揃えた大きさを得る(DIB幅計算用)
#define DwordBitSize(bits) ((((bits) + 31) & 0xffffffe0) >> 3)

// 相対ディレクトリかどうかを判定する
#define IsRelativeDir(path) (((path)[0] == '.') && (((path)[1] == '\0') || (((path)[1] == '.') && ((path)[2] == '\0'))))
#define IsParentDir(path) (((path)[0] == '.') && ((path)[1] == '.') && ((path)[2] == '\0'))
#define TypedefWinAPI(retvar, name, param) typedef retvar (WINAPI *imp ## name) param
#define DefineWinAPI(retvar, name, param) typedef retvar (WINAPI *imp ## name) param; imp ## name D ## name
#define ExternWinAPI(retvar, name, param) typedef retvar (WINAPI *imp ## name) param; extern imp ## name D ## name
#define ValueWinAPI(name) imp ## name D ## name
#define GETDLLPROC(handle, name) D ## name = (imp ## name)GetProcAddress((handle), #name);
#define GETDLLPROCT(handle, name) D ## name = (imp ## name)GetProcAddress((handle), #name TAPITAIL);

#ifdef __cplusplus
	#define POINTERCAST(type, value) (reinterpret_cast<type>(value))
	#define CONSTCAST(type, value) (const_cast<type>(value))
	#define STATICCAST(type, value) (static_cast<type>(value))
	#define FUNCCAST(type, value) (static_cast<type>(static_cast<LONG_PTR>(value)))
#else
	#define POINTERCAST(type, value) ((type)(value))
	#define CONSTCAST(type, value) ((type)(value))
	#define STATICCAST(type, value) ((type)(value))
	#define FUNCCAST(type, value) ((type)(DWORD_PTR)(value))
#endif

#if (_WIN32_WINNT >= 0x0500)
	#define OPENFILEEXTDEFINE ,NULL, 0, 0
#else
	#define OPENFILEEXTDEFINE
#endif

//-------------------------------------------------------- SAL
#ifndef _In_
	#define _In_
	#define _In_opt_
	#define _In_z_ // string z
	#define _In_opt_z_
	#define _Out_
	#define _Out_opt_
	#define _Inout_
	#define _Ret_
	#define _Ret_z_
#endif
#ifndef _Ret_notnull_
	#define _Ret_notnull_
#endif
#ifndef  _Success_
	#define _Success_(SuccessValue)
#endif
#ifndef _Outptr_
	#define _Outptr_
#endif
#ifndef  _Inout_opt_
	#define _Outptr_opt_
	#define _Inout_opt_
#endif
#ifndef _In_reads_bytes_
	#define _In_reads_bytes_(ByteSize)
	#define _In_reads_(BufferSize)
	#define _In_reads_z_(BufferSize)
	#define _In_reads_opt_z_(BufferSize)
	#define _Out_writes_z_(BufferSize)
	#define _Out_writes_opt_z_(BufferSize)
#endif
//-------------------------------------------------------- (古い)SDKで無い定義
#ifdef WINEGCC
	#define memicmp(src1, src2, size) (CompareStringA(GetUserDefaultLCID(), NORM_IGNORECASE, (LPCSTR)(src1), (size), (LPCSTR)(src2), (size)) - 2)

	#define stricmp lstrcmpiA
	#define strnicmp(str1, str2, size) (CompareStringA(GetUserDefaultLCID(), NORM_IGNORECASE, (str1), (size), (str2), (size)) - 2)
	#define strupr(src) CharUpperA(src)
	#define strlwr CharLowerA
//	#define ImmAssociateContext(hWnd, hIMC) // 定義されていない
#endif
#ifdef __ms_va_list // wine で変更されている va_list
	#define t_va_list __ms_va_list
	#define t_va_start __ms_va_start
	#define t_va_end __ms_va_end
	#define t_va_arg va_arg // 変更無し
#else
	#define t_va_list va_list
	#define t_va_start va_start
	#define t_va_end va_end
	#define t_va_arg va_arg // 変更無し
#endif

// Win64
#if 1 // 現在のWin64 は 64bit pointer をフルに使用していない
	#define PTRMAX64 0x00000fffffffffff
	#define PTRUSERMAX64 0x0000080000000000
	#define PTR64SHORT
	#define PTRLEN64 12
#else
	#define PTRMAX64 0xffffffffffffffff
	#define PTRUSERMAX64 0x8000000000000000
	#define PTRLEN64 16
#endif
#define PTRPRINTFORMAT64 "%I64X" // "%p" だと、行頭 0 付きになる
#define PTRPRINTFORMAT32 "%X" // "%p"

#ifndef _WIN64
	#define PTRBYTELEN 4
	#define PTRMAX 0xffffffff
	#define PTRUSERMAX 0xc0000000
	#define PTRPRINTFORMAT PTRPRINTFORMAT32
	#define PTRLEN 8
	#define MAXLPARAM ((LPARAM)MAX32)
	#define CalcMulDiv(int_x, int_y, int_div) MulDiv((int_x), (int_y), (int_div))
#else
	#define PTRBYTELEN 8
	#define PTRMAX PTRMAX64
	#define PTRUSERMAX PTRUSERMAX64
	#define PTRPRINTFORMAT PTRPRINTFORMAT64
	#define PTRLEN PTRLEN64
	#define MAXLPARAM ((LPARAM)MAX64)
	#define CalcMulDiv(int_x, int_y, int_div) ((int)(__int64)(((__int64)(int_x) * (__int64)(int_y)) / (__int64)(int_div)))
#endif
#ifndef GetWindowLongPtr
	typedef DWORD DWORD_PTR;
	typedef LONG LONG_PTR;
	typedef ULONG ULONG_PTR;
	typedef UINT UINT_PTR;
	typedef int INT_PTR;

	#define GetWindowLongPtr GetWindowLong
	#define SetWindowLongPtr SetWindowLong
	#define GWLP_WNDPROC	GWL_WNDPROC
	#define GWLP_HINSTANCE	GWL_HINSTANCE
	#define GWLP_ID		GWL_ID
	#define GWLP_USERDATA	GWL_USERDATA
	#define DWLP_DLGPROC	DWL_DLGPROC
	#define DWLP_MSGRESULT	DWL_MSGRESULT
	#define DWLP_USER		DWL_USER

	#define GCLP_HBRBACKGROUND GCL_HBRBACKGROUND
	#define GCLP_HMODULE	GCL_HMODULE
	#define GCLP_MENUNAME	GCL_MENUNAME
	#define GCLP_WNDPROC	GCL_WNDPROC
#endif

#ifndef GetClassLongPtr
	#define GetClassLongPtr GetClassLong
	#define SetClassLongPtr SetClassLong
	#define GCLP_HCURSOR GCL_HCURSOR
	#define GCLP_HICON GCL_HICON
#endif

// タスクバーの作成時に発生(IE4)
#define WMTASKBARCREATE "TaskbarCreated"

#ifndef WM_DWMCOMPOSITIONCHANGED
	#define WM_DWMCOMPOSITIONCHANGED 0x031E
	//HRESULT WINAPI DwmIsCompositionEnabled(BOOL* pfEnabled);
#endif
#ifndef DWMWA_EXTENDED_FRAME_BOUNDS
  #define DWMWA_EXTENDED_FRAME_BOUNDS 9
#endif

// クリップボードフォーマット
#ifndef CFSTR_SHELLURL
	#define CFSTR_SHELLURL T("UniformResourceLocator")
#endif

// ファイルエントリの属性
#ifndef FILE_ATTRIBUTE_LABEL
	#define FILE_ATTRIBUTE_LABEL	B3
#endif
#ifndef FILE_ATTRIBUTE_ENCRYPTED
	#define FILE_ATTRIBUTE_DEVICE B6
	#define FILE_ATTRIBUTE_SPARSE_FILE B9
	#define FILE_ATTRIBUTE_REPARSE_POINT B10
	#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED B13
	#define FILE_ATTRIBUTE_ENCRYPTED B14
#endif
#ifndef FILE_ATTRIBUTE_VIRTUAL
	#define FILE_ATTRIBUTE_VIRTUAL	B16
#endif

// マウス関連
#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL 0x020a
	#define WHEEL_DELTA 120
#endif
#define WHEEL_STANDARD_LINES 3 // ホイールの１ノッチで移動する行数
#ifndef MOUSE_WHEELED
	#define MOUSE_WHEELED 0x0004
#endif
#ifndef WM_MOUSEHWHEEL
	#define WM_MOUSEHWHEEL 0x20e
#endif
#ifndef MK_XBUTTON1
	#define MK_XBUTTON1 0x0020
	#define MK_XBUTTON2 0x0040
#endif

#ifndef VK_XBUTTON1
	#define VK_XBUTTON1 0x05
	#define VK_XBUTTON2 0x06
	#define WM_NCXBUTTONDOWN   0x00AB
	#define WM_NCXBUTTONUP     0x00AC
	#define WM_NCXBUTTONDBLCLK 0x00AD
	#define WM_XBUTTONDOWN     0x020B
	#define WM_XBUTTONUP       0x020C
	#define WM_XBUTTONDBLCLK   0x020D
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif

//
#ifndef COLOR_HOTLIGHT
	#define COLOR_HOTLIGHT					26
	#define COLOR_GRADIENTACTIVECAPTION	27
	#define COLOR_GRADIENTINACTIVECAPTION	28
#endif

// ウィンドウスタイル / レイヤーウィンドウ
#ifndef WS_EX_NOACTIVATE
	#define WS_EX_NOACTIVATE 0x08000000L
#endif
#ifndef WS_EX_LAYERED
	#define WS_EX_LAYERED	0x00080000
#endif
#ifndef LWA_COLORKEY
	#define AC_SRC_OVER  0x00
	#define AC_SRC_ALPHA 0x01

	#define LWA_COLORKEY	1
	#define LWA_ALPHA		2
	#define ULW_ALPHA  0x00000002
	#define ULW_OPAQUE 0x00000004
#endif
//
#ifndef IMC_GETOPENSTATUS
	#define IMC_GETOPENSTATUS 0x0005
#endif

#ifndef EM_SETCUEBANNER
	#define EM_SETCUEBANNER 0x1501
#endif

// GetVolumeInformation
#ifndef FS_FILE_ENCRYPTION
	#define FILE_VOLUME_QUOTAS				0x00000020
	#define FILE_SUPPORTS_SPARSE_FILES		0x00000040
	#define FILE_SUPPORTS_REPARSE_POINTS	0x00000080
	#define FILE_SUPPORTS_REMOTE_STORAGE	0x00000100
	#define FILE_SUPPORTS_OBJECT_IDS		0x00010000
	#define FS_FILE_ENCRYPTION				0x00020000
	#define FILE_NAMED_STREAMS				0x00040000
#endif

// WIN32_STREAM_ID
#ifndef BACKUP_PROPERTY
	#define BACKUP_PROPERTY		0x00000006
#endif
#ifndef BACKUP_OBJECT_ID
	#define BACKUP_OBJECT_ID		0x00000007
	#define BACKUP_REPARSE_DATA	0x00000008
	#define BACKUP_SPARSE_BLOCK	0x00000009
#endif

// FlashWindowEx
#ifndef FLASHW_STOP
	typedef struct {
		UINT  cbSize;
		HWND  hwnd;
		DWORD dwFlags;
		UINT  uCount;
		DWORD dwTimeout;
	} FLASHWINFO, *PFLASHWINFO;
	WINUSERAPI BOOL WINAPI FlashWindowEx(PFLASHWINFO pfwi);
	#define FLASHW_STOP      0
	#define FLASHW_TRAY      0x02
	#define FLASHW_ALL       (FLASHW_CAPTION | FLASHW_TRAY)
	#define FLASHW_TIMER     0x04
	#define FLASHW_TIMERNOFG 0x0C
	#define FLASHW_CAPTION   0x01
#endif

// マルチモニタ
#if (WINVER < 0x0500) && ((!defined(HMONITOR_DECLARED) && !defined(SM_CMONITORS)) || (defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(_WIN64) ))
	typedef struct tagMONITORINFO {
		DWORD   cbSize;
		RECT    rcMonitor;
		RECT    rcWork;
		DWORD   dwFlags;
	} MONITORINFO, *LPMONITORINFO;
#endif
#ifndef SM_CMONITORS
	#define SM_XVIRTUALSCREEN  76
	#define SM_YVIRTUALSCREEN  77
	#define SM_CXVIRTUALSCREEN 78
	#define SM_CYVIRTUALSCREEN 79
	#define SM_CMONITORS       80

	#if !defined(HMONITOR_DECLARED)
		DECLARE_HANDLE(HMONITOR);
	#endif
#endif

#ifndef MONITOR_DEFAULTTONULL
	#define MONITOR_DEFAULTTONULL       0x00000000
	#define MONITOR_DEFAULTTOPRIMARY    0x00000001
	#define MONITOR_DEFAULTTONEAREST    0x00000002
	#define MONITORINFOF_PRIMARY        0x00000001
#endif

// Tree control
#ifndef TVM_SETBKCOLOR
	#define TVM_SETBKCOLOR		(TV_FIRST + 29)
	#define TreeView_SetBkColor(hwnd, clr) \
		(COLORREF)SendMessage((hwnd), TVM_SETBKCOLOR, 0, (LPARAM)(clr))

	#define TVM_SETTEXTCOLOR	(TV_FIRST + 30)
	#define TreeView_SetTextColor(hwnd, clr) \
		(COLORREF)SendMessage((hwnd), TVM_SETTEXTCOLOR, 0, (LPARAM)(clr))

	#define TVM_GETBKCOLOR		(TV_FIRST + 31)
	#define TreeView_GetBkColor(hwnd) \
		(COLORREF)SendMessage((hwnd), TVM_GETBKCOLOR, 0, 0)

	#define TVM_GETTEXTCOLOR	(TV_FIRST + 32)
	#define TreeView_GetTextColor(hwnd) \
		(COLORREF)SendMessage((hwnd), TVM_GETTEXTCOLOR, 0, 0)
#endif

#if (_WIN32_IE >= 0x0400)
	#define TreeInsertItemValue(tvi)	UNIONNAME(tvi, item)
#else
	#define TreeInsertItemValue(tvi)	tvi.item
#endif

// Tab control
#ifndef TCS_HOTTRACK
	#define TCS_HOTTRACK			0x0040
	#define TCS_FLATBUTTONS		0x0008
	#define TCS_EX_FLATSEPARATORS	0x00000001
	#define TCS_EX_REGISTERDROP	0x00000002
#endif
#ifndef TCN_SELCHANGING
	#define TCN_SELCHANGING	(TCN_FIRST - 2)
	#define TCN_GETOBJECT		(TCN_FIRST - 3)
	#define TCN_FOCUSCHANGE	(TCN_FIRST - 4)
#endif

#ifndef TabCtrl_SetExtendedStyle
	#define TCM_SETEXTENDEDSTYLE (TCM_FIRST + 52)
	#define TabCtrl_SetExtendedStyle(hwnd, dw)\
		SNDMSG((hwnd), TCM_SETEXTENDEDSTYLE, 0, dw)
#endif

// IContextMenu
#ifndef CMF_EXTENDEDVERBS
	#define CMF_EXTENDEDVERBS 0x100
#endif

// Menu
#ifndef TPM_RECURSE
	#define TPM_RECURSE 0x0001
	#define WM_MENURBUTTONUP	0x0122
	#define WM_MENUDRAG		0x0123
	#define WM_MENUGETOBJECT	0x0124
	#define WM_UNINITMENUPOPUP	0x0125
	#define WM_MENUCOMMAND		0x0126

	#define MNS_NOCHECK		0x80000000
	#define MNS_MODELESS		0x40000000
	#define MNS_DRAGDROP		0x20000000
	#define MNS_AUTODISMISS	0x10000000
	#define MNS_NOTIFYBYPOS	0x08000000
	#define MNS_CHECKORBMP		0x04000000

	#define MIM_MAXHEIGHT	0x00000001
	#define MIM_BACKGROUND	0x00000002
	#define MIM_HELPID		0x00000004
	#define MIM_MENUDATA	0x00000008
	#define MIM_STYLE		0x00000010
	#define MIM_APPLYTOSUBMENUS 0x80000000

	#ifndef __GNUC__
		typedef struct {
			DWORD   cbSize;
			DWORD   fMask;
			DWORD   dwStyle;
			UINT    cyMax;
			HBRUSH  hbrBack;
			DWORD   dwContextHelpID;
			ULONG_PTR dwMenuData;
		} MENUINFO, *LPMENUINFO;
		typedef MENUINFO CONST *LPCMENUINFO;
	#endif
#endif

#ifndef CMIC_MASK_UNICODE
typedef struct _CMInvokeCommandInfoEx {
	DWORD cbSize;
	DWORD fMask;
	HWND hwnd;
	LPCSTR lpVerb;
	LPCSTR lpParameters;
	LPCSTR lpDirectory;
	int nShow;
	DWORD dwHotKey;
	HANDLE hIcon;
	LPCSTR lpTitle;
	LPCWSTR lpVerbW;
	LPCWSTR lpParametersW;
	LPCWSTR lpDirectoryW;
	LPCWSTR lpTitleW;
} CMINVOKECOMMANDINFOEX, *LPCMINVOKECOMMANDINFOEX;
#define CMIC_MASK_UNICODE 0x00004000
#endif

#ifndef SPI_GETWHEELSCROLLLINES
	#define SPI_GETWHEELSCROLLLINES 104
#endif

#ifndef ICC_LISTVIEW_CLASSES
	#define ICC_LISTVIEW_CLASSES 0x00000001
	#define ICC_TREEVIEW_CLASSES 0x00000002
	#define ICC_BAR_CLASSES      0x00000004
	#define ICC_TAB_CLASSES      0x00000008
	#define ICC_UPDOWN_CLASS     0x00000010
	#define ICC_PROGRESS_CLASS   0x00000020
	#define ICC_HOTKEY_CLASS     0x00000040
	#define ICC_ANIMATE_CLASS    0x00000080
	#define ICC_WIN95_CLASSES    0x000000FF
	#define ICC_DATE_CLASSES     0x00000100
	#define ICC_USEREX_CLASSES   0x00000200
	#define ICC_COOL_CLASSES     0x00000400
	#define ICC_INTERNET_CLASSES 0x00000800
	#define ICC_PAGESCROLLER_CLASS 0x00001000
	#define ICC_NATIVEFNTCTL_CLASS 0x00002000
#endif

#ifndef IO_REPARSE_TAG_MOUNT_POINT
	#define IO_REPARSE_TAG_MOUNT_POINT	0xA0000003
#endif
#ifndef IO_REPARSE_TAG_SYMLINK
	#define IO_REPARSE_TAG_SYMLINK		0xA000000C
#endif
#ifndef IO_REPARSE_TAG_LX_SYMLINK
	#define IO_REPARSE_TAG_LX_SYMLINK	0xA000001D
#endif

#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
	#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE	( 16 * 1024 )
	#define FILE_ANY_ACCESS					0
#endif
#ifndef FILE_FLAG_OPEN_REPARSE_POINT
	#define FILE_SPECIAL_ACCESS				(FILE_ANY_ACCESS)
	#define FILE_FLAG_OPEN_REPARSE_POINT		0x00200000
#endif

// REPARSE_DATA_BUFFER を加工した構造体
typedef struct {
	DWORD	ReparseTag;
	WORD	ReparseDataLength;
	WORD	Reserved;
	union {
		struct { // GUID ReparseGuid;
			WORD	SubstituteNameOffset;
			WORD	SubstituteNameLength;
			WORD	PrintNameOffset;
			WORD	PrintNameLength;
			WCHAR	PathBuffer[1];
		} ReparseGuid;
		struct {
			DWORD	FileType;
			char	PathBuffer[1];
		} LxSymlinkReparseBuffer;
	} data;
	struct {
		BYTE DataBuffer[/*1*/MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	} GenericReparseBuffer;
} REPARSE_DATA_IOBUFFER, *PREPARSE_DATA_IOBUFFER;

#ifndef FSCTL_SET_REPARSE_POINT
	#define FSCTL_SET_REPARSE_POINT		CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
	#define FSCTL_GET_REPARSE_POINT		CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
	#define FSCTL_DELETE_REPARSE_POINT	CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#endif

//------------------------------------- ToolBar
#ifndef TB_HITTEST
#define TB_HITTEST (WM_USER + 69)
#endif

#ifndef BTNS_SHOWTEXT
#define BTNS_SHOWTEXT 0x0040	// IE5.01
#define TBSTYLE_EX_MIXEDBUTTONS 0x00000008 // IE5.01
#endif

#ifndef TBSTYLE_FLAT
#define TBSTYLE_DROPDOWN 0x0008	// IE3(4.70)
#define TBSTYLE_AUTOSIZE 0x0010	// IE4(4.71)
#define TBSTYLE_FLAT            0x0800 // IE3
#define TBSTYLE_LIST            0x1000 // IE3
#define TBSTYLE_REGISTERDROP    0x4000 // IE4
#define TBSTYLE_EX_DRAWDDARROWS 0x00000001 // IE4
#define TBN_DROPDOWN            (TBN_FIRST - 10)
#define TB_GETRECT              (WM_USER + 51) // IE3
#define TB_SETMAXTEXTROWS       (WM_USER + 60)
#define TB_GETMAXSIZE           (WM_USER + 83)
#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)

#ifdef TOOLBARCLASSNAME
#ifndef __GNUC__
typedef struct tagNMTOOLBARA {
	NMHDR	hdr;
	int		iItem;
	TBBUTTON tbButton;
	int		cchText;
	LPSTR	pszText;
#if (_WIN32_IE >= 0x500)
	RECT	rcButton;
#endif // _WIN32_IE
} NMTOOLBARA, *LPNMTOOLBARA;

typedef struct tagNMTOOLBARW {
	NMHDR	hdr;
	int		iItem;
	TBBUTTON tbButton;
	int		cchText;
	LPWSTR	pszText;
#if (_WIN32_IE >= 0x500)
	RECT    rcButton;
#endif // _WIN32_IE
} NMTOOLBARW, *LPNMTOOLBARW;
#endif // __GNUC__

#ifdef UNICODE
#define NMTOOLBAR   NMTOOLBARW
#define LPNMTOOLBAR LPNMTOOLBARW
#else
#define NMTOOLBAR   NMTOOLBARA
#define LPNMTOOLBAR LPNMTOOLBARA
#endif // UNICODE
#endif // TOOLBARCLASSNAME
#endif // TBSTYLE_FLAT

#ifndef CCM_DPISCALE
#define CCM_DPISCALE 0x200c
#endif

#if 1
 // VS2015以降 は IUnknown の定義が破損
#undef  INTERFACE
#define INTERFACE XIExtractIcon
DECLARE_INTERFACE_(XIExtractIcon, IUnknown)
{
	STDMETHOD(QueryInterface) (THIS_ REFIID, LPVOID *) PURE;
	STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
	STDMETHOD_(ULONG, Release) (THIS) PURE;
	STDMETHOD(GetIconLocation)(THIS_ UINT, LPTSTR, UINT, int *, UINT *) PURE;
	STDMETHOD(Extract)(THIS_ LPCTSTR pszFile, UINT, HICON *, HICON *, UINT) PURE;
};
typedef XIExtractIcon * XLPEXTRACTICON;
#else
#define XLPEXTRACTICON LPEXTRACTICON
#endif

// タッチ操作関係
#if !defined(WM_GESTURENOTIFY)
#define WM_GESTURE 0x0119
#define WM_GESTURENOTIFY 0x011A
#define WM_TOUCH 0x0240
#endif

#ifndef TOUCHEVENTF_MOVE
#if !defined(WINEGCC) || (WINEGCC < 300) // Wine 3 は定数定義無しなのに構造体のみ定義されている
DECLARE_HANDLE(HGESTUREINFO);
typedef struct tagGESTUREINFO {
	UINT cbSize;
	DWORD dwFlags;
	DWORD dwID;
	HWND hwndTarget;
	POINTS ptsLocation;
	DWORD dwInstanceID;
	DWORD dwSequenceID;
	ULONGLONG ullArguments;
	UINT cbExtraArgs;
} GESTUREINFO, *PGESTUREINFO;
typedef GESTUREINFO const * PCGESTUREINFO;

typedef struct tagGESTURECONFIG {
	DWORD dwID;
	DWORD dwWant;
	DWORD dwBlock;
} GESTURECONFIG, *PGESTURECONFIG;

DECLARE_HANDLE(HTOUCHINPUT);
typedef struct tagTOUCHINPUT {
	LONG x;
	LONG y;
	HANDLE hSource;
	DWORD dwID;
	DWORD dwFlags;
	DWORD dwMask;
	DWORD dwTime; // TOUCHINPUTMASKF_TIMEFROMSYSTEM
	ULONG_PTR dwExtraInfo; // dwExtraInfo
	DWORD cxContact; // 大きさ情報、TOUCHINPUTMASKF_CONTACTAREA
	DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const * PCTOUCHINPUT;
#endif
#endif // TOUCHEVENTF_MOVE

#ifndef TOUCHEVENTF_MOVE
#define TOUCHEVENTF_MOVE       0x01
#define TOUCHEVENTF_DOWN       0x02
#define TOUCHEVENTF_UP         0x04
#define TOUCHEVENTF_INRANGE    0x08 // ホバー？
#define TOUCHEVENTF_PRIMARY    0x10 // 1st
#define TOUCHEVENTF_NOCOALESCE 0x20
#define TOUCHEVENTF_PEN        0x40
#define TOUCHEVENTF_PALM       0x80
#endif

#ifndef GF_BEGIN
#define GF_BEGIN   0x00000001
#define GF_INERTIA 0x00000002
#define GF_END     0x00000004

#define TOUCHINPUTMASKF_TIMEFROMSYSTEM 0x01 // dwTime
#define TOUCHINPUTMASKF_EXTRAINFO      0x02 // dwExtraInfo
#define TOUCHINPUTMASKF_CONTACTAREA    0x04 // cxContact, cyContact

#define GID_BEGIN        1
#define GID_END          2
#define GID_ZOOM         3
#define GID_PAN          4
#define GID_ROTATE       5
#define GID_TWOFINGERTAP 6
#define GID_PRESSANDTAP  7
#endif

#ifndef GC_ZOOM
#define GC_ZOOM                                0x01
#define GC_PAN                                 0x01
#define GC_PAN_WITH_SINGLE_FINGER_VERTICALLY   0x02
#define GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY 0x04
#define GC_PAN_WITH_GUTTER                     0x08
#define GC_PAN_WITH_INERTIA                    0x10
#define GC_ROTATE                              0x01
#define GC_TWOFINGERTAP                        0x01
#define GC_PRESSANDTAP                         0x01
#endif

#ifndef SHCONTF_FASTITEMS
	#define SHCONTF_FASTITEMS 0x2000
#endif

#ifndef IS_INTRESOURCE
	#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#endif
#define WNDCLASSBRUSH(id) ((HBRUSH)(LONG_PTR)(id)) // WNCLASS のブラシに定数使う時用

#ifdef WINEGCC // 5.0 で機能しないマクロ
	#undef WC_BUTTON
	#undef WC_EDIT
#endif
#ifndef WC_BUTTON
#define WC_BUTTON T("Button")
#endif
#ifndef WC_EDIT
#define WC_EDIT T("Edit")
#endif
#ifndef WC_COMBOBOX
#define WC_COMBOBOX T("ComboBox")
#endif
#ifndef WC_STATIC
#define WC_STATIC T("Static")
#endif
#ifndef WC_LISTBOX
#define WC_LISTBOX T("ListBox")
#endif
#ifndef WC_SCROLLBAR
#define WC_SCROLLBAR T("ScrollBar")
#endif
