@echo off
@del csv2obj.exe  2> nul
@del dati_OUT.obj 2> nul
make
csv2obj.exe dati_in.pgm dati_OUT.obj
pause