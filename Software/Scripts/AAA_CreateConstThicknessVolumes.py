#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import subprocess
from pathlib import Path

THICKNESS_DIR = Path("./Thickness")

if Path("../4_Geometry/blood.vtp").exists():
    subprocess.call([os.environ["AAA_CREATE_WALL_PATH"],
                     Path("../4_Geometry/Exterior.vtp"),
                     Path("../4_Geometry/blood.vtp"),
                     THICKNESS_DIR,
                     Path("./WallSurface.vtp"),
                     Path("./ILTSurface.vtp"),
                     Path("."), "-n", "20"])
else:
    subprocess.call([os.environ["AAA_CREATE_WALL_PATH"],
                     Path("../4_Geometry/Exterior.vtp"),
                     Path("../4_Geometry/Exterior.vtp"),
                     THICKNESS_DIR,
                     Path("./WallSurface.vtp"),
                     Path("./ILTSurface.vtp"),
                     Path("."), "-n", "20"])

# TODO: Raise exception if above is unsuccessful

print("Done!")
