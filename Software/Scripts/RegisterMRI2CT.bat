@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---
ECHO 	Registering MRI to CT images ...

REM Set to 1 to start debug mode
SET Debug=0	

IF EXIST %cd%\registerMRI2CT.h5 (
	del /Q %cd%\registerMRI2CT.h5
)
IF EXIST %cd%\MRI_aligned_to_CT.nrrd (
	del /Q %cd%\MRI_aligned_to_CT.nrrd
)

IF %Debug% EQU 1 (
	%SLICER_PATH% --python-script %SCRIPTS_PATH%\RegisterMRI2CT.py %cd% %Debug%
) ELSE (
	%SLICER_PATH% --no-main-window --python-script %SCRIPTS_PATH%\RegisterMRI2CT.py %cd% %Debug%
)

IF NOT EXIST "registerMRI2CT.h5" (
	ECHO Error registering MRI to CT label maps!
	EXIT /B 1
)

IF NOT EXIST "MRI_aligned_to_CT.nrrd" (
	ECHO Error registering MRI to CT!
	EXIT /B 1
)

ECHO 	Done!
