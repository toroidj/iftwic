/*-----------------------------------------------------------------------------
	Windows Imaging Component Susie Plug-in		Copyright (c) TORO
 ----------------------------------------------------------------------------*/
#define STRICT
#define UNICODE
#undef WINVER
#define WINVER 0x0501
#include <windows.h>
#include <shlobj.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include "TOROWIN.H"
#include "TCFG.H"
#include "iftwic.h"

// 設定
char DefaultDisableTypes[] = "bmp"; // 拡張子１つのみ記載可能(DumpConfig)

#ifndef FORVIEW // 透過処理等を自前でするアプリケーション向け設定
int X_ExifRotate = 1;			// 回転するか (bool)
int X_Conv32NoAlpha = 0;		// alpha なし 32bit 画像を 24bit にする (bool)
int X_WipeAlphaOnPalette = 0;	// パレットの alpha をなくす (bool)
int X_SusieFix = 0;				// Susie 向けの調整をするか
int X_MergeAlpha = 0;			// 透過調整
int X_FixAspect = 0;			// アスペクト比調整
COLORREF X_GridColor1 = 0xd8d8d8;	// グリッド一色目
COLORREF X_GridColor2 = 0xe0e0e0;	// グリッド二色目
int X_TinyPage = 0;				// 簡易ページめくり

#else // 単に表示できればよいアプリケーション向け設定
int X_ExifRotate = 1;			// 回転するか (bool)
int X_Conv32NoAlpha = 0;		// alpha なし 32bit 画像を 24bit にする (bool)
int X_WipeAlphaOnPalette = 1;	// パレットの alpha をなくす (bool)
int X_SusieFix = 0;				// Susie 向けの調整をするか
int X_MergeAlpha = 1;			// 透過調整
int X_FixAspect = 1;			// アスペクト比調整
COLORREF X_GridColor1 = 0xe0e0e0;	// グリッド一色目
COLORREF X_GridColor2 = 0xe0e0e0;	// グリッド二色目
int X_TinyPage = 0;				// 簡易ページめくり
#endif

int X_InfoText = 0;				// Exif等の情報を出力

#define XCP_OFF 0
#define XCP_sRGB 1 // sRGB に変換
#define XCP_Embed 2 // BMPv5として埋め込む
int X_ColorProfile = XCP_OFF;

#define SAVE_NOSETTING -1 // 不明
#define SAVE_NONE 0 // 存在しない
#define SAVE_SHARE 1 // exeと同じ場所
#define SAVE_USERAPP 2 // %APPDATA%
int X_SaveAppData = SAVE_NOSETTING;

DWORD default_flags; // X_xxx の要約 SUSIE_DECODE_REQUEST_xxx

// codec 情報
struct CODECLISTITEMSTRUCT {
	WCHAR *ext;
	CLSID cid;
	BOOL enable;
};

struct CODECSSTRUCT {
	CODECLISTITEMSTRUCT *list; // コーデック毎の内容
	int count;		// コーデック数
} codecs = { NULL, -1 };
#define EXTCODECS 1	// 内蔵コーデックの数

// その他
const BITMAPINFOHEADER DefaultBmpInfo =
	{sizeof(BITMAPINFOHEADER), 0, 0, 1, 24, BI_RGB, 0, 3000, 3000, 0, 0};

const char CurHeader[4]  = {0, 0, 2, 0};
const char RiffHeader[4] = {'R', 'I', 'F', 'F'};
const char ACONHeader[4] = {'A', 'C', 'O', 'N'};
const char WebpHeader[4] = {'W', 'E', 'B', 'P'};
const char PngHeader[4]  = {'\x89', 'P', 'N', 'G'};

const GUID IID_ANICUR = { 0x10ae54b3, 0x4838, 0x4baa, { 0xa1, 0xa8, 0x75, 0xb, 0xc7, 0xdd, 0x52, 0xbb } };
const CODECLISTITEMSTRUCT CurAniCodec = { L"\5cur\0\5ani\0\0", IID_ANICUR, TRUE };

typedef BOOL (STDAPICALLTYPE *DSHGETSPECIALFOLDERPATH)(HWND hwnd, TCHAR *pszPath, int csidl, BOOL fCreate);

const WCHAR decoder_none[1] = L"";

// 内部情報
#define SUSIE_DECODE_SUSIE 0x80000000 // InitSusieDecoder を使っているとき

// WIC 初期化状況
#define ESV_NOINIT -1 // 未初期化
#define ESV_ENABLED 0 // 初期化済み
#define ESV_NOWINIT 1 // 初期化中
volatile LONG EnabledSettings = ESV_NOINIT; // 設定の初期化済み？

HINSTANCE DLLhInst;

//		直前に取り扱った画像の情報
const WCHAR *previous_decoder = decoder_none; // 直前に使用したデコーダ
DWORD previous_decoder_tid; // 直前のスレッドID
UINT tiny_next_page = 0; // 簡易ページめくりで前回表示したフレーム
DWORD tiny_next_id = 0;  // 簡易ページめくりで前回表示したファイルのID

#define TEXTLINESIZE 0x1000

IID xGUID_WICPixelFormat8bppAlpha = {0xe6cd0116, 0xeeba, 0x4161, {0xaa, 0x85, 0x27, 0xdd, 0x9f, 0xb3, 0xa8, 0x95}};
IID xGUID_WICPixelFormat16bppBGRA5551 = {0x05ec7c2b, 0xf1e6, 0x4961, {0xad, 0x46, 0xe1, 0xcc, 0x81, 0x0a, 0x87, 0xd2}};
IID xGUID_WICPixelFormat32bppPRGBA = {0x3cc4a650, 0xa527, 0x4d37, {0xa9, 0x16, 0x31, 0x42, 0xc7, 0xeb, 0xed, 0xba}};
IID xGUID_WICPixelFormat32bppRGBA = {0xf5c7ad2d, 0x6a8d, 0x43dd, {0xa7, 0xa8, 0xa2, 0x99, 0x35, 0x26, 0x1a, 0xe9}};
IID xGUID_WICPixelFormat64bppBGRA = {0x1562ff7c, 0xd352, 0x46f9, {0x97, 0x9e, 0x42, 0x97, 0x6b, 0x79, 0x22, 0x46}};
IID xGUID_WICPixelFormat64bppPBGRA = {0x8c518e8e, 0xa4ec, 0x468b, {0xae, 0x70, 0xc9, 0xa3, 0x5a, 0x9c, 0x55, 0x30}};
IID xGUID_WICPixelFormat64bppBGRAFixedPoint = {0x356de33c, 0x54d2, 0x4a23, {0xbb, 0x4, 0x9b, 0x7b, 0xf9, 0xb1, 0xd4, 0x2d}};
IID xGUID_WICPixelFormat64bppPRGBAHalf = {0x58ad26c2, 0xc623, 0x4d9d, {0xb3, 0x20, 0x38, 0x7e, 0x49, 0xf8, 0xc4, 0x42}};
IID xGUID_WICPixelFormat32bppRGBA1010102 = {0x25238D72, 0xFCF9, 0x4522, {0xb5, 0x14, 0x55, 0x78, 0xe5, 0xad, 0x55, 0xe0}};
IID xGUID_WICPixelFormat32bppRGBA1010102XR = {0x00DE6B9A, 0xC101, 0x434b, {0xb5, 0x02, 0xd0, 0x16, 0x5e, 0xe1, 0x12, 0x2c}};
IID xGUID_WICPixelFormat32bppR10G10B10A2 = {0x604e1bb5, 0x8a3c, 0x4b65, {0xb1, 0x1c, 0xbc, 0x0b, 0x8d, 0xd7, 0x5b, 0x7f}};
IID xGUID_WICPixelFormat32bppR10G10B10A2HDR10 = {0x9c215c5d, 0x1acc, 0x4f0e, {0xa4, 0xbc, 0x70, 0xfb, 0x3a, 0xe8, 0xfd, 0x28}};

LPDLGTEMPLATE GetDialogTemplate(HMODULE hinst, LPCTSTR lpszTemplate)
{
	BYTE *dialog;
	HRSRC hRc;
	DWORD dialogsize, fontlength;
	int i;
	BYTE *dp, *dpitem;
	WCHAR fontdata[LF_FACESIZE + 1];
	NONCLIENTMETRICS ncm;

	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

#ifdef UNICODE
	fontlength = TSTRSIZE(ncm.lfMessageFont.lfFaceName);
	memcpy(&fontdata[1], ncm.lfMessageFont.lfFaceName, fontlength);
	fontlength += sizeof(WCHAR);
#else
	fontlength = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
			ncm.lfMessageFont.lfFaceName, -1, &fontdata[1],
			sizeof(fontdata) / sizeof(WCHAR) - 2) + 1;
	fontlength *= sizeof(WCHAR);
#endif
									// テンプレートの読み込み
	hRc = FindResource(hinst, lpszTemplate, RT_DIALOG);
	if ( hRc == NULL ) return NULL;
	dialogsize = SizeofResource(hinst, hRc);
	dialog = (BYTE *)HeapAlloc(GetProcessHeap(), 0, dialogsize + fontlength + sizeof(WORD));
	if ( dialog == NULL ) return NULL;
	memcpy(dialog, LockResource(LoadResource(hinst, hRc)), dialogsize);

	// dp = dialog + sizeof(DLGTEMPLATE) 相当
	dp = dialog + (sizeof(DWORD) * 2 + sizeof(WORD) * 5);

	// フォントサイズ(pixelでなく、Pt)
	if ( ncm.lfMessageFont.lfHeight < 0 ){
		ncm.lfMessageFont.lfHeight = -ncm.lfMessageFont.lfHeight;
	}
	fontdata[0] = (WCHAR)ncm.lfMessageFont.lfHeight;

									// メニュー、クラス名、タイトル名を飛ばす
	for ( i = 0 ; i < 3 ; i++ ){
		WORD id;

		id = *(WORD *)dp;
		if ( id == 0 ){			// なし
			dp += sizeof(WORD);
		} else if ( id == 0xffff ){ // WORD値
			dp += sizeof(WORD) * 2;
		} else { // UCS-2 - z
			dp += sizeof(WORD);
			while ( *(WORD *)dp != 0 ) dp += sizeof(WORD);
			dp += sizeof(WORD);
		}
	}
	dpitem = dp;
									// 規定のフォントがあれば飛ばす
	if ( ((DLGTEMPLATE *)dialog)->style & DS_SETFONT ){
		dpitem += sizeof(WORD); // font point
		while ( *(WORD *)dpitem != '\0' ) dpitem += sizeof(WORD);
		dpitem += sizeof(WORD); // NIL
	}
								// dword 補正1
	if ( ALIGNMENT_BITS(dpitem) & 3 ) dpitem += sizeof(WORD);
								// dword 補正2
	if ( (ALIGNMENT_BITS(dp) + fontlength) & 3 ) fontlength += sizeof(WORD);
									// テンプレートの修正
	setflag(((DLGTEMPLATE *)dialog)->style, DS_SETFONT);
	memmove(dp + fontlength, dpitem, dialogsize - (dpitem - dialog));
	memcpy(dp, fontdata, fontlength);

	return (LPDLGTEMPLATE)dialog;
}

INT_PTR ExtDialogBoxParam(HMODULE hinst, const TCHAR *lpszTemplate, HWND hwndOwner, DLGPROC dlgprc, LPARAM lParamInit)
{
	LPDLGTEMPLATE dialog;
	INT_PTR result;

	dialog = GetDialogTemplate(hinst, lpszTemplate);
	if ( dialog == NULL ) return -1;
	result = DialogBoxIndirectParam(hinst, dialog, hwndOwner, dlgprc, lParamInit);
	HeapFree(GetProcessHeap(), 0, dialog);
	return result;
}

int USEFASTCALL GetNumber(const char *ptr)
{
	int n = 0;

	while( (*ptr >= '0') && (*ptr <= '9') ){
		n = n * 10 + (*ptr++ - '0');
	}
	return n;
}

BOOL USEFASTCALL LoadNumber(char *linebottom, char *keyname, char *param, int *data)
{
	if ( strcmp(linebottom, keyname) ) return FALSE;
	*data = GetNumber(CFGFixParam(param, NULL));
	return TRUE;
}

BOOL USEFASTCALL LoadColor(char *linebottom, char *keyname, char *param, COLORREF *data)
{
	if ( strcmp(linebottom, keyname) ) return FALSE;
	*data = GetNumber(CFGFixParam(param, &param));
	*data |= GetNumber(CFGFixParam(param, &param)) << 8;
	*data |= GetNumber(CFGFixParam(param, NULL)) << 16;
	return TRUE;
}

void USEFASTCALL MakeCfgFilename(WCHAR *filename, int WriteMode)
{
	WCHAR orgname[MAX_PATH];
	WCHAR *orgptr = NULL;
	WCHAR appdataname[MAX_PATH];
									// 取りあえずオリジナルファイル名を作成
	orgname[0] = '\0';
	if ( GetModuleFileNameW(DLLhInst, orgname, MAX_PATH) != 0 ){
		orgptr = wcsrchr(orgname, '.');
		if ( orgptr == NULL ) orgptr = orgname + wcslen(orgname);
		wcscpy(orgptr, L".cfg");
		orgptr = wcsrchr(orgname, '\\');
		if ( orgptr == NULL ) orgptr = orgname;
		if ( wcslen(orgptr) > 90 ) orgptr = NULL;
	}
	if ( orgptr == NULL ) orgptr = L"\\" UNICODESTR(APPNAME) L".cfg";
									// AppData があればそれを使う
	appdataname[0] = '\0';
	ExpandEnvironmentStringsW(L"%APPDATA%", appdataname, MAX_PATH - 100);
	wcscat(appdataname, L"\\toroid");
	if ( GetFileAttributesW(appdataname) != BADATTR ){
		wcscat(appdataname, orgptr);
	}else{
		if ( WriteMode == SAVE_USERAPP ){
			CreateDirectoryW(appdataname, NULL);
			wcscat(appdataname, orgptr);
		}else{
			appdataname[0] = '\0';
		}
	}

	if ( WriteMode == SAVE_NONE ){
		if ( (appdataname[0] != '\0') && (GetFileAttributesW(appdataname) != BADATTR) ){
			X_SaveAppData = SAVE_USERAPP;
			wcscpy(filename, appdataname);
		}else{
			X_SaveAppData = SAVE_SHARE;
			wcscpy(filename, orgname);
		}
	}else{
		if ( WriteMode == SAVE_USERAPP ){
			wcscpy(filename, appdataname);
		}else{
			wcscpy(filename, orgname);
			if ( appdataname[0] != '\0' ){ // %APPDATA% 内のファイルを削除
				DeleteFileW(appdataname);
				*wcsrchr(appdataname, '\\') = '\0';
				RemoveDirectoryW(appdataname); // toroid dir を削除
			}
		}
	}
}

