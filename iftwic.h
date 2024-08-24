/*-----------------------------------------------------------------------------
	Windows Imaging Component Susie Plug-in	Copyright (c) TORO
-----------------------------------------------------------------------------*/
#define APPNAME "iftwic"

#define CFG_COMPANY	"TOROID"	// AppData �̒����Ɏg�p���閼�O
#define CFG_APPNAME	APPNAME		// CFG_COMPANY �̉��Ɏg�p���閼�O
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

// Susie Plug-in �֘A�̒�` ---------------------------------------------------
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

// �R�[���o�b�N
typedef int (__stdcall *SUSIE_PROGRESS)(int nNum, int nDenom, LONG_PTR lData);

// Susie �p�� UNIX ����
typedef ULONG_PTR susie_time_t;

// Susie �\����
#pragma pack(push, 1)
typedef struct PictureInfo{
	long    left, top;	// �摜��W�J����ʒu
	long    width;	// �摜�̕�(pixel)
	long    height;	// �摜�̍���(pixel)
	WORD    x_density;	// ��f�̐����������x
	WORD    y_density;	// ��f�̐����������x
	short   colorDepth;	// �P��f�������bit��
	#ifdef _WIN64
	  char  dummy[2]; // �A���C�������g
	#endif
	HLOCAL  hInfo;	// �摜���̃e�L�X�g���
} SUSIE_PICTUREINFO;

typedef struct {
	DWORD struct_size;  // [IN] sizeof(SUSIE_DECODE_PARAMETERS)
	DWORD input_flags;  // [IN] Plug-in �ɓn�� SUSIE_DECODE_xxx
	DWORD reserved;     // ���g�p�A�A���C�������g�����p
	DWORD page_number;  // [IN] �}���`�y�[�W�̎��̎擾�y�[�W 0: 1�Ԗ�

	const WCHAR *filename; // [IN] �t�@�C���� (fileimage���L���̎��ł����ʗp�ɗ��p)
	LONG_PTR file_offset;  // [IN] �t�@�C����ǂݍ��ݏo���ʒu
	const char *fileimage; // [IN] �t�@�C���̃������C���[�W (������� NULL)
	LONG_PTR image_size;   // [IN] fileimage �̑傫��

	SUSIE_PROGRESS progressCallback; // [IN] �o�ߕ\���E���f�p�R�[���o�b�N
	LONG_PTR lData;                  // [IN] progressCallback �̃��[�U��`

	char *infotext; // [OUT] �t�@�C�����(LocalFree �ŉ�����K�v�ANULL: �Ȃ�)
	HLOCAL *pHBInfo, *pHBm; // [OUT,OPTION] �f�R�[�h���� BMP
		// pHBInfo BITMAPINFOHEADER ���� BITMAPV5HEADER
		// pHBm    �r�b�g�}�b�v��f

	DWORD output_flags; // [OUT] Plug-in ����񍐂���� SUSIE_DECODE_xxx
	DWORD rotate;       // [OUT] ��]�␳���B�␳�ς݂ł������l
	DWORD all_pages;    // [OUT] �}���`�y�[�W�̃y�[�W�� 1: �S1�y�[�W
	DWORD animate_time; // [OUT,OPTION] �A�j���[�V�����̎��̎��̃t���[���܂ł̑ҋ@ ms
	POINT left_top;     // [OUT] �\���J�n�ʒu(layer, icon��)
	SIZE request_size;  // [IN,OPTION] vector / preview �Ŋ�]����傫��(SUSIE_DECODE_ENABLE_SIZE)
} SUSIE_DECODE_PARAMETERS;

#define SUSIE_DECODE_REQUEST_BITMAP        0x01 // [IN] BITMAP ���擾
		// GetPicture �� pHBInfo, pHBm ���擾����

#define SUSIE_DECODE_REQUEST_INFOTEXT      0x02 // [IN] infotext������Ύ擾
		// GetPictureInfo �� hInfo ���擾�\�Ȃ�擾����

#define SUSIE_DECODE_ALLOW_BMPV5           0x04 // [IN] BMPv5 �g�p�\
		// �ʏ�� pHBInfo �� BITMAPINFOHEADER ��Ԃ����A
		// ������w�肷��� BITMAPV5HEADER �ŕԂ����Ƃ�����

