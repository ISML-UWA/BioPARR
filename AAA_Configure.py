#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
from pathlib import Path

print("Configuring paths to all tools ...")

os.environ["SLICER_PATH"] = str(Path(
    "/home/gerry/Applications/Slicer-SuperBuild-Debug/Slicer-build/Slicer"))
os.environ["PARAVIEW_PATH"] = str(Path(
    "/home/gerry/Applications/ParaView/paraview"))

os.environ["AAA_CREATE_WALL_PATH"] = str(Path(
    "./Software/SlicerModules/AAA_CreateWall").resolve())

os.environ["AAA_EXTRACT_SURFACES_PATH"] = str(Path(
    "./Software/SlicerModules/AAA_ExtractSurfaces").resolve())

os.environ["AAA_GENERATE_ABAQUS_FILE_PATH"] = str(Path(
    "./Software/SlicerModules/AAA_GenerateAbaqusFile").resolve())

os.environ["AAA_COMPUTE_RPI_PATH"] = str(Path(
    "./Software/SlicerModules/AAA_ComputeRPI").resolve())

os.environ["AAA_AVERAGE_STRESS_PATH"] = str(Path(
    "./Software/SlicerModules/AAA_AverageStress").resolve())


os.environ["ACVD_PATH"] = str(Path(
    "./Software/cvd1.1/ACVDQ").resolve())

os.environ["GMSH_PATH"] = str(Path(
    "./Software/gmsh/gmsh").resolve())

os.environ["SCRIPTS_PATH"] = str(Path(
    "./Software/Scripts").resolve())

print("Done!")