void SetDefaultFlags(void)
{
	default_flags = 0;
	if ( X_ExifRotate ) default_flags |= SUSIE_DECODE_REQUEST_ROTATE;
	if ( X_MergeAlpha ) default_flags |= SUSIE_DECODE_REQUEST_MERGE_ALPHA;
	if ( X_FixAspect )  default_flags |= SUSIE_DECODE_REQUEST_FIX_ASPECT;
	if ( X_ColorProfile != XCP_OFF ){
		if ( X_ColorProfile == XCP_sRGB ){
			default_flags |= SUSIE_DECODE_REQUEST_FIX_sRGB;
		}else{ // XCP_Embed
			default_flags |= SUSIE_DECODE_ALLOW_BMPV5 | SUSIE_DECODE_REQUEST_COLOR_PROFILE;
		}
	}
}

#define CodecIs_Enable 0x2000
#define CodecIs_Disable 0x1000
#define CodecIs_Check(hDlg, IDV, index) ((((UINT)SendDlgItemMessageW((hDlg), (IDV), LVM_GETITEMSTATE, (WPARAM)(index), LVIS_STATEIMAGEMASK)) >> 12) - 1)


void DumpConfig(HWND hDlg, int WriteMode)
{
	HANDLE hFile;
	char buf[TEXTLINESIZE], extbuf[MAX_PATH];
	DWORD size;
	WCHAR CFGPath[MAX_PATH];	// 定義ファイル名

	MakeCfgFilename(CFGPath, WriteMode);
	hFile = CreateFileW(CFGPath, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) return;
	size = wsprintfA(buf,
			"ColorProfile\t= %d\r\n"
			"ExifRotate\t= %d\r\n"
			"Conv32NoAlpha\t= %d\r\n"
			"WipeAlphaOnPalette\t= %d\r\n"
			"MergeAlpha\t= %d\r\n"
			"FixAspect\t= %d\r\n"
			"SusieFix\t= %d\r\n"
			"GridColor1\t= %d,%d,%d\r\n"
			"GridColor2\t= %d,%d,%d\r\n"
			"TinyPage\t= %d\r\n"
			"InfoText\t= %d\r\n",
			X_ColorProfile,
			X_ExifRotate,
			X_Conv32NoAlpha,
			X_WipeAlphaOnPalette,
			X_MergeAlpha,
			X_FixAspect,
			X_SusieFix,
			(X_GridColor1 & 0xff), ((X_GridColor1 >> 8) & 0xff), X_GridColor1 >> 16,
			(X_GridColor2 & 0xff), ((X_GridColor2 >> 8) & 0xff), X_GridColor2 >> 16,
			X_TinyPage,
			X_InfoText);
	{
		CODECLISTITEMSTRUCT *codeclistp = codecs.list;
		int leftcodecs = codecs.count, index = 0;

		// コーデックの有効状態がデフォルトと同じかどうかをチェックする
		while ( index < codecs.count ){
			// VM_GETITEMSTATE が エラーのときは、チェック有り扱い
			codeclistp->enable = CodecIs_Check(hDlg, IDV_CODECS, index);

			if ( wcscmp(codeclistp->ext + 1, L"bmp") == 0 ){
				if ( codeclistp->enable ) break;
			}else{
				if ( !codeclistp->enable ) break;
			}
			codeclistp++;
			index++;
		}
		if ( index == codecs.count ){ // 全てデフォルトと同じ
			size += wsprintfA(buf + size, "Disables\t= %s\r\n", DefaultDisableTypes);
			SizedWriteFile(hFile, buf, size); // 失敗しても保存可能なところは保存
		}else{ // デフォルトと異なるため、個別に設定を保存
			SizedWriteFile(hFile, buf, size); // 失敗しても保存可能なところは保存
			codeclistp = codecs.list;
			leftcodecs = codecs.count;
			index = 0;

			while ( leftcodecs-- ){
				codeclistp->enable = CodecIs_Check(hDlg, IDV_CODECS, index);
				if ( codeclistp->enable == FALSE ){
					LPOLESTR cidstr;

					StringFromCLSID(codeclistp->cid, &cidstr);
					UnicodeToAnsi(cidstr, extbuf, MAX_PATH);
					CoTaskMemFree(cidstr);
					size = wsprintfA(buf, "%s =\r\n", extbuf);
					SizedWriteFile(hFile, buf, size); // 失敗しても保存可能なところは保存
				}
				codeclistp++;
				index++;
			}
		}
	}
	CloseHandle(hFile);
}

void SetDisables(char *extlist)
{
	WCHAR bufw[TEXTLINESIZE], *extp, *nextextp;

	if ( AnsiToUnicode(extlist, bufw, TEXTLINESIZE) == 0 ) return;
	extp = bufw;
	while ( *extp != '\0' ){
		CODECLISTITEMSTRUCT *codeclistp;
		int leftcodecs;

		nextextp = wcschr(extp, ',');
		if ( nextextp != NULL ) *nextextp++ = '\0';

		codeclistp = codecs.list;
		leftcodecs = codecs.count;
		while ( leftcodecs-- ){
			if ( wcscmp(codeclistp->ext + 1, extp) == 0 ){
				codeclistp->enable = FALSE;
				break;
			}
			codeclistp++;
		}
		if ( nextextp == NULL ) break;
		extp = nextextp;
	}
}

void SetDisableCodec(char *strcid)
{
	WCHAR bufW[128];
	CLSID cid;
	CODECLISTITEMSTRUCT *codeclistp;
	int leftcodecs;

	if ( AnsiToUnicode(strcid, bufW, 128) == 0 ) return;
	if ( FAILED(CLSIDFromString(bufW, &cid)) ) return;

	codeclistp = codecs.list;
	leftcodecs = codecs.count;
	while ( leftcodecs-- ){
		if ( memcmp(&codeclistp->cid, &cid, sizeof(cid)) == 0 ){
			codeclistp->enable = FALSE;
			break;
		}
		codeclistp++;
	}
}

void LoadCfg(void)
{
	DWORD offset = 0;
	char buf[TEXTLINESIZE];
	char *ptr;
	CFG cfg;
	WCHAR CFGPath[MAX_PATH];	// 定義ファイル名
	BOOL DoneDisables = FALSE;

	MakeCfgFilename(CFGPath, 0);

	if ( CFGOpen(&cfg, CFGPath) == FALSE ){
		X_SaveAppData = SAVE_NONE;
		SetDisables(DefaultDisableTypes);
		SetDefaultFlags();
		return;
	}
	while ( CFGGetLine(&cfg, &offset, buf, sizeof(buf)) ){
		ptr = strchr(buf, '=');
		if ( ptr == NULL ) continue;
														// キーワード部分を抽出
		SkipSpaceB(buf, ptr - 1);
		ptr++;
		if ( FALSE != LoadNumber(buf, "ColorProfile", ptr, &X_ColorProfile) ) continue;
		if ( FALSE != LoadNumber(buf, "ExifRotate", ptr, &X_ExifRotate) ) continue;
		if ( FALSE != LoadNumber(buf, "Conv32NoAlpha", ptr, &X_Conv32NoAlpha) ) continue;
		if ( FALSE != LoadNumber(buf, "WipeAlphaOnPalette", ptr, &X_WipeAlphaOnPalette) ) continue;
		if ( FALSE != LoadNumber(buf, "SusieFix", ptr, &X_SusieFix) ) continue;
		if ( FALSE != LoadColor(buf, "GridColor1", ptr, &X_GridColor1) ) continue;
		if ( FALSE != LoadColor(buf, "GridColor2", ptr, &X_GridColor2) ) continue;
		if ( FALSE != LoadNumber(buf, "FixAspect", ptr, &X_FixAspect) ) continue;
		if ( FALSE != LoadNumber(buf, "TinyPage", ptr, &X_TinyPage) ) continue;
		if ( FALSE != LoadNumber(buf, "InfoText", ptr, &X_InfoText) ) continue;
		if ( FALSE != LoadNumber(buf, "MergeAlpha", ptr, &X_MergeAlpha) ){
			DoneDisables = TRUE;
			continue;
		}
		if ( strcmp(buf, "Disables") == 0 ){
			SkipSpaceA((const char **)&ptr);
			SetDisables(ptr);
			DoneDisables = TRUE;
		}
		if ( buf[0] == '{' ){
			SetDisableCodec(buf);
			DoneDisables = TRUE;
		}
	}
	CFGClose(&cfg);
	SetDefaultFlags();
	if ( DoneDisables == FALSE ) SetDisables(DefaultDisableTypes);

}

// WIC Plugin を無効化する
// 無効のときは、ConfigurationDlg の IDV_CODECS の 表示で判断可能
void DisablePlugin(void)
{
	codecs.count = 0;
	InterlockedExchange(&EnabledSettings, ESV_ENABLED);
}

