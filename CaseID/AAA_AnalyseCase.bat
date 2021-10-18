@ECHO OFF
ECHO ---	
ECHO 	Author: Grand Roman Joldes
ECHO 	E-mail: grand.joldes@uwa.edu.au
ECHO ---
ECHO Analysing AAA case ...

SET AAA_REMOVE_INTERMEDIATE_RESULTS=1
SET AAA_ANALYSE_VAR_THICKNESS=1
SET AAA_ANALYSE_CONST_THICKNESS=1
SET AAA_USE_QUADRATIC_ELEMENTS=1
SET AAA_USE_HYBRID_ELEMENTS=1

SET CASE_FOLDER=%cd%

REM Configuring paths
cd ..
CALL AAA_Configure
cd %CASE_FOLDER%

IF NOT EXIST %cd%\1_Segmentation_CT (
	ECHO 	Creating folder structure ...
	xcopy %SCRIPTS_PATH%\CaseTemplate %cd% /q /e /y >nul 2>&1
	ECHO 	Done!
)

ECHO 	Creating AAA wall ...
IF NOT EXIST %cd%\1_Segmentation_CT\CT_wall.vtp (
	IF NOT EXIST %cd%\1_Segmentation_CT\CT_cropped.nrrd (
		ECHO Error: Please crop the CT volume and save it as 1_Segmentation_CT\CT_cropped.nrrd!
		EXIT /B 1
	)
	IF NOT EXIST %cd%\1_Segmentation_CT\CT_AAA_label.nrrd (
		ECHO Error: Please segment the AAA from CT and save label as 1_Segmentation_CT\CT_AAA_label.nrrd!
		EXIT /B 1
	)
	IF NOT EXIST %cd%\1_Segmentation_CT\CT_blood_label.nrrd (
		ECHO Warning: No blood label found in .\1_Segmentation_CT\CT_blood_label.nrrd!
		ECHO 	ILT volume will not be created!
		CHOICE /C YN /T 15 /D N /M "Do you want to continue"
		IF ERRORLEVEL == 2 EXIT /B 1
	)
	cd 1_Segmentation_CT
	CALL %SCRIPTS_PATH%\ExtractWall
	IF ERRORLEVEL 1 (
		cd ..
		ECHO Error extracting AAA Wall. Check script ExtractWall.bat!
		EXIT /B 1
	)
	cd ..
) ELSE (
	ECHO Skipped! - 1_Segmentation_CT\CT_wall.vtp already exists
)

IF %AAA_REMOVE_INTERMEDIATE_RESULTS% NEQ 0 (
	del /Q 1_Segmentation_CT\CT_Wall_label.nrrd >nul 2>&1
)

ECHO 	Registering MRI to CT ...
IF NOT EXIST %cd%\2_Segmentation_MRI\MRI_aligned_to_CT.nrrd (
	IF NOT EXIST %cd%\2_Segmentation_MRI\MRI_cropped.nrrd (
		ECHO Warning: Please crop the MRI volume and save it as 2_Segmentation_MRI\MRI_cropped.nrrd!
		ECHO 	Registration of MRI to CT skipped!
		GOTO FINISH_REGISTRATION
	)
	IF NOT EXIST %cd%\2_Segmentation_MRI\MRI_AAA_label.nrrd (
		ECHO Warning: Please segment the AAA from MRI and save label as 2_Segmentation_MRI\MRI_AAA_label.nrrd!
		ECHO 	Registration of MRI to CT skipped!
		GOTO FINISH_REGISTRATION
	)
	cd 2_Segmentation_MRI
	CALL %SCRIPTS_PATH%\RegisterMRI2CT
	IF ERRORLEVEL 1 (
		cd ..
		ECHO Error registering MRI to CT. Check script RegisterMRI2CT.bat!
		EXIT /B 1
	)
	cd ..
) ELSE (
	ECHO Skipped! - 2_Segmentation_MRI\MRI_aligned_to_CT.nrrd already exists
)
:FINISH_REGISTRATION 
IF %AAA_REMOVE_INTERMEDIATE_RESULTS% NEQ 0 (
	del /Q 2_Segmentation_MRI\registerMRI2CT.h5 2_Segmentation_MRI\registerMRI2CT_Inverse.h5 >nul 2>&1
)

