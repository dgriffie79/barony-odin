@echo off
cd /d C:\dev\barony-odin\tmp
"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-cl.exe" /nologo /EHsc /Od /I. /Fe:C:\dev\barony-odin\tmp\item_probe.exe C:\dev\barony-odin\tmp\item_probe.cpp
if errorlevel 1 exit /b 1
C:\dev\barony-odin\tmp\item_probe.exe
