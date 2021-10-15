// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkSmartPointer.h>

#include <vtkUnstructuredGridReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkGeometryFilter.h>
#include <vtkAppendFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkQuadraticTetra.h>
#include <vtkMath.h>
#include <vtkIdTypeArray.h>
#include <vtkPolyData.h>
#include <vtkKdTreePointLocator.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkQuadraticTetra.h>
#include <vtkCellArray.h>
#include <vtkMergePoints.h>
#include <vtkCellDataToPointData.h>


#include "AAA_GenerateAbaqusFileCLP.h"
#include "AAA_defines.h"

#define LENGTH_PER_CAP_SIZE 200

int main( int argc, char * argv[] )
{
	PARSE_ARGS;
	vtkDebugLeaks::SetExitError(true);

	// Check inputs
	if (meshFile.size() == 0)
	{
		std::cerr << "ERROR: no input mesh specified!" << std::endl;
		return EXIT_FAILURE;
	}

	if (inSurface.size() == 0)
	{
		std::cerr << "ERROR: no surface specified!" << std::endl;
		return EXIT_FAILURE;
	} 

	// Read input mesh
	vtkSmartPointer<vtkUnstructuredGridReader> pdReader = vtkSmartPointer<vtkUnstructuredGridReader>::New();
	pdReader->SetFileName(meshFile.c_str() );
	pdReader->Update();
	if( pdReader->GetErrorCode() != 0 )
    {
		std::cerr << "ERROR: Failed to read mesh from " << meshFile.c_str() << std::endl;
		return EXIT_FAILURE;
    }

	// Read surface
	vtkSmartPointer<vtkXMLPolyDataReader> pdReader1 = vtkSmartPointer<vtkXMLPolyDataReader>::New();
	pdReader1->SetFileName(inSurface.c_str() );
	pdReader1->Update();
	if( pdReader1->GetErrorCode() != 0 )
    {
		std::cerr << "ERROR: Failed to read surface from " << inSurface.c_str() << std::endl;
		return EXIT_FAILURE;
    }
	vtkPolyData *pSplitSurface = pdReader1->GetOutput();

	// check which volume we are handling (ILT/Wall)
	std::string partName;
	bool boHandlingILT = false;
	vtkIdType numCellsSurf = pSplitSurface->GetNumberOfCells();
	pSplitSurface->GetCellData()->SetActiveScalars(SURF_ID_NAME);
	for (vtkIdType i = 0; i < numCellsSurf; i++)
	{
		double *pSurfID = pSplitSurface->GetCellData()->GetScalars()->GetTuple(i);
		if ((*pSurfID) == INT_ILT_SURF_ID) 
		{
			boHandlingILT = true;
			break;
		}
	}
	if (boHandlingILT) partName="ILT";
	else partName="Wall";

	// Extract outside mesh surface
	vtkSmartPointer<vtkGeometryFilter> pdExtractSurf = vtkSmartPointer<vtkGeometryFilter>::New();
	pdExtractSurf->SetInputConnection(pdReader->GetOutputPort());
	pdExtractSurf->Update();

	// Merge surface with mesh
	vtkSmartPointer<vtkAppendFilter> pdAppendFilter = vtkSmartPointer<vtkAppendFilter>::New();
	pdAppendFilter->MergePointsOn();
	pdAppendFilter->AddInputConnection(pdReader->GetOutputPort());
	pdAppendFilter->AddInputConnection(pdExtractSurf->GetOutputPort());
	pdAppendFilter->Update();

	vtkUnstructuredGrid *mesh = pdAppendFilter->GetOutput();
	vtkIdType numNodes = mesh->GetNumberOfPoints();
	vtkPoints *pPoints = mesh->GetPoints();
	vtkIdType totalNumNodes = numNodes;

	// find number of tetras
	vtkIdType numTetras = 0;
	vtkIdType numTrias = 0;
	vtkIdType numElements = mesh->GetNumberOfCells();
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (mesh->GetCellType(i) == VTK_TETRA) numTetras++;
		else if (mesh->GetCellType(i) == VTK_TRIANGLE) numTrias++;
	}
	if (numTetras+numTrias != numElements)
	{
		std::cout << "WARNING: Unexpected element type in the input file!" << std::endl;
	}
	
	// create 10 noded elements
	vtkSmartPointer<vtkCellArray> cellsQuadratic = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkUnsignedCharArray> cellsIdsQuadratic = vtkSmartPointer<vtkUnsignedCharArray>::New();
	vtkSmartPointer<vtkIdTypeArray> cellLocations = vtkSmartPointer<vtkIdTypeArray>::New();

	cellsIdsQuadratic->SetNumberOfValues(numElements);
	cellLocations->SetNumberOfValues(numElements);
	cellsQuadratic->Allocate(numElements*11);
	vtkIdType location = 0;
	if (boQuadratic)
	{
		// merge middle nodes when inserting
		vtkSmartPointer<vtkMergePoints> mergePoints = vtkSmartPointer<vtkMergePoints>::New();
		mergePoints->SetDataSet(mesh);
		//mergePoints->SetDivisions(100,100,100);
		mergePoints->AutomaticOn();
		mergePoints->SetNumberOfPointsPerBucket(10);
		mergePoints->InitPointInsertion(mesh->GetPoints(), mesh->GetBounds());
		mergePoints->SetTolerance(0.0000000001);

		// insert the existing points
		vtkIdType n = mesh->GetNumberOfPoints();
		for (vtkIdType i = 0; i < n; i++)
		{
			double p[3];
			pPoints->GetPoint(i, p);
			mergePoints->InsertNextPoint(p);
		}
		
		// create middle points
		vtkIdType nodeIdx;
		for (vtkIdType i = 0; i < numElements; i++)
		{
			if (mesh->GetCellType(i) == VTK_TETRA)
			{
				// change cell types
				vtkIdType pQTetraIds[10];
				// create intermediate points
				vtkIdType nodeId = mesh->GetCell(i)->GetPointId(0);
				pQTetraIds[0] = nodeId;
				double n1[3];
				pPoints->GetPoint(nodeId, n1);
				nodeId = mesh->GetCell(i)->GetPointId(1);
				pQTetraIds[1] = nodeId;
				double n2[3];
				pPoints->GetPoint(nodeId, n2);
				nodeId = mesh->GetCell(i)->GetPointId(2);
				pQTetraIds[2] = nodeId;
				double n3[3];
				pPoints->GetPoint(nodeId, n3);
				nodeId = mesh->GetCell(i)->GetPointId(3);
				pQTetraIds[3] = nodeId;
				double n4[3];
				pPoints->GetPoint(nodeId, n4);
				double n[3];
				// generate center nodes
				// 5: 1-2
				vtkMath::Add(n1, n2, n);
				vtkMath::MultiplyScalar(n, 0.5);
				mergePoints->InsertUniquePoint(n, nodeIdx);
				pQTetraIds[4] = nodeIdx;
				// 6: 2-3
				vtkMath::Add(n3, n2, n);
				vtkMath::MultiplyScalar(n, 0.5);
				mergePoints->InsertUniquePoint(n, nodeIdx);
				pQTetraIds[5] = nodeIdx;
				// 7: 1-3
				vtkMath::Add(n3, n1, n);
				vtkMath::MultiplyScalar(n, 0.5);
				mergePoints->InsertUniquePoint(n, nodeIdx);
				pQTetraIds[6] = nodeIdx;
				// 8: 1-4
				vtkMath::Add(n4, n1, n);
				vtkMath::MultiplyScalar(n, 0.5);
				mergePoints->InsertUniquePoint(n, nodeIdx);
				pQTetraIds[7] = nodeIdx;
				// 9: 2-4
				vtkMath::Add(n4, n2, n);
				vtkMath::MultiplyScalar(n, 0.5);
				mergePoints->InsertUniquePoint(n, nodeIdx);
				pQTetraIds[8] = nodeIdx;
				// 10: 3-4
				vtkMath::Add(n4, n3, n);
				vtkMath::MultiplyScalar(n, 0.5);
				mergePoints->InsertUniquePoint(n, nodeIdx);
				pQTetraIds[9] = nodeIdx;
				cellsIdsQuadratic->SetValue(i, VTK_QUADRATIC_TETRA);
				cellsQuadratic->InsertNextCell(10, pQTetraIds);
				cellLocations->SetValue(i, location);
				location += 11;

			}
			else
			{
				cellsQuadratic->InsertNextCell(mesh->GetCell(i));
				cellsIdsQuadratic->SetValue(i, mesh->GetCellType(i));
				cellLocations->SetValue(i, location);
				location += mesh->GetCell(i)->GetNumberOfPoints()+1;
			}

		}
		totalNumNodes = mesh->GetNumberOfPoints();
		mesh->SetCells(cellsIdsQuadratic, cellLocations, cellsQuadratic);
	}

	// create output file
	FILE *fid_out = fopen(outputFile.c_str(), "w");
	if (!fid_out)
	{
		std::cerr << "Failed to create file " << outputFile.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	// write header
	fprintf(fid_out, "*Heading\n"
		"** Generated by Slicer module AAA_GenerateAbaqusFile\n");
	// Create a part
	fprintf(fid_out, "*Part, name=%s\n", partName.c_str());
	// Write nodes
	fprintf(fid_out, "*NODE\n");
	for (vtkIdType i = 0; i < numNodes; i++)
	{
		double pcoord[3];
		pPoints->GetPoint(i, pcoord);
		fprintf(fid_out, "%d, %.10g, %.10g, %.10g\n", i+1, pcoord[0], pcoord[1], pcoord[2]);
	}

	// create middle nodes for bilinear tetra elements
	if (boQuadratic)
	{
		for (vtkIdType i = numNodes; i < totalNumNodes; i++)
		{
			double pcoord[3];
			pPoints->GetPoint(i, pcoord);
			fprintf(fid_out, "%d, %.10g, %.10g, %.10g\n", i+1, pcoord[0], pcoord[1], pcoord[2]);
		}	
	}

	// Write tetras
	fprintf(fid_out, "*Element, type=C3D%d%s\n", boQuadratic?10:4, boHybrid?"H":"");
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (mesh->GetCellType(i) == VTK_TETRA)
		{	
			vtkIdType n1 = mesh->GetCell(i)->GetPointId(0);
			vtkIdType n2 = mesh->GetCell(i)->GetPointId(1);
			vtkIdType n3 = mesh->GetCell(i)->GetPointId(2);
			vtkIdType n4 = mesh->GetCell(i)->GetPointId(3);
			fprintf(fid_out, "%d, %d, %d, %d, %d\n", i+1, n1+1, n2+1, n3+1, n4+1);
		}
		else if (boQuadratic && (mesh->GetCellType(i) == VTK_QUADRATIC_TETRA))
		{
			vtkIdType n1 = mesh->GetCell(i)->GetPointId(0);
			vtkIdType n2 = mesh->GetCell(i)->GetPointId(1);
			vtkIdType n3 = mesh->GetCell(i)->GetPointId(2);
			vtkIdType n4 = mesh->GetCell(i)->GetPointId(3);
			vtkIdType n5 = mesh->GetCell(i)->GetPointId(4);
			vtkIdType n6 = mesh->GetCell(i)->GetPointId(5);
			vtkIdType n7 = mesh->GetCell(i)->GetPointId(6);
			vtkIdType n8 = mesh->GetCell(i)->GetPointId(7);
			vtkIdType n9 = mesh->GetCell(i)->GetPointId(8);
			vtkIdType n10 = mesh->GetCell(i)->GetPointId(9);
			fprintf(fid_out, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", i+1, n1+1, n2+1, n3+1, n4+1, 
				n5+1, n6+1, n7+1, n8+1, n9+1, n10+1);
		}
	}
	// define an element set with all elements
	fprintf(fid_out, "*Elset, elset=AllElements, generate\n"
		" 1, %d, 1\n", numTetras);
	// define a node set with all nodes (except middle ones)
	fprintf(fid_out, "*Nset, Nset=AllNodes, generate\n"
		" 1, %d, 1\n", numNodes);
	
	// Create node sets at the ends
	double bounds[6];
	mesh->GetBounds(bounds);
	double zMin = bounds[4];
	double zMax = bounds[5];
	double dz = (zMax - zMin)/LENGTH_PER_CAP_SIZE;

	fprintf(fid_out, "*NSET, nset=Upper_cap\n");
	vtkIdType np = 0;
	for (vtkIdType i = 0; i < numNodes; i++)
	{
		double p[3];
		pPoints->GetPoint(i, p);
		if(p[2] > zMax - dz)
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}

	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*NSET, nset=Lower_cap\n");
	np = 0;
	for (vtkIdType i = 0; i < numNodes; i++)
	{
		double p[3];
		pPoints->GetPoint(i, p);
		if(p[2] < zMin + dz)
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");

	// find surface nodes close to the mesh surface
	vtkSmartPointer<vtkKdTreePointLocator> locator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locator->SetDataSet(pSplitSurface);
	locator->BuildLocator();

	vtkSmartPointer<vtkCellDataToPointData> transferCellDataToPoints = vtkSmartPointer<vtkCellDataToPointData>::New();
	transferCellDataToPoints->SetInputData(pSplitSurface);
	transferCellDataToPoints->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS, SURF_ID_NAME);
	transferCellDataToPoints->Update();

	vtkIdTypeArray *surfIds = (vtkIdTypeArray *)transferCellDataToPoints->GetPolyDataOutput()->GetPointData()->GetScalars(SURF_ID_NAME);

	double pCoord[3];//the coordinates of the test point 

	// Find neighbours of triangular cells
	int intSurfID, extSurfID;
	if (boHandlingILT)
	{
		intSurfID = INT_ILT_SURF_ID;
		extSurfID = INT_WALL_SURF_ID;
	}
	else
	{
		intSurfID = INT_WALL_SURF_ID;
		extSurfID = EXT_WALL_SURF_ID;
	}
	vtkSmartPointer<vtkIdList> nIds = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> tetraNodeIds = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> faceIds = vtkSmartPointer<vtkIdList>::New();
	faceIds->SetNumberOfIds(numElements);
	for (vtkIdType i = 0; i < numElements; i++) { faceIds->SetId(i, 0);};
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (mesh->GetCellType(i) == VTK_TRIANGLE)
		{	
			// get node ids
			nIds = mesh->GetCell(i)->GetPointIds();
			vtkIdType res = 0;
			for (vtkIdType k = 0; k < 3; k++)
			{
				// get coordinates of node
				pPoints->GetPoint(nIds->GetId(k), pCoord);
				vtkIdType id = locator->FindClosestPoint(pCoord);
				if (surfIds->GetValue(id) == intSurfID) 
				{
					res = intSurfID;
					break;
				}
				if (surfIds->GetValue(id) == extSurfID) 
				{
					res = extSurfID;
					break;
				}
			}
			if (res > 0)
			{
				// triangle on the interior or exterior surface
				mesh->GetCellNeighbors(i, nIds, cellIds);
				for (vtkIdType n = 0; n < cellIds->GetNumberOfIds(); n++)
				{
					vtkIdType id = cellIds->GetId(n);
					if (mesh->GetCellType(id) != VTK_TRIANGLE)
					{
						// check which face 
						tetraNodeIds = mesh->GetCell(id)->GetPointIds();
						if (nIds->GetId(0) != tetraNodeIds->GetId(0) &&
							nIds->GetId(1) != tetraNodeIds->GetId(0) &&
							nIds->GetId(2) != tetraNodeIds->GetId(0))
						{
							// node 1 not in common face
							faceIds->SetId(id, res*10+3);
						}
						else if (nIds->GetId(0) != tetraNodeIds->GetId(1) &&
							nIds->GetId(1) != tetraNodeIds->GetId(1) &&
							nIds->GetId(2) != tetraNodeIds->GetId(1))
						{
							// node 2 not in common face
							faceIds->SetId(id, res*10+4);
						}
						else if (nIds->GetId(0) != tetraNodeIds->GetId(2) &&
							nIds->GetId(1) != tetraNodeIds->GetId(2) &&
							nIds->GetId(2) != tetraNodeIds->GetId(2))
						{
							// node 3 not in common face
							faceIds->SetId(id, res*10+2);
						}
						else faceIds->SetId(id, res*10+1);
					}
				}
			}
		}
	}

	// create interior surface set
	fprintf(fid_out, "*Elset, elset=_INTERIORS_S1, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (intSurfID*10+1))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*Elset, elset=_INTERIORS_S2, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (intSurfID*10+2))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*Elset, elset=_INTERIORS_S3, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (intSurfID*10+3))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*Elset, elset=_INTERIORS_S4, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (intSurfID*10+4))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	// create interior surface
	fprintf(fid_out, "*Surface, name=InteriorS, TYPE=ELEMENT\n"
		"_INTERIORS_S1, S1\n"
		"_INTERIORS_S2, S2\n"
		"_INTERIORS_S3, S3\n"
		"_INTERIORS_S4, S4\n");

	// create exterior surface set
	fprintf(fid_out, "*Elset, elset=_EXTERIORS_S1, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (extSurfID*10+1))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*Elset, elset=_EXTERIORS_S2, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (extSurfID*10+2))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*Elset, elset=_EXTERIORS_S3, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (extSurfID*10+3))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}
	if (np != 0) fprintf(fid_out, "\n");
	fprintf(fid_out, "*Elset, elset=_EXTERIORS_S4, internal\n");
	np = 0;
	for (vtkIdType i = 0; i < numElements; i++)
	{
		if (faceIds->GetId(i) == (extSurfID*10+4))
		{
			np++;
			fprintf(fid_out, "%d, ", i+1);
			if (np == 8)
			{
				np = 0;
				fprintf(fid_out, "\n");
			}
		}
	}

	if (np != 0) fprintf(fid_out, "\n");
	// create interior surface
	fprintf(fid_out, "*Surface, name=ExteriorS, TYPE=ELEMENT\n"
		"_EXTERIORS_S1, S1\n"
		"_EXTERIORS_S2, S2\n"
		"_EXTERIORS_S3, S3\n"
		"_EXTERIORS_S4, S4\n");

	// define section
	fprintf(fid_out, "*Solid Section, elset=AllElements, material=%s\n", partName.c_str());
	// end part and start assembly with node sets
	fprintf(fid_out, "*End Part\n");

	fclose(fid_out);

	return EXIT_SUCCESS;
}