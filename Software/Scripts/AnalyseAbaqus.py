#   CONTENT
#   Author: Grand Roman Joldes
#   E-mail: grand.joldes@uwa.edu.au
#   PYTHON TRANSLATION
#   AUTHOR: Gerry Gralton
#   EMAIL:  gerry.gralton@uwa.edu.au

# Analysing a case using Abaqus

import os
import subprocess
import shutil
from pathlib import Path

if not Path("AAA.odb").exists():
    shutil.copy(Path(os.environ["SCRIPTS_PATH"], "abaqus_v6.env"),
                Path("."))
    subprocess.call(["Abaqus", "job=AAA", "interactive"])
