@echo off

set CommonCompilerFlags=-MD -I..\SDL2-2.0.3\include -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7
set CommonLinkerFlags=..\SDL2-2.0.3\lib\SDL2main.lib ..\SDL2-2.0.3\lib\SDL2.lib -incremental:no -opt:ref  -SUBSYSTEM:WINDOWS

pushd build
echo WAITING FOR PDB > gamedll.lock
cl -nologo -Z7 ..\game.cpp -Fmgame.map -LD /link -incremental:no -PDB:game_%random%.pdb -EXPORT:GameUpdateAndRender
del gamedll.lock
cl -nologo -Z7 -Gm- -GR- -EHa- -EHsc ..\sdl_platform.cpp -MD -Od -Oi -FC -fp:fast -Z7 -I ..\SDL2-2.0.3\include -link ..\SDL2-2.0.3\lib\x64\SDL2main.lib ..\SDL2-2.0.3\lib\x64\SDL2.lib -SUBSYSTEM:WINDOWS
popd
