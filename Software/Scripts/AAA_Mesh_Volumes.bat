@ECHO OFF
ECHO ---	
ECHO 	Author: Grand Roman Joldes
ECHO 	E-mail: grand.joldes@uwa.edu.au
ECHO ---
ECHO Meshing volumes ...

IF NOT EXIST "Wall.vtk" (
	ECHO Merge "extSurf.stl"; > Surf.geo
	ECHO Merge "intSurf.stl"; >> Surf.geo
	%GMSH_PATH% Mesh_wall.geo -3 -o Wall.vtk -bin 2>error.txt
)

IF NOT EXIST "Wall.vtk" (
	ECHO 	Error meshing the AAA wall!
	EXIT /B 1
)

IF EXIST "ILTSurf.stl" (
	IF NOT EXIST "ILT.vtk" (
		ECHO Merge "intSurf.stl"; > Surf.geo
		ECHO Merge "ILTSurf.stl"; >> Surf.geo
		%GMSH_PATH% Mesh_wall.geo -3 -o ILT.vtk -bin 2>error.txt
	)

	IF NOT EXIST "ILT.vtk" (
		ECHO 	Error meshing the ILT!
		EXIT /B 1
	)
)

EXIT /B 0