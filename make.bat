@ECHO OFF
ECHO.

SET SOLUTION=libnetworkio.sln

IF "%~1"=="" GOTO COMMANDS
IF "%~1"=="all" GOTO MAKE_ALL
IF "%~1"=="release" GOTO MAKE_RELEASE
IF "%~1"=="debug" GOTO MAKE_DEBUG
IF "%~1"=="clean" GOTO MAKE_CLEAN
IF "%~1"=="test_release" GOTO MAKE_TEST_RELEASE
IF "%~1"=="test_debug" GOTO MAKE_TEST_DEBUG
IF "%~1"=="update_modules" GOTO MAKE_UPDATE_MODULES

:COMMANDS
ECHO COMMANDS
GOTO DONE

:MAKE_ALL
make.bat release
GOTO DONE

:MAKE_RELEASE
mkdir build\release
cd build\release
cmake -A x64 -DCMAKE_BUILD_TYPE=Release ..\..
where /q msbuild
IF ERRORLEVEL 1 (
    ECHO No developer tools found. Is this the Developer Command Prompt?
) ELSE (
    msbuild %SOLUTION% /p:Configuration=Release
)
cd ..
cd ..
GOTO DONE

:MAKE_DEBUG
ECHO MAKE_DEBUG
mkdir build\debug
cd build\debug
cmake -A x64 -DCMAKE_BUILD_TYPE=Debug ..\..
where /q msbuild
IF ERRORLEVEL 1 (
    ECHO No developer tools found. Is this the Developer Command Prompt?
) ELSE (
    msbuild %SOLUTION% /p:Configuration=Debug
)
cd ..
cd ..
GOTO DONE

:MAKE_CLEAN
rmdir /s /Q build
GOTO DONE

:MAKE_TEST_RELEASE
cd build\release
ctest -V all
cd ..
cd ..
GOTO DONE

:MAKE_TEST_DEBUG
cd build\debug
ctest -V all
cd ..
cd ..
GOTO DONE

:MAKE_UPDATE_MODULES
git submodule init
git submodule sync
git submodule update --init --recursive
git submodule foreach --recursive git checkout master
git submodule foreach --recursive git pull origin master
GOTO DONE

:DONE