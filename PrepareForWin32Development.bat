@echo off

:: NOTE: Using :: for comments is problematic inside (...) blocks, so I'll be using rem
:: See https://stackoverflow.com/questions/12407800/which-comment-style-should-i-use-in-batch-files

setlocal
set THISFOLDER=%~dp0
pushd %THISFOLDER%

if not exist "./temp" (
	md temp
)

rem My development setup normally has all my personal project repositories in <ROOTFOLDER>\crazygaze\<REPO>,
rem so if there is a czmut folder at the same level as czmicroaudio, then it's probably the thing to use,
rem and we create a link.
rem If there isn't one, the we clone the repo
if not exist "./temp/czmut" (
	if exist "./../czmut" (
		rem Link to the already existing repo
		mklink /J "./temp/czmut" "../czmut"
	) else (
		echo ***Cloning czmut***
		echo on
		git clone https://github.com/ruifig/czmut "./temp/czmut"
		echo off
	)
)

if not exist "./build" (
	md build
	cd build
	echo ***Generating Visual Studio project***
	echo on
	cmake -G "Visual Studio 17 2022" -A Win32 ..
	echo off
	cd ..
	if %errorlevel% neq 0 goto error
)

:: Everything ok
endlocal
goto end

:: Error
:error
endlocal
echo ERROR!!
exit /b %errorlevel%

:end
popd
pause


