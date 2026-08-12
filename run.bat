@echo off
rem Run barony under cdb to catch the inventory-open crash.
rem When it crashes, cdb pauses and prints the exception. Type 'kn 20' then
rem '!analyze -v' to see the stack, then 'q' to quit.
set CDB="C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\cdb.exe"
%CDB% -g -c ".sympath C:\dev\barony-odin\builddir\src; sxe av; sxe eh; sxe 0x40000015; sxe 0x4000001e; g" ^
  C:\dev\barony-odin\builddir\src\barony.exe -datadir="C:\Program Files (x86)\Steam\steamapps\common\Barony"
