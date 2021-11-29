#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import shutil
from pathlib import Path
import runpy
import subprocess

if Path("Wall.inp").exists():
    if Path("ILT.inp").exists():
        AAA_ANALYSIS_CASES = ("NoILT", "ILTPressure", "WallPressure")
    else:
        AAA_ANALYSIS_CASES = ("NoILT")
else:
    AAA_ANALYSIS_CASES = ()

for case in AAA_ANALYSIS_CASES:
    print("Creating directory %s ..." % case)
    os.mkdir(case)
    shutil.copy(Path("ILT.inp"), Path(case))
    shutil.copy(Path("Wall.inp"), Path(case))

for case in AAA_ANALYSIS_CASES:
    print("Abaqus analysis in %s ..." % case)
    if not Path(case, "stress.vtk").exists():
        shutil.copy(Path(os.environ["SCRIPTS_PATH"], "Abaqus",
                         case, "AAA.inp"), Path(case))
        os.chdir(case)
        print("Abaqus analysis ...")
        runpy.run_path(Path(os.environ["SCRIPTS_PATH"], "AnalyseAbaqus.py"))
        # TODO: Catch failure of AnalyseAbaqus.py
        print("Extracting stresses ...")
        runpy.run_path(Path(os.environ["SCRIPTS_PATH"], "ExtractResults.py"))
        subprocess.call(["Abaqus", "cae", "-noGUI",
                         Path(os.environ["SCRIPTS_PATH"], "ExtractResults.py")])
        # TODO: Catch failure of ExtractResults.py
        os.chdir("..")
        print("Done!")
    else:
        print("Skipped! - %s/stress.vtk already exists" % case)

# TODO: Remove intermediate results

if not Path("ILTSurface.vtp").exists():
    print("RPI computation skipped! - ILTSurface.vtp does not exist")
else:
    for case in AAA_ANALYSIS_CASES:
        print("Computing RPI in %s ..." % case)
        if not Path(case, "RPI.vtp").exists():
            os.chdir(case)
            print("Computing RPI ...")
            runpy.run_path(Path(os.environ["SCRIPTS_PATH"], "ComputeRPI.py"))
