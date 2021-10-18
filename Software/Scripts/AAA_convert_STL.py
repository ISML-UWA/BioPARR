# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

#### import the simple module from the paraview
from paraview.simple import *

# create a new 'XML PolyData Reader'
bloodvtp = XMLPolyDataReader(FileName=['blood0.vtp'])

# create a new 'Triangulate'
Triangulate1 = Triangulate(Input=bloodvtp )

# save data
SaveData('blood.stl', proxy=Triangulate1, FileType='Ascii')

# create a new 'XML PolyData Reader'
exteriorvtp = XMLPolyDataReader(FileName=['Exterior0.vtp'])

# create a new 'Triangulate'
Triangulate2 = Triangulate(Input=exteriorvtp  )

# save data
SaveData('Exterior.stl', proxy=Triangulate2, FileType='Ascii')
