# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

#### import the simple module from the paraview
from paraview.simple import *

# create a new 'PLY Reader'
Exteriorply = PLYReader(FileName='simplification.ply')

# create a new 'Smooth'
smooth1 = Smooth(Input=Exteriorply)

# Properties modified on smooth1
smooth1.NumberofIterations = 100

# save data
SaveData('Exterior.vtp', proxy=smooth1, CompressorType='ZLib')

# create a new 'PLY Reader'
bloodply = PLYReader(FileName='simplification1.ply')

# create a new 'Smooth'
smooth2 = Smooth(Input=bloodply)

# Properties modified on smooth1
smooth2.NumberofIterations = 100

# save data
SaveData('blood.vtp', proxy=smooth2, CompressorType='ZLib')



