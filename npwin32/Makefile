TARGET=win32api
DEFFILE=$(TARGET).def
SOURCE=$(TARGET).cc
OBJ=npobj.obj win32api.obj memmgr.obj
RES=$(TARGET).res

CC=CL
LINK=link.exe
INCLUDEDIR=/I"include"

#CFLAGS=/DDEBUG /DWIN32 /D_WINDOWS /DXP_WIN32 /DMOZILLA_STRICT_API /DXPCOM_GULE /DXP_WIN /D_X86_ /DNPSIMPLE_EXPORTS /DXULRUNNER_SDK /W4 /O2 /GS /LD $(INCLUDEDIR) 
CFLAGS=/DWIN32 /D_WINDOWS /DXP_WIN32 /DMOZILLA_STRICT_API /DXPCOM_GULE /DXP_WIN /D_X86_ /DNPSIMPLE_EXPORTS /DXULRUNNER_SDK /W4 /O2 /GS /LD $(INCLUDEDIR) 
LFLAGS=/DEF:$(DEFFILE) /DLL /DYNAMICBASE:NO /MACHINE:X86 /RELEASE kernel32.lib user32.lib gdi32.lib
RCFLAGS=/d "NDEBUG" /d "_VC80_UPGRADE=0x0600" /l 0x409

$(TARGET).dll : $(DEFFILE) $(OBJ) $(RES)
	$(LINK) $(LFLAGS) $(OBJ) $(RES)

cc.obj:
	echo $(CC) $(CFLAGS) /c $<
	$(CC) $(CFLAGS) /c $<

res.rc:
	echo $(RC) $(RCFLAGS) /Fo$*.res $<
	$(RC) $(RCFLAGS) /Fo$*.res $<

clean : 
	@if exist $(TARGET).dll del $(TARGET).dll
	@if exist $(TARGET).lib del $(TARGET).lib
	@if exist $(TARGET).exp del $(TARGET).exp
	@if exist $(TARGET).res del $(TARGET).res
	@if exist $(TARGET).obj del $(TARGET).obj
	@if exist $(TARGET).pdb del $(TARGET).pdb
	@for %%a in ( $(OBJ) ) do @if exist %%a del %%a
	@for %%a in ( $(OBJ) ) do @if exist %%~na.exp del %%~na.exp
	@for %%a in ( $(OBJ) ) do @if exist %%~na.lib del %%~na.lib

install :
	copy $(TARGET).dll sample\\$(TARGET).dll

