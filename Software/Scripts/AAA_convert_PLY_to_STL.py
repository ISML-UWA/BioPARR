# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

#### import the simple module from the paraview
from paraview.simple import *

# create a new 'XML PolyData Reader'
ply_file = PLYReader(FileNames=['simplification.ply'])

# save data
SaveData('AAA_ILT_Internal.stl', proxy=ply_file)
