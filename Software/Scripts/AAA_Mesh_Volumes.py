#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import subprocess
from pathlib import Path

if not Path("Wall.vtk").exists():
    f = open("Surf.geo", "w")
    f.write("Merge \"extSurf.stl\";\n")
    f.write("Merge \"intSurf.stl\";\n")
    f.close()
    subprocess.call([os.environ["GMSH_PATH"],
                     Path(os.environ["SCRIPTS_PATH"], "Mesh_surf.geo"),
                     "-3", "-o", "Wall.vtk", "-bin", "2>error.txt"])

    if not Path("Wall.vtk").exists():
        raise("Error meshing the AAA wall!")

if Path("ILTSurf.stl").exists():
    if not Path("ILT.vtk").exists():
        f = open("Surf.geo", "w")
        f.write("Merge \"intSurf.stl\";\n")
        f.write("Merge \"ILTSurf.stl\";\n")
        f.close()
        subprocess.call([os.environ["GMSH_PATH"],
                         Path(os.environ["SCRIPTS_PATH"], "Mesh_surf.geo"),
                         "-3", "-o", "ILT.vtk", "-bin", "2>error.txt"])
        if not Path("ILT.vtk").exists():
            raise("Error meshing the ILT!")
