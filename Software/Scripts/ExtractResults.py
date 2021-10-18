# Author: Grand Roman Joldes
# E-mail: grand.joldes@uwa.edu.au

from odbAccess import *
from abaqusConstants import *
import visualization

# create odb object from odb file
odbName='AAA.odb'
outputDatabase=session.openOdb(name=odbName)
session.viewports['Viewport: 1'].setValues(displayedObject=outputDatabase)


# get access to the nodal displacement data
frame = outputDatabase.steps[ 'Pressure' ].frames[-1]      

# get access to the part instance -- thru which u can access the undeformed nodal position coordinates
my_part_instance = outputDatabase.rootAssembly.instances['WALL1']

allNodes = my_part_instance.nodeSets['ALLNODES']
numNodesTotal = len(allNodes.nodes)
  
if 1:
	# Write results to vtk file
	
	outFile = open( 'stress.vtk' , 'w' )

	# write vtk header

	outFile.write( '# vtk DataFile Version 3.0' )
	outFile.write( '\nvtk output' )
	outFile.write( '\nASCII' )
	outFile.write( '\nDATASET UNSTRUCTURED_GRID' )

	# write points

	outFile.write( '\n\nPOINTS ' + str( numNodesTotal ) + ' double' )

	for i in range( numNodesTotal ):

	    curNode =  my_part_instance.nodes[i]

	    NodePos = curNode.coordinates

	    outFile.write( '\n' )

	    for j in range( 3 ):

	        outFile.write( str( NodePos[j] ) + ' ' )

	# write cells

	numElementsTotal = len( my_part_instance.elements )

	outFile.write( '\nCELLS ' + str( numElementsTotal ) + ' ' + str( numElementsTotal * 5 ) )

	for i in range( numElementsTotal ):

	    connectivity =  my_part_instance.elements[i].connectivity

	    outFile.write('\n4 '+ str( connectivity[0]-1 ) + ' ' + str( connectivity[1]-1 ) + ' '+ str( connectivity[2]-1 ) + ' '+ str( connectivity[3]-1 ) + ' ' )

	# write cell types

	outFile.write( '\nCELL_TYPES ' + str( numElementsTotal ) )

	for i in range( numElementsTotal ):

	    outFile.write( '\n10' )

	# write cell data

	outFile.write( '\nCELL_DATA ' + str( numElementsTotal ) )

	# write point data

	outFile.write( '\nPOINT_DATA ' + str( numNodesTotal ) )
	
	# write scalars
		
	# Extrapolate stresses at nodes
	session.odbData[odbName].setValues(activeFrames=(('Pressure', (-1, )), ))
	xyData = session.xyDataListFromField(odb=outputDatabase, outputPosition=NODAL, variable=(('S', 
	    INTEGRATION_POINT, ((INVARIANT, 'Mises'), (INVARIANT, 'Max. Principal'), )), ), 
	    nodeSets=('WALL1.ALLNODES', ))
	
	outFile.write( '\nSCALARS S:Mises double' )
	outFile.write( '\nLOOKUP_TABLE S:Mises  ' )
	for i in range( numNodesTotal ):
		outFile.write( '\n' +  str( xyData[i][0][1]))
		
	outFile.write( '\nSCALARS S:MaxPrincipal double' )
	outFile.write( '\nLOOKUP_TABLE S:MaxPrincipal  ' )
	for i in range( numNodesTotal ):
		outFile.write( '\n' +  str( xyData[numNodesTotal+i][0][1]))
	
	
	outFile.close()

	outputDatabase.close()