//	WICのコーデック一覧を取得
int LoadCodecList(void)
{
	IWICImagingFactory *wicimage;
	IEnumUnknown *codecenum;
	IUnknown *enumitem;
	int allocsize = 8;
	HRESULT InitResult;
	int codecscount;
	int settings;
	CODECLISTITEMSTRUCT *codecslist;

	for(;;){
		settings = InterlockedCompareExchange(&EnabledSettings, ESV_NOWINIT, ESV_NOINIT);
		if ( settings == ESV_ENABLED ) return codecs.count;
		if ( settings == ESV_NOINIT ) break;
		// if ( settings == ESV_NOWINIT ) // 初期化待機
		Sleep(10);
		continue;
	}
	// ESV_NOWINIT
	codecslist = static_cast<CODECLISTITEMSTRUCT *>(HeapAlloc(
			GetProcessHeap(), 0, sizeof(CODECLISTITEMSTRUCT) * allocsize));
	if ( codecslist == NULL ){ // メモリを再確保できる場合があるので、元に戻す
		InterlockedExchange(&EnabledSettings, ESV_NOINIT);
		return 0;
	}
	codecscount = 0;

	InitResult = ::CoCreateInstance(CLSID_WICImagingFactory, NULL,
			CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void**>(&wicimage));
	if ( FAILED(InitResult) ){
		if ( InitResult == CO_E_NOTINITIALIZED ){
			CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
			if( FAILED(::CoCreateInstance(CLSID_WICImagingFactory, NULL,
					CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
					reinterpret_cast<void**>(&wicimage))) ){
				CoUninitialize();
				DisablePlugin();
				return 0;
			}
		}else{
			DisablePlugin();
			return 0;
		}
	}

	if ( SUCCEEDED(wicimage->CreateComponentEnumerator(WICDecoder,
			WICComponentEnumerateDefault, &codecenum)) ){
		while ( S_OK == codecenum->Next(1, &enumitem, 0) ){
			IWICBitmapDecoderInfo *wicinfo;
			HRESULT result;

			result = enumitem->QueryInterface(IID_IWICBitmapDecoderInfo, (void **)&wicinfo);
			enumitem->Release();
			if ( FAILED(result) ) continue;

			if ( allocsize <= (codecscount + 1) ){ // 一覧リサイズ
				allocsize += 8;
				CODECLISTITEMSTRUCT *newptr = static_cast<CODECLISTITEMSTRUCT *>
						(HeapReAlloc(GetProcessHeap(), 0, codecslist,
								sizeof(CODECLISTITEMSTRUCT) * allocsize));
				if ( newptr == NULL ) break;
				codecslist = newptr;
			}

			UINT extlen = 0;

			if ( SUCCEEDED(wicinfo->GetFileExtensions(0, 0, &extlen)) &&
				 (extlen != 0) ){
				WCHAR *cptr, *cnptr;

				codecslist[codecscount].enable = TRUE;
				codecslist[codecscount].ext = cptr = static_cast<WCHAR *>(
						HeapAlloc(GetProcessHeap(), 0,
						sizeof(WCHAR) * (extlen + 2)));
				if ( codecslist[codecscount].ext != NULL ){
					// .jpeg,.jpe,.jpg,.jfif,.exif 形式で取得できる。
					wicinfo->GetFileExtensions(extlen,
							codecslist[codecscount].ext, &extlen);
					cptr[extlen] = '\0';
					cptr[extlen + 1] = '\0';
					wicinfo->GetCLSID(&codecslist[codecscount].cid);
					for (;;) { // (skip1)ext1\0 ... (skipn)extn\0 \0 に変換
						cnptr = wcschr(cptr, ',');
						if ( cnptr == NULL ) { // 最後
							*cptr = static_cast<WCHAR>(wcslen(cptr)) + 1;
							break;
						}

						*cptr = static_cast<WCHAR>(cnptr - cptr) + 1;
						cptr = cnptr + 1;
						*cnptr = '\0';
					}
					codecscount++;
				}
			}
			wicinfo->Release();
		}
		codecenum->Release();
	}
	wicimage->Release();

	if ( codecscount > 0 ){
		codecslist[codecscount] = CurAniCodec;
		codecs.list = codecslist;
		codecs.count = codecscount + EXTCODECS;

		if ( X_SaveAppData < SAVE_NONE ) LoadCfg();
	}else{
		HeapFree(GetProcessHeap(), 0, codecslist);
	}

	if ( InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();

	if ( codecs.count <= 0 ){
		DisablePlugin();
	}else{
		InterlockedExchange(&EnabledSettings, ESV_ENABLED);
	}
	return codecs.count;
}

void SetCodecInfo(HWND hCodecsListWnd, int codecscount, IWICComponentInfo *wicinfo)
{
	WCHAR buf[TEXTLINESIZE];
	WCHAR *extlist = codecs.list[codecscount].ext;
	WCHAR *extdst = buf;
	UINT len;
	LV_ITEM lvi;

	for (;;){
		wcscpy(extdst, extlist + 1);
		extdst += wcslen(extdst);
		extlist += *extlist;
		if ( *extlist == 0 ) break;
		*extdst++ = ',';
	}
	lvi.pszText = buf;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = codecscount;
	lvi.iSubItem = 0;
	lvi.iItem = ListView_InsertItem(hCodecsListWnd, &lvi);

	if ( wicinfo != NULL ){
		if ( SUCCEEDED(wicinfo->GetFriendlyName(TSIZEOF(buf), buf, &len)) ){
			lvi.iSubItem = 1;
			ListView_SetItem(hCodecsListWnd, &lvi);
		}
		if ( SUCCEEDED(wicinfo->GetVersion(TSIZEOF(buf), buf, &len)) ){
			lvi.iSubItem = 2;
			ListView_SetItem(hCodecsListWnd, &lvi);
		}
		if ( SUCCEEDED(wicinfo->GetAuthor(TSIZEOF(buf), buf, &len)) ){
			lvi.iSubItem = 3;
			ListView_SetItem(hCodecsListWnd, &lvi);
		}
	}else{ // cur / ani 用
		wcscpy(buf, (codecscount == (codecs.count - 1)) ?
				L"Embedded animation cursor" : L"?");
		lvi.iSubItem = 1;
		ListView_SetItem(hCodecsListWnd, &lvi);
	}

	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = codecs.list[codecscount].enable ? CodecIs_Enable : CodecIs_Disable;
	SendMessage(hCodecsListWnd, LVM_SETITEMSTATE, (WPARAM)codecscount, (LPARAM)&lvi);
}

//	WICのコーデック一覧を設定
void ListCodecList(HWND hCodecsListWnd)
{
	IWICImagingFactory *wicimage;
	HRESULT InitResult;
	int codecscount;
	int strdelta = (LOWORD(GetUserDefaultLCID()) != LCID_JAPANESE) ? 0 : IDS_STRINGJP;
	WCHAR buf[TEXTLINESIZE];
	LV_COLUMN lvc;

	SendMessage(hCodecsListWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = 200;
	LoadString(DLLhInst, IDS_COLUMN1E + strdelta, buf, TSIZEOF(buf));
	lvc.pszText = buf;
	ListView_InsertColumn(hCodecsListWnd, 0, &lvc);
	LoadString(DLLhInst, IDS_COLUMN2E + strdelta, buf, TSIZEOF(buf));
	ListView_InsertColumn(hCodecsListWnd, 1, &lvc);
	LoadString(DLLhInst, IDS_COLUMN3E + strdelta, buf, TSIZEOF(buf));
	ListView_InsertColumn(hCodecsListWnd, 2, &lvc);
	LoadString(DLLhInst, IDS_COLUMN4E + strdelta, buf, TSIZEOF(buf));
	ListView_InsertColumn(hCodecsListWnd, 3, &lvc);

	if ( codecs.count <= 0 ) return;

	InitResult = ::CoCreateInstance(CLSID_WICImagingFactory, NULL,
			CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void**>(&wicimage));
	if ( FAILED(InitResult) ){
		if ( InitResult == CO_E_NOTINITIALIZED ){
			CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
			if( FAILED(::CoCreateInstance(CLSID_WICImagingFactory, NULL,
					CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
					reinterpret_cast<void**>(&wicimage))) ){
				CoUninitialize();
				return;
			}
		}else{
			return;
		}
	}

	for ( codecscount = 0 ; codecscount < (codecs.count - 1) ; codecscount++ ){
		IWICComponentInfo *wicinfo;

		if ( SUCCEEDED(wicimage->CreateComponentInfo(codecs.list[codecscount].cid, &wicinfo)) ){
			SetCodecInfo(hCodecsListWnd, codecscount, wicinfo);
			wicinfo->Release();
		}else{
			SetCodecInfo(hCodecsListWnd, codecscount, NULL);
		}
	}
	wicimage->Release();
	SetCodecInfo(hCodecsListWnd, codecs.count - 1, NULL); // .cur .ani
	if ( InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();

	SendMessage(hCodecsListWnd, LVM_SETCOLUMNWIDTH, 1, LVSCW_AUTOSIZE);
	SendMessage(hCodecsListWnd, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE);
	SendMessage(hCodecsListWnd, LVM_SETCOLUMNWIDTH, 3, LVSCW_AUTOSIZE);
	return;
}

class WIC_Susie_class
{
	public:
		int PixelConvert; // 色数変更が必要
		int AlphaConvert; // 透過処理が必要

HRESULT Init(void)
{
	HRESULT result;

	m_InitResult = result = ::CoCreateInstance(CLSID_WICImagingFactory, NULL,
			CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void**>(&m_image));
	if ( m_InitResult == CO_E_NOTINITIALIZED ){
		CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
		result = ::CoCreateInstance(CLSID_WICImagingFactory, NULL,
				CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
				reinterpret_cast<void**>(&m_image));
		if ( FAILED(result) ) CoUninitialize();
	}
	return result;
}

void FixAnimateicon(BYTE *memptr, const BYTE *orgmem, LONG_PTR len)
{
	const BYTE *imgptr, *imgmax;
	DWORD frame_no;
	size_t maxlen;

	imgptr = orgmem + 12; // ANIHEADER
	imgmax = orgmem + len; // len は 0x30 以上が保障済み

	frame_no = m_frame_no;
	for (;;){
		DWORD chunksize;

		maxlen = imgmax - imgptr;
		if ( maxlen < 8 ) break;
		chunksize = *(DWORD *)(imgptr + 4);
		if ( chunksize > (maxlen - 8) ) break;
		if ( (maxlen >= 12) && !memcmp(imgptr, "LIST", 4) && !memcmp(imgptr + 8, "fram", 4) ){
			const BYTE *frameptr, *framemax;

			frameptr = imgptr + 12;
			framemax = imgptr + chunksize + 8;
			if ( framemax > imgmax ) framemax = imgmax;
			for (;;){
				DWORD dataleft, datasize;

				dataleft = (DWORD)(size_t)(framemax - frameptr);
				if ( dataleft < 8 ) break;
				datasize = *(DWORD *)(frameptr + 4);
				if ( datasize > (dataleft - 8) ) break;
				if ( !memcmp(frameptr, "icon", 4) ){
					if ( frame_no != 0 ){
						frame_no--;
					}else{
						memcpy(memptr, frameptr + 8, datasize);
						return;
					}
				}
				frameptr += datasize + 8;
			}
		}else if ( (maxlen >= 0x28) && !memcmp(imgptr, "anih", 4) ){
			m_frame_count = *(DWORD *)(BYTE *)(imgptr + 0x10); // dwSteps
			if ( m_frame_count > 1 ){
				m_frame_rate = *(DWORD *)(BYTE *)(imgptr + 0x24) * 16; // dwDefaultRate
				if ( m_frame_rate == 0 ) m_frame_rate = 20;
			}
			if ( frame_no >= m_frame_count ) frame_no = 0;
		}
		imgptr += chunksize + 8;
	}
}

SUSIE_ERROR InitDecoderIconFile(SUSIE_DECODE_PARAMETERS *params)
{
	const WCHAR *ext;
	char *memimage;
	HANDLE hFile;
	SUSIE_DECODE_PARAMETERS icon_params;
	int result;
	LARGE_INTEGER filesize;

	ext = wcsrchr(params->filename, '.');
	if ( (ext == NULL) ||
		 ((wcsicmp(ext, L".ani") != 0) &&
		  (wcsicmp(ext, L".cur") != 0) &&
		  (wcsicmp(ext, L".ico") != 0) )){
		return SUSIEERROR_UNKNOWNFORMAT;
	}

	hFile = CreateFileW(params->filename, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) return SUSIEERROR_UNKNOWNFORMAT;

	// エラー若しくは大きすぎの場合はエラー
	if ( (FALSE == GetFileSizeEx(hFile, &filesize)) || (filesize.QuadPart > 0x10000000) ){
		result = SUSIEERROR_FAULTREAD;
	}else{
		memimage = static_cast<char *>(HeapAlloc(GetProcessHeap(), 0, (SIZE_T)filesize.QuadPart));
		if ( memimage != NULL ){
			if ( SizedReadFile(hFile, memimage, (DWORD)filesize.QuadPart, NULL) ){
				memset(&icon_params, 0, sizeof(icon_params));
				icon_params.struct_size = sizeof(icon_params);
				icon_params.input_flags = params->input_flags;
				icon_params.page_number = params->page_number;

//				icon_params.filename = NULL;
				icon_params.fileimage = memimage;
				icon_params.image_size = (LONG_PTR)filesize.QuadPart;

				result = InitDecoder(&icon_params);
			}else{
				result = SUSIEERROR_FAULTREAD;
			}
			HeapFree(GetProcessHeap(), 0, memimage);
		}else{
			result = SUSIEERROR_EMPTYMEMORY;
		}
	}
	CloseHandle(hFile);
	return result;
}

SUSIE_ERROR InitDecoder(SUSIE_DECODE_PARAMETERS *params)
{
	PixelConvert = 0;
	AlphaConvert = 0;

	m_SrcStream = NULL;
	m_decoder = NULL;
	m_bmp = NULL;
	m_frame = NULL;
	m_conv = NULL;
//	m_PixelInfo = NULL;
	m_metadata = NULL;
	m_colortransform = NULL;
	m_ICCcontext = NULL;
	m_Thumbnailbmp = NULL;
	m_flip = NULL;
	m_scale = NULL;
	m_bits_size = 0;
	m_ICCsize = 0;
	m_result_flags = 0;
	m_frame_count = 0;
	m_frame_no = 0;
	m_rotate = 0;

	if ( params->struct_size < sizeof(SUSIE_DECODE_PARAMETERS) ){
		return SUSIEERROR_NOTSUPPORT;
	}
	m_flags = params->input_flags;
	m_frame_no = params->page_number;

	if ( params->fileimage == NULL ){ // ファイル名のみ
		if ( params->filename == NULL ) return SUSIEERROR_FAULTREAD;
		if ( (X_TinyPage > 0) && (m_flags & SUSIE_DECODE_SUSIE) ){
			DWORD file_id = 0;
			const WCHAR *ptr = params->filename;

			while ( *ptr != '\0' ) file_id += (file_id << 2) + (DWORD)*ptr++;
			m_file_id = file_id;
		}
		if ( FAILED(m_image->CreateDecoderFromFilename(params->filename, NULL,
			 GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_decoder)) ){
			return InitDecoderIconFile(params);
		}
	}else { // メモリイメージ
		HGLOBAL hGlobal;
		BYTE *memptr;
		const BYTE *image;
		LONG_PTR size;

		size = params->image_size;
		if ( size < 4 ) return SUSIEERROR_UNKNOWNFORMAT;
		hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
		if ( hGlobal == NULL ) return SUSIEERROR_EMPTYMEMORY;

		// PNG形式icon ?
		image = (const BYTE *)params->fileimage;
		m_file_id = (DWORD)size;
		if ( (size > 0x40) && !memcmp(image, "\0\0\1", 4) ){
			DWORD offset = *(DWORD *)(BYTE *)(image + 0x12); // ICONDIRENTRY.dwBytesInRes
			if ( ((DWORD_PTR)(size - 0x40) > (DWORD_PTR)offset) &&
				 (memcmp(image + offset, "\x89PNG", 4) == 0) ){
				image = image + offset;
				size -= offset;
			}
		}

		memptr = static_cast<BYTE *>(GlobalLock(hGlobal));
		if ( memptr == NULL ){
			GlobalFree(hGlobal);
			return SUSIEERROR_EMPTYMEMORY;
		}
		memcpy(memptr, image, size);

		if ( size > 0x30 ){
			if ( !memcmp(memptr, RiffHeader, sizeof(RiffHeader)) && // .ani ?
				 !memcmp(memptr + 8, ACONHeader, sizeof(ACONHeader)) ){
				FixAnimateicon(memptr, image, size);
			}
			if ( !memcmp(memptr, CurHeader, sizeof(CurHeader)) && // .cur を .ico 扱いにする
				(*(DWORD *)(memptr + 0x12) == ((DWORD)*(WORD *)(memptr + 4) * 0x10 + 6)) ){
				*(memptr + 2) = 1;
			}
		}
		GlobalUnlock(hGlobal);
		if ( FAILED(CreateStreamOnHGlobal(hGlobal, TRUE, &m_SrcStream)) ){
			GlobalFree(hGlobal);
			return SUSIEERROR_EMPTYMEMORY;
		}
		if ( FAILED(m_image->CreateDecoderFromStream(m_SrcStream, 0,
				WICDecodeMetadataCacheOnDemand, &m_decoder)) ){
			return SUSIEERROR_NOTSUPPORT; // 不明…ファイル名があれば成功するかもしれないので、NOTSUPPORT で返す
		}
	}
	return SUSIEERROR_NOERROR;
}

SUSIE_ERROR InitSusieDecoder(LPCWSTR buf, LONG_PTR len, unsigned int flag)
{
	SUSIE_DECODE_PARAMETERS params;

	params.struct_size = sizeof(params);
	params.input_flags = default_flags | SUSIE_DECODE_SUSIE;
	params.page_number = 0;

	if ( (flag & SUSIE_SOURCE_MASK) == SUSIE_SOURCE_MEM ){ // メモリイメージ
		params.filename = NULL;
		params.fileimage = (const char *)buf;
		params.image_size = len;
	}else if ( (flag & SUSIE_SOURCE_MASK) == SUSIE_SOURCE_DISK ){
		params.filename = buf;
		params.fileimage = NULL;
	}else{
		return SUSIEERROR_NOTSUPPORT;
	}
	return InitDecoder(&params);
}

SUSIE_ERROR GetFrame(void)
{
	double dpiX, dpiY;

	if ( (X_TinyPage > 0) && (m_flags & SUSIE_DECODE_SUSIE) ){
		UINT frame_count;

		if ( m_frame_count ){ // 自前デコード
			frame_count = m_frame_count;
		}else{
			if ( FAILED(m_decoder->GetFrameCount(&frame_count)) ) frame_count = 1;
		}
		if ( (m_file_id != tiny_next_id) ||
			 (++tiny_next_page >= frame_count) ){
			tiny_next_page = 0;
		}
		tiny_next_id = m_file_id;
		if ( FAILED(m_decoder->GetFrame(tiny_next_page, &m_frame)) ){
			return SUSIEERROR_BROKENDATA;
		}
	}else{
		if ( FAILED(m_decoder->GetFrame(m_frame_count ? 0 : m_frame_no, &m_frame)) ){
			return SUSIEERROR_BROKENDATA;
		}
	}
	// ここでは、BITMAPINFOHEADER の分だけ初期化。
	// BITMAPV5HEADER の分は、SetBitmapinfoHeader で必要に応じて初期化。
	memcpy(&m_bmpinfo, &DefaultBmpInfo, sizeof(DefaultBmpInfo));
	m_bmp = m_frame;
	m_bmp->GetSize((UINT *)&m_bmpinfo.bV5Width, (UINT *)&m_bmpinfo.bV5Height);
	if ( (m_bmpinfo.bV5Height == 0) || (m_bmpinfo.bV5Width == 0) ){
		return SUSIEERROR_BROKENDATA;
	}

	if ( SUCCEEDED(m_frame->GetResolution(&dpiX, &dpiY)) ){
		m_bmpinfo.bV5XPelsPerMeter = static_cast<LONG>(dpiX * (1 / 0.0254f));
		m_bmpinfo.bV5YPelsPerMeter = static_cast<LONG>(dpiY * (1 / 0.0254f));
	}
	m_frame->GetMetadataQueryReader(&m_metadata);
	return SUSIEERROR_NOERROR;
}

SUSIE_ERROR GetThumbnail(void)
{
	if ( FAILED(m_decoder->GetThumbnail(&m_Thumbnailbmp)) ){
		// GetPreview は WDP のみサポートしているらしい
		if ( FAILED(m_decoder->GetPreview(&m_Thumbnailbmp)) ){
			int result = GetFrame();

			if ( result != SUSIEERROR_NOERROR ) return result;
			if ( FAILED(m_frame->GetThumbnail(&m_Thumbnailbmp)) ){
				return SUSIEERROR_NOERROR;
			}
			// JPEG, TIFF, WDP は IWICBitmapFrameDecode でサポートしているらしい
		}
	}
	memcpy(&m_bmpinfo, &DefaultBmpInfo, sizeof(DefaultBmpInfo));
	m_bmp = m_Thumbnailbmp;
	m_bmp->GetSize((UINT *)&m_bmpinfo.bV5Width, (UINT *)&m_bmpinfo.bV5Height);
	if ( (m_bmpinfo.bV5Height != 0) && (m_bmpinfo.bV5Width != 0) ){
		return SUSIEERROR_NOERROR;
	}
	return SUSIEERROR_BROKENDATA;
}

void InitColorContext(void)
{
	UINT ret, ccount;

	if ( !(m_flags & (SUSIE_DECODE_REQUEST_FIX_sRGB | SUSIE_DECODE_REQUEST_COLOR_PROFILE)) ){
		return; // 処理不要
	}

	if ( (m_frame == NULL) || FAILED(m_frame->GetColorContexts(0, NULL, &ccount)) ){
		return;
	}

	IWICColorContext *srcCc = NULL;

	if ( ccount > 0 ){
		if ( FAILED(m_image->CreateColorContext(&srcCc)) ) return;
		if ( FAILED(m_frame->GetColorContexts(1, &srcCc, &ret)) ){
			srcCc->Release();
			return;
		}
		m_result_flags |= SUSIE_DECODE_IN_COLOR_PROFILE;
	}else{
		// EXIFタグの場所が違う可能性がある？？
		PROPVARIANT v;
		PropVariantInit(&v);

		if ( m_metadata == NULL ) return;
		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/app1/ifd/exif/subifd:{uint=40961}", &v)) ||
			 SUCCEEDED(m_metadata->GetMetadataByName(L"/ifd/exif/subifd:{uint=40961}", &v))){
			UINT cs = (v.vt == VT_UI2) ? v.uiVal : 0;

			PropVariantClear(&v);
			if ( cs == 1 ) return; // sRGB になっているので変換不要
			if ( cs == 0 ) return;

			if ( FAILED(m_image->CreateColorContext(&srcCc)) ) return;
			if ( SUCCEEDED(srcCc->InitializeFromExifColorSpace(cs)) ){
				m_result_flags |= SUSIE_DECODE_IN_COLOR_PROFILE;
			}
		}
	}

	if ( srcCc == NULL ) return;

	if ( m_flags & SUSIE_DECODE_REQUEST_FIX_sRGB ){
		IWICColorContext *dstCc;

		if ( SUCCEEDED(m_image->CreateColorContext(&dstCc)) ){
			if ( SUCCEEDED(dstCc->InitializeFromExifColorSpace(1)) ){ // sRGBのカラープロファイルを使用
			// 変換
				if ( SUCCEEDED(m_image->CreateColorTransformer(&m_colortransform)) ){
					if ( SUCCEEDED(m_colortransform->Initialize(m_frame, srcCc, dstCc, GUID_WICPixelFormat24bppBGR)) ){
						m_bmp = m_colortransform;
						m_result_flags |= SUSIE_DECODE_FIX_sRGB;
					}
				}
			}
			dstCc->Release();
		}
		srcCc->Release();
	}else{ // m_flags & SUSIE_DECODE_REQUEST_COLOR_PROFILE
		UINT ProfileSize;

		if ( SUCCEEDED(srcCc->GetProfileBytes(0, NULL, &ProfileSize)) ){
			m_ICCsize = ProfileSize;
			m_ICCcontext = srcCc;
		}else{
			srcCc->Release();
		}
	}
}

SUSIE_ERROR GetPixelInfo(void)
{
	// 一行の大きさを計算 & 不明のフォーマットは24bitにする
	m_bmp->GetPixelFormat(&m_pixelFormat);
	m_colors = 0;

	// 24bit BMP
	if ( m_pixelFormat == GUID_WICPixelFormat24bppBGR ){
		m_bmpinfo.bV5BitCount = 24;
		// 変換不要
		return SUSIEERROR_NOERROR;
	}

	// 24bit RGB
	if ( m_pixelFormat == GUID_WICPixelFormat24bppRGB ){
		m_bmpinfo.bV5BitCount = 24;
		return SetColorbitsTransform(GUID_WICPixelFormat24bppBGR, WICBitmapPaletteTypeCustom);
	}

	// 32bit
	if ( X_Conv32NoAlpha &&
		(m_pixelFormat == GUID_WICPixelFormat32bppBGR) ){
		m_bmpinfo.bV5BitCount = 24;
		return SetColorbitsTransform(GUID_WICPixelFormat24bppBGR, WICBitmapPaletteTypeCustom);
	}
	if ( ((m_flags & SUSIE_DECODE_REQUEST_MERGE_ALPHA) &&
			 (m_pixelFormat == xGUID_WICPixelFormat16bppBGRA5551)) ||
		 (m_pixelFormat == xGUID_WICPixelFormat8bppAlpha) ||
		 (m_pixelFormat == GUID_WICPixelFormat32bppBGRA) ||
		 (m_pixelFormat == GUID_WICPixelFormat32bppPBGRA) ||
		 (m_pixelFormat == xGUID_WICPixelFormat32bppPRGBA) ||
		 (m_pixelFormat == xGUID_WICPixelFormat32bppRGBA) ||
		 (m_pixelFormat == GUID_WICPixelFormat64bppRGBA) ||
		 (m_pixelFormat == xGUID_WICPixelFormat64bppBGRA) ||
		 (m_pixelFormat == GUID_WICPixelFormat64bppPRGBA) ||
		 (m_pixelFormat == xGUID_WICPixelFormat64bppPBGRA) ||
		 (m_pixelFormat == GUID_WICPixelFormat128bppRGBAFloat) ||
		 (m_pixelFormat == GUID_WICPixelFormat64bppRGBAFixedPoint) ||
		 (m_pixelFormat == xGUID_WICPixelFormat64bppBGRAFixedPoint) ||
		 (m_pixelFormat == GUID_WICPixelFormat128bppRGBAFixedPoint) ||
		 (m_pixelFormat == GUID_WICPixelFormat128bppPRGBAFloat) ||
		 (m_pixelFormat == GUID_WICPixelFormat64bppRGBAHalf) ||
		 (m_pixelFormat == xGUID_WICPixelFormat64bppPRGBAHalf) ||
		 (m_pixelFormat == xGUID_WICPixelFormat32bppRGBA1010102) ||
		 (m_pixelFormat == xGUID_WICPixelFormat32bppRGBA1010102XR) ||
		 (m_pixelFormat == xGUID_WICPixelFormat32bppR10G10B10A2) ||
		 (m_pixelFormat == xGUID_WICPixelFormat32bppR10G10B10A2HDR10) ||
		 (m_pixelFormat == GUID_WICPixelFormat40bppCMYKAlpha) ||
		 (m_pixelFormat == GUID_WICPixelFormat80bppCMYKAlpha) ){
		m_bmpinfo.bV5BitCount = 32;
		int result = SetColorbitsTransform(GUID_WICPixelFormat32bppBGRA, WICBitmapPaletteTypeCustom);
		if ( m_bmpinfo.bV5BitCount == 32 ){
			AlphaConvert = m_flags & SUSIE_DECODE_REQUEST_MERGE_ALPHA;
			m_result_flags |= SUSIE_DECODE_ALPHA;
		}
		return result;
	}

	if ( (m_pixelFormat == GUID_WICPixelFormat32bppBGR) ||
		(m_pixelFormat == GUID_WICPixelFormat32bppBGRA) ){
		m_result_flags |= SUSIE_DECODE_ALPHA;
		m_bmpinfo.bV5BitCount = 32;
		if ( PixelConvert || (X_Conv32NoAlpha && X_SusieFix) ){
			m_bmpinfo.bV5BitCount = 24;
			return SetColorbitsTransform(GUID_WICPixelFormat24bppBGR, WICBitmapPaletteTypeCustom);
		}
		return SUSIEERROR_NOERROR;
	}

	// アスペクト比調整のためにフォーマット変更が必要なフォーマット
	if ( (m_flags & SUSIE_DECODE_REQUEST_FIX_ASPECT) && (m_bmpinfo.bV5XPelsPerMeter != m_bmpinfo.bV5YPelsPerMeter) ){
		// 各種白黒・グレー画像
		if ( (m_pixelFormat == GUID_WICPixelFormatBlackWhite) ||
			 (m_pixelFormat == GUID_WICPixelFormat2bppGray) ||
			 (m_pixelFormat == GUID_WICPixelFormat4bppGray) ||
			 (m_pixelFormat == GUID_WICPixelFormat8bppGray) ){
			m_bmpinfo.bV5BitCount = 8;
			m_colors = 256;
			return SetColorbitsTransform(GUID_WICPixelFormat8bppGray, WICBitmapPaletteTypeFixedGray256);
		}
		// 不明形式
		m_bmpinfo.bV5BitCount = 24;
		return SetColorbitsTransform(GUID_WICPixelFormat24bppBGR, WICBitmapPaletteTypeCustom);
	}

	// Indexd 8 bit
	if ( m_pixelFormat == GUID_WICPixelFormat8bppIndexed ){
		m_bmpinfo.bV5BitCount = 8;
		m_colors = 256;
		return SUSIEERROR_NOERROR;
	}

	// Indexd 1 bit
	if ( m_pixelFormat == GUID_WICPixelFormat1bppIndexed ){
		m_bmpinfo.bV5BitCount = 1;
		m_colors = 2;
		return SUSIEERROR_NOERROR;
	}
	// Indexd 4 bit
	if ( m_pixelFormat == GUID_WICPixelFormat2bppIndexed ){
		m_bmpinfo.bV5BitCount = 4;
		m_colors = 16;
		return SetColorbitsTransform(GUID_WICPixelFormat4bppIndexed, WICBitmapPaletteTypeMedianCut);
	}
	if ( m_pixelFormat == GUID_WICPixelFormat4bppIndexed ){
		m_bmpinfo.bV5BitCount = 4;
		m_colors = 16;
		return SUSIEERROR_NOERROR;
	}

	// B/W, Gray
	if ( m_pixelFormat == GUID_WICPixelFormatBlackWhite ){
		m_bmpinfo.bV5BitCount = 1;
		m_colors = 2;
		return SetColorbitsTransform(GUID_WICPixelFormatBlackWhite, WICBitmapPaletteTypeFixedBW);
	}
	if ( m_pixelFormat == GUID_WICPixelFormat2bppGray ){
		m_bmpinfo.bV5BitCount = 4;
		m_colors = 16;
		return SetColorbitsTransform(GUID_WICPixelFormat4bppIndexed, WICBitmapPaletteTypeFixedGray4);
	}
	if ( m_pixelFormat == GUID_WICPixelFormat4bppGray ){
		m_bmpinfo.bV5BitCount = 4;
		m_colors = 16;
		return SetColorbitsTransform(GUID_WICPixelFormat4bppIndexed, WICBitmapPaletteTypeFixedGray16);
	}
	if ( m_pixelFormat == GUID_WICPixelFormat8bppGray ){
		m_bmpinfo.bV5BitCount = 8;
		m_colors = 256;
//		SetColorbitsTransform(GUID_WICPixelFormat8bppIndexed, WICBitmapPaletteTypeFixedGray256); // 不要
		return SUSIEERROR_NOERROR;
	}

	// 16bit
	// GUID_WICPixelFormat16bppGray は24bitで
	if ( (m_pixelFormat == GUID_WICPixelFormat16bppBGR555) ){
		m_bmpinfo.bV5BitCount = 16;
		return SUSIEERROR_NOERROR;
	}
	if ( (m_pixelFormat == GUID_WICPixelFormat16bppBGR565) ){
		m_bits_size = sizeof(RGBQUAD) * 3;
		m_bmpinfo.bV5BitCount = 16;
		m_bmpinfo.bV5Compression = BI_BITFIELDS;
		m_bmpinfo.bV5RedMask = 0x00f800;
		m_bmpinfo.bV5GreenMask = 0x0007e0;
		m_bmpinfo.bV5BlueMask = 0x1f;
		return SUSIEERROR_NOERROR;
	}

	// 不明
	m_bmpinfo.bV5BitCount = 24;
	return SetColorbitsTransform(GUID_WICPixelFormat24bppBGR, WICBitmapPaletteTypeCustom);
}

SUSIE_ERROR SetBitmapinfoHeader(HLOCAL *pHBInfo)
{
	if ( m_ICCsize == 0 ){ // BITMAPINFOHEADER + RGBA mask のみ
		*pHBInfo = LocalAlloc(LMEM_FIXED,
			sizeof(BITMAPINFOHEADER) + m_bits_size + (sizeof(RGBQUAD) * m_colors));
		if ( *pHBInfo == NULL ) return SUSIEERROR_EMPTYMEMORY;
	}else{ // ICC profile 埋め込み
		UINT ProfileSize;

		*pHBInfo = LocalAlloc(LMEM_FIXED,
			sizeof(BITMAPV5HEADER) + (sizeof(RGBQUAD) * m_colors) + m_ICCsize);
		if ( *pHBInfo == NULL ) return SUSIEERROR_EMPTYMEMORY;

		// BITMAPINFOHEADER を BITMAPV5HEADER に変換
		m_bmpinfo.bV5Size = sizeof(BITMAPV5HEADER);
		// BITMAPV5HEADER の BITMAPINFOHEADER(+ m_bits_size) 以降の未初期化分を初期化
		memset((char *)&m_bmpinfo + sizeof(BITMAPINFOHEADER) + m_bits_size, 0,
				sizeof(BITMAPV5HEADER) - sizeof(BITMAPINFOHEADER) - m_bits_size);
		// BITMAPV5HEADER の値を設定
		if ( m_bits_size == 0 ){
			m_bmpinfo.bV5RedMask = 0xff;
			m_bmpinfo.bV5GreenMask = 0xff00;
			m_bmpinfo.bV5BlueMask = 0xff0000;
		}
		if ( m_pixelFormat == GUID_WICPixelFormat32bppBGRA ){
			m_bmpinfo.bV5AlphaMask = 0xff000000;
		}
		m_bmpinfo.bV5CSType = PROFILE_EMBEDDED;
//		m_bmpinfo.bV5Endpoints = 0;
//		m_bmpinfo.bV5GammaRed = 0;
//		m_bmpinfo.bV5GammaGreen = 0;
//		m_bmpinfo.bV5GammaBlue = 0;
//		m_bmpinfo.bV5Intent = 0;
		m_bmpinfo.bV5ProfileData = sizeof(BITMAPV5HEADER) + (sizeof(RGBQUAD) * m_colors);
		m_bmpinfo.bV5ProfileSize = m_ICCsize;
//		m_bmpinfo.bV5Reserved = 0;

		m_bits_size = 0;
		m_ICCcontext->GetProfileBytes(m_bmpinfo.bV5ProfileSize, static_cast<BYTE *>(*pHBInfo) + m_bmpinfo.bV5ProfileData, &ProfileSize);
		m_ICCcontext->Release();
		m_ICCcontext = NULL;
	}
	m_HeaderPtr = static_cast<BITMAPINFOHEADER *>(*pHBInfo);
	memcpy(m_HeaderPtr, &m_bmpinfo, m_bmpinfo.bV5Size + m_bits_size);
	return SUSIEERROR_NOERROR;
}

void SetRotate(void)
{
	if ( FAILED(m_image->CreateBitmapFlipRotator(&m_flip)) ) return;

	int ropt = WICBitmapTransformFlipVertical;

	if ( m_metadata != NULL ){
		PROPVARIANT v;
		PropVariantInit(&v);

		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/app1/ifd/{ushort=274}", &v)) ){
			m_rotate = (v.vt == VT_UI2) ? v.uiVal : 0;
			m_result_flags |= SUSIE_DECODE_ROTATE;
			PropVariantClear(&v);
		}
		if ( (m_rotate > 1) && (m_flags & SUSIE_DECODE_REQUEST_ROTATE) ){
			m_result_flags |= SUSIE_DECODE_FIX_ROTATE;
			switch(m_rotate){
				case 2: // 水平反転
					ropt = WICBitmapTransformFlipHorizontal | WICBitmapTransformFlipVertical;
					break;

				case 3: // 180度回転
					ropt = WICBitmapTransformRotate180 | WICBitmapTransformFlipVertical;
					break;

				case 4: // 180度回転+水平反転
					ropt = WICBitmapTransformRotate180 | WICBitmapTransformFlipHorizontal | WICBitmapTransformFlipVertical;
					break;

				case 5: // 270度回転+水平反転
					ropt = WICBitmapTransformRotate270;
					SwapXY();
					break;

				case 6: // 90度回転
					ropt = WICBitmapTransformRotate90 | WICBitmapTransformFlipHorizontal;
					SwapXY();
					break;

				case 7: // 90度回転+水平反転
					ropt = WICBitmapTransformRotate90;
					SwapXY();
					break;

				case 8: // 270度回転
					ropt = WICBitmapTransformRotate270 | WICBitmapTransformFlipHorizontal;
					SwapXY();
					break;

				default:
					m_result_flags &= ~SUSIE_DECODE_FIX_ROTATE;
			}
		}
	}
	if ( SUCCEEDED(m_flip->Initialize(m_bmp, static_cast<WICBitmapTransformOptions>(ropt))) ){
		m_bmp = m_flip;
	}else{ // 失敗したときは、エラーよりも向き補正していない画像出力を優先する
		// m_flip は、Release() で解放
		m_result_flags &= ~SUSIE_DECODE_FIX_ROTATE;
		if ( (m_flags & SUSIE_DECODE_REQUEST_ROTATE) &&
			 (m_rotate >= 5) && (m_rotate <= 8) ){
			SwapXY(); // 元に戻す
		}
	}
}

void FixAspectRate()
{
	int newWidth, newHeight;
	double dpiX, dpiY, rate;

	if ( !(m_flags & SUSIE_DECODE_REQUEST_FIX_ASPECT) ||
		 (m_bmpinfo.bV5XPelsPerMeter == m_bmpinfo.bV5YPelsPerMeter) ){
		return;
	}

	newWidth = m_HeaderPtr->biWidth;
	newHeight = m_HeaderPtr->biHeight;
	if ( FAILED(m_frame->GetResolution(&dpiX, &dpiY)) ) return;

	if ( (dpiX < 1.0) || (dpiX > 9999.0) || (dpiY < 1.0) || (dpiY > 9999.0) ){
		return;
	}

	rate = dpiX / dpiY;
	if ( (rate < 0.2) || (rate > 5.0) ) return; // 比率が5越えの時は異常

	if ( rate > 1.0 ){ // widthが高解像度
		newHeight =
			static_cast<DWORD>(static_cast<double>(newHeight) * dpiX / dpiY);
	}else{ // heightが高解像度
		newWidth =
			static_cast<DWORD>(static_cast<double>(newWidth) * dpiY / dpiX);
	}
	if ( FAILED(m_image->CreateBitmapScaler(&m_scale)) ) return;

	if ( FAILED(m_scale->Initialize(m_bmp, newWidth, newHeight, WICBitmapInterpolationModeFant)) ){
		return; // 失敗した m_scale は、Release() で解放
	}

	if ( rate > 1.0 ){ // widthが高解像度
		m_HeaderPtr->biHeight = newHeight;
		m_HeaderPtr->biYPelsPerMeter = m_bmpinfo.bV5XPelsPerMeter;
	}else{ // heightが高解像度
		m_HeaderPtr->biWidth = newWidth;
		m_HeaderPtr->biXPelsPerMeter = m_bmpinfo.bV5YPelsPerMeter;
	}

	m_result_flags |= SUSIE_DECODE_FIX_ASPECT;
	m_bmp = m_scale;
}

#pragma pack(push, 1)
typedef struct {
	BYTE B, G, R, A;
} MaskBits;
#pragma pack(pop)
#define backsize 6

void ConvertAlpha(MaskBits *bmpptr, MaskBits *maxbit)
{
	MaskBits *bit;
	int backleft, backcountX, backcountY;
	WORD backLcolor, backcolor, backXcolor;

	bit = bmpptr;
	backleft = backsize;
	backcountX = 0;
	backcountY = 0;
	backLcolor = backcolor = static_cast<WORD>(X_GridColor1 & 0xff);
	backXcolor = static_cast<WORD>((X_GridColor1 ^ X_GridColor2) & 0xff);

	for ( ; bit < maxbit ; bit++ ){
		WORD A, backAcolor;

		A = (WORD)bit->A;
		if ( A != 0xff ){
			backAcolor = (WORD)(backcolor * (0x100 - A));
			bit->B = (BYTE)(WORD)( (((WORD)bit->B * A) + backAcolor) >> 8);
			bit->G = (BYTE)(WORD)( (((WORD)bit->G * A) + backAcolor) >> 8);
			bit->R = (BYTE)(WORD)( (((WORD)bit->R * A) + backAcolor) >> 8);
			bit->A = 0xff;
		}
		if ( --backleft == 0 ){
			backcolor ^= backXcolor;
			backleft = backsize;
			backcountX += backsize;
			if ( (backcountX + backsize) >= m_HeaderPtr->biWidth ){
				if ( backcountX < m_HeaderPtr->biWidth ){
					backleft = m_HeaderPtr->biWidth - backcountX;
				}else{
					backcountX = 0;
					backcountY++;
					if ( backcountY >= backsize ){
						backLcolor ^= backXcolor;
						backcountY = 0;
					}
					backcolor = backLcolor;
				}
			}
		}
	}
}

void ScaleTo24(HLOCAL *pHBm, DWORD Wsize)
{
	DWORD *src, w, h = m_HeaderPtr->biHeight, newsize;
	BYTE *dest;
	HLOCAL hNewBm;

	src = (DWORD *)*pHBm;
	dest = (BYTE *)*pHBm;
	for (;;) {
		w = m_HeaderPtr->biWidth;
		for (;;) {
			DWORD pixel;

			pixel = *src++;
			dest[0] = (BYTE)(DWORD)(pixel);
			dest[1] = (BYTE)(DWORD)(pixel >> 8);
			dest[2] = (BYTE)(DWORD)(pixel >> 16);
			dest += 3;
			if ( --w > 0 ) continue;
			break;
		}
		w = m_HeaderPtr->biWidth & 3; // DWORD境界に揃える
		while ( w ){
			*dest++ = 0;
			w--;
		}
		if ( --h > 0 ) continue;
		break;
	}
	newsize = dest - (BYTE *)*pHBm;
	hNewBm = LocalReAlloc(*pHBm, newsize, 0);
	if ( hNewBm != NULL ){
		*pHBm = hNewBm;
		m_HeaderPtr->biSizeImage = newsize;
		m_HeaderPtr->biBitCount = 24;
	}
}

SUSIE_ERROR CopyPixels(HLOCAL *pHBm)
{
	DWORD Wsize;
	__int64 bmpsize;

	// 計算中にオーバーフローしないかチェック
	// 1 <= m_HeaderPtr->biBitCount <= 32 は保証済み(GetPixelInfo で bV5BitCount に設定)
	if ( (m_HeaderPtr->biWidth < 0) || (m_HeaderPtr->biHeight < 0) || (m_HeaderPtr->biWidth > 0x3ffffff) ){
		return SUSIEERROR_BROKENDATA;
	}
	// １行のバイト数を算出
	Wsize = ((m_HeaderPtr->biWidth * m_HeaderPtr->biBitCount + 7) / 8 + 3) & ~3;

	bmpsize = static_cast<__int64>(Wsize) * static_cast<__int64>(m_HeaderPtr->biHeight);
	if ( bmpsize >= 0x7fffffff ) return SUSIEERROR_EMPTYMEMORY;

	m_HeaderPtr->biSizeImage = static_cast<DWORD>(bmpsize);
	*pHBm = LocalAlloc(LMEM_FIXED, m_HeaderPtr->biSizeImage);
	if ( *pHBm == NULL ) return SUSIEERROR_EMPTYMEMORY;

	if ( FAILED(m_bmp->CopyPixels(NULL, Wsize, m_HeaderPtr->biSizeImage, (BYTE *)*pHBm)) ){
		// 失敗。1行を DWORD アライメントから BYTE アライメントに変更して再実行
		// ※ 該当エラーコードは WINCODEC_ERR_INSUFFICIENTBUFFER
		//    (Webp Decoder 1.2.47.0) であるが、念のために全ての
		//    エラーコードに対して再実行する
		BYTE *dest = (BYTE *)*pHBm;
		DWORD MWsize = (m_HeaderPtr->biWidth * m_HeaderPtr->biBitCount + 7) / 8;
		WICRect box = {0, 0, m_HeaderPtr->biWidth, 1};

		for(;;){
			if ( FAILED(m_bmp->CopyPixels(&box, MWsize, Wsize, dest)) ){
				LocalFree(*pHBm);
				*pHBm = NULL;
				return SUSIEERROR_BROKENDATA;
			}
			box.Y++;
			dest += Wsize;
			if ( box.Y >= m_HeaderPtr->biHeight ) break;
		}
	}

	// 成功
	if ( AlphaConvert ){
		m_result_flags |= SUSIE_DECODE_MERGE_ALPHA;
		ConvertAlpha(reinterpret_cast<MaskBits *>(*pHBm),
				reinterpret_cast<MaskBits *>(reinterpret_cast<BYTE *>(*pHBm) + m_HeaderPtr->biSizeImage) );
		if ( X_SusieFix || X_Conv32NoAlpha ) ScaleTo24(pHBm, Wsize);
	}
	return SUSIEERROR_NOERROR;
}

void CopyPalette(void)
{
	IWICPalette *wicpal;
	WICColor *palette;

	if ( m_colors == 0 ) return;

	// ダミーパレットを生成
	palette = reinterpret_cast<WICColor *>((BYTE *)m_HeaderPtr + m_HeaderPtr->biSize);
	if ( m_colors == 2 ){
		palette[0] = 0;
		palette[1] = 0xffffff;
	}else{
		for ( UINT pn = 0; pn < m_colors ; pn++ ){
			palette[pn] = pn + pn * 0x100 + pn * 0x10000;
		}
	}

	// 画像のパレットを上書き
	if ( FAILED(m_image->CreatePalette(&wicpal)) ) return;
	m_bmp->CopyPalette(wicpal);
	wicpal->GetColors(m_colors, palette, (UINT *)&m_HeaderPtr->biClrUsed);
	wicpal->Release();

	if ( X_WipeAlphaOnPalette ){
		for ( UINT pn = 0; pn < m_colors ; pn++ ){ // alpha を削除
			palette[pn] &= 0xffffff;
		}
	}
}

#define NullRelease(c) { if ( c != NULL ) c->Release(); }
void Release(void)
{
	NullRelease(m_scale);
	NullRelease(m_flip);
	NullRelease(m_colortransform);
	NullRelease(m_ICCcontext);
	NullRelease(m_metadata);
//	NullRelease(m_PixelInfo);
	NullRelease(m_conv);
	NullRelease(m_frame);
	NullRelease(m_Thumbnailbmp);
	NullRelease(m_decoder);
	NullRelease(m_SrcStream);
	// m_bmp は、m_flip 等のコピー値なので Release 不要
	m_image->Release();
	if ( m_InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();
}

char *PropvToStr(PROPVARIANT &v, char *dest, char *destmax)
{
	size_t len; // 保存可能な文字数(\0 は別途確保済み)

	len = destmax - dest;
	if ( len <= 16 ) return dest; // 十分な余裕がない
	switch (v.vt){
		case VT_EMPTY:
			break;

		case VT_NULL:
			dest += wsprintfA(dest, "(null)"); // min 8 chars
			break;

		case VT_UI2 | VT_BYREF:
			dest += wsprintfA(dest, "%u", *v.puiVal); // min 16 chars
			break;

		case VT_UI2 | VT_VECTOR:
			if ( v.caui.cElems == 0 ) break;
			dest += wsprintfA(dest, "%u", v.caui.pElems[0]); // min 16 chars
			if ( v.caui.cElems > 1 ){
				ULONG left = 1;
				do {
					if ( (destmax - dest) < 16 ) break;
					dest += wsprintfA(dest, ",%u", v.caui.pElems[left]);
				}while( ++left < v.caui.cElems);
			}
			break;

		case VT_UI4 | VT_VECTOR:
			if ( v.caul.cElems == 0 ) break;
			dest += wsprintfA(dest, "%u", v.caul.pElems[0]); // min 16 chars
			if ( v.caul.cElems > 1 ){
				ULONG left = 1;
				do {
					if ( (destmax - dest) < 16 ) break;
					dest += wsprintfA(dest, ",%u", v.caul.pElems[left]);
				}while( ++left < v.caul.cElems);
			}
			break;

		case VT_UI1 | VT_VECTOR:
			if ( v.caub.cElems == 0 ) break;
			dest += wsprintfA(dest, "%u", v.caub.pElems[0]); // min 16 chars
			if ( v.caub.cElems > 1 ){
				ULONG left = 1;
				do {
					if ( (destmax - dest) < 16 ) break;
					dest += wsprintfA(dest, ",%u", v.caub.pElems[left]);
				}while( ++left < v.caub.cElems);
			}
			break;

		case VT_UI1:
			dest += wsprintfA(dest, "%u", v.bVal); // min 8 chars
			break;

		case VT_UI2:
			dest += wsprintfA(dest, "%u", v.uiVal); // min 8 chars
			break;

		case VT_UI4:
			dest += wsprintfA(dest, "%u", v.lVal); // min 16 chars
			break;

		case VT_UI8:
			if ( len > 32 ) dest += wsprintfA(dest, "0x%Ix", v.uhVal);
			break;

		case VT_UI4 | VT_BYREF:
			dest += wsprintfA(dest, "%u", *v.puintVal); // min 16 chars
			break;

		case VT_I8:
			if ( len > 32 ) dest += wsprintfA(dest, "0x%Ix", v.hVal);
			break;

		case VT_LPSTR:
			if ( v.pszVal != NULL ){
				size_t copylen;

				copylen = strlen(v.pszVal);
				if ( len > copylen ){
					memcpy(dest, v.pszVal, copylen + 1);
					dest += copylen;
				}
			}
			break;

		case VT_LPWSTR:
			if ( v.pwszVal != NULL ){
				DWORD copylen;

				copylen = UnicodeToAnsi(v.pwszVal, dest, len + 1);
				if ( copylen >= 1 ) copylen--; // '\0' を除去
				dest += copylen;
			}
			break;

		case VT_BOOL:
			dest += wsprintfA(dest, v.boolVal ? "true" : "false"); // min 8 chars
			break;

		case VT_BLOB:
			if ( len > 32 ){
				dest += wsprintfA(dest, "bin data:%d", v.blob.cbSize);
			}
			break;

		default:
			if ( len > 24 ){
				dest += wsprintfA(dest, "unknown type %x", v.vt);
			}
			break;
	}
	return dest;
}

/*
struct PropIDnames {
	WCHAR *id;
	char *name;
} ;


PropIDnames App1[] = {
	{ L"{ushort=0}", "app1/NULL"},
	{ L"{ushort=271}", "maker"},
	{ L"{ushort=272}", "model"},
	{ L"{ushort=274}", "rotate"},
	{ L"{ushort=282}", "width density"},
	{ L"{ushort=283}", "heiht density"},
	{ L"{ushort=296}", "density unit"},
	{ L"{ushort=306}", "write date time"},

	{ L"{ushort=306}", "write date time"},
	{ L"{ushort=33434}", "露出"},
	{ NULL, NULL }
};
*/

char *EnumProps(IWICMetadataQueryReader *metadata, char *dest, char *destmax)
{
	IEnumString *enummeta;

	if ( SUCCEEDED(metadata->GetEnumerator(&enummeta)) ){
		LPOLESTR str;
		ULONG fetched;

		while ( ((destmax - dest) > 8) &&
				SUCCEEDED(enummeta->Next(1, &str, &fetched)) &&
				(fetched == 1) ){
			PROPVARIANT propvar;
			size_t copylen;

			copylen = UnicodeToAnsi(str, dest, (destmax - dest));
			if ( copylen <= 1 ){ // 不足しているので列挙終了
				CoTaskMemFree(str);
				break;
			}
			copylen--;
			dest += copylen;

			PropVariantInit(&propvar);
			if ( SUCCEEDED(metadata->GetMetadataByName(str, &propvar)) ){
				if ( propvar.vt == VT_UNKNOWN ){
					IWICMetadataQueryReader *child;

					if ( SUCCEEDED(propvar.punkVal->QueryInterface(IID_IWICMetadataQueryReader, (void **)&child)) ){
						dest = EnumProps(child, dest, destmax);
						child->Release();
					}
				}else{
					dest = PropvToStr(propvar, dest, destmax);
				}
				PropVariantClear(&propvar);
			}
			if ( (destmax - dest) > 1 ) *dest++ = '\n';
			CoTaskMemFree(str);
		}
		enummeta->Release();
	}
	return dest;
}

char *EnumPropsName(IWICMetadataQueryReader *metadata, WCHAR *name, char *dest, char *destmax)
{
	PROPVARIANT propvar;

	PropVariantInit(&propvar);
	if ( SUCCEEDED(metadata->GetMetadataByName(name, &propvar)) ){
		if ( propvar.vt == VT_UNKNOWN ){
			IWICMetadataQueryReader *child;

			if ( SUCCEEDED(propvar.punkVal->QueryInterface(IID_IWICMetadataQueryReader, (void **)&child)) ){
				dest = EnumProps(child, dest, destmax);
				child->Release();
			}
		}else{
			dest = PropvToStr(propvar, dest, destmax);
		}
		PropVariantClear(&propvar);
		if ( (destmax - dest) > 1 ) *dest++ = '\n';
	}
	return dest;
}

#define DEFINFOSIZE 0x4000
HLOCAL GetInfoText(void)
{
	HLOCAL hInfo;
	UINT frames;
	char *dest, *destmax;

	hInfo = LocalAlloc(LMEM_FIXED, DEFINFOSIZE);
	if ( hInfo == NULL ) return NULL;
	dest = reinterpret_cast<char *>(hInfo);
	destmax = dest + DEFINFOSIZE - 1;

	if ( FAILED(m_decoder->GetFrameCount(&frames)) ) frames = 1;
	dest += wsprintfA(dest, "Frames : %d\n", frames);

	{
		IWICMetadataQueryReader *metadata;

		if ( SUCCEEDED(m_decoder->GetMetadataQueryReader(&metadata)) ){
			dest = EnumProps(metadata, dest, destmax);
			metadata->Release();
		}
	}
/*
	GIF
		Global
		/commentext/TextEntry		VT_LPSTR
		/logscrdesc/PixelAspectRatio
		Frame
		/grctlext/TransparentColorIndex

	PNG
		/iTXt/TextEntry		LPWSTR

	TIFF
		/ifd/exif

	Jpeg
		/app1/ifd/exif
*/
	if ( m_metadata != NULL ){
		PROPVARIANT v;

		dest = EnumPropsName(m_metadata, L"/xmp", dest, destmax);
		dest = EnumPropsName(m_metadata, L"/app1/ifd/exif", dest, destmax);
		dest = EnumPropsName(m_metadata, L"/ifd/exif", dest, destmax);
		dest = EnumProps(m_metadata, dest, destmax);

		PropVariantInit(&v);
		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/app1/ifd/exif/subifd:{uint=40961}", &v)) ||
			 SUCCEEDED(m_metadata->GetMetadataByName(L"/ifd/exif/subifd:{uint=40961}", &v))){
			if ( (destmax - dest) > 20 ){
				dest += wsprintfA(dest, "ColorContext; ");
				dest = PropvToStr(v, dest, destmax);
			}
			PropVariantClear(&v);
		}

		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/xmp/xmp:カメラの製造元", &v)) ){
			if ( (destmax - dest) > 8 ){
				dest += wsprintfA(dest, "?; ");
				dest = PropvToStr(v, dest, destmax);
			}
			PropVariantClear(&v);
		}
		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/xmp/xmp:CreatorTool", &v)) ){
			if ( (destmax - dest) > 8 ){
				dest += wsprintfA(dest, "?; ");
				dest = PropvToStr(v, dest, destmax);
			}
			PropVariantClear(&v);
		}
		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/xmp/xmp:creatortool", &v)) ){
			if ( (destmax - dest) > 8 ){
				dest += wsprintfA(dest, "?; ");
				dest = PropvToStr(v, dest, destmax);
			}
			PropVariantClear(&v);
		}
	}
	*dest = '\0';
	return hInfo;
}

