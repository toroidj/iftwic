name = iftwic
# nmake を用いる場合、$.→$@、$&→*に書き換え、@&&|～|ブロックの対処が必要

UseDebug = 0	# 1 なら debug指定
!ifndef X64
X64 = 0		# 1 なら x64版(常にUNICODE版)
!else
X64 = 1
!endif
!ifndef ARM
ARM = 0		# 1 なら Arm/Arm64版(常にUNICODE版)
!endif
!ifndef UseUnicode
 UseUnicode = 0	# 1 なら UNICODE 版
!endif

!ifdef RELEASE
TAIL = R
UseDebug = 0
Copt = /DRELEASE
!endif

!ifdef FORVIEW
FORVIEW = /DFORVIEW
!endif

!if $(X64)
UseUnicode	= 0
TAIL	= $(TAIL)64
spi	= sph
!else
SSEOPT = /arch:SSE2
spi	= spi
!endif

!if $(ARM)
UseUnicode	= 0
TAIL	= $(TAIL)A
spi	= $(spi)a
!endif

!if $(UseUnicode)
  UnicodeCopt	= -DUNICODE
!endif
#============================================================= Microsoft 用設定
!if $(UseDebug)
Copt	= /Od /Zi /GS #/analyze
DebugLopt	= /DEBUG
!else # UseDebug
!ifdef RELEASE
Copt	= /Gz /O2 /Os /Oy /DRELEASE
!else # RELEASE
Copt	= /Gz /O2 /Os /Oy
!endif # RELEASE
!endif # UseDebug

!if $(X64) # x64, ARM64
X64Copt	= /wd4244 /wd4267
# ↑サイズ違い警告を消す(※できるだけこの指定を使わないこと)
!if !$(ARM) # x64
LINK64	= /MACHINE:AMD64 /LARGEADDRESSAWARE
RC64	= /d_WIN64
!else # ARM64
LINK64	= /MACHINE:ARM64 /LARGEADDRESSAWARE
Copt	= /O2 /Os /Oy /DRELEASE
X64Copt	= /wd4244 /wd4267
RC64	= /d_WIN64 /d_ARM64_
!endif # ARM64
!else # !$(X64), x86, ARM32
!if $(ARM) # ARM32
LINK64	= /MACHINE:ARM /LARGEADDRESSAWARE
!endif # ARM32
!endif # X64

#-------------------------------------- コンパイラ
cc	= @"cl" /GF /nologo /c /DWINVER=0x400 $(Copt) $(UnicodeCopt) $(WineCopt) $(X64Copt) $(FORVIEW) /W3 /wd4068 /wd4996
# 4068:不明な #pragma	4996:古い関数を使用した	/Wp64:64bit移植性チェック
#-------------------------------------- リンカ共通指定
linkopt = @"link" /NOLOGO /INCREMENTAL:NO /MAP /OPT:REF /OPT:ICF /NXCOMPAT /DYNAMICBASE

C0DLL	= Kernel32.Lib AdvAPI32.Lib Ole32.Lib Gdi32.Lib\
	  Shell32.Lib User32.Lib comctl32.lib Comdlg32.lib
C0GUI	= $(C0DLL)
C0CON	= $(C0DLL)
#-------------------------------------- リンカ(GUI)
linkexe	= $(linkopt) /SUBSYSTEM:WINDOWS
#-------------------------------------- リンカ(CONSOLE)
linkcon	= $(linkopt) /SUBSYSTEM:CONSOLE
#-------------------------------------- リンカ(DLL)
linkdll	= $(linkopt) /DLL
#-------------------------------------- リソースコンパイラ
rc	= @%COMSPEC% /C RC /dSDKRC $(RC64) $(UnicodeCopt)

#-------------------------------------- 暗黙の指定
.c.obj:
  $(cc) /Fo$@ $<

.cpp.obj:
  $(cc) /Fo$@ $<

.rc.res:
  $(rc) $<

#------------------------------------------------------------------------------
allFiles:	makefile	CODE$(TAIL).OBJ	$(name).$(spi)

#------------------------------------------------------ code体系切換用
CODE$(TAIL).OBJ:
  -@del *.obj 2> nul
  -@del *.res 2> nul
  -@if exist %CSM% del %CSM%\*.CSM
  @copy nul CODE$(TAIL).OBJ > nul

#------------------------------------------------------------------------------
$(name).$(spi):	$(name).obj	$(name).RES	TCFG.OBJ
  $(linkdll) $(C0DLL) /DEF:$&.DEF windowscodecs.lib TCFG.OBJ $& /OUT:$&.$(spi) $(name).RES

$(name).obj: $(name).cpp $(name).h
TCFG.obj: TCFG.c
