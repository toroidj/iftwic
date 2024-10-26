#=======================================
targetname = iftwic

ifndef ARM
ARM = 0
endif
ifndef X64
X64 = 0
endif
ifndef RELEASE
RELEASE = 0
endif

ifeq ($(strip $(ARM)),1)
  TAIL	= 64A
else
  ifeq ($(strip $(X64)),1)
    TAIL	= 64
    targetext	= sph
  else
    TAIL	= 32
    targetext	= spi
  endif
endif
objdir = .obj

all:	$(objdir) code$(TAIL)$(RELEASE).o $(targetname).$(targetext)
objs = $(objdir)/$(targetname).o $(objdir)/TCFG.o $(objdir)/$(targetname).coff

gccname	= gcc -finput-charset=CP932 -fexec-charset=CP932
releaseopt	= -O2 -Os -flto -s
devopt	= -g
cc	= @$(gccname) $(WarnOpt)
lddll	= @$(gccname) -shared -static $(WarnOpt)
Rcn	= wrc -l0x11 --nostdinc -l0x11 -I. -I/usr/include/wine/windows -I/usr/include/wine/wine/windows -I/usr/local/include/wine/windows

WarnOpt = -Wall -Wno-unknown-pragmas -Wl,--kill-at

link	= @$(gccname) -s
libs	= -lm -lpthread -lstdc++ -lole32 -loleaut32 -luuid


.SUFFIXES: .coff .mc .rc .mc.rc .res .res.o .spec .spec.o .idl .tlb .h  .ico .RC .c .C .cpp .CPP

$(objdir)/%.o: %.C | $(objdir)
	$(cc) -x c -c $< -o $@
$(objdir)/%.o: %.CPP | $(objdir)
	$(cc) -x c++ -c $< -o $@

# 大文字小文字を同一視する make の場合は以下２つの定義で警告が出る
$(objdir)/%.o: %.c | $(objdir)
	$(cc) -x c -c $< -o $@
$(objdir)/%.o: %.cpp | $(objdir)
	$(cc) -x c++ -c $< -o $@


$(objdir)/%.coff: %.rc | $(objdir)
	@echo $<
        ifeq ($(strip $(OldMinGW)),1)
	  $(Rcn) -fo$(basename $<).res $<
	  @windres -i $(basename $<).res -o $@
        else
	  @windres -DWINDRES -i $(basename $<).rc -o $@
        endif

.RC.res:
	@echo $<
	$(Rcn) $< -o $(basename $<).res

.spec.spec.o:
	winebuild -D_REENTRANT -fPIC --as-cmd "as" --dll -o $@ --main-module $(MODULE) --export $<

#------------------------------------------------------ code体系切換用
code$(TAIL)$(RELEASE).o:
	-@cmd.exe /c del "$(targetname)$(TAIL).$(targetext)"
	-@cmd.exe /c del "*.o"
	-@cmd.exe /c del "$(objdir)\\*.o"
	-@cmd.exe /c del "$(objdir)\\*.coff"
	-@cmd.exe /c copy nul "code$(TAIL)$(RELEASE).o"

$(objdir):
	-@cmd.exe /c md "$(objdir)"

#------------------------------------------------------ 本体
$(targetname).$(targetext): $(objs)
  ifeq ($(strip $(RELEASE)),1)
	$(lddll) $(releaseopt) $^ $(libs) -o $(targetname).$(targetext)
  else
	$(lddll) $(devopt) $^ $(libs) -o $(targetname).$(targetext)
  endif

(objdir)/$(targetname).o: $(targetname).cpp
(objdir)/$(targetname).coff: $(targetname).rc
