#   CONTENT
#   AUTHOR: Grand Roman Joldes
#   EMAIL: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
from pathlib import Path

print("Delete all results for this AAA case ...")

for dir in [Path("4_Geometry"),
            Path("5_Mesh_Const_Thickness"),
            Path("5_Mesh_Const_Thickness/Mesh"),
            Path("6_Mesh_Var_Thickness"),
            Path("6_Mesh_Var_Thickness/Mesh")]:
    print("Delete all results from %s ..." % dir)
    files = next(os.walk(dir))[2]
    for f in files:
        os.remove(Path(dir, f))

for dir in [Path("7_Abaqus_Const_Thickness"),
            Path("8_Abaqus_Var_Thickness")]:
    print("Delete all results from %s ..." % dir)
    files = next(os.walk(dir))[2]
    for f in files:
        os.remove(Path(dir, f))

    dirs = next(os.walk(dir))[1]
    for d in dirs:
        os.rmdir(d)

print("Done!")
