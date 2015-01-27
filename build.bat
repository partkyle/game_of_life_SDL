@echo off

pushd build
echo WAITING FOR PDB > game.dll.lock
cl -nologo -Z7 ..\game.cpp -Fmgame.map -LD -link -incremental:no -PDB:game_%random%.pdb -EXPORT:GameUpdateAndRender
del game.dll.lock
cl -nologo -Z7 -Gm- -GR- -EHa- -EHsc ..\sdl_platform.cpp -MD -Od -Oi -FC -fp:fast -Z7 -I ..\vendor\SDL2-2.0.3\include -link ..\vendor\SDL2-2.0.3\lib\x64\SDL2main.lib ..\vendor\SDL2-2.0.3\lib\x64\SDL2.lib -SUBSYSTEM:WINDOWS
popd
