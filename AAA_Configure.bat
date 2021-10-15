@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---
ECHO 	Configuring paths to all tools ...

SET SLICER_PATH="C:\Program Files\Slicer 4.4.0\Slicer.exe"
SET PARAVIEW_PATH="C:\Program Files (x86)\ParaView 4.3.1\bin\pvpython.exe"

SET AAA_CREATE_WALL_PATH="%cd%\Software\SlicerModules\AAA_CreateWall.exe"
SET AAA_EXTRACT_SURFACES_PATH="%cd%\Software\SlicerModules\AAA_ExtractSurfaces.exe"
SET AAA_GENERATE_ABAQUS_FILE_PATH="%cd%\Software\SlicerModules\AAA_GenerateAbaqusFile.exe"
SET AAA_COMPUTE_RPI_PATH="%cd%\Software\SlicerModules\AAA_ComputeRPI.exe"
SET AAA_AVERAGE_STRESS_PATH="%cd%\Software\SlicerModules\AAA_AverageStress.exe"

SET ACVD_PATH="%cd%\Software\acvd1.1\ACVDQ.exe"
SET GMSH_PATH="%cd%\Software\gmsh\gmsh.exe"

SET SCRIPTS_PATH="%cd%\Software\Scripts"

ECHO 	Done!

