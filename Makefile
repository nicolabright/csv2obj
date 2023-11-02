all:    csv2obj.c
	@del csv2obj.exe 2> nul
	gcc csv2obj.c -o csv2obj.exe

clean:
	del csv2obj.exe 2> nul