REM Checking for thickness information
IF NOT EXIST %cd%\3_Thickness\M*.acsv (
	ECHO Warning: No thickness info in 3_Thickness\!
	ECHO 	Only constant thickness AAA will be created!
	CHOICE /C YN /T 15 /D N /M "Do you want to continue"
	IF ERRORLEVEL == 2 EXIT /B 1
	SET AAA_ANALYSE_VAR_THICKNESS=0
)

ECHO 	Extracting surfaces ...
IF NOT EXIST %cd%\4_Geometry\Exterior.vtp (
	cd 4_Geometry
	START "Extracting surfaces ..."/W %SCRIPTS_PATH%\AAA_Extract_surfaces ^& exit
	IF ERRORLEVEL 1 (
		cd ..
		ECHO Error extracting surfaces. Check script AAA_Extract_surfaces.bat!
		EXIT /B 1
	)
	cd ..
	ECHO 	Done!
) ELSE (
	ECHO Skipped! - .\4_Geometry\Exterior.vtp already exists
)
IF %AAA_REMOVE_INTERMEDIATE_RESULTS% NEQ 0 (
	del /Q 4_Geometry\*.stl 4_Geometry\*0.vtp 4_Geometry\*.ply >nul 2>&1
)

IF %AAA_ANALYSE_CONST_THICKNESS% NEQ 0 (
	ECHO 	Creating constant thickness Volumes ...
	IF NOT EXIST %cd%\5_Mesh_Const_Thickness\AAA_Wall_External.stl (
		cd 5_Mesh_Const_Thickness
		START "Creating constant thickness Volumes ..."/W %SCRIPTS_PATH%\AAA_CreateConstThicknessVolumes ^& exit
		IF ERRORLEVEL 1 (
			cd ..
			ECHO Error creating constant thickness volumes. Check script AAA_CreateConstThicknessVolumes.bat!
			EXIT /B 1
		)
		cd ..
		ECHO 	Done!
	) ELSE (
		ECHO Skipped! - 5_Mesh_Const_Thickness\AAA_Wall_External.stl already exists
	)
)

IF %AAA_ANALYSE_VAR_THICKNESS% NEQ 0 (
	ECHO 	Creating variable thickness Volumes ...
	IF NOT EXIST %cd%\6_Mesh_Var_Thickness\AAA_Wall_External.stl (
		cd 6_Mesh_Var_Thickness
		START "Creating variable thickness Volumes ..."/W %SCRIPTS_PATH%\AAA_CreateVarThicknessVolumes ^& exit
		IF ERRORLEVEL 1 (
			cd ..
			ECHO Error creating variable thickness volumes. Check script AAA_CreateVarThicknessVolumes.bat!
			EXIT /B 1
		)
		cd ..
		ECHO 	Done!
	) ELSE (
		ECHO Skipped! - 5_Mesh_Var_Thickness\AAA_Wall_External.stl already exists
	)
)

IF %AAA_REMOVE_INTERMEDIATE_RESULTS% NEQ 0 (
	FOR %%D IN (5_Mesh_Const_Thickness, 6_Mesh_Var_Thickness) DO (
		del /Q %%D\AAA.msh %%D\AAA_Merged.vtp %%D\AAA_Wall_Cap.stl %%D\AAA_ILT_Cap.stl >nul 2>&1
	)
)

SET AAA_MESH_DIRS=
IF %AAA_ANALYSE_CONST_THICKNESS% NEQ 0 (
	IF %AAA_ANALYSE_VAR_THICKNESS% NEQ 0 (
		SET AAA_MESH_DIRS=5_Mesh_Const_Thickness, 6_Mesh_Var_Thickness
	) ELSE (
		SET AAA_MESH_DIRS=5_Mesh_Const_Thickness
	)
) ELSE (
	IF %AAA_ANALYSE_VAR_THICKNESS% NEQ 0 (
		SET AAA_MESH_DIRS=6_Mesh_Var_Thickness
	)
)

SET AAA_ELEMENT_CONFIG=
IF %AAA_USE_QUADRATIC_ELEMENTS% NEQ 0 (
	SET AAA_ELEMENT_CONFIG=--quad
)
IF %AAA_USE_HYBRID_ELEMENTS% NEQ 0 (
	SET AAA_ELEMENT_CONFIG=%AAA_ELEMENT_CONFIG% --hybrid
)

echo %AAA_ELEMENT_CONFIG%

