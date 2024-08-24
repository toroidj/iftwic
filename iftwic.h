/*-----------------------------------------------------------------------------
	Windows Imaging Component Susie Plug-in	Copyright (c) TORO
-----------------------------------------------------------------------------*/
#define APPNAME "iftwic"

#define CFG_COMPANY	"TOROID"	// AppData の直下に使用する名前
#define CFG_APPNAME	APPNAME		// CFG_COMPANY の下に使用する名前
#define VERSION		2, 2, 0
#define VERSIONS	"2.2"
#define Copyright	"Copyright (c)2013-2024 TORO"
#define ENABLE_INFOTEXT 0

#define IDD_CONFIGE	10600
#define IDD_CONFIG	10601
#define IDX_EXIFROTATE		10602
#define IDX_COLORPROFILE	10603
#define IDX_EMBEDPROFILE	10604
#define IDX_SUSIEFIX		10605
#define IDX_CONV32NOALPHA	10606
#define IDX_WIPEALPHAONPALETTE	10607
#define IDX_SAVEAPPDATA		10608
#define IDE_EXTEXT			10609
#define IDV_CODECS			10610
#define IDX_TRANSP			10611
#define IDX_ASPECT			10612
#define IDX_TinyPage		10613

#define IDS_COLUMN1E		10640
#define IDS_COLUMN2E		10641
#define IDS_COLUMN3E		10642
#define IDS_COLUMN4E		10643
#define IDS_STRINGJP		10
#define IDS_COLUMN1J		(IDS_COLUMN1E + IDS_STRINGJP)
#define IDS_COLUMN2J		(IDS_COLUMN2E + IDS_STRINGJP)
#define IDS_COLUMN3J		(IDS_COLUMN3E + IDS_STRINGJP)
#define IDS_COLUMN4J		(IDS_COLUMN4E + IDS_STRINGJP)

// Susie Plug-in 関連の定義 ---------------------------------------------------
#define SUSIEERROR_NOERROR       0
#define SUSIEERROR_NOTSUPPORT   -1
#define SUSIEERROR_USERCANCEL    1
#define SUSIEERROR_UNKNOWNFORMAT 2
#define SUSIEERROR_BROKENDATA    3
#define SUSIEERROR_EMPTYMEMORY   4
#define SUSIEERROR_FAULTMEMORY   5
#define SUSIEERROR_FAULTREAD     6
#define SUSIEERROR_RESERVED      7
#define SUSIEERROR_INTERNAL      8

#define SUSIE_SOURCE_MASK 7
#define SUSIE_SOURCE_DISK 0
#define SUSIE_SOURCE_MEM 1
#define SUSIE_IGNORECASE 0x80
#define SUSIE_DEST_MASK 0x700
#define SUSIE_DEST_DISK 0
#define SUSIE_DEST_MEM 0x100
#define SUSIE_DEST_REJECT_UNKNOWN_TYPE 0x800

#define SUSIE_CHECK_SIZE (2 * 1024)
#define SUSIE_PATH_MAX 200

// コールバック
typedef int (__stdcall *SUSIE_PROGRESS)(int nNum, int nDenom, LONG_PTR lData);

// Susie 用の UNIX 時刻
typedef ULONG_PTR susie_time_t;

// Susie 構造体
#pragma pack(push, 1)
typedef struct PictureInfo{
	long    left, top;	// 画像を展開する位置
	long    width;	// 画像の幅(pixel)
	long    height;	// 画像の高さ(pixel)
	WORD    x_density;	// 画素の水平方向密度
	WORD    y_density;	// 画素の垂直方向密度
	short   colorDepth;	// １画素当たりのbit数
	#ifdef _WIN64
	  char  dummy[2]; // アラインメント
	#endif
	HLOCAL  hInfo;	// 画像内のテキスト情報
} SUSIE_PICTUREINFO;

typedef struct {
	DWORD struct_size;  // [IN] sizeof(SUSIE_DECODE_PARAMETERS)
	DWORD input_flags;  // [IN] Plug-in に渡す SUSIE_DECODE_xxx
	DWORD reserved;     // 未使用、アラインメント調整用
	DWORD page_number;  // [IN] マルチページの時の取得ページ 0: 1番目

	const WCHAR *filename; // [IN] ファイル名 (fileimageが有効の時でも判別用に利用)
	LONG_PTR file_offset;  // [IN] ファイルを読み込み出す位置
	const char *fileimage; // [IN] ファイルのメモリイメージ (無ければ NULL)
	LONG_PTR image_size;   // [IN] fileimage の大きさ

	SUSIE_PROGRESS progressCallback; // [IN] 経過表示・中断用コールバック
	LONG_PTR lData;                  // [IN] progressCallback のユーザ定義

	char *infotext; // [OUT] ファイル情報(LocalFree で解放が必要、NULL: なし)
	HLOCAL *pHBInfo, *pHBm; // [OUT,OPTION] デコードした BMP
		// pHBInfo BITMAPINFOHEADER 又は BITMAPV5HEADER
		// pHBm    ビットマップ画素

	DWORD output_flags; // [OUT] Plug-in から報告される SUSIE_DECODE_xxx
	DWORD rotate;       // [OUT] 回転補正情報。補正済みでも同じ値
	DWORD all_pages;    // [OUT] マルチページのページ数 1: 全1ページ
	DWORD animate_time; // [OUT,OPTION] アニメーションの時の次のフレームまでの待機 ms
	POINT left_top;     // [OUT] 表示開始位置(layer, icon等)
	SIZE request_size;  // [IN,OPTION] vector / preview で希望する大きさ(SUSIE_DECODE_ENABLE_SIZE)
} SUSIE_DECODE_PARAMETERS;

