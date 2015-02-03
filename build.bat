@echo off

if not exist "build" mkdir build
pushd build
if not exist "data" mkdir data
cp ../data/* data
echo WAITING FOR PDB > game.dll.lock
del game*.pdb
cl -nologo  -Z7 -Gm- -GR- -EHa- -EHsc -Od -Oi -FC -fp:fast ..\game.cpp -Fmgame.map -LD -link -incremental:no -PDB:game_%random%.pdb -EXPORT:GameUpdateAndRender
del game.dll.lock
cl -nologo -Z7 -Gm- -GR- -EHa- -EHsc ..\sdl_platform.cpp -MD -Od -Oi -FC -fp:fast -I ..\vendor\SDL2-2.0.3\include -link ..\vendor\SDL2-2.0.3\lib\x64\SDL2main.lib ..\vendor\SDL2-2.0.3\lib\x64\SDL2.lib -SUBSYSTEM:WINDOWS
popd
