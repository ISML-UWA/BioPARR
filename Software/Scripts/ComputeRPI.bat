@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---
ECHO 	Computing RPI ...

%AAA_COMPUTE_RPI_PATH% %cd%\stress.vtk %ILT_SURFACE_FILE% %cd%\RPI_noRS.vtp

%AAA_AVERAGE_STRESS_PATH% %cd%\stress.vtk %cd%\average_stress.vtp

%AAA_COMPUTE_RPI_PATH% %cd%\average_stress.vtp %ILT_SURFACE_FILE% %cd%\RPI.vtp

IF NOT EXIST RPI.vtp (
	EXIT /B 1
)

ECHO 	Done!