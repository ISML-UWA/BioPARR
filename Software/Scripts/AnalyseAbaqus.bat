@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---

REM Analysing a case using Abaqus

IF NOT EXIST %cd%\AAA.odb (
	copy /Y %SCRIPTS_PATH%\abaqus_v6.env %cd% >nul 2>&1
	Abaqus job=AAA interactive
	IF ERRORLEVEL 1 (
		EXIT /B %errorlevel%
	)
)

EXIT /B %errorlevel%
