#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

import os
import subprocess
from pathlib import Path

subprocess.call([os.environ["AAA_COMPUTE_RPI_PATH"],
                 Path("stress.vtk"),
                 Path("ILTSurface.vtp"),
                 Path("RSI_noRS.vtp")])

subprocess.call([os.environ["AAA_AVERAGE_STRESS_PATH"],
                 Path("stress.vtk"),
                 Path("average_stress.vtp")])

subprocess.call([os.environ["AAA_COMPUTE_RPI_PATH"],
                 Path("average_stress.vtp"),
                 Path("ILTSurface.vtp"),
                 Path("RPI.vtp")])

if not Path("RPI.vtp").exists():
    raise Exception("Computing RPI failed.")

print("Done!")
