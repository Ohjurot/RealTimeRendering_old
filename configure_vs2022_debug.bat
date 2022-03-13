@echo off
REM Aquire dependencys using conan
conan install . --build missing -s build_type=Debug
REM Run premake
"./vendor/premake/premake5" vs2022