void SetResult(SUSIE_DECODE_PARAMETERS *params)
{
	PROPVARIANT v;

	params->infotext = NULL;
	params->animate_time = 0;
	params->left_top.x = params->left_top.y = 0;

	PropVariantInit(&v);
	if ( m_metadata != NULL ){
		// GIF
		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/grctlext/Delay", &v))){
			m_result_flags |= SUSIE_DECODE_ANIMATION;
			params->animate_time = v.uiVal; // VT_UI2
		}
		// Webp
		if ( SUCCEEDED(m_metadata->GetMetadataByName(L"/ANMF/FrameDuration", &v))){
			m_result_flags |= SUSIE_DECODE_ANIMATION;
			params->animate_time = v.lVal; // VT_UI4
		}
	}
	if ( m_frame_count > 0 ){
		m_result_flags |= SUSIE_DECODE_ANIMATION;
		params->all_pages = m_frame_count;
		params->animate_time = m_frame_rate;
	}else{
		if ( FAILED(m_decoder->GetFrameCount((UINT *)&params->all_pages)) ){
			params->all_pages = 1;
		}
	}
	params->rotate = m_rotate;

	if ( params->input_flags & SUSIE_DECODE_REQUEST_INFOTEXT ){
		params->infotext = reinterpret_cast<char *>(GetInfoText());
		if ( params->infotext != NULL ){
			m_result_flags |= SUSIE_DECODE_REQUEST_INFOTEXT;
		}
	}

	params->output_flags = m_result_flags;
}

	private:
		IWICBitmapSource *m_bmp; // m_frame とか m_conv とか m_colortransform とかを示す、加工済み BitmapSource

		IWICImagingFactory *m_image;
		IWICBitmapDecoder *m_decoder;
		IWICBitmapFrameDecode *m_frame;
		IWICFormatConverter *m_conv;
