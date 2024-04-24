@echo off

set TurnOffWarnings=
set CommonCompilerFlags=-Zi -nologo  -WX -W4 %TurnOffWarnings% 
@REM Add build macros
set CommonCompilerFlags= -D DEBUG_BUILD=1 %CommonCompilerFlags%
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib


IF NOT EXIST .build mkdir .build
pushd .build

set files=..\src\main.c

@REM 64-bit build command
cl %CommonCompilerFlags% %files% -link %CommonLinkerFlags%
popd
