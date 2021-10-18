@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---
ECHO 	Extracting wall surfaces ...

REM Extracting interior and exterior surfaces ...
%AAA_EXTRACT_SURFACES_PATH% "..\\1_Segmentation_CT\\CT_wall.vtp" blood0.vtp Exterior0.vtp
IF NOT EXIST "Exterior0.vtp" (
	ECHO Error running AAA_ExtractSurfaces.exe!
	EXIT /B 1
)

IF NOT EXIST "blood0.vtp" (
	ECHO Error running AAA_ExtractSurfaces.exe!
	EXIT /B 1
)


REM Converting to .stl ...
%PARAVIEW_PATH% %SCRIPTS_PATH%\AAA_convert_STL.py
IF NOT EXIST "blood.stl" (
	ECHO Error converting blood0.vtp using ParaView!
	EXIT /B 1
)

REM Surface meshing ...
%ACVD_PATH% blood.stl 10000 0.2 -s 10 -d 0
IF NOT EXIST "simplification.ply" (
	ECHO Error meshing blood.stl using ACVDQ!
	EXIT /B 1
)

move /Y simplification.ply simplification1.ply 

%ACVD_PATH% Exterior.stl 10000 0.2 -s 10 -d 0
IF NOT EXIST "simplification.ply" (
	ECHO Error meshing Exterior.stl using ACVDQ!
	EXIT /B 1
)

REM Smoothing and converting to .vtp ...
%PARAVIEW_PATH% %SCRIPTS_PATH%\AAA_convert_VTP.py
IF NOT EXIST "Exterior.vtp" (
	ECHO Error converting simplification.ply using ParaView!
	EXIT /B 1
)


ECHO 	Done!
EXIT /B 0