//		IWICPixelFormatInfo *m_PixelInfo;
		IWICMetadataQueryReader *m_metadata;
		IWICColorTransform *m_colortransform;
		IWICBitmapFlipRotator *m_flip;
		IWICBitmapScaler *m_scale;
		IWICBitmapSource *m_Thumbnailbmp;
		LPSTREAM m_SrcStream;

		BITMAPV5HEADER m_bmpinfo;
		WICPixelFormatGUID m_pixelFormat;

		DWORD m_colors, m_bits_size;
		BITMAPINFOHEADER *m_HeaderPtr;

		DWORD m_ICCsize;
		IWICColorContext *m_ICCcontext;
		HRESULT m_InitResult;

		DWORD m_frame_count, m_frame_rate;
		DWORD m_frame_no; // 読み込みを行う frame 番号
//		DWORD m_frame_delay; // 1以上…アニメーションあり、待機時間(ms)
		DWORD m_flags; // SUSIE_DECODE_REQUEST_xxx
		DWORD m_result_flags; // SUSIE_DECODE_xxx
		DWORD m_rotate;
		DWORD m_file_id;

SUSIE_ERROR SetColorbitsTransform(const WICPixelFormatGUID &pixformat, const WICBitmapPaletteType &palettetype)
{
	if ( FAILED(m_image->CreateFormatConverter(&m_conv)) ){
		return SUSIEERROR_INTERNAL;
	}

	if ( SUCCEEDED(m_conv->Initialize(m_bmp, pixformat,
				WICBitmapDitherTypeNone, NULL, 0.0f, palettetype)) ){
		m_bmp = m_conv;
		return SUSIEERROR_NOERROR;
	}

	// 24bpp BGR で再度試す
	if ( SUCCEEDED(m_conv->Initialize(m_bmp, GUID_WICPixelFormat24bppBGR,
				WICBitmapDitherTypeNone, NULL, 0.0f,
				WICBitmapPaletteTypeCustom)) ){
		m_bmpinfo.bV5BitCount = 24;
		m_bmp = m_conv;
		return SUSIEERROR_NOERROR;
	}

	m_conv->Release();
	m_conv = NULL;
	return SUSIEERROR_INTERNAL;
}

