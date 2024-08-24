/*-----------------------------------------------------------------------------
	定義ファイル操作パッケージ	※縮小版						(c)TORO 2000
-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char	*bottom;			// メモリ上のファイルの先頭
	DWORD	size;				// ファイルの大きさ(メモリ上はもう少し大きい)
	int		write;				// 変更があれば真
//	TCHAR	filename[MAX_PATH];	// 定義ファイル名(書き込み用)
} CFG;
//======================================================================== 雑用
extern void USEFASTCALL SkipSpaceB(char *b, char *p);
extern BYTE GetLineParam(TCHAR **str, TCHAR *param);
extern BYTE USEFASTCALL SkipSpaceA(char **str);
#ifdef UNICODE
extern WORD USEFASTCALL SkipSpaceW(TCHAR **str);
#define SkipSpace SkipSpaceW
#else
#define SkipSpace SkipSpaceA
#endif

//============================================================ 定義ファイル操作
// filename で示した定義ファイルをメモリ上に取込み、操作可能にする
extern BOOL CFGOpen(CFG *cfg, WCHAR *filename);

// メモリ上の定義ファイルを閉じる。
extern BOOL CFGClose(CFG *cfg);

// 指定の offset から、一行分を読み込み string に書き込む。
// offset には次の行の先頭を書く
extern BOOL USEFASTCALL CFGGetLine(CFG *cfg, DWORD *offset, char *string);

// パラメータを抽出する
extern char *CFGFixParam(char *string, char **next);

#ifdef __cplusplus
}
#endif // __cplusplus