#define SUSIE_DECODE_REQUEST_BITMAP        0x01 // [IN] BITMAP を取得
		// GetPicture の pHBInfo, pHBm を取得する

#define SUSIE_DECODE_REQUEST_INFOTEXT      0x02 // [IN] infotextがあれば取得
		// GetPictureInfo の hInfo が取得可能なら取得する

#define SUSIE_DECODE_ALLOW_BMPV5           0x04 // [IN] BMPv5 使用可能
		// 通常は pHBInfo で BITMAPINFOHEADER を返すが、
		// これを指定すると BITMAPV5HEADER で返すことがある

#define SUSIE_DECODE_ENABLE_SIZE           0x08 // [IN] request_size が有効
		// vector 形式や preview image があるとき、request_size に近い大きさで
		// 画像を用意する

#define SUSIE_DECODE_REQUEST_ROTATE        0x10 // [IN] 回転補正を希望
		// Plug-in に回転の補正機能があれば使用する

#define SUSIE_DECODE_REQUEST_MERGE_ALPHA   0x20 // [IN] 透過補正を希望
		// Plug-in に透過の補正機能があれば使用する
		// 呼び出し元に透過処理機能が無いときに使用する

#define SUSIE_DECODE_REQUEST_FIX_ASPECT    0x40 // [IN] アスペクト比調整を希望
		// Plug-in にアスペクト比調整機能があれば使用する
		// 呼び出し元にアスペクト比調整機能が無いときに使用する

#define SUSIE_DECODE_REQUEST_COLOR_PROFILE 0x80 // [IN] カラープロファイル受取希望
		// 要 SUSIE_DECODE_ALLOW_BMPV5。カラープロファイルを BITMAPV5HEADER の
		// 後ろに配置して返す

#define SUSIE_DECODE_REQUEST_FIX_sRGB     0x100 // [IN] sRGBに補正希望
		// Plug-in にsRGB補正機能があれば使用する
		// 呼び出し元にsRGB補正機能が無いときに使用する


#define SUSIE_DECODE_ALLOW_TOP_DOWN       0x200 // [IN] 上下反転(top down)を許可

#define SUSIE_DECODE_USE_SIZE              0x08 // [OUT] request_size を参照した
		// SUSIE_DECODE_ENABLE_SIZE に基づく画像を用意した

#define SUSIE_DECODE_FIX_ROTATE            0x10 // [OUT] 回転補正をした
		// SUSIE_DECODE_REQUEST_ROTATE による回転補正をした

#define SUSIE_DECODE_MERGE_ALPHA           0x20 // [OUT] 透過補正をした
		// SUSIE_DECODE_REQUEST_MERGE_ALPHA による透過補正をした

#define SUSIE_DECODE_FIX_ASPECT            0x40 // [OUT] アスペクト比調整をした
		// SUSIE_DECODE_REQUEST_FIX_ASPECT によるアスペクト比調整をした
		// ※ SUSIE_DECODE_REQUEST_FIX_ASPECT がないときは、
		//    biXPelsPerMeter / biYPelsPerMeter を使って自分で調整

#define SUSIE_DECODE_FIX_sRGB             0x100 // [OUT] sRGBに補正
		// SUSIE_DECODE_REQUEST_FIX_sRGB によるsRGB 補正をした

#define SUSIE_DECODE_VECTOR             0x20000 // [OUT]
		// 画像のサイズが可変。例えば vector 画像、preview あり等

#define SUSIE_DECODE_ROTATE             0x40000 // [OUT] 回転補正情報有り

#define SUSIE_DECODE_ALPHA              0x80000 // [OUT] 透過情報有り(Alpha有)

#define SUSIE_DECODE_IN_COLOR_PROFILE 0x0200000 // [OUT] カラープロファイルがある
		// SUSIE_DECODE_REQUEST_COLOR_PROFILE 指定にかかわらず、
		// 画像にカラープロファイルがある場合に設定

#define SUSIE_DECODE_ANIMATION        0x0800000 // [OUT] アニメーションが可能
		// all_pages が frame 数。page_number が現在 frame

#define SUSIE_DECODE_LAYER            0x1000000 // [OUT] レイヤーがある
		// all_pages が layer 数。page_number が取得 layer
#pragma pack(pop)