void SwapXY(void)
{
	int temp = m_HeaderPtr->biHeight;
	m_HeaderPtr->biHeight = m_HeaderPtr->biWidth;
	m_HeaderPtr->biWidth = temp;
	temp = m_HeaderPtr->biYPelsPerMeter;
	m_HeaderPtr->biYPelsPerMeter = m_HeaderPtr->biXPelsPerMeter;
	m_HeaderPtr->biXPelsPerMeter = temp;
}

};

/*-----------------------------------------------------------------------------
	dll の初期化／終了処理
-----------------------------------------------------------------------------*/
BOOL WINAPI DllMain(HINSTANCE hInst, unsigned long reason, void *)
{
	// DLL の終了処理 -----------------------------------------------------
	if ( reason == DLL_PROCESS_ATTACH ){
		DLLhInst = hInst;
	}else if ( (reason == DLL_PROCESS_DETACH) && (codecs.list != NULL) ){
		// CODECSSTRUCT で使ったメモリを解放する
		CODECLISTITEMSTRUCT *codeclistp = codecs.list;
		int leftcodecs = codecs.count - EXTCODECS;

		while ( leftcodecs-- > 0 ){
			HeapFree(GetProcessHeap(), 0, codeclistp->ext);
			codeclistp++;
		}
		HeapFree(GetProcessHeap(), 0, codecs.list);
		codecs.count = 0;
	}
	return TRUE;
}

/*-----------------------------------------------------------------------------
	Plug-in interface
-----------------------------------------------------------------------------*/
const char InfoAPIType[] = "00IN";
const char InfoCopyright[] = "WIC Plug-in Version " VERSIONS " " Copyright;
const char InfoTypeNOWIC[] = "WIC Load Error";
const char InfoExtNOWIC[] = "*.NO-WIC";
const char InfoTypeHeader[] = "WIC-";

int PluginInfoStr(LPSTR buf, int buflen, const char *str, int strlength)
{
	int copylen;

	copylen = (buflen > strlength) ? strlength - 1 : buflen - 1;
	memcpy(buf, str, copylen);
	buf[copylen] = '\0';
	return copylen;
}

int ExtEnum(int index, LPSTR buf, int buflen)
{
	char *dest = buf;

	*dest = '\0';
	if ( LoadCodecList() <= 0 ){
		if ( index == 2 ){
			return PluginInfoStr(buf, buflen, InfoTypeNOWIC, sizeof(InfoTypeNOWIC));
		}
		if ( index == 3 ){
			return PluginInfoStr(buf, buflen, InfoExtNOWIC, sizeof(InfoExtNOWIC));
		}
		return 0; // 戻り値は codecs.count 相当
	}
	int leftcodecs = codecs.count;
	CODECLISTITEMSTRUCT *codeclistp = codecs.list;

	while ( leftcodecs-- ){
		if ( codeclistp->enable != FALSE ){
			if ( index < 2 ){
				int leftlen = buflen, lens, len;
				WCHAR *extlist = codeclistp->ext;

				if ( index == 1 ){ // +1 種類表示
					lens = PluginInfoStr(buf, leftlen, InfoTypeHeader, sizeof(InfoTypeHeader));
					len = UnicodeToAnsi(extlist + 1, buf + lens, leftlen - lens);
					if ( len <= 0 ) len = 1; // 下の式との整合性を取る
					dest += lens + len - 1;
				}else{ // +0 拡張子列挙
					for (;;){
						if ( (leftlen < 5) || (*extlist == '\0') ) break;
						dest[0] = '*';
						dest[1] = '.';
						len = UnicodeToAnsi(extlist + 1, dest + 2, leftlen - 3);
						if ( len == 0 ) break;
						dest += len - 1 + 2;
						*dest++ = ';';
						leftlen -= len + 3;
						extlist += *extlist;
					}
				}
				*dest = '\0';
				return dest - buf;
			}
			index -= 2;
		}
		codeclistp++;
	}
	return 0;
}

#pragma argsused
extern "C" int __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	if ( (buf == NULL) || (buflen <= 0) ) return 0;
	if ( (infono >= 0) && (infono <= 3) ){
		if ( (infono == 3) && (codecs.count > 0) ){
			if ( (previous_decoder[0] != '\0') && (previous_decoder_tid == GetCurrentThreadId()) ){
				int copylen, copylen2;

				copylen = PluginInfoStr(buf, buflen, InfoTypeHeader, sizeof(InfoTypeHeader));
				copylen2 = UnicodeToAnsi(previous_decoder, buf + copylen, buflen - copylen);
				return (copylen2 <= 0) ? copylen : (copylen + copylen2 - 1);
			}
			// ExtEnum で処理
		}else if ( infono == 0 ){
			return PluginInfoStr(buf, buflen, InfoAPIType, sizeof(InfoAPIType));
		}else if ( infono == 1 ){
			return PluginInfoStr(buf, buflen, InfoCopyright, sizeof(InfoCopyright));
		}
		// infono == 2 はこのブロックで何もしない
	}
	if ( infono >= 2 ) return ExtEnum(infono - 2, buf, buflen);
	*buf = '\0';
	return 0;
}

