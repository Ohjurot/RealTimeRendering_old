@echo off
REM Aquire dependencys using conan
conan install . --build missing -s build_type=Release
REM Run premake
"./vendor/premake/premake5" vs2022
