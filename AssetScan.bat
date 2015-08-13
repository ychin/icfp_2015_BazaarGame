@echo off
if exist ..\AssetScan\AssetScan.exe (
    ..\AssetScan\AssetScan.exe
	goto done
)
if exist ..\..\AssetScan\AssetScan.exe (
    ..\..\AssetScan\AssetScan.exe
	goto done
)
if exist ..\..\..\AssetScan\AssetScan.exe (
    ..\..\..\AssetScan\AssetScan.exe
	goto done
)
if exist ..\..\..\..\AssetScan\AssetScan.exe (
    ..\..\..\..\AssetScan\AssetScan.exe
	goto done
)

echo Didin't find AssetScan.exe

:done