extern "C" int __stdcall GetPluginInfoW(int infono, LPWSTR buf, int buflen)
{
	char bufA[0x400];
	int len;

	if ( (buf == NULL) || (buflen <= 0) ) return 0;
	len = GetPluginInfo(infono, bufA, sizeof(bufA));
	if ( len <= 0 ){
		buf[0] = '\0';
		return 0;
	}
	len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, bufA, -1, buf, buflen);
	if ( len <= 1 ){
		len = 0;
	}else{
		len = (len <= buflen) ? (len - 1) : (buflen - 1);
	}
	buf[len] = '\0';
	return len;
}
//-----------------------------------------------------------------------------
extern "C" int __stdcall IsSupportedW(LPCWSTR filename, void *dw)
{
	BYTE buf[SUSIE_CHECK_SIZE], *header;

	if ( LoadCodecList() <= 0 ) return 0;

	previous_decoder = decoder_none;
	if ( (DWORD_PTR)dw & ~(DWORD_PTR)0xffff ){ // 2K メモリイメージ
		header = (BYTE *)dw;
	}else{ // ファイルハンドル
		DWORD size = 0, readsize;

		header = (BYTE *)buf;
		for (;;){
			if ( ReadFile(reinterpret_cast<HANDLE>(dw), header, sizeof(buf) - size, &readsize, NULL) == FALSE ){
				return 0; // 読み込み失敗
			}
			if ( readsize != 0 ){
				size += readsize;
				if ( size >= sizeof(buf) ) break;
				header += readsize;
			}else{
				memset(buf + size, 0, sizeof(buf) - size);
				break;
			}
		}
		header = (BYTE *)buf;
	}

	// ファイルフォーマットによる判別
	if ( /* BMP */	!memcmp(header, "BM", 2) ){
		filename = L".bmp";
	}else

	if ( /* GIF */	!memcmp(header, "GIF8", 4) ){
		filename = L".gif";
	}else

	if ( !memcmp(header, RiffHeader, sizeof(RiffHeader)) ){ // RIFF
		if ( !memcmp(header + 8, ACONHeader, sizeof(ACONHeader)) ){ // ani
			filename = L".ico";
		}else if ( !memcmp(header + 8, WebpHeader, sizeof(WebpHeader)) ){ // webp
			filename = L".webp";
		}
	}else

	if ( /* icon */	(!memcmp(header, "\0\0\1", 4) ||
		 /* cur */	 (((DWORD_PTR)dw & ~(DWORD_PTR)0xffff) &&
					  !memcmp(header, CurHeader, sizeof(CurHeader))) ) &&
		(*(DWORD *)(header + 0x12) == ((DWORD)*(WORD *)(header + 4) * 0x10 + 6)) ){
		filename = L".ico";
	}else

	if ( /* PNG */	!memcmp(header, PngHeader , sizeof(PngHeader)) ){
		filename = L".png";
	}else

	if ( /* TIFF */	!memcmp(header, "II*", 4) || !memcmp(header, "MM\0*", 4) ){
		filename = L".tif";
	}else

	if ( /* WDP */	!memcmp(header, "II\xbc\1", 4) ){
		filename = L".wdp";
	}else

	if ( /* JPEG/EXIF */	((*header == 0xff) && (*(header + 1) == 0xd8)) ){
		filename = L".jpg";
	}

	if ( filename != NULL ){// 拡張子による判別
		const WCHAR *extp;
		CODECLISTITEMSTRUCT *codeclistp;

		extp = wcsrchr(filename, '.');
		if ( extp == NULL ) return 0;
		extp++;

		codeclistp = codecs.list;
		int leftcodecs = codecs.count;
		while ( leftcodecs-- ){
			WCHAR *extlist = codeclistp->ext;

			if ( codeclistp->enable ) for (;;){
				if ( *extlist == '\0' ) break;
				if ( wcsicmp(extp, extlist + 1) == 0 ){
					previous_decoder = codeclistp->ext + 1;
					previous_decoder_tid = GetCurrentThreadId();
					return 1;
				}
				extlist += *extlist;
			}
			codeclistp++;
		}
	}
	return 0;
}

extern "C" int __stdcall IsSupported(LPCSTR filename, void *dw)
{
	WCHAR wext[0x100];
	const char *ptr;

	if ( filename == NULL ){
		ptr = NULL;
	}else{
		ptr = strrchr(filename, '.');
	}
	if ( ptr == NULL ){
		wext[0] = '\0';
	}else{
		if ( AnsiToUnicode(ptr, wext, 0x100) == 0 ) return 0;
		wext[0x100 - 1] = '\0';
	}

	return IsSupportedW(wext, dw);
}

//-----------------------------------------------------------------------------
extern "C" SUSIE_ERROR __stdcall GetPictureInfoW(LPCWSTR buf, LONG_PTR len, unsigned int flags, struct PictureInfo *lpInfo)
{
	int result;
	WIC_Susie_class wic;
	SUSIE_DECODE_PARAMETERS params;

	params.struct_size = sizeof(params);
	params.input_flags = default_flags;
;
	params.page_number = 0;

	if ( (flags & 7) == 1 ){ // メモリイメージ
		params.filename = NULL;
		params.fileimage = (const char *)buf;
		params.image_size = len;
	}else if ( (flags & 7) == 0 ){
		params.filename = buf;
		params.fileimage = NULL;
	}else{
		return SUSIEERROR_NOTSUPPORT;
	}

	if ( LoadCodecList() <= 0 ) return SUSIEERROR_INTERNAL;
	if ( FAILED(wic.Init()) ) return SUSIEERROR_INTERNAL;

	for (;;){
		HLOCAL pHBInfo;

		result = wic.InitDecoder(&params);
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.GetFrame();
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.GetPixelInfo();
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.SetBitmapinfoHeader(&pHBInfo);
		if ( result != SUSIEERROR_NOERROR ) break;
		wic.FixAspectRate();
		wic.SetRotate();
		lpInfo->left = lpInfo->top = 0;
		lpInfo->width = static_cast<BITMAPINFOHEADER *>(pHBInfo)->biWidth;
		lpInfo->height = static_cast<BITMAPINFOHEADER *>(pHBInfo)->biHeight;
		lpInfo->x_density = static_cast<WORD>(static_cast<BITMAPINFOHEADER *>(pHBInfo)->biXPelsPerMeter / 39);
		lpInfo->y_density = static_cast<WORD>(static_cast<BITMAPINFOHEADER *>(pHBInfo)->biYPelsPerMeter / 39);
		lpInfo->colorDepth = static_cast<BITMAPINFOHEADER *>(pHBInfo)->biBitCount;
		if ( X_InfoText ){
			lpInfo->hInfo = wic.GetInfoText();
		}else{
			lpInfo->hInfo = NULL;
		}
		LocalFree(pHBInfo);
		break;
	}
	wic.Release();
	return result;
}

extern "C" SUSIE_ERROR __stdcall GetPictureInfo(LPCSTR buf, LONG_PTR len, unsigned int flags, struct PictureInfo *lpInfo)
{
	if ( (flags & 7) == 0 ){ // ファイル名指定
		WCHAR filenameW[MAX_PATH];

		filenameW[0] = '\0';
		if ( AnsiToUnicode(buf, filenameW, MAX_PATH) == 0 ){
			return SUSIEERROR_FAULTREAD;
		}
		filenameW[MAX_PATH - 1] = '\0';
		return GetPictureInfoW(filenameW, len, flags, lpInfo);
	}else{
		return GetPictureInfoW((LPCWSTR)buf, len, flags, lpInfo);
	}
}
//-----------------------------------------------------------------------------
extern "C" SUSIE_ERROR __stdcall DecodePictureW(SUSIE_DECODE_PARAMETERS *params)
{
	int result;
	WIC_Susie_class wic;

	if ( LoadCodecList() <= 0 ) return SUSIEERROR_INTERNAL;
	if ( FAILED(wic.Init()) ) return SUSIEERROR_INTERNAL;

	for (;;){
		if ( params->progressCallback != NULL ) params->progressCallback(0, 100, params->lData);
		result = wic.InitDecoder(params);
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.GetFrame();
		if ( result != SUSIEERROR_NOERROR ) break;
		wic.InitColorContext();
		result = wic.GetPixelInfo();
		if ( result != SUSIEERROR_NOERROR ) break;
		if ( params->input_flags & SUSIE_DECODE_REQUEST_BITMAP ){
			result = wic.SetBitmapinfoHeader(params->pHBInfo);
			if ( result != SUSIEERROR_NOERROR ) break;
			wic.FixAspectRate();
			wic.SetRotate();
			result = wic.CopyPixels(params->pHBm);
			if ( result != SUSIEERROR_NOERROR ){
				LocalFree(*params->pHBInfo);
				*params->pHBInfo = NULL;
				break;
			}
			wic.CopyPalette();
		}
		wic.SetResult(params);
		if ( params->progressCallback != NULL ) params->progressCallback(100, 100, params->lData);
		break;
	}
	wic.Release();
	return result;
}

extern "C" SUSIE_ERROR __stdcall GetPictureW(LPCWSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, SUSIE_PROGRESS progressCallback, LONG_PTR lData)
{
	int result;
	WIC_Susie_class wic;

	if ( LoadCodecList() <= 0 ) return SUSIEERROR_INTERNAL;
	if ( FAILED(wic.Init()) ) return SUSIEERROR_INTERNAL;

	for (;;){
		if ( progressCallback != NULL ) progressCallback(0, 100, lData);
		result = wic.InitSusieDecoder(buf, len, flag);
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.GetFrame();
		if ( result != SUSIEERROR_NOERROR ) break;
		if ( X_ColorProfile ) wic.InitColorContext();
		result = wic.GetPixelInfo();
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.SetBitmapinfoHeader(pHBInfo);
		if ( result != SUSIEERROR_NOERROR ) break;
		wic.FixAspectRate();
		wic.SetRotate();
		result = wic.CopyPixels(pHBm);
		if ( result != SUSIEERROR_NOERROR ){
			LocalFree(*pHBInfo);
			*pHBInfo = NULL;
			break;
		}
		wic.CopyPalette();
		if ( progressCallback != NULL ) progressCallback(100, 100, lData);
		break;
	}
	wic.Release();
	return result;
}

extern "C" SUSIE_ERROR __stdcall GetPicture(LPCSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, SUSIE_PROGRESS progressCallback, LONG_PTR lData)
{
	if ( (flag & SUSIE_SOURCE_MASK) == SUSIE_SOURCE_DISK ){ // ファイル名指定
		WCHAR filenameW[MAX_PATH];

		filenameW[0] = '\0';
		if ( AnsiToUnicode(buf, filenameW, MAX_PATH) == 0 ){
			return SUSIEERROR_FAULTREAD;
		}
		filenameW[MAX_PATH - 1] = '\0';
		return GetPictureW(filenameW, len, flag, pHBInfo, pHBm, progressCallback, lData);
	}else{
		return GetPictureW((LPCWSTR)buf, len, flag, pHBInfo, pHBm, progressCallback, lData);
	}
}
//-----------------------------------------------------------------------------
extern "C" SUSIE_ERROR __stdcall GetPreviewW(LPCWSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, SUSIE_PROGRESS progressCallback, LONG_PTR lData)
{
	int result;
	WIC_Susie_class wic;

	if ( LoadCodecList() <= 0 ) return SUSIEERROR_INTERNAL;
	if ( FAILED(wic.Init()) ) return SUSIEERROR_INTERNAL;

	for (;;){
		if ( progressCallback != NULL ) progressCallback(0, 100, lData);
		result = wic.InitSusieDecoder(buf, len, flag);
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.GetThumbnail();
		if ( result != SUSIEERROR_NOERROR ) break;
		if ( X_ColorProfile ) wic.InitColorContext();
		#ifndef _WIN64
			// Susie 0.4x では 32bit画像を扱えないので変換する
			if ( X_SusieFix && ((flag & SUSIE_SOURCE_MASK) == SUSIE_SOURCE_DISK) ){
				wic.PixelConvert = 24;
			}
		#endif
		result = wic.GetPixelInfo();
		if ( result != SUSIEERROR_NOERROR ) break;
		result = wic.SetBitmapinfoHeader(pHBInfo);
		if ( result != SUSIEERROR_NOERROR ) break;
		wic.FixAspectRate();
		wic.SetRotate();
		result = wic.CopyPixels(pHBm);
		if ( result != SUSIEERROR_NOERROR ){
			LocalFree(*pHBInfo);
			*pHBInfo = NULL;
			break;
		}
		wic.CopyPalette();
		if ( progressCallback != NULL ) progressCallback(100, 100, lData);
		break;
	}
	wic.Release();
	return result;
}

extern "C" SUSIE_ERROR __stdcall GetPreview(LPCSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, SUSIE_PROGRESS progressCallback, LONG_PTR lData)
{
	if ( (flag & SUSIE_SOURCE_MASK) == SUSIE_SOURCE_DISK ){ // ファイル名指定
		WCHAR filenameW[MAX_PATH];

		filenameW[0] = '\0';
		if ( AnsiToUnicode(buf, filenameW, MAX_PATH) == 0 ){
			return SUSIEERROR_FAULTREAD;
		}
		filenameW[MAX_PATH - 1] = '\0';
		return GetPreviewW(filenameW, len, flag, pHBInfo, pHBm, progressCallback, lData);
	}else{
		return GetPreviewW((LPCWSTR)buf, len, flag,  pHBInfo, pHBm, progressCallback, lData);
	}
}
//-----------------------------------------------------------------------------
void GetDesktopRect(HWND hWnd, RECT *desktop)
{
	if ( GetSystemMetrics(SM_CMONITORS) > 1 ){	// マルチモニタ
		HMONITOR hMonitor;
		MONITORINFO MonitorInfo;

		hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MonitorInfo.cbSize = sizeof(MonitorInfo);
		GetMonitorInfoA(hMonitor, &MonitorInfo);
		*desktop = MonitorInfo.rcWork;
	}else{										// シングルモニタ
		SystemParametersInfo(SPI_GETWORKAREA, 0, desktop, 0);
	}
}

void MoveCenterWindow(HWND hWnd, HWND hParentWnd)
{
	RECT box, pbox, desk;
	int parentheight;

	GetDesktopRect(hParentWnd ? hParentWnd : hWnd, &desk);
	GetWindowRect(hWnd, &box);
	if ( hParentWnd != NULL ){
		GetWindowRect(hParentWnd, &pbox);
	}else{
		pbox = desk;
	}
								// 幅と高さに変換
	box.right -= box.left;
	box.bottom -= box.top;
								// 左右を中央に移動
	box.left = pbox.left + (((pbox.right  - pbox.left) - box.right) / 2);

	parentheight = pbox.bottom - pbox.top;
	if ( parentheight > box.bottom ){ // 親の方が背が高い…上下を中央に移動
		box.top  = pbox.top  + ((parentheight - box.bottom) / 2);
	}else{ // 親の方が背が低い…親の下に移動
		box.top  = pbox.bottom - (parentheight / 3);
	}
								// 左右がはみ出しているか？
	if ( (box.left + box.right) > desk.right ){
		box.left = desk.right - box.right;
	}
	if ( box.left < desk.left ) box.left = desk.left;
								// 上下がはみ出しているか？
	if ( (box.top + box.bottom) > desk.bottom ){
		box.top = desk.bottom - box.bottom;
	}
	if ( box.top < desk.top ) box.top = desk.top;

	MoveWindow(hWnd, box.left, box.top, box.right, box.bottom, TRUE);
}