#define SUSIE_DECODE_ENABLE_SIZE           0x08 // [IN] request_size ���L��
		// vector �`���� preview image ������Ƃ��Arequest_size �ɋ߂��傫����
		// �摜��p�ӂ���

#define SUSIE_DECODE_REQUEST_ROTATE        0x10 // [IN] ��]�␳����]
		// Plug-in �ɉ�]�̕␳�@�\������Ύg�p����

#define SUSIE_DECODE_REQUEST_MERGE_ALPHA   0x20 // [IN] ���ߕ␳����]
		// Plug-in �ɓ��߂̕␳�@�\������Ύg�p����
		// �Ăяo�����ɓ��ߏ����@�\�������Ƃ��Ɏg�p����

#define SUSIE_DECODE_REQUEST_FIX_ASPECT    0x40 // [IN] �A�X�y�N�g�䒲������]
		// Plug-in �ɃA�X�y�N�g�䒲���@�\������Ύg�p����
		// �Ăяo�����ɃA�X�y�N�g�䒲���@�\�������Ƃ��Ɏg�p����

#define SUSIE_DECODE_REQUEST_COLOR_PROFILE 0x80 // [IN] �J���[�v���t�@�C������]
		// �v SUSIE_DECODE_ALLOW_BMPV5�B�J���[�v���t�@�C���� BITMAPV5HEADER ��
		// ���ɔz�u���ĕԂ�

#define SUSIE_DECODE_REQUEST_FIX_sRGB     0x100 // [IN] sRGB�ɕ␳��]
		// Plug-in ��sRGB�␳�@�\������Ύg�p����
		// �Ăяo������sRGB�␳�@�\�������Ƃ��Ɏg�p����


#define SUSIE_DECODE_ALLOW_TOP_DOWN       0x200 // [IN] �㉺���](top down)������

#define SUSIE_DECODE_USE_SIZE              0x08 // [OUT] request_size ���Q�Ƃ���
		// SUSIE_DECODE_ENABLE_SIZE �Ɋ�Â��摜��p�ӂ���

#define SUSIE_DECODE_FIX_ROTATE            0x10 // [OUT] ��]�␳������
		// SUSIE_DECODE_REQUEST_ROTATE �ɂ���]�␳������

#define SUSIE_DECODE_MERGE_ALPHA           0x20 // [OUT] ���ߕ␳������
		// SUSIE_DECODE_REQUEST_MERGE_ALPHA �ɂ�铧�ߕ␳������

#define SUSIE_DECODE_FIX_ASPECT            0x40 // [OUT] �A�X�y�N�g�䒲��������
		// SUSIE_DECODE_REQUEST_FIX_ASPECT �ɂ��A�X�y�N�g�䒲��������
		// �� SUSIE_DECODE_REQUEST_FIX_ASPECT ���Ȃ��Ƃ��́A
		//    biXPelsPerMeter / biYPelsPerMeter ���g���Ď����Œ���

#define SUSIE_DECODE_FIX_sRGB             0x100 // [OUT] sRGB�ɕ␳
		// SUSIE_DECODE_REQUEST_FIX_sRGB �ɂ��sRGB �␳������

#define SUSIE_DECODE_VECTOR             0x20000 // [OUT]
		// �摜�̃T�C�Y���ρB�Ⴆ�� vector �摜�Apreview ���蓙

#define SUSIE_DECODE_ROTATE             0x40000 // [OUT] ��]�␳���L��

#define SUSIE_DECODE_ALPHA              0x80000 // [OUT] ���ߏ��L��(Alpha�L)

#define SUSIE_DECODE_IN_COLOR_PROFILE 0x0200000 // [OUT] �J���[�v���t�@�C��������
		// SUSIE_DECODE_REQUEST_COLOR_PROFILE �w��ɂ�����炸�A
		// �摜�ɃJ���[�v���t�@�C��������ꍇ�ɐݒ�

#define SUSIE_DECODE_ANIMATION        0x0800000 // [OUT] �A�j���[�V�������\
		// all_pages �� frame ���Bpage_number ������ frame

#define SUSIE_DECODE_LAYER            0x1000000 // [OUT] ���C���[������
		// all_pages �� layer ���Bpage_number ���擾 layer
#pragma pack(pop)
