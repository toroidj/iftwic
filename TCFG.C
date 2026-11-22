/*-----------------------------------------------------------------------------
	定義ファイル操作パッケージ 簡易版							(c)TORO 2000
-----------------------------------------------------------------------------*/
#define STRICT
#include <windows.h>
#include <string.h>
#include "TOROWIN.H"
#include "TCFG.H"

//======================================================================== 雑用
// sizeptr 分だけ確実に読み込む ReadFile
BOOL SizedReadFile(_In_ HANDLE hFile, _Out_ void *buffer, _In_ DWORD size, _Inout_opt_ DWORD *sizeptr)
{
	DWORD readsize, totalsize = 0;
	char *bufptr = buffer;

	for(;;){
		if ( ReadFile(hFile, bufptr, size, &readsize, NULL) ){
			totalsize += readsize;
			if ( size <= readsize ) break;
			if ( readsize != 0 ){
				size -= readsize;
				bufptr += readsize;
				continue;
			}
			// EOF 到着。*sizeptr 以上読み込んでいたら正常終了
			if ( (sizeptr != NULL) && (*sizeptr <= totalsize ) ) break;
			SetLastError(ERROR_READ_FAULT);
		}
		if ( sizeptr != NULL ) *sizeptr = totalsize;
		return FALSE;
	}
	if ( sizeptr != NULL ) *sizeptr = totalsize;
	return TRUE;
}

// size 分だけ確実に書き込む WriteFile
BOOL SizedWriteFile(_In_ HANDLE hFile, _In_ const void *buffer, _In_ DWORD size)
{
	DWORD writesize;
	const char *bufptr = buffer;

	for(;;){
		if ( WriteFile(hFile, bufptr, size, &writesize, NULL) ){
			if ( size <= writesize ) return TRUE;
			if ( writesize != 0 ){
				size -= writesize;
				bufptr += writesize;
				continue;
			}
			// 書き込みできなくなった
			SetLastError(ERROR_WRITE_FAULT);
		}
		return FALSE;
	}
}

/*-------------------------------------
	行末(\0,\a,\d)か？

RET:	==0 :EOL	!=0 :Code
--------------------------------------*/
BYTE USEFASTCALL IsEOLA(const char **str)
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

/*-------------------------------------
	空白(space, tab）をスキップする

RET:	==0 :EOL	!=0 :Code
--------------------------------------*/
BYTE USEFASTCALL SkipSpaceA(const char **str)
{
	BYTE code;

	for(;;){
		code = IsEOLA(str);
		if ( (code != ' ') && (code != '\t') ) break;
		(*str)++;
	}
	return code;
}

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
//============================================================ 定義ファイル操作
//------------------------------------- 定義ファイルを読み込む
BOOL CFGOpen(CFG *cfg, WCHAR *filename)
{
	HANDLE hFile;
	BOOL result = TRUE;
	LARGE_INTEGER filesize;

	cfg->bottom = NULL;
	cfg->size = 0;
//	cfg->write = 0;
//	tstrcpy(cfg->filename, filename);

	hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;
	if ( (GetFileSizeEx(hFile, &filesize) == FALSE) || (filesize.QuadPart > 0x10000000) ){ // エラー若しくは大きすぎの場合はエラー
		cfg->bottom = NULL;
	}else{
		cfg->size = (DWORD)filesize.QuadPart;
		cfg->bottom = (char *)HeapAlloc(GetProcessHeap(), 0, cfg->size + CFG_MARGIN);
	}
	if ( cfg->bottom == NULL ){
		CloseHandle(hFile);
		return FALSE;
	}
	result = SizedReadFile(hFile, cfg->bottom, cfg->size, &cfg->size);
	CloseHandle(hFile);
	if ( result ){
		// UTF8HEADER があれば空白に変換。CFGGetLine で読み込むこと前提
		if ( (cfg->size >= UTF8HEADERSIZE) &&
			 (memcmp(cfg->bottom, UTF8HEADER, UTF8HEADERSIZE) == 0) ){
			cfg->bottom[0] = ' ';
			cfg->bottom[1] = ' ';
			cfg->bottom[2] = ' ';
		}
		memset(cfg->bottom + cfg->size, 0, CFG_MARGIN);
	}else{
		CFGClose(cfg);
	}
	return result;
}
//------------------------------------- 定義ファイルを閉じる
BOOL CFGClose(CFG *cfg)
{
	if ( (cfg == NULL) || (cfg->bottom == NULL) ) return FALSE;
	HeapFree(GetProcessHeap(), 0, cfg->bottom);
	cfg->bottom = NULL;
	return TRUE;
}
/*-------------------------------------
	指定の offset から、一行分を読み込み
	string に書き込む。
	offset には次の行の先頭を書く
-------------------------------------*/
BOOL USEFASTCALL CFGGetLine(CFG *cfg, DWORD *offset, char *string, DWORD maxlength)
{
	const char *bottom, *p, *end = NULL, *strmax;

	if ( cfg->bottom == NULL ) return FALSE;
	bottom = cfg->bottom + *offset;
	if ( *bottom == '\0' ) return FALSE;
											// 先頭の空白を削除 ---------------
	SkipSpaceA(&bottom);
	p = bottom;
											// 行末&コメントの検索 ------------
	for ( ; *p != '\0' ; p++ ){
#if 0
		if ( *p == ';' ){
			if ( (*(p + 1) != ';') && (end == NULL) ){ // コメント
				end = p;
				continue;
			}else{
				p++;
			}
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
	strmax = string + maxlength - 1;
	for ( ; bottom < end ; bottom++ ){
		if ( *bottom == '\t' ) continue;
//		if ( *bottom == ';' ) bottom++;
		*string++ = *bottom;
		if ( string >= strmax ) break;
	}
	*string = '\0';
	return TRUE;
}
//------------------------------------- パラメータを抽出する
char *CFGFixParam(char *string, char **next)
{
	char *ptr, *lastptr;

	SkipSpaceA((const char **)&string);
							// 項目区切りを検索
	ptr = string;
	while ( *ptr && (*ptr != ',') ) ptr++;
	lastptr = ptr;
	if ( *ptr == ',' ) ptr++;	// 次の項目の頭出し
							// 末尾の空白を除去
	while ( (lastptr > string) && (*(lastptr - 1) == ' ') ) lastptr--;
	*lastptr = '\0';

	if ( next != NULL ) *next = ptr;
	return string;
}
