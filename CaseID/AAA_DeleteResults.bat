@ECHO OFF
ECHO ---	
ECHO 	Author: Grand Roman Joldes
ECHO 	E-mail: grand.joldes@uwa.edu.au
ECHO ---
ECHO Delete all results for this AAA case ...

FOR %%D IN (4_Geometry, 5_Mesh_Const_Thickness, 5_Mesh_Const_Thickness\Mesh, 6_Mesh_Var_Thickness, 6_Mesh_Var_Thickness\Mesh) DO (
	ECHO 	Deleting all results from %%D ...
	del /f /q %%D\*.* >nul 2>&1
)

FOR %%D IN (7_Abaqus_Const_Thickness, 8_Abaqus_Var_Thickness) DO (
	ECHO 	Deleting all results from %%D ...
	del /s /f /q %%D\*.* >nul 2>&1
	for /f %%f in ('dir /ad /b %cd%\%%D') do rd /s /q %cd%\%%D\%%f
)

ECHO Done!

