@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---
ECHO 	Creating variable thickness volumes ...


SET THICKNESS_DIR="%cd%\\..\\3_Thickness"

IF EXIST "%cd%\\..\\4_Geometry\\blood.vtp" (
	%AAA_CREATE_WALL_PATH% "%cd%\\..\\4_Geometry\\Exterior.vtp" "%cd%\\..\\4_Geometry\\blood.vtp" %THICKNESS_DIR% "%cd%\\WallSurface.vtp" "%cd%\\ILTSurface.vtp" %cd% -n 20
) ELSE (
	%AAA_CREATE_WALL_PATH% "%cd%\\..\\4_Geometry\\Exterior.vtp" "%cd%\\..\\4_Geometry\\Exterior.vtp" %THICKNESS_DIR% "%cd%\\WallSurface.vtp" "%cd%\\ILTSurface.vtp" %cd% -n 20
)

IF %errorlevel% NEQ 0 (
	ECHO 	Error creating AAA wall!
	EXIT /B %errorlevel%
)

ECHO 	Done!

EXIT /B 0


