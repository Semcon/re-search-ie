REM Build Plugins
CALL "%VS140COMNTOOLS%\VsDevCmd.bat"

REM Restore nuget packages
IF NOT EXIST packages (
	nuget.exe restore ReSearchIE.sln
)

REM Build libraries
msbuild ReSearchIE.sln /m /property:Configuration=Release /property:Platform=x86
msbuild ReSearchIE.sln /m /property:Configuration=Release /property:Platform=x64

REM Cleanup from old build
del tmp\*.wixobj
del bin\*.msi
del bin\*.wixpdb

SET PATH=%PATH%;%WIX%\bin

REM Generate installer
candle -arch x86 installer\installer.wxs -out tmp\x86_installer.wixobj
candle -arch x64 installer\installer.wxs -out tmp\x64_installer.wixobj

light -out bin\ReSearchIE32.msi -b installer -sice:ICE03 -sice:ICE61 -sice:ICE82 -ext WixUIExtension tmp\x86_installer.wixobj 
light -out bin\ReSearchIE.msi -b installer -sice:ICE03 -sice:ICE61 -sice:ICE80 -sice:ICE82 -ext WixUIExtension tmp\x64_installer.wixobj
