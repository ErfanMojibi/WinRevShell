CC_64=x86_64-w64-mingw32-gcc
revshell:
	$(CC_64) -o revshell.exe revshell.c -lws2_32
