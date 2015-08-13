REM switch to this directory
pushd "~dp0"

REM copy build of game.exe if it exists
REM to package for judges to play with
if exist windows\Release\Game.exe (
    copy windows\Release\Game.exe .\Game.exe
)

REM delete stuff!
rmdir /s /q windows\Debug
rmdir /s /q windows\Release
rmdir /s /q windows\ipch
rmdir /s /q windows\HoneyRunner\Debug
rmdir /s /q windows\HoneyRunner\Release
rmdir /s /q xbin
del /f /q output*.txt
del /f /q windows\Honey.sdf

REM switch back
popd