FOR %%D IN (%AAA_MESH_DIRS%) DO (
	ECHO 	Meshing Volumes in %%D ...
	IF NOT EXIST %cd%\%%D\Mesh\Wall.vtk (
		cd %%D\Mesh
		copy /Y %SCRIPTS_PATH%\Mesh_surf.geo >nul 2>&1
		copy /Y %SCRIPTS_PATH%\Mesh_wall.geo >nul 2>&1
		START "Meshing Volumes ..."/W %SCRIPTS_PATH%\AAA_Mesh ^& exit
		IF ERRORLEVEL 1 (
			cd ..\..
			ECHO Error meshing volumes. Check script AAA_Mesh.bat!
			EXIT /B 1
		)
		cd ..\..
		ECHO 	Done!
	) ELSE (
		ECHO Skipped! - %%D\Mesh\Wall.vtk already exists
	)
	del /Q %%D\Mesh\*.geo %%D\Mesh\*.msh %%D\Mesh\*.pos %%D\Mesh\*.ply >nul 2>&1
	IF %AAA_REMOVE_INTERMEDIATE_RESULTS% NEQ 0 (
		IF EXIST %cd%\%%D\Mesh\Wall.vtk (
			del /Q %%D\Mesh\*.stl >nul 2>&1
		)
	)
	ECHO 	Creating Abaqus input files in %%D\Mesh ...
	IF NOT EXIST %cd%\%%D\Mesh\Wall.inp (
		%AAA_GENERATE_ABAQUS_FILE_PATH% %cd%\%%D\Mesh\Wall.vtk %cd%\%%D\WallSurface.vtp -o %cd%\%%D\Mesh\Wall.inp %AAA_ELEMENT_CONFIG%
		IF ERRORLEVEL 1 (
			ECHO Error creating %%D\Mesh\Wall.inp!
			EXIT /B 1
		)
		IF EXIST %cd%\%%D\Mesh\ILT.vtk (
			%AAA_GENERATE_ABAQUS_FILE_PATH% %cd%\%%D\Mesh\ILT.vtk %cd%\%%D\ILTSurface.vtp -o %cd%\%%D\Mesh\ILT.inp %AAA_ELEMENT_CONFIG%
			IF ERRORLEVEL 1 (
				ECHO Error creating %%D\Mesh\ILT.inp!
				EXIT /B 1
			)
		)
		ECHO 	Done!
	) ELSE (
		ECHO Skipped! - %%D\Mesh\Wall.inp already exists
	)
			
)

REM Prepare parts for Abaqus analysis
IF %AAA_ANALYSE_CONST_THICKNESS% NEQ 0 (
	copy 5_Mesh_Const_Thickness\Mesh\Wall.inp 7_Abaqus_Const_Thickness >nul 2>&1
	copy 5_Mesh_Const_Thickness\Mesh\ILT.inp 7_Abaqus_Const_Thickness >nul 2>&1
	copy 5_Mesh_Const_Thickness\ILTSurface.vtp 7_Abaqus_Const_Thickness >nul 2>&1
)
IF %AAA_ANALYSE_VAR_THICKNESS% NEQ 0 (
	copy 6_Mesh_Var_Thickness\Mesh\Wall.inp 8_Abaqus_Var_Thickness >nul 2>&1
	copy 6_Mesh_Var_Thickness\Mesh\ILT.inp 8_Abaqus_Var_Thickness >nul 2>&1
	copy 6_Mesh_Var_Thickness\ILTSurface.vtp 8_Abaqus_Var_Thickness >nul 2>&1
)

SET AAA_ABAQUS_DIRS=
IF %AAA_ANALYSE_CONST_THICKNESS% NEQ 0 (
	IF %AAA_ANALYSE_VAR_THICKNESS% NEQ 0 (
		SET AAA_ABAQUS_DIRS=7_Abaqus_Const_Thickness, 8_Abaqus_Var_Thickness
	) ELSE (
		SET AAA_ABAQUS_DIRS=7_Abaqus_Const_Thickness
	)
) ELSE (
	IF %AAA_ANALYSE_VAR_THICKNESS% NEQ 0 (
		SET AAA_ABAQUS_DIRS=8_Abaqus_Var_Thickness
	)
)


FOR %%D IN (%AAA_ABAQUS_DIRS%) DO (
	ECHO 	Abaqus analysis in %%D ...
	cd %%D
	CALL %SCRIPTS_PATH%\AnalyseAllCases
	cd ..
	ECHO 	Done!
)

ECHO Done!

