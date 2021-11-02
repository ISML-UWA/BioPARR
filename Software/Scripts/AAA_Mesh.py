#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import subprocess
from pathlib import Path
import runpy

print("Surface meshing ...")

if Path("../AAA_Wall_External.stl").exists():
    if not Path("extSurf.stl").exists():
        f = open("Surf.geo", "w")
        f.write("Merge \"%s\";\n" % Path("../AAA_Wall_External.stl"))
        f.close()
        subprocess.call([os.environ["GMSH_PATH"],
                         Path(os.environ["SCRIPTS_PATH"], "Mesh_surf.geo"),
                         "-2", "-o", "extSurf.stl", "-bin", "2>error.txt"])
        if not Path("extSurf.stl").exists():
            raise Exception("Error meshing the external AAA surface!")

if Path("../AAA_Wall_Internal.stl").exists():
    if not Path("intSurf.stl").exists():
        f = open("Surf.geo", "w")
        f.write("Merge \"%s\";\n" % Path("../AAA_ILT_External.stl"))
        f.close()
        subprocess.call([os.environ["GMSH_PATH"],
                         Path(os.environ["SCRIPTS_PATH"], "Mesh_surf.geo"),
                         "-2", "-o", "intSurf.stl", "-bin", "2>error.txt"])
        if not Path("intSurf.stl").exists():
            raise Exception("Error meshing the internal AAA surface!")

if Path("../AAA_ILT_Internal.stl").exists():
    if not Path("ILTSurf.stl").exists():
        subprocess.call([os.environ["GMSH_PATH"],
                         Path(os.environ["SCRIPTS_PATH"], "Mesh_surf.geo"),
                         "-2", "-o", "ILTSurf.stl", "-bin", "2>error.txt"])
        if not Path("ILTSurf.stl").exists():
            if not Path("simplification.ply").exists():
                print("Re-meshing ../AAA_ILT_Internal.stl")
                subprocess.call([os.environ["ACVD_PATH"],
                                 Path("../AAA_ILT_Internal.stl"),
                                 "10000 0.2 -s 20 -d 0"])
                if not Path("simplification.ply").exists():
                    raise Exception(
                        "Error meshing ../AAA_ILT_Internal.stl using ACVDQ!")
        if not Path("AAA_ILT_Internal.stl").exists():
            subprocess.call([os.environ["PARAVIEW_PATH"],
                             Path(os.environ["SCRIPTS_PATH"],
                                  "AAA_convert_PLY_to_STL.py"),
                             "10000 0.2 -s 20 -d 0"])
            if not Path("AAA_ILT_Internal.stl").exists():
                raise Exception(
                    "Error converting simplification.ply using ParaView!")

            f = open("Surf.geo", "w")
            f.write("Merge \"%s\";\n" % Path("../AAA_ILT_External.stl"))
            f.close()
            subprocess.call([os.environ["GMSH_PATH"],
                             Path(os.environ["SCRIPTS_PATH"], "Mesh_surf.geo"),
                             "-2", "-o", "ILTSurf.stl", "-bin", "2>error.txt"])
            if not Path("ILTSURF.stl").exists():
                raise Exception("Error meshing the ILT surface!")

runpy.run_path(Path(os.environ["SCRIPTS_PATH"],
                    "AAA_mesh_volumes.py"))
