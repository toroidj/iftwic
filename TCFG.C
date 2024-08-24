/*-----------------------------------------------------------------------------
	定義ファイル操作パッケージ 簡易版							(c)TORO 2000
-----------------------------------------------------------------------------*/
#define STRICT
#include <windows.h>
#include <string.h>
#include "TOROWIN.H"
#include "TCFG.H"

//======================================================================== 雑用
/*-------------------------------------
	行末(\0,\a,\d)か？

RET:	==0 :EOL	!=0 :Code
--------------------------------------*/
BYTE USEFASTCALL IsEOLA(char **str)
{
	BYTE code;

	code = **str;
	if ( code == '\0' ) return '\0';
	if ( code == 0xd ){
		if ( *(*str + 1) == 0x0a ) (*str)++;
		return '\0';
	}
	if ( code == 0xa ){
		if ( *(*str + 1) == 0x0d ) (*str)++;
		return '\0';
	}
	return code;
}
#ifdef UNICODE
WCHAR USEFASTCALL IsEOLW(TCHAR **str)
{
	WCHAR code;

	code = **str;
	if ( code == '\0' ) return '\0';
	if ( code == 0xd ){
		if ( *(*str + 1) == 0x0a ) (*str)++;
		return '\0';
	}
	if ( code == 0xa ){
		if ( *(*str + 1) == 0x0d ) (*str)++;
		return '\0';
	}
	return code;
}
#define IsEOL IsEOLW
#else
#define IsEOL IsEOLA
#endif
/*-------------------------------------
	空白(space, tab）をスキップする

RET:	==0 :EOL	!=0 :Code
--------------------------------------*/
BYTE USEFASTCALL SkipSpaceA(char **str)
{
	BYTE code;

	while(1){
		code = IsEOLA(str);
		if ( (code != ' ') && (code != '\t') ) break;
		(*str)++;
	}
	return code;
}
#ifdef UNICODE
WORD USEFASTCALL SkipSpaceW(TCHAR **str)
{
	WORD code;

	while(1){
		code = IsEOLW(str);
		if ( (code != ' ') && (code != '\t') ) break;
		(*str)++;
	}
	return code;
}
#endif
//------------------------------------- 行末空白を除去(b:先頭, p:末尾)
void USEFASTCALL SkipSpaceB(char *b, char *p)
{
	while( b < p ){
		if ( (*p == ' ') || (*p == '\t') ){
			p--;
			continue;
		}
		break;
	}
	*(p+1) = '\0';
}
/*-----------------------------------------------------------------------------
	一つ分のパラメータを抽出する。先頭と末尾の空白は除去する

RET:	先頭の文字(何もなかったら 0)
-----------------------------------------------------------------------------*/
BYTE GetLineParam(TCHAR **str, TCHAR *param)
{
	UTCHAR code, bottom;
	TCHAR *src, *dst;

	src = *str;
	dst = param;
	bottom = SkipSpace(&src);
	if (!bottom){
		*str = src;
		return '\0';
	}
	src++;
	if (bottom == '\"'){
		bottom = *src;
		while( '\0' != (code = IsEOL(&src)) ){
			src++;
			if ( (code == '\"') && (*src <= ' ') )	break;
			*dst++ = code;
		}
	}else{
		*dst++ = bottom;
		while( '\0' != (code = IsEOL(&src)) ){
			src++;
			if (code <= ' ')	break;
			*dst++ = code;
		}
	}
	*dst = '\0';
	*str = src;
	return bottom;
}
//============================================================ 定義ファイル操作
//------------------------------------- 定義ファイルを読み込む
BOOL CFGOpen(CFG *cfg, WCHAR *filename)
{
	HANDLE hFile;

	cfg->bottom = NULL;
	cfg->size = 0;
	cfg->write = 0;
//	tstrcpy(cfg->filename, filename);

	hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	cfg->size = GetFileSize(hFile, NULL);
	cfg->bottom = (char *)HeapAlloc(GetProcessHeap(), 0, cfg->size + 256);
	ReadFile(hFile, cfg->bottom, cfg->size, &cfg->size, NULL);
	CloseHandle(hFile);

	memset(cfg->bottom + cfg->size, 0, 256);
	return TRUE;
}
//------------------------------------- 定義ファイルを閉じる
BOOL CFGClose(CFG *cfg)
{
	HeapFree(GetProcessHeap(), 0, cfg->bottom);
	return TRUE;
}
/*-------------------------------------
	指定の offset から、一行分を読み込み
	string に書き込む。
	offset には次の行の先頭を書く
-------------------------------------*/
BOOL USEFASTCALL CFGGetLine(CFG *cfg, DWORD *offset, char *string)
{
	char *bottom, *p, *end = NULL;

	bottom = cfg->bottom + *offset;
	if ( *bottom == '\0' ) return FALSE;
											// 先頭の空白を削除 ---------------
	SkipSpaceA(&bottom);
	p = bottom;
											// 行末&コメントの検索 ------------
	for ( ; *p != '\0' ; p++ ){
#if 0
		if ( (*p == ';') && !end ){				// コメント
			end = p;
			continue;
		}
#endif
		if ( (*p == '\r') || (*p == '\n') ){	// CR/LF
			if ( end == NULL ) end = p;
			p++;
			while ( (*p == '\r') || (*p == '\n') ) p++;
			break;
		}
	}
	*offset = (DWORD)(p - cfg->bottom);
	if ( end == NULL ) end = p;
											// 行末の空白を削除 ---------------
	while ( (end > bottom) && (*(end - 1) == ' ') ) end--;
											// 複写 & TAB 除去 ----------------
	for ( ; bottom < end ; bottom++ ){
		if ( *bottom == '\t' ) continue;
		*string++ = *bottom;
	}
	*string = '\0';
	return TRUE;
}
//------------------------------------- パラメータを抽出する
char *CFGFixParam(char *string, char **next)
{
	char *ptr, *q;

	SkipSpaceA(&string);
	ptr = string;
	while ( *ptr && (*ptr != ',') ) ptr++;
	q = ptr;
	if ( *ptr == ',' ) ptr++;
	while ( (q > string) && (*(q - 1) == ' ') ) q--;
	*q = '\0';

	if ( next ) *next = ptr;
	return string;
}
