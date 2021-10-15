@ECHO OFF
ECHO ---	
ECHO 	Author: Grand Roman Joldes
ECHO 	E-mail: grand.joldes@uwa.edu.au
ECHO ---
ECHO Meshing ...

ECHO 	Surface meshing ...

IF EXIST "..\AAA_Wall_External.stl" (
	IF NOT EXIST extSurf.stl (
		ECHO Merge "..\AAA_Wall_External.stl"; > Surf.geo
		%GMSH_PATH% Mesh_surf.geo -2 -o extSurf.stl -bin 2>error.txt
		IF NOT EXIST extSurf.stl (
			%GMSH_PATH% Mesh_surf1.geo -2 -o extSurf.stl -bin 2>error.txt
			IF NOT EXIST extSurf.stl (
				ECHO 	Error meshing the external AAA surface!
				EXIT /B 1
			)
		)
	)
)

IF EXIST "..\AAA_Wall_Internal.stl" (
	IF NOT EXIST intSurf.stl (
		ECHO Merge "..\AAA_Wall_Internal.stl"; > Surf.geo
		%GMSH_PATH% Mesh_surf.geo -2 -o intSurf.stl -bin 2>error.txt
		IF NOT EXIST intSurf.stl (
			%GMSH_PATH% Mesh_surf1.geo -2 -o intSurf.stl -bin 2>error.txt
			IF NOT EXIST intSurf.stl (
				ECHO 	Error meshing the internal AAA surface!
				EXIT /B 1
			)
		)
	)
)


IF EXIST "..\AAA_ILT_Internal.stl" (
	IF NOT EXIST ILTSurf.stl (
		ECHO Merge "..\AAA_ILT_Internal.stl"; > Surf.geo
		%GMSH_PATH% Mesh_surf.geo -2 -o ILTSurf.stl -bin 2>error.txt
		IF NOT EXIST ILTSurf.stl (
			IF NOT EXIST "simplification.ply" (
				ECHO Re-meshing ..\AAA_ILT_Internal.stl
				%ACVD_PATH% ..\AAA_ILT_Internal.stl 10000 0.2 -s 20 -d 0
				IF NOT EXIST "simplification.ply" (
					ECHO Error meshing ..\AAA_ILT_Internal.stl using ACVDQ!
					EXIT /B 1
				)
			)
			IF NOT EXIST "AAA_ILT_Internal.stl" (
				%PARAVIEW_PATH% %SCRIPTS_PATH%\AAA_convert_PLY_to_STL.py
				IF NOT EXIST "AAA_ILT_Internal.stl" (
					ECHO Error converting simplification.ply using ParaView!
					EXIT /B 1
				)
				ECHO Merge "AAA_ILT_Internal.stl"; > Surf.geo
				%GMSH_PATH% Mesh_surf.geo -2 -o ILTSurf.stl -bin 2>error.txt
				IF NOT EXIST ILTSurf.stl (
					ECHO 	Error meshing the ILT surface!
					EXIT /B 1
				)
			)	
		)	
	)	
)

CALL %SCRIPTS_PATH%\AAA_mesh_volumes

EXIT /B %errorlevel%
