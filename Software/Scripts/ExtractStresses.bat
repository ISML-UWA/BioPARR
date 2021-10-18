@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---

REM Extract stresses from Abaqus output database

IF NOT EXIST stress.vtk (
	Abaqus cae -noGUI %SCRIPTS_PATH%\ExtractResults.py
	IF ERRORLEVEL 1 (
		EXIT /B %errorlevel%
	)
)

EXIT /B %errorlevel%
