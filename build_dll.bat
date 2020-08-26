:: Build qDecoder DLL for Windows against MinGW64
:: Tested under Windows 7 Pro 64-bit against TDM64-GCC 4.7.1
@echo off
echo Build qDecoder DLL...
if not exist bin\ mkdir bin
gcc -m64 -Wall -O2 -s -shared -D_GNU_SOURCE src\msw_missing.c src\internal.c src\qcgireq.c src\qcgires.c src\qcgisess.c src\qentry.c -o bin\qdecoder.dll
