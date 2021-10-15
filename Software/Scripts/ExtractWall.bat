@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---
ECHO 	Extracting AAA wall ...

%SLICER_PATH% --python-script %SCRIPTS_PATH%\ExtractWall.py %cd%

IF NOT EXIST "CT_wall.vtp" (
	EXIT /B 1
)

ECHO 	Done!