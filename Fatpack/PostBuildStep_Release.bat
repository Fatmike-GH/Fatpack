cd..
cd build
cd x64
cd Release

START /WAIT ResourceAdder Loader_Console_Resource.exe Fatpack.exe 1000
powershell -command "Start-Sleep -Milliseconds 1000"
START /WAIT ResourceAdder Loader_Windows_Resource.exe Fatpack.exe 1001
powershell -command "Start-Sleep -Milliseconds 1000"

START /WAIT ResourceAdder Loader_Console_Section.exe Fatpack.exe 1002
powershell -command "Start-Sleep -Milliseconds 1000"
START /WAIT ResourceAdder Loader_Windows_Section.exe Fatpack.exe 1003
powershell -command "Start-Sleep -Milliseconds 1000"