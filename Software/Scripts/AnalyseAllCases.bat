@ECHO OFF
REM ---	
REM 	Author: Grand Roman Joldes
REM 	E-mail: grand.joldes@uwa.edu.au
REM ---

SET AAA_ANALYSIS_CASES=
IF EXIST Wall.inp (
	IF EXIST ILT.inp (
		SET AAA_ANALYSIS_CASES=NoILT, ILTPressure, WallPressure 
	) ELSE (
		SET AAA_ANALYSIS_CASES=NoILT
	)
)

FOR %%S IN (%AAA_ANALYSIS_CASES%) DO (
	ECHO 		Creating directory %%S ...
 	md %%S >nul 2>&1
	copy /Y ILT.inp %%S >nul 2>&1
	copy /Y Wall.inp %%S >nul 2>&1
)

FOR %%S IN (%AAA_ANALYSIS_CASES%) DO (
	ECHO 		Abaqus analysis in %%S ...
	IF NOT EXIST %%S\stress.vtk (
		copy /Y %SCRIPTS_PATH%\Abaqus\%%S\AAA.inp %%S >nul 2>&1
		cd %%S
		START "Abaqus analysis ..."/W %SCRIPTS_PATH%\AnalyseAbaqus ^& exit
		IF ERRORLEVEL 1 (
			cd ..
			ECHO Error running Abaqus. Check script AnalyseAbaqus.bat!
			EXIT /B 1
		)
		START "Extract stresses ..."/W %SCRIPTS_PATH%\ExtractStresses ^& exit
		IF ERRORLEVEL 1 (
			cd ..
			ECHO Error running Abaqus. Check script ExtractStresses.bat!
			EXIT /B 1
		)
		cd ..
		ECHO 		Done!
	) ELSE (
		ECHO Skipped! - %%S\stress.vtk already exists
	)
	IF %AAA_REMOVE_INTERMEDIATE_RESULTS% NEQ 0 (
		cd %%S
		move /Y AAA.odb results.odb >nul 2>&1
		move /Y AAA.inp results.inp >nul 2>&1
		del /Q AAA.* abaqus*.* >nul 2>&1
		move /Y results.odb AAA.odb >nul 2>&1
		move /Y results.inp AAA.inp >nul 2>&1
		cd ..
	)
)

SET ILT_SURFACE_FILE="%cd%\ILTSurface.vtp"
IF NOT EXIST %ILT_SURFACE_FILE% (
	ECHO RPI computation skipped! - ILTSurface.vtp does not exist
) ELSE (  
	FOR %%S IN (%AAA_ANALYSIS_CASES%) DO (
		ECHO 		Computing RPI in %%S ...
		IF NOT EXIST %%S\RPI.vtp (
			cd %%S
			START "Computing RPI ..."/W %SCRIPTS_PATH%\ComputeRPI ^& exit
			IF ERRORLEVEL 1 (
				cd ..
				ECHO Error creating %%S\RPI.vtp!
			) ELSE (
				cd ..
				ECHO 		Done!
			)
		) ELSE (
			ECHO Skipped! - %%S\RPI.vtp already exists
		)
	)
)

EXIT /B %errorlevel%
