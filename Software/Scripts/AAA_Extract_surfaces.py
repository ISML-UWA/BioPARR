#   CONTENT
#   AUTHOR: Grand Roman Joldes
#   EMAIL: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import subprocess
from pathlib import Path

print("Extracting wall surfaces ...")

# Extracting interior and exterior surfaces ...
subprocess.call([os.environ["AAA_EXTRACT_SURFACES_PATH"],
                 Path("..", "1_Segmentation_CT", "CT_wall.vtp"),
                 "blood0.vtp", "Exterior0.vtp"])

if not Path("Exterior0.vtp").exists() or not Path("blood0.vtp").exists():
    raise Exception("Error running AAA_ExtractSurfaces.exe!")

# Converting to .stl ...
subprocess.call([os.environ["PARAVIEW_PATH"],
                 Path(os.environ["SCRIPTS_PATH"], "AAA_convert_STL.py")])

if not Path("blood.stl").exists():
    raise Exception("Error converting blood0.vtp using ParaView!")

# Surface meshing ...
subprocess.call([os.environ["ACVD_PATH"],
                 "blood.stl", "10000 0.2 -s 10 -d 0"])

if not Path("simplification.ply").exists():
    raise Exception("Error meshing blood.stl using ACVDQ!")

os.rename("simplification.ply", "simplification1.ply")

subprocess.call([os.environ["ACVD_PATH"],
                 "Exterior.stl", "10000 0.2 -s 10 -d 0"])

if not Path("simplification.ply").exists():
    raise Exception("Error meshing Exterior.stl using ACVDQ!")

# Smoothing and converting to .vtp ...
subprocess.call([os.environ["PARAVIEW_PATH"],
                 Path(os.environ["SCRIPTS_PATH"], "AAA_convert_VTP.py")])

if not Path("Exterior.vtp").exists():
    raise Exception("Error converting simplification.ply using ParaView!")

print("Done!")
