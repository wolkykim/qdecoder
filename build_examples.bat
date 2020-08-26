:: Build qDecoder examples for Windows against MinGW64
:: Tested under Windows 7 Pro 64-bit against TDM64-GCC 4.7.1
@echo off
echo Build qDecoder examples...
for %%f in (examples\*.c) do (
    echo . %%~nf.cgi
    gcc -m64 -Wall -O2 -s -Isrc examples\%%~nf.c -Lbin -lqdecoder -o bin\%%~nf.cgi
)
if not exist bin\index.html copy examples\index.html bin\