void ConfigProc_InitDialog(HWND hDlg)
{
	HWND hCodecsListWnd;

	MoveCenterWindow(hDlg, GetParent(hDlg));
	hCodecsListWnd = GetDlgItem(hDlg, IDV_CODECS);
	if ( LoadCodecList() > 0 ){ // コーデック一覧・設定読み込み
		ListCodecList(hCodecsListWnd); // コーデック一覧

		// 各種設定
		CheckDlgButton(hDlg, IDX_EXIFROTATE, X_ExifRotate);
		CheckDlgButton(hDlg, IDX_COLORPROFILE, X_ColorProfile == XCP_sRGB);
		CheckDlgButton(hDlg, IDX_EMBEDPROFILE, X_ColorProfile == XCP_Embed);
		CheckDlgButton(hDlg, IDX_CONV32NOALPHA, X_Conv32NoAlpha);
		CheckDlgButton(hDlg, IDX_WIPEALPHAONPALETTE, X_WipeAlphaOnPalette);
#ifndef _WIN64
		CheckDlgButton(hDlg, IDX_SUSIEFIX, X_SusieFix);
#endif
		CheckDlgButton(hDlg, IDX_TRANSP, X_MergeAlpha);
		CheckDlgButton(hDlg, IDX_ASPECT, X_FixAspect);
		CheckDlgButton(hDlg, IDX_SAVEAPPDATA, X_SaveAppData != SAVE_SHARE);
		CheckDlgButton(hDlg, IDX_TinyPage, X_TinyPage);

	}else{ // WIC 使用不可
		LV_ITEM lvi;
		LV_COLUMN lvc;

		lvc.mask = LVCF_WIDTH;
		lvc.cx = 2000;
		ListView_InsertColumn(hCodecsListWnd, 0, &lvc);

		lvi.pszText = L"**WIC Load Error**";
		lvi.mask = LVIF_TEXT;
		lvi.iItem = 0;
		lvi.iSubItem = 0;
		ListView_InsertItem(hCodecsListWnd, &lvi);
	}
}

INT_PTR CALLBACK ConfigProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( iMsg ){
		case WM_INITDIALOG:
			ConfigProc_InitDialog(hDlg);
			break;

		case WM_COMMAND:
			switch ( LOWORD(wParam) ){
				case IDOK:
					if ( codecs.count > 0 ){
						X_ExifRotate = IsDlgButtonChecked(hDlg, IDX_EXIFROTATE);
						X_ColorProfile = XCP_OFF;
						if ( IsDlgButtonChecked(hDlg, IDX_COLORPROFILE) ){
							X_ColorProfile = XCP_sRGB;
						}
						if ( IsDlgButtonChecked(hDlg, IDX_EMBEDPROFILE) ){
							X_ColorProfile = XCP_Embed;
						}
						X_Conv32NoAlpha = IsDlgButtonChecked(hDlg, IDX_CONV32NOALPHA);
						X_WipeAlphaOnPalette = IsDlgButtonChecked(hDlg, IDX_WIPEALPHAONPALETTE);
#ifndef _WIN64
						X_SusieFix = IsDlgButtonChecked(hDlg, IDX_SUSIEFIX);
#endif
						X_MergeAlpha = IsDlgButtonChecked(hDlg, IDX_TRANSP);
						X_FixAspect = IsDlgButtonChecked(hDlg, IDX_ASPECT);
						X_SaveAppData = IsDlgButtonChecked(hDlg,
								IDX_SAVEAPPDATA) ? SAVE_USERAPP : SAVE_SHARE;
						X_TinyPage = IsDlgButtonChecked(hDlg, IDX_TinyPage);
						DumpConfig(hDlg, X_SaveAppData);
						SetDefaultFlags();
					}
					EndDialog(hDlg, TRUE);
					break;

				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					break;

				case IDX_COLORPROFILE:
					CheckDlgButton(hDlg, IDX_EMBEDPROFILE, FALSE);
					break;

				case IDX_EMBEDPROFILE:
					CheckDlgButton(hDlg, IDX_COLORPROFILE, FALSE);
					break;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

INITCOMMONCONTROLSEX icc = {
	sizeof(INITCOMMONCONTROLSEX),
	ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES
};

extern "C" int __stdcall ConfigurationDlg(HWND hWnd, int function)
{
	HMODULE hComctl32;
	DefineWinAPI(void, InitCommonControlsEx, (INITCOMMONCONTROLSEX *));

	if ( function != 1 ) return SUSIEERROR_NOTSUPPORT;

	hComctl32 = LoadLibrary(T("COMCTL32.DLL"));
	if ( hComctl32 == NULL ) return SUSIEERROR_NOTSUPPORT;
	GETDLLPROC(hComctl32, InitCommonControlsEx);
	if ( DInitCommonControlsEx == NULL ){
		FreeLibrary(hComctl32);
		return SUSIEERROR_NOTSUPPORT;
	}
	DInitCommonControlsEx(&icc);

	ExtDialogBoxParam(DLLhInst,
			MAKEINTRESOURCE( (LOWORD(GetUserDefaultLCID()) != LCID_JAPANESE) ?
				IDD_CONFIGE : IDD_CONFIG),
			hWnd, ConfigProc, 0);
	FreeLibrary(hComctl32);
	return SUSIEERROR_NOERROR;
}

extern "C" SUSIE_ERROR __stdcall CreatePictureW(LPCWSTR filepath, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, struct PictureInfo *, SUSIE_PROGRESS progressCallback, LONG_PTR lData)
{
	HRESULT InitResult;
	int result = SUSIEERROR_INTERNAL;
	IWICImagingFactory *wicimage;
	IWICBitmapEncoder *encoder;
	IWICBitmapDecoder *decoder;
	IWICStream *targetStream;
	LPSTREAM srcStream;
	const GUID *imgtype = NULL;
	const WCHAR *ext;
	GUID encoder_id;

	ext = wcsrchr(filepath, '.');
	if ( ext != NULL ){
		if ( (wcsicmp(ext, L".jpeg") == 0) ||
			 (wcsicmp(ext, L".jpg") == 0) ){
			imgtype = &GUID_ContainerFormatJpeg;
		}else if ( wcsicmp(ext, L".png") == 0 ){
			imgtype = &GUID_ContainerFormatPng;
		}else if ( wcsicmp(ext, L".bmp") == 0 ){
			imgtype = &GUID_ContainerFormatBmp;
		}else if ( wcsicmp(ext, L".gif") == 0 ){
			imgtype = &GUID_ContainerFormatGif;
		}
	}else{
		ext = L"."; // 拡張子無し
	}

	InitResult = ::CoCreateInstance(CLSID_WICImagingFactory, NULL,
			CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void**>(&wicimage));
	if ( FAILED(InitResult) ){
		if ( InitResult == CO_E_NOTINITIALIZED ){
			CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
			if( FAILED(::CoCreateInstance(CLSID_WICImagingFactory, NULL,
					CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
					reinterpret_cast<void**>(&wicimage))) ){
				CoUninitialize();
				return SUSIEERROR_INTERNAL;
			}
		}else{
			return SUSIEERROR_INTERNAL;
		}
	}

	if ( imgtype == NULL ){ // コーデック一覧から検索
		IEnumUnknown *codecenum;
		IUnknown *enumitem;

		if ( FAILED(wicimage->CreateComponentEnumerator(WICEncoder,
				WICComponentEnumerateDefault, &codecenum)) ){
			wicimage->Release();
			if ( InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();
			return SUSIEERROR_INTERNAL;
		}
		while ( S_OK == codecenum->Next(1, &enumitem, 0) ){
			IWICBitmapEncoderInfo *wicinfo;
			HRESULT hr;
			WCHAR cbuf[512];
			UINT extlen;

			hr = enumitem->QueryInterface(IID_IWICBitmapEncoderInfo, (void **)&wicinfo);
			enumitem->Release();
			if ( FAILED(hr) ) continue;

			extlen = sizeof(cbuf) / sizeof(WCHAR);
			if ( SUCCEEDED(wicinfo->GetFileExtensions(extlen, cbuf, &extlen)) ){
				WCHAR *extp, *extp_next;

				extp = cbuf;
				for (;;){
					extp_next = wcschr(extp, ',');
					if ( extp_next != NULL ) *extp_next = '\0';
					if ( wcsicmp(extp, ext) == 0 ){
						wicinfo->GetContainerFormat(&encoder_id);
						imgtype = const_cast<const GUID *>(&encoder_id);
						break;
					}
					if ( extp_next == NULL ) break;
					extp = extp_next + 1;
				}
			}
			wicinfo->Release();
			if ( imgtype != NULL ) break;
		}
		codecenum->Release();
		if ( imgtype == NULL ){
			if ( flag & SUSIE_DEST_REJECT_UNKNOWN_TYPE ){
				wicimage->Release();
				if ( InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();
				return SUSIEERROR_UNKNOWNFORMAT;
			}else{
				imgtype = &GUID_ContainerFormatPng;
			}
		}
	}

	if ( progressCallback != NULL ) progressCallback(0, 100, lData);

	// bmp から decoder を作成する
	{
		BYTE *image;
		DWORD infosize, bitmapsize;

		infosize = LocalSize(*pHBInfo);
		bitmapsize = LocalSize(*pHBm);
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPFILEHEADER) + infosize + bitmapsize );
		if ( hGlobal == NULL ){
			wicimage->Release();
			if ( InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();
			return SUSIEERROR_EMPTYMEMORY;
		}
		image = reinterpret_cast<BYTE *>(GlobalLock(hGlobal));
		if ( image != NULL ){
			BITMAPFILEHEADER *bmpfh = reinterpret_cast<BITMAPFILEHEADER *>(image);
			bmpfh->bfType = 'B' + 'M' * 0x100;
			bmpfh->bfSize = sizeof(BITMAPFILEHEADER) + infosize + bitmapsize;
			bmpfh->bfReserved1 = bmpfh->bfReserved2 = 0;
			bmpfh->bfOffBits = sizeof(BITMAPFILEHEADER) + infosize;

			memcpy(image + sizeof(BITMAPFILEHEADER), LocalLock(*pHBInfo), infosize);
			LocalUnlock(*pHBInfo);
			memcpy(image + sizeof(BITMAPFILEHEADER) + infosize, LocalLock(*pHBm), bitmapsize);
			LocalUnlock(*pHBm);
			GlobalUnlock(hGlobal);
			if ( FAILED(CreateStreamOnHGlobal(hGlobal, TRUE, &srcStream)) ){
				GlobalFree(hGlobal);
				srcStream = NULL;
			}
		}else{
			GlobalFree(hGlobal);
			srcStream = NULL;
		}
	}
	if ( (srcStream == NULL) ||
		 FAILED(wicimage->CreateDecoderFromStream(srcStream, 0,
				WICDecodeMetadataCacheOnDemand, &decoder)) ){
		decoder = NULL;
	}

	if ( (decoder == NULL) ||
		 FAILED(wicimage->CreateEncoder(*imgtype, NULL, &encoder)) ){
		encoder = NULL;
	}

	if ( (encoder == NULL) || FAILED(wicimage->CreateStream(&targetStream)) ){
		targetStream = NULL;
	}
	if ( (targetStream != NULL) && SUCCEEDED(targetStream->InitializeFromFilename(filepath, GENERIC_WRITE)) ){
		UINT frameCount = 0;

		if ( SUCCEEDED(encoder->Initialize(targetStream, WICBitmapEncoderNoCache)) ){
			if ( FAILED(decoder->GetFrameCount(&frameCount)) ) frameCount = 1;
		}

		for ( UINT index = 0 ; index < frameCount ; ++index ){
			IWICBitmapFrameDecode *sourceFrame;
			IWICBitmapFrameEncode *targetFrame;
			UINT width = 0;
			UINT height = 0;
			GUID pixelFormat = { 0 };
			UINT ccount;

			if ( FAILED(decoder->GetFrame(index, &sourceFrame)) ){
				frameCount = 0;
				break;
			}

			sourceFrame->GetSize(&width, &height);
			sourceFrame->GetPixelFormat(&pixelFormat);

			if ( FAILED(encoder->CreateNewFrame(&targetFrame, 0)) ){
				sourceFrame->Release();
				frameCount = 0;
				break;
			}
			targetFrame->Initialize(NULL);
			targetFrame->SetSize(width, height);
			targetFrame->SetPixelFormat(&pixelFormat);

			if ( SUCCEEDED(sourceFrame->GetColorContexts(0, NULL, &ccount)) ){
				UINT ret;
				IWICColorContext *srcCc = NULL;

				if ( ccount > 0 ){
					if ( FAILED(wicimage->CreateColorContext(&srcCc)) ){
						srcCc = NULL;
					}else if ( FAILED(sourceFrame->GetColorContexts(1, &srcCc, &ret)) ){
						srcCc->Release();
						srcCc = NULL;
					}
				}else{
					PROPVARIANT v;
					IWICMetadataQueryReader *metadata = NULL;

					sourceFrame->GetMetadataQueryReader(&metadata);
					if ( metadata != NULL ){
						PropVariantInit(&v);

						if ( SUCCEEDED(metadata->GetMetadataByName(L"/app1/ifd/exif/subifd:{uint=40961}", &v)) ||
							 SUCCEEDED(metadata->GetMetadataByName(L"/ifd/exif/subifd:{uint=40961}", &v))){
							UINT cs = (v.vt == VT_UI2) ? v.uiVal : 0;

							PropVariantClear(&v);
							wicimage->CreateColorContext(&srcCc);
							if ( srcCc != NULL ){
								srcCc->InitializeFromExifColorSpace(cs);
							}
						}
						metadata->Release();
					}
				}
				if ( srcCc != NULL ){
					targetFrame->SetColorContexts(1, &srcCc);
					srcCc->Release();
				}
			}

			if ( FAILED(targetFrame->WriteSource(sourceFrame, 0)) ){
				targetFrame->Release();
				sourceFrame->Release();
				frameCount = 0;
				break;
			}
			targetFrame->Commit();
			targetFrame->Release();
			sourceFrame->Release();
		}
		if ( frameCount > 0 ){
			if ( SUCCEEDED(encoder->Commit()) ) result = SUSIEERROR_NOERROR;
		}
	}
	if ( targetStream != NULL ) targetStream->Release();
	if ( encoder != NULL ) encoder->Release();
	if ( decoder != NULL ) decoder->Release();
	if ( srcStream != NULL ) srcStream->Release();
	wicimage->Release();
	if ( InitResult == CO_E_NOTINITIALIZED ) CoUninitialize();
	if ( progressCallback != NULL ) progressCallback(100, 100, lData);
	return result;
}

extern "C" SUSIE_ERROR __stdcall CreatePicture(LPCSTR filepath, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, struct PictureInfo *lpInfo, SUSIE_PROGRESS progressCallback, LONG_PTR lData)
{
	WCHAR filenameW[MAX_PATH];

	filenameW[0] = '\0';
	if ( AnsiToUnicode(filepath, filenameW, MAX_PATH) == 0 ){
		return SUSIEERROR_FILEWRITE;
	}
	filenameW[MAX_PATH - 1] = '\0';
	return CreatePictureW(filenameW, flag, pHBInfo, pHBm, lpInfo, progressCallback, lData);
}
