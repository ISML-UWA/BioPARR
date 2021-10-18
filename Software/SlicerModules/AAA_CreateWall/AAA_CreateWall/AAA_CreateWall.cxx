#include <stdio.h>

// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkSmartPointer.h>
#include <vtkIdTypeArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkIdList.h>
#include <vtkClipPolyData.h>
#include <vtkPoints.h>
#include <vtkCell.h>
#include <vtkPlane.h>
#include <vtkTriangleFilter.h>
#include <vtkKdTreePointLocator.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkFeatureEdges.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkAppendPolyData.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkPlaneCollection.h>
#include <vtkClipClosedSurface.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkReverseSense.h>
#include <vtkCubeSource.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkFillHolesFilter.h>
#include <vtkCellData.h>
#include <vtkSTLWriter.h>
#include <vtkThreshold.h>
#include <vtkGeometryFilter.h>
#include <vtkKdTreePointLocator.h>
#include <vtkCellArray.h>
#include <vtkMeshQuality.h>
#include <vector>

#include <vtkXMLPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkMath.h>
#include <vtkXMLPolyDataWriter.h>

#include "AAA_CreateWallCLP.h"
#include "tinydir.h"
#include "AAA_defines.h"

#define EPS 1.0

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

bool boRemoveAllEdgeTrias(vtkPolyData *pPoly)
{
	// extract boundary edges
	vtkSmartPointer<vtkFeatureEdges> edgeExtractor = vtkSmartPointer<vtkFeatureEdges>::New();
	edgeExtractor->BoundaryEdgesOn();
	edgeExtractor->FeatureEdgesOff();
	edgeExtractor->ManifoldEdgesOff();
	edgeExtractor->NonManifoldEdgesOff();
	edgeExtractor->SetColoring(0);
	edgeExtractor->SetInputData(pPoly);
	edgeExtractor->Update();

	vtkSmartPointer<vtkPolyData> boundaryEdges = vtkSmartPointer<vtkPolyData>::New();
	boundaryEdges = edgeExtractor->GetOutput();

	vtkIdType numEdges = boundaryEdges->GetNumberOfCells();
	bool boRemovedCells = false;

	if (numEdges > 0)
	{
		// locate edges in input data and remove neighbouring polys
		vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
		kDTreeLocator->SetDataSet(pPoly);
		kDTreeLocator->BuildLocator();
		pPoly->BuildLinks();

		vtkIdList *EdgePolyNeighbors = vtkIdList::New();
		vtkIdList *CellsToDelete = vtkIdList::New();
		vtkIdType id0, id1;
		double pP0[3], pP1[3];
		for (vtkIdType i = 0; i < numEdges; i++)
		{
			// get edge points
			vtkCell *pEdge = boundaryEdges->GetCell(i);
			id0 = pEdge->GetPointId(0);
			boundaryEdges->GetPoint(id0, pP0);
			id1 = pEdge->GetPointId(1);
			boundaryEdges->GetPoint(id1, pP1);
			// locate points in mesh
			id0 = kDTreeLocator->FindClosestPoint(pP0);
			id1 = kDTreeLocator->FindClosestPoint(pP1);
			// find neighbours
			EdgePolyNeighbors->Initialize();
			pPoly->GetCellEdgeNeighbors(-1, id0, id1, EdgePolyNeighbors);
			vtkIdType numPolys = EdgePolyNeighbors->GetNumberOfIds();
			for (vtkIdType k = 0; k < numPolys; k++)
			{
				vtkIdType cellId = EdgePolyNeighbors->GetId(k);
				CellsToDelete->InsertUniqueId(cellId);
			}
		}
		vtkIdType numRemoved = CellsToDelete->GetNumberOfIds();
		for (vtkIdType i = 0; i < numRemoved; i++)
		{
			vtkIdType cellId = CellsToDelete->GetId(i);
			pPoly->DeleteCell(cellId);
		}
		EdgePolyNeighbors->Delete();
		CellsToDelete->Delete();
		if (numRemoved > 0)
		{
			pPoly->RemoveDeletedCells();
			boRemovedCells = true;
			std::cout << "Removed  " << numRemoved << " boundary triangles!"<< std::endl;
		}
	}
	return boRemovedCells;
}

bool boRemoveNonManifoldTrias(vtkPolyData *pPoly)
{
	// extract non-Manifold edges
	vtkSmartPointer<vtkFeatureEdges> edgeExtractor = vtkSmartPointer<vtkFeatureEdges>::New();
	edgeExtractor->BoundaryEdgesOff();
	edgeExtractor->FeatureEdgesOff();
	edgeExtractor->ManifoldEdgesOff();
	edgeExtractor->NonManifoldEdgesOn();
	edgeExtractor->SetColoring(0);
	edgeExtractor->SetInputData(pPoly);
	edgeExtractor->Update();

	vtkSmartPointer<vtkPolyData> nonManifoldEdges = vtkSmartPointer<vtkPolyData>::New();
	nonManifoldEdges = edgeExtractor->GetOutput();

	vtkIdType numEdges = nonManifoldEdges->GetNumberOfCells();
	std::cout << "Found  " << numEdges << " non-manifold edges!"<< std::endl;

	bool boFoundEdges = (numEdges>0)?true:false;
	if (boFoundEdges)
	{
		// locate edges in input data and remove neighbouring polys
		vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
		kDTreeLocator->SetDataSet(pPoly);
		kDTreeLocator->BuildLocator();
		pPoly->BuildLinks();

		vtkIdList *EdgePolyNeighbors = vtkIdList::New();
		vtkIdType id0, id1;
		vtkIdType numRemoved = 0;
		double pP0[3], pP1[3];
		for (vtkIdType i = 0; i < numEdges; i++)
		{
			// get edge points
			vtkCell *pEdge = nonManifoldEdges->GetCell(i);
			id0 = pEdge->GetPointId(0);
			nonManifoldEdges->GetPoint(id0, pP0);
			id1 = pEdge->GetPointId(1);
			nonManifoldEdges->GetPoint(id1, pP1);
			// locate points in mesh
			id0 = kDTreeLocator->FindClosestPoint(pP0);
			id1 = kDTreeLocator->FindClosestPoint(pP1);
			// find neighbours
			EdgePolyNeighbors->Initialize();
			pPoly->GetCellEdgeNeighbors(-1, id0, id1, EdgePolyNeighbors);
			numRemoved += EdgePolyNeighbors->GetNumberOfIds();
			for (vtkIdType k = 0; k < EdgePolyNeighbors->GetNumberOfIds(); k++)
			{
				pPoly->DeleteCell(EdgePolyNeighbors->GetId(k));
			}
		}
		EdgePolyNeighbors->Delete();
		pPoly->RemoveDeletedCells();

		std::cout << "Removed  " << numRemoved << " non-manifold polys!"<< std::endl;
	}
	return boFoundEdges;
}

bool boHasHoles(vtkPolyData *pPoly)
{
	// extract boundary edges
	vtkSmartPointer<vtkFeatureEdges> edgeExtractor = vtkSmartPointer<vtkFeatureEdges>::New();
	edgeExtractor->BoundaryEdgesOn();
	edgeExtractor->FeatureEdgesOff();
	edgeExtractor->ManifoldEdgesOff();
	edgeExtractor->NonManifoldEdgesOff();
	edgeExtractor->SetColoring(0);
	edgeExtractor->SetInputData(pPoly);
	edgeExtractor->Update();

	vtkSmartPointer<vtkPolyData> boundaryEdges = vtkSmartPointer<vtkPolyData>::New();
	boundaryEdges = edgeExtractor->GetOutput();
	vtkIdType numEdges = boundaryEdges->GetNumberOfCells();
	if (numEdges > 0) return true;
	return false;
}

bool boRemoveEdgeSkinnyTrias(vtkPolyData *pPoly)
{
	// extract boundary edges
	vtkSmartPointer<vtkFeatureEdges> edgeExtractor = vtkSmartPointer<vtkFeatureEdges>::New();
	edgeExtractor->BoundaryEdgesOn();
	edgeExtractor->FeatureEdgesOff();
	edgeExtractor->ManifoldEdgesOff();
	edgeExtractor->NonManifoldEdgesOff();
	edgeExtractor->SetColoring(0);
	edgeExtractor->SetInputData(pPoly);
	edgeExtractor->Update();

	vtkSmartPointer<vtkPolyData> boundaryEdges = vtkSmartPointer<vtkPolyData>::New();
	boundaryEdges = edgeExtractor->GetOutput();

	vtkIdType numEdges = boundaryEdges->GetNumberOfCells();

	bool boRemovedCells = false;
	// locate edges in input data and remove neighbouring polys
	vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTreeLocator->SetDataSet(pPoly);
	kDTreeLocator->BuildLocator();
	pPoly->BuildLinks();

	vtkIdList *EdgePolyNeighbors = vtkIdList::New();
	vtkIdList *CellsToDelete = vtkIdList::New();
	vtkIdType id0, id1;
	double pP0[3], pP1[3];
	for (vtkIdType i = 0; i < numEdges; i++)
	{
		// get edge points
		vtkCell *pEdge = boundaryEdges->GetCell(i);
		id0 = pEdge->GetPointId(0);
		boundaryEdges->GetPoint(id0, pP0);
		id1 = pEdge->GetPointId(1);
		boundaryEdges->GetPoint(id1, pP1);
		// locate points in mesh
		id0 = kDTreeLocator->FindClosestPoint(pP0);
		id1 = kDTreeLocator->FindClosestPoint(pP1);
		// find neighbours
		EdgePolyNeighbors->Initialize();
		pPoly->GetCellEdgeNeighbors(-1, id0, id1, EdgePolyNeighbors);
		vtkIdType numPolys = EdgePolyNeighbors->GetNumberOfIds();
		for (vtkIdType k = 0; k < numPolys; k++)
		{
			vtkIdType cellId = EdgePolyNeighbors->GetId(k);
			vtkCell *pCell = pPoly->GetCell(cellId);
			if (vtkMeshQuality::TriangleAspectRatio(pCell) > 10)
			{
				CellsToDelete->InsertUniqueId(cellId);
			}
		}
	}
	vtkIdType numRemoved = CellsToDelete->GetNumberOfIds();
	for (vtkIdType i = 0; i < numRemoved; i++)
	{
		vtkIdType cellId = CellsToDelete->GetId(i);
		pPoly->DeleteCell(cellId);
	}
	EdgePolyNeighbors->Delete();
	CellsToDelete->Delete();
	if (numRemoved > 0)
	{
		pPoly->RemoveDeletedCells();
		boRemovedCells = true;
		std::cout << "Removed  " << numRemoved << " skinny triangles from boundary!"<< std::endl;
	}
	return boRemovedCells;
}

int iWriteOutput(const char *FileName, vtkDataObject *pSurf)
{
	// Write output
	vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	pdWriter->SetFileName(FileName);
	pdWriter->SetDataModeToBinary();
	pdWriter->SetCompressorTypeToZLib();
	pdWriter->SetInputData(pSurf);
	if (!pdWriter->Write())
	{
		std::cerr << "Failed to write surface to " << FileName << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void vReadMeasurementsFromFile(const char *pcFileName, vtkSmartPointer<vtkDoubleArray> &aP1, vtkSmartPointer<vtkDoubleArray> &aP2)
{
	char line[512];
	FILE *pf = fopen(pcFileName, "rt");
	if (pf == NULL) return;
	char *pl = fgets(line, 512, pf);
	float x1[3], x2[3];
	//double P[3];
	while (pl)
	{

		if (_strnicmp(pl, "point", 5) == 0)
		{
			int n = sscanf(pl, "point|%f|%f|%f|", &x1[0], &x1[1], &x1[2]);
			if (n != 3) return;
			pl = fgets(line, 512, pf);
			if (_strnicmp(pl, "point", 5) == 0)
			{
				n = sscanf(pl, "point|%f|%f|%f|", &x2[0], &x2[1], &x2[2]);
				if (n != 3) {
					fclose(pf);
					return;
				}
				aP1->InsertNextTuple(x1);
				aP2->InsertNextTuple(x2);
				fclose(pf);
				return;
			}
		}
		pl = fgets(line, 512, pf);
	}
	fclose(pf);
}

void vInsertUniqueID(vtkSmartPointer<vtkIdTypeArray> &aIds, vtkIdType id)
{
	vtkIdType size = aIds->GetNumberOfTuples();
	bool boExists = false;
	for (vtkIdType i = 0; i < size; i++)
	{
		if (aIds->GetValue(i) == id)
		{
			boExists = true;
			break;
		}
	}
	if (!boExists)
	{
		aIds->InsertNextValue(id);
	}
}

bool boIdNotInList(vtkSmartPointer<vtkIdTypeArray> &aIds, vtkIdType id)
{
	vtkIdType size = aIds->GetNumberOfTuples();
	bool boExists = false;
	for (vtkIdType i = 0; i < size; i++)
	{
		if (aIds->GetValue(i) == id)
		{
			boExists = true;
			break;
		}
	}
	return !boExists;
}

FILE *pfCreateMSHFile(const char *pcFileName, vtkPoints *pPoints)
{
	FILE *pf = fopen(pcFileName, "wt");
	if (pf == NULL) return pf;
	fprintf(pf, "$MeshFormat\n");
	fprintf(pf, "2.2 0 8\n");
	fprintf(pf, "$EndMeshFormat\n");
	fprintf(pf, "$Nodes\n");
	vtkIdType numPoints = pPoints->GetNumberOfPoints();
	fprintf(pf, "%d\n", numPoints);
	double p[3];
	for (vtkIdType i = 0; i < numPoints; i++)
	{
		pPoints->GetPoint(i, p);
		fprintf(pf, "%d %.16f %.16f %.16f\n", i+1, p[0], p[1], p[2]);
	}
	fprintf(pf, "$EndNodes\n");
	return pf;
}

vtkIdType nAddMSHElements(FILE *pf, vtkIdType firstIndex, int physical, int elementary, vtkPolyData *pPart, vtkSmartPointer<vtkKdTreePointLocator> &locatorAAANodes, int cell_type)
{
	vtkIdType index = firstIndex;
	vtkIdType numCells = pPart->GetNumberOfCells();
	vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
	double point[3];
	for (vtkIdType i = 0; i < numCells; i++)
	{
		if (cell_type == pPart->GetCellType(i))
		{
			if (cell_type == VTK_LINE)
			{
				pPart->GetCellPoints(i, pointIds);
				pPart->GetPoint(pointIds->GetId(0), point);
				vtkIdType pointId1 = locatorAAANodes->FindClosestPoint(point);
				pPart->GetPoint(pointIds->GetId(1), point);
				vtkIdType pointId2 = locatorAAANodes->FindClosestPoint(point);
				fprintf(pf, "%d 1 2 %d %d %d %d\n", index, physical, elementary, pointId1+1, pointId2+1);
				index++;
			}
			else if (cell_type == VTK_TRIANGLE)
			{
				pPart->GetCellPoints(i, pointIds);
				pPart->GetPoint(pointIds->GetId(0), point);
				vtkIdType pointId1 = locatorAAANodes->FindClosestPoint(point);
				pPart->GetPoint(pointIds->GetId(1), point);
				vtkIdType pointId2 = locatorAAANodes->FindClosestPoint(point);
				pPart->GetPoint(pointIds->GetId(2), point);
				vtkIdType pointId3 = locatorAAANodes->FindClosestPoint(point);
				fprintf(pf, "%d 2 2 %d %d %d %d %d\n", index, physical, elementary, pointId1+1, pointId2+1, pointId3+1);
				index++;
			}
		}
	}
	return index;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
	PARSE_ARGS;
	vtkDebugLeaks::SetExitError(true);

	std::string fileName;
	bool boCreateILT = true;
	// Check inputs
	if (exteriorSurface.size() == 0)
	{
		std::cerr << "ERROR: no exterior surface specified!" << std::endl;
		return EXIT_FAILURE;
	}
	if ((bloodSurface.size() == 0) || (bloodSurface == exteriorSurface))
	{
		std::cerr << "WARNING: no blood surface specified - ILT volume will not be created!" << std::endl;
		boCreateILT = false;
	}
	if (dirMeasurements.size() == 0)
	{
		std::cerr << "ERROR: no directory containing measurements specified!" << std::endl;
		return EXIT_FAILURE;
	}
	if (dirOutputs.size() == 0)
	{
		std::cerr << "ERROR: no directory for output STL surfaces specified!" << std::endl;
		return EXIT_FAILURE;
	}
	if (WallSurface.size() == 0)
	{
		std::cerr << "WARNING: no wall surface output file specified! Exterior surface file will be modified." << std::endl;
		WallSurface = exteriorSurface;
	}

	if (boCreateILT)
	{
		if (ILTSurface.size() == 0)
		{
			std::cerr << "WARNING: no ILT surface output file specified! Blood surface file will be modified." << std::endl;
			ILTSurface = bloodSurface;
		}
	}

	// read measurements
	std::cout << "Reading thickness measurements ..." << std::endl;
	vtkSmartPointer<vtkDoubleArray> aP1 = vtkSmartPointer<vtkDoubleArray>::New();
	vtkSmartPointer<vtkDoubleArray> aP2 = vtkSmartPointer<vtkDoubleArray>::New();
	aP1->SetNumberOfComponents(3);
	aP2->SetNumberOfComponents(3);

	tinydir_dir dir;
	if (tinydir_open(&dir, dirMeasurements.c_str()) == -1)
	{
		std::cerr << "WARNING: could not open directory " << dirMeasurements.c_str() << std::endl;
		std::cerr << "    A 1.5mm constant thickness AAA wall will be created."  << std::endl;
	}
	else {
		aP1->SetNumberOfTuples(dir.n_files);
		aP2->SetNumberOfTuples(dir.n_files);
		while (dir.has_next)
		{
			tinydir_file file;
			if (tinydir_readfile(&dir, &file) != -1)
			{
				if (file.is_reg)
				{
					// check file extension
					char *dot = strrchr(file.name, '.');
					if ((NULL != dot) && (_strnicmp(dot, ".acsv", 5) == 0))
					{
						// read measurements
						vReadMeasurementsFromFile(file.path, aP1, aP2);
					}
				}
			}
			tinydir_next(&dir);
		}
	}
	
	std::cout << "Reading input surfaces ..." << std::endl;
	// Read inputs
	vtkSmartPointer<vtkXMLPolyDataReader> pdReader1 = vtkSmartPointer<vtkXMLPolyDataReader>::New();
	if (boCreateILT)
	{
		pdReader1->SetFileName(bloodSurface.c_str() );
		pdReader1->Update();
		if( pdReader1->GetErrorCode() != 0 )
		{
			std::cerr << "ERROR: Failed to read blood surface from " << bloodSurface.c_str() << std::endl;
			return EXIT_FAILURE;
		}
	}

	vtkSmartPointer<vtkXMLPolyDataReader> pdReader2 = vtkSmartPointer<vtkXMLPolyDataReader>::New();
	pdReader2->SetFileName(exteriorSurface.c_str() );
	pdReader2->Update();
	if( pdReader2->GetErrorCode() != 0 )
    {
		std::cerr << "ERROR: Failed to read exterior surface from " << exteriorSurface.c_str() << std::endl;
		return EXIT_FAILURE;
    }

	vtkSmartPointer<vtkPolyData> SurfaceAAA = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(pdReader2->GetOutput());
	cleaner->ToleranceIsAbsoluteOn();
	cleaner->SetAbsoluteTolerance(EPS/5);
	cleaner->Update();
	vtkSmartPointer<vtkTriangleFilter> triaFilter = vtkSmartPointer<vtkTriangleFilter>::New();
	triaFilter->SetInputData(cleaner->GetOutput());
	triaFilter->PassLinesOff();
	triaFilter->PassVertsOff();
	triaFilter->Update();
	SurfaceAAA->DeepCopy(triaFilter->GetOutput());

	vtkSmartPointer<vtkPolyData> SurfaceBlood = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkReverseSense> pReverser = vtkSmartPointer<vtkReverseSense>::New();
	if (boCreateILT)
	{
		cleaner->RemoveAllInputs();
		cleaner->SetInputData(pdReader1->GetOutput());
		cleaner->Update();
		triaFilter->RemoveAllInputs();
		triaFilter->SetInputData(cleaner->GetOutput());
		triaFilter->Update();
		pReverser->SetInputData(triaFilter->GetOutput());
		pReverser->Update();
		SurfaceBlood->DeepCopy(pReverser->GetOutput());
	}

	// ------------------------------------------- //
	// Clean input surfaces
	// ------------------------------------------- //
	std::cout << "Cleaning input surfaces ..." << std::endl;
	vtkSmartPointer<vtkFillHolesFilter> holeFiller = vtkSmartPointer<vtkFillHolesFilter>::New();
	double boundsData[6];
	double zMin, zMax;
	SurfaceAAA->ComputeBounds();
	SurfaceAAA->GetBounds(boundsData);
	zMin = boundsData[4];
	zMax = boundsData[5];
	holeFiller->SetHoleSize((zMax-zMin)/20);

	bool boRemoved = boRemoveNonManifoldTrias(SurfaceAAA);
	bool boRemoved1 = boRemoveEdgeSkinnyTrias(SurfaceAAA);
		
	if (boRemoved || boRemoved1)
	{
		holeFiller->SetInputData(SurfaceAAA);
		holeFiller->Update();
		SurfaceAAA->Initialize();
		SurfaceAAA->DeepCopy(holeFiller->GetOutput());
	}

	if (boCreateILT)
	{
		boRemoved = boRemoveNonManifoldTrias(SurfaceBlood);
		boRemoved1 = boRemoveEdgeSkinnyTrias(SurfaceBlood);

		if (boRemoved || boRemoved1)
		{
			holeFiller->RemoveAllInputs();
			holeFiller->SetInputData(SurfaceBlood);
			holeFiller->Update();
			SurfaceBlood->Initialize();
			SurfaceBlood->DeepCopy(holeFiller->GetOutput());
		}
	}

	// ------------------------------------------- //
	// create the interior AAA surface by displacing the exterior surface by thickness along normals
	// ------------------------------------------- //
	std::cout << "Mapping measurements to the AAA surface ..." << std::endl;
	vtkSmartPointer<vtkPolyData> extPoints = vtkSmartPointer<vtkPolyData>::New();
	extPoints->ShallowCopy(SurfaceAAA);
	extPoints->BuildLinks();

	// clipping filter is used to extract the caps at the upper and lower ends of the input surface
	vtkSmartPointer<vtkClipPolyData> Clipper = vtkSmartPointer<vtkClipPolyData>::New();

	double dz = 2;
	vtkSmartPointer<vtkPlane> Plane = vtkSmartPointer<vtkPlane>::New();
	Plane->SetNormal(0,0,1);
	Plane->SetOrigin(0,0,zMax - dz);
	Clipper->SetInputData(SurfaceAAA);
	Clipper->SetClipFunction(Plane);
	Clipper->InsideOutOff();
	Clipper->GenerateClippedOutputOn();
	Clipper->Update();
	Clipper->GetOutput()->ComputeBounds();
	double boundsTop[6];
	Clipper->GetOutput()->GetBounds(boundsTop);

	Plane->SetNormal(0,0,-1);
	Plane->SetOrigin(0,0,zMin + dz);
	Clipper->Update();
	Clipper->GetOutput()->ComputeBounds();
	double boundsBottom[6];
	Clipper->GetOutput()->GetBounds(boundsBottom);

	aP1->Squeeze();
	vtkIdType numMeasurements = aP1->GetNumberOfTuples();
	double thickness = 1.5;
	double *testPoint;
	double *testPoint1;
	if (numMeasurements == 1) 
	{
		testPoint = aP1->GetTuple3(0);
		testPoint1 = aP2->GetTuple3(0);
		thickness = sqrt(vtkMath::Distance2BetweenPoints(testPoint, testPoint1));
		std::cout << "Thickness set to " << thickness << "!" << std::endl;
	}

	// set thickness 1.5 mm at top and bottom of the AAA
	float x1[3], x2[3];
	x1[0] = boundsBottom[0];
	x1[1] = boundsBottom[2];
	x1[2] = boundsBottom[4] - 15;
	x2[0] = x1[0]; x2[1] = x1[1]; x2[2] = x1[2]-thickness; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[0] = boundsBottom[1];
	x2[0] = x1[0]; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[1] = boundsBottom[3];
	x2[1] = x1[1]; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[0] = boundsBottom[0];
	x2[0] = x1[0]; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[0] = boundsTop[0];
	x1[1] = boundsTop[2];
	x1[2] = boundsTop[5] + 15;
	x2[0] = x1[0]; x2[1] = x1[1]; x2[2] = x1[2]+thickness; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[0] = boundsTop[1];
	x2[0] = x1[0]; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[1] = boundsTop[3];
	x2[1] = x1[1]; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	x1[0] = boundsTop[0];
	x2[0] = x1[0]; 
	aP1->InsertNextTuple(x1); aP2->InsertNextTuple(x2);

	vtkIdType numPolys = extPoints->GetNumberOfCells();
	aP1->Squeeze();
	numMeasurements = aP1->GetNumberOfTuples();
	vtkIdType numPoints = extPoints->GetNumberOfPoints();

	if ((numPoints == 0) || (numPolys == 0))
	{
		std::cerr << "ERROR: Invalid surface data in " << exteriorSurface.c_str() << std::endl;
		return EXIT_FAILURE;
    }

	if (numMeasurements == 0)
	{
		std::cerr << "ERROR: No thickness measurements found in " << dirMeasurements.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	// build locator
	vtkSmartPointer<vtkKdTreePointLocator> kDTree2 = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTree2->SetDataSet(extPoints);
	kDTree2->BuildLocator();

	// find closest point for each of the measurement points
	vtkSmartPointer<vtkDoubleArray> thicknessExtPoints = vtkSmartPointer<vtkDoubleArray>::New();
    thicknessExtPoints->SetName(THICKNESS_NAME);
	thicknessExtPoints->SetNumberOfValues(numPoints);

	vtkSmartPointer<vtkIdTypeArray> measurementPointMask = vtkSmartPointer<vtkIdTypeArray>::New();
	measurementPointMask->SetName("MeasurementPoints");
	measurementPointMask->SetNumberOfValues(numPoints);

	vtkSmartPointer<vtkIdTypeArray> measurementPointIds = vtkSmartPointer<vtkIdTypeArray>::New();
	measurementPointIds->SetNumberOfValues(numMeasurements);

	for (vtkIdType i = 0; i < numPoints; i++)
	{
		thicknessExtPoints->SetValue(i, 0);
		measurementPointMask->SetValue(i, 0);
	}

	double closestPoint[3];
	for (vtkIdType i = 0; i < numMeasurements; i++)
	{
		testPoint = aP1->GetTuple3(i);
		// get ID of closest point
		vtkIdType idClosestPoint = kDTree2->FindClosestPoint(testPoint);
		// get location of closest point
		kDTree2->GetDataSet()->GetPoint(idClosestPoint, closestPoint);
		// compute distance
		double dist = vtkMath::Distance2BetweenPoints(testPoint, closestPoint);

		testPoint1 = aP2->GetTuple3(i);
		// get ID of closest point
		vtkIdType idClosestPoint1 = kDTree2->FindClosestPoint(testPoint1);
		// get location of closest point
		kDTree2->GetDataSet()->GetPoint(idClosestPoint1, closestPoint);
		// compute distance
		double dist1 = vtkMath::Distance2BetweenPoints(testPoint1, closestPoint);
		if (dist1 < dist) idClosestPoint = idClosestPoint1; // this is the closest point

		// compute thickness
		dist = vtkMath::Distance2BetweenPoints(testPoint, testPoint1);
		thicknessExtPoints->SetValue(idClosestPoint, sqrt(dist));
		measurementPointMask->SetValue(idClosestPoint, 1);
		measurementPointIds->SetValue(i, idClosestPoint);
	}

	std::cout << "Interpolating measurements over the AAA surface ..." << std::endl;
	// Perform closest point interpolation over the mesh
	// Propagate fronts from each measurement marking triangles as closest to the measurement points
	vtkSmartPointer<vtkIdTypeArray> triaClasifiedMask = vtkSmartPointer<vtkIdTypeArray>::New();
	triaClasifiedMask->SetNumberOfValues(numPolys);
	vtkSmartPointer<vtkIdTypeArray> triaLayerIndex = vtkSmartPointer<vtkIdTypeArray>::New();
	triaLayerIndex->SetNumberOfValues(numPolys);
	for (vtkIdType i = 0; i < numPolys; i++)
	{
		triaClasifiedMask->SetValue(i, 0);
		triaLayerIndex->SetValue(i, 0);
	}

	std::vector<vtkSmartPointer<vtkIdTypeArray>> aPointAddedTriasIds;
	std::vector<vtkSmartPointer<vtkIdTypeArray>> aBoundaryNodesIds;
	std::vector<vtkSmartPointer<vtkIdTypeArray>> aPointTriasIds;
	for (vtkIdType i = 0; i < numMeasurements; i++)
	{
		vtkSmartPointer<vtkIdTypeArray> addedTriasIds = vtkSmartPointer<vtkIdTypeArray>::New();
		aPointAddedTriasIds.push_back(addedTriasIds);
		vtkSmartPointer<vtkIdTypeArray> pointTriasIds = vtkSmartPointer<vtkIdTypeArray>::New();
		aPointTriasIds.push_back(pointTriasIds);
		vtkSmartPointer<vtkIdTypeArray> boundaryNodesIds = vtkSmartPointer<vtkIdTypeArray>::New();
		boundaryNodesIds->InsertNextValue(measurementPointIds->GetValue(i));
		aBoundaryNodesIds.push_back(boundaryNodesIds);
	}

	bool boContinue = true;
	vtkSmartPointer<vtkIdList> pointIds = vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();

	vtkIdType iterationNo = 0;
	while (boContinue)
	{
		if (iterationNo > numPolys)
		{
			std::cerr << "ERROR: Failed to interpolate measurements over surface " << exteriorSurface.c_str() << std::endl;
			return EXIT_FAILURE;
		}
		iterationNo++;
		boContinue = false;
		// for each measurement point
		for (vtkIdType i = 0; i < numMeasurements; i++)
		{
			aPointAddedTriasIds[i]->Initialize();
			// get all trias surrounding boundary nodes
			vtkIdType numBoundaryNodes = aBoundaryNodesIds[i]->GetNumberOfTuples();
			for (vtkIdType n = 0; n < numBoundaryNodes; n++)
			{
				vtkIdType nodeId = aBoundaryNodesIds[i]->GetValue(n);
				// get all cells around this node
				extPoints->GetPointCells(nodeId, cellIds);
				vtkIdType numNeighbours = cellIds->GetNumberOfIds();
				for (vtkIdType c = 0; c < numNeighbours; c++)
				{
					// If cell not already classified add it to the list of boundary cells
					vtkIdType cId = cellIds->GetId(c);
					if (!triaClasifiedMask->GetValue(cId))
					{
						vInsertUniqueID(aPointAddedTriasIds[i], cId);
					}
				}
			}
			// for all added trias, get the next layer of boundary nodes
			vtkSmartPointer<vtkIdTypeArray> boundaryNodesIds = vtkSmartPointer<vtkIdTypeArray>::New();
			boundaryNodesIds->DeepCopy(aBoundaryNodesIds[i]);
			aBoundaryNodesIds[i]->Initialize();
			vtkIdType numAddedTrias = aPointAddedTriasIds[i]->GetNumberOfTuples();
			for (vtkIdType t = 0; t < numAddedTrias; t++)
			{
				vtkIdType triaId = aPointAddedTriasIds[i]->GetValue(t);
				// get all nodes of this tria
				extPoints->GetCellPoints(triaId, pointIds);
				vtkIdType numNeighbours = pointIds->GetNumberOfIds();
				for (vtkIdType p = 0; p < numNeighbours; p++)
				{
					vtkIdType pId = pointIds->GetId(p);
					// check that it was not in the previous layer
					if (boIdNotInList(boundaryNodesIds, pId))
					{
						vInsertUniqueID(aBoundaryNodesIds[i], pId);
					}
				}
				// transfer layer of added trias to the trias associated with this node
				aPointTriasIds[i]->InsertNextValue(triaId);
				triaLayerIndex->SetValue(triaId, iterationNo);
			}
		}
		// mark all trias in the boundary layers as classified
		for (vtkIdType i = 0; i < numMeasurements; i++)
		{
			vtkIdType numAddedTrias = aPointAddedTriasIds[i]->GetNumberOfTuples();
			for (vtkIdType t = 0; t < numAddedTrias; t++)
			{
				vtkIdType triaId = aPointAddedTriasIds[i]->GetValue(t);
				triaClasifiedMask->SetValue(triaId, 1);
			}
		}
		// check if all trias have been classified
		for (vtkIdType i = 0; i < numPolys; i++)
		{
			if (!triaClasifiedMask->GetValue(i))
			{
				boContinue = true;
			}
		}	
	}

	// Set the value of thickness for each triangle
	vtkSmartPointer<vtkDoubleArray> aPolyThickness = vtkSmartPointer<vtkDoubleArray>::New();
	aPolyThickness->SetNumberOfValues(numPolys);

	vtkSmartPointer<vtkIdTypeArray> aNumPolysOverlapping = vtkSmartPointer<vtkIdTypeArray>::New();
	aNumPolysOverlapping->SetNumberOfValues(numPolys);

	vtkSmartPointer<vtkIntArray> aThicknessMeasurementArea = vtkSmartPointer<vtkIntArray>::New();
	aThicknessMeasurementArea->SetNumberOfValues(1);
	aThicknessMeasurementArea->SetName("MeasurementArea");
	aThicknessMeasurementArea->SetNumberOfTuples(numPolys);

	for (vtkIdType i = 0; i < numPolys; i++)
	{
		aPolyThickness->SetValue(i, 0);
		aNumPolysOverlapping->SetValue(i, 0);
	}
	for (vtkIdType i = 0; i < numMeasurements; i++)
	{
		vtkIdType numAllocatedTrias = aPointTriasIds[i]->GetNumberOfTuples();
		for (vtkIdType t = 0; t < numAllocatedTrias; t++)
		{		
			vtkIdType tria = aPointTriasIds[i]->GetValue(t);
			double thick = aPolyThickness->GetValue(tria);
			vtkIdType measurementPointId = measurementPointIds->GetValue(i);
			thick += thicknessExtPoints->GetValue(measurementPointId);
			aPolyThickness->SetValue(tria, thick);
			aThicknessMeasurementArea->SetValue(tria, i);

			vtkIdType n = aNumPolysOverlapping->GetValue(tria);
			n++;
			aNumPolysOverlapping->SetValue(tria, n);
		}
	}
	// divide to number of contributing triangles
	for (vtkIdType i = 0; i < numPolys; i++)
	{
		double thick = aPolyThickness->GetValue(i);
		vtkIdType n = aNumPolysOverlapping->GetValue(i);
		aPolyThickness->SetValue(i, thick/n);
	}
	
	// clean up
	aPointAddedTriasIds.clear();
	aBoundaryNodesIds.clear();
	aPointTriasIds.clear();
	
	// interpolate thickness over surface
	vtkSmartPointer<vtkDoubleArray> aIntermediateThickness = vtkSmartPointer<vtkDoubleArray>::New();
	aIntermediateThickness->SetNumberOfValues(numPoints);

	for (int numIter = 0; numIter < iNumSmoothingIterations; numIter++)
	{
		// smooth over cells
		for (vtkIdType i = 0; i < numPoints; i++)
		{
			extPoints->GetPointCells(i, cellIds);
			double dThickness = 0;
			vtkIdType numNeighbours = cellIds->GetNumberOfIds();
			for (vtkIdType c = 0; c < numNeighbours; c++)
			{
				vtkIdType cId = cellIds->GetId(c);
				dThickness += aPolyThickness->GetValue(cId);
			}
			thicknessExtPoints->SetValue(i, dThickness/numNeighbours);
		}
		// smooth over points
		for (vtkIdType i = 0; i < numPoints; i++)
		{
			aIntermediateThickness->SetValue(i, thicknessExtPoints->GetValue(i));
		}
		for (vtkIdType i = 0; i < numPolys; i++)
		{
			if (triaLayerIndex->GetValue(i) > iSmoothingTreshold)
			{
				extPoints->GetCellPoints(i, pointIds);
				double dThickness = 0;
				vtkIdType numNeighbours = pointIds->GetNumberOfIds();
				for (vtkIdType p = 0; p < numNeighbours; p++)
				{
					vtkIdType pId = pointIds->GetId(p);
					dThickness += aIntermediateThickness->GetValue(pId);
				}
				aPolyThickness->SetValue(i, dThickness/numNeighbours);
			}
		}
	}

	// Add point data
	extPoints->GetPointData()->SetActiveScalars(THICKNESS_NAME);
	extPoints->GetPointData()->SetScalars(thicknessExtPoints);

	extPoints->GetPointData()->SetActiveScalars("MeasurementPoints");
	extPoints->GetPointData()->SetScalars(measurementPointMask);

	extPoints->GetCellData()->SetActiveScalars("MeasurementArea");
	extPoints->GetCellData()->SetScalars(aThicknessMeasurementArea);

	fileName = dirOutputs + "\\AAA_Exterior.vtp";
	iWriteOutput(fileName.c_str(), extPoints);

	std::cout << "Creating interior AAA surface ..." << std::endl;
	// Compute normals
	vtkSmartPointer<vtkPolyDataNormals> pNormalsFilter = vtkSmartPointer<vtkPolyDataNormals>::New();
	pNormalsFilter->SetInputData(extPoints);
	pNormalsFilter->SplittingOff();
	pNormalsFilter->ComputeCellNormalsOff();
	pNormalsFilter->ComputePointNormalsOn();
	pNormalsFilter->NonManifoldTraversalOff();
	pNormalsFilter->ConsistencyOn();
	pNormalsFilter->Update();

	vtkSmartPointer<vtkPolyData> outAAASurface = vtkSmartPointer<vtkPolyData>::New();
	outAAASurface->ShallowCopy(pNormalsFilter->GetOutput());
	outAAASurface->GetPointData()->SetActiveScalars(THICKNESS_NAME);
	vtkSmartPointer<vtkPolyData> intAAASurface = vtkSmartPointer<vtkPolyData>::New();
	intAAASurface->DeepCopy(outAAASurface);
	
	// move points along normals
	vtkFloatArray *pNormals = vtkFloatArray::SafeDownCast(outAAASurface->GetPointData()->GetNormals());
	double normal[3];
	double *pPoint;
	for (vtkIdType i = 0; i < numPoints; i++)
	{
		pPoint = intAAASurface->GetPoint(i);
		pNormals->GetTuple(i, normal);
		if ((pPoint[2] > (zMax - EPS)) || (pPoint[2] < (zMin + EPS)))
		{
			normal[2] = 0;
		}
		double thickness = thicknessExtPoints->GetValue(i);
		vtkMath::MultiplyScalar(normal, thickness);
		vtkMath::Subtract(pPoint, normal, pPoint);
		intAAASurface->GetPoints()->SetPoint(i, pPoint);
	}
	intAAASurface->GetPointData()->RemoveArray("Normals");
	vtkSmartPointer<vtkSmoothPolyDataFilter> smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
	smoother->SetInputData(intAAASurface);
	smoother->SetNumberOfIterations(20);
	smoother->Update();

	cleaner->RemoveAllInputs();
	cleaner->SetInputData(smoother->GetOutput());
	cleaner->Update();

	triaFilter->RemoveAllInputs();
	triaFilter->SetInputData(cleaner->GetOutput());
	triaFilter->Update();

	intAAASurface->Initialize();
	intAAASurface->DeepCopy(triaFilter->GetOutput());

	boRemoved = boRemoveNonManifoldTrias(intAAASurface);
	boRemoved1 = boRemoveEdgeSkinnyTrias(intAAASurface);
	if (boRemoved || boRemoved1)
	{
		holeFiller->RemoveAllInputs();
		holeFiller->SetInputData(intAAASurface);
		holeFiller->Update();
		intAAASurface->Initialize();
		intAAASurface->DeepCopy(holeFiller->GetOutput());
	}
	
	// ------------------------------------------- //
	// create ILT surface 
	// ------------------------------------------- //

	std::cout << "Creating ILT surface ..." << std::endl;
	// compute normals for the interior wall
	vtkSmartPointer<vtkPolyData> SurfaceILT = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> minILTSurface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkBooleanOperationPolyDataFilter> merger = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
	merger->SetOperationToIntersection();
	vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter =
		vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();

	if (boCreateILT)
	{
		minILTSurface->DeepCopy(outAAASurface);
		// move points along normals
		for (vtkIdType i = 0; i < numPoints; i++)
		{
			pPoint = minILTSurface->GetPoint(i);
			pNormals->GetTuple(i, normal);
			if ((pPoint[2] > (zMax - EPS)) || (pPoint[2] < (zMin + EPS)))
			{
				normal[2] = 0;
			}
			double thickness = thicknessExtPoints->GetValue(i);
			vtkMath::MultiplyScalar(normal, thickness+dmin_ILT_thickness);
			vtkMath::Subtract(pPoint, normal, pPoint);
			minILTSurface->GetPoints()->SetPoint(i, pPoint);
		}
		minILTSurface->GetPointData()->RemoveArray("Normals");

		smoother->RemoveAllInputs();
		smoother->SetInputData(minILTSurface);
		smoother->SetNumberOfIterations(20);
		smoother->Update();
		
		cleaner->RemoveAllInputs();
		cleaner->SetInputData(smoother->GetOutput());
		cleaner->Update();

		triaFilter->RemoveAllInputs();
		triaFilter->SetInputData(cleaner->GetOutput());
		triaFilter->Update();

		minILTSurface->Initialize();
		minILTSurface->DeepCopy(triaFilter->GetOutput());

		boRemoved = boRemoveNonManifoldTrias(minILTSurface);
		boRemoved1 = boRemoveEdgeSkinnyTrias(minILTSurface);
		if (boRemoved || boRemoved1)
		{
			holeFiller->RemoveAllInputs();
			holeFiller->SetInputData(minILTSurface);
			holeFiller->Update();
			minILTSurface->Initialize();
			minILTSurface->DeepCopy(holeFiller->GetOutput());
		}

		// merge minimum ILT surface and blood surface
		merger->SetInputData( 0, minILTSurface );
		merger->SetInputData( 1, SurfaceBlood );
		merger->Update();

		cleaner->RemoveAllInputs();
		cleaner->SetInputData(merger->GetOutput());
		cleaner->SetAbsoluteTolerance(EPS/100);
		cleaner->ToleranceIsAbsoluteOn();
		cleaner->Update();
		
		SurfaceILT->DeepCopy(cleaner->GetOutput());

		boRemoved = boRemoveNonManifoldTrias(SurfaceILT);
		boRemoved1 = boRemoveEdgeSkinnyTrias(SurfaceILT);
		if (boRemoved || boRemoved1)
		{
			holeFiller->RemoveAllInputs();
			holeFiller->SetInputData(SurfaceILT);
			holeFiller->Update();
			SurfaceILT->Initialize();
			SurfaceILT->DeepCopy(holeFiller->GetOutput());
		}

		cleaner->RemoveAllInputs();
		cleaner->SetInputData(SurfaceILT);
		cleaner->Update();

		triaFilter->RemoveAllInputs();
		triaFilter->SetInputData(cleaner->GetOutput());
		triaFilter->Update();

		SurfaceILT->Initialize();
		SurfaceILT->DeepCopy(triaFilter->GetOutput());

		if (boHasHoles(SurfaceILT))
		{
			holeFiller->RemoveAllInputs();
			holeFiller->SetInputData(SurfaceILT);
			holeFiller->Update();
			SurfaceILT->Initialize();
			SurfaceILT->DeepCopy(holeFiller->GetOutput());
		}

		connectivityFilter->SetExtractionModeToLargestRegion();
		connectivityFilter->SetInputData(SurfaceILT);
		connectivityFilter->Update();

		smoother->RemoveAllInputs();
		smoother->SetInputData(connectivityFilter->GetOutput());
		smoother->SetNumberOfIterations(20);
		smoother->Update();
		
		SurfaceILT->Initialize();
		SurfaceILT->DeepCopy(smoother->GetOutput());

		// check that the merge is succesfull
		if (SurfaceILT->GetNumberOfCells() == 0)
		{
			std::cerr << "ERROR: Could not create interior ILT surface! Check the blood surface in " << bloodSurface.c_str() << std::endl;
			return EXIT_FAILURE;
		}

		// ------------------------------------------- //
		// Compute thickness for ILT surface 
		// ------------------------------------------- //
		std::cout << "Computing ILT thickness ..." << std::endl;
		// build locator
		vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
		kDTreeLocator->SetDataSet(intAAASurface);
		kDTreeLocator->BuildLocator();

		// find closest point
		vtkIdType numILTPoints = SurfaceILT->GetNumberOfPoints();

		vtkSmartPointer<vtkDoubleArray> distClosestPoint = vtkSmartPointer<vtkDoubleArray>::New();
		distClosestPoint->SetNumberOfValues(1);
		distClosestPoint->SetName(THICKNESS_NAME);
		distClosestPoint->SetNumberOfTuples(numILTPoints);

		double testPointCoord[3];
		for (vtkIdType i = 0; i < numILTPoints; i++)
		{
			SurfaceILT->GetPoint(i, testPointCoord);
			// get ID of closest point
			vtkIdType idClosestPoint = kDTreeLocator->FindClosestPoint(testPointCoord);
			// get location of closest point
			kDTreeLocator->GetDataSet()->GetPoint(idClosestPoint, closestPoint);
			// compute distance
			double dist = vtkMath::Distance2BetweenPoints(testPointCoord, closestPoint);
			dist = sqrt(dist);
			distClosestPoint->SetValue(i, dist);
		}
		// Add point data
		SurfaceILT->GetPointData()->SetActiveScalars(THICKNESS_NAME);
		SurfaceILT->GetPointData()->SetScalars(distClosestPoint);
	}

	// ------------------------------------------- //
	// Create output file with size info for GMSH 
	// ------------------------------------------- //
	std::cout << "Creating element size file for meshing ..." << std::endl;
	fileName = dirOutputs + "\\size.bin";
	FILE *pf = fopen(fileName.c_str(), "wb");
	if (pf == NULL)
	{
		std::cerr << "Failed to write size info to " << fileName.c_str() << std::endl;
		return EXIT_FAILURE;
	}
	// generate size data
	double O[3]; // origine
	double D[3]; // spacing
	int N[3]; // number of points
	double point[3];
	double el_size;

	O[0] = boundsData[0]-1;
	O[1] = boundsData[2]-1;
	O[2] = boundsData[4]-1;
	D[0] = 1; D[1] = 1; D[2] = 1;
	N[0] = ((boundsData[1]+1-O[0])/D[0]) + 1;
	N[1] = ((boundsData[3]+1-O[1])/D[1]) + 1;
	N[2] = ((boundsData[5]+1-O[2])/D[2]) + 1;

	fwrite(O, sizeof(double), 3, pf);
	fwrite(D, sizeof(double), 3, pf);
	fwrite(N, sizeof(int), 3, pf);

	// build locators
	vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocatorExt = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTreeLocatorExt->SetDataSet(outAAASurface);
	kDTreeLocatorExt->BuildLocator();
	outAAASurface->GetPointData()->SetActiveScalars(THICKNESS_NAME);
	vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocatorILT = vtkSmartPointer<vtkKdTreePointLocator>::New();
	vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocatorInt = vtkSmartPointer<vtkKdTreePointLocator>::New();
	if (boCreateILT)
	{
		kDTreeLocatorILT->SetDataSet(SurfaceILT);
		kDTreeLocatorILT->BuildLocator();
		kDTreeLocatorInt->SetDataSet(intAAASurface);
		kDTreeLocatorInt->BuildLocator();
		SurfaceILT->GetPointData()->SetActiveScalars(THICKNESS_NAME);
	}

	for (int i1 = 0; i1 < N[0]; i1++)
	{
		for (int i2 = 0; i2 < N[1]; i2++)
		{
			for (int i3 = 0; i3 < N[2]; i3++)
			{
				point[0] = O[0]+i1*D[0];
				point[1] = O[1]+i2*D[1];
				point[2] = O[2]+i3*D[2];
				
				// get ID of closest point on Ext surface
				vtkIdType idClosestPoint = kDTreeLocatorExt->FindClosestPoint(point);
				// get location of closest point
				kDTreeLocatorExt->GetDataSet()->GetPoint(idClosestPoint, closestPoint);
				// compute distance
				double dist = vtkMath::Distance2BetweenPoints(point, closestPoint);
				dist = sqrt(dist);
				// extract thickness
				double *thick = outAAASurface->GetPointData()->GetScalars()->GetTuple(idClosestPoint);
				el_size = (*thick)/wallNumLayers/1.2;
				if ((*thick) < dist)
				{
					if (boCreateILT)
					{
						// get ID of closest point on ILT surface
						idClosestPoint = kDTreeLocatorILT->FindClosestPoint(point);
						// get location of closest point
						kDTreeLocatorILT->GetDataSet()->GetPoint(idClosestPoint, closestPoint);
						// compute distance
						double distILT = vtkMath::Distance2BetweenPoints(point, closestPoint);
						distILT = sqrt(distILT);
						// extract thickness
						double *thickILT = SurfaceILT->GetPointData()->GetScalars()->GetTuple(idClosestPoint);
						double el_sizeILT = (*thickILT)/ILTNumLayers/1.2;
						// get ID of closest point on int surface
						idClosestPoint = kDTreeLocatorInt->FindClosestPoint(point);
						// get location of closest point
						kDTreeLocatorInt->GetDataSet()->GetPoint(idClosestPoint, closestPoint);
						// compute distance
						double distInt = vtkMath::Distance2BetweenPoints(point, closestPoint);
						distInt = sqrt(distInt);
						if (distInt < 1)
						{
							if (el_size > el_sizeILT) el_size = el_sizeILT;
						}
						else if (distILT < 1)
						{
							if (el_sizeILT < 1) el_size = el_sizeILT;
							else el_size = 1;
						} else
						{
							double k = 1.2;
							double t1 = k *(distILT - 1) + el_sizeILT;
							double t2 = k *(distInt - 1) + el_size;
							if (t1 < t2) el_size = t1;
							else el_size = t2;
							if (el_size > 4) el_size = 4;
						}
					}
				}
				fwrite(&el_size, sizeof(double), 1, pf);
			}
		}
	}
	fclose(pf);

	
	// ------------------------------------------- //
	// Mark surfaces using SurfID 
	// ------------------------------------------- //
	std::cout << "Marking the different surfaces ..." << std::endl;
	vtkIdType numILTPolys = SurfaceILT->GetNumberOfPolys();
	vtkSmartPointer<vtkIntArray> aSurfIdILT = vtkSmartPointer<vtkIntArray>::New();
	aSurfIdILT->SetNumberOfValues(1);
	aSurfIdILT->SetName(SURF_ID_NAME);
	aSurfIdILT->SetNumberOfTuples(numILTPolys);
	for (vtkIdType i = 0; i < numILTPolys; i++)
	{
		aSurfIdILT->SetValue(i, INT_ILT_SURF_ID);
	}
	SurfaceILT->GetCellData()->SetActiveScalars(SURF_ID_NAME);
	SurfaceILT->GetCellData()->SetScalars(aSurfIdILT);

	numPolys = intAAASurface->GetNumberOfPolys();
	vtkSmartPointer<vtkIntArray> aSurfIdIntWall = vtkSmartPointer<vtkIntArray>::New();
	aSurfIdIntWall->SetNumberOfValues(1);
	aSurfIdIntWall->SetName(SURF_ID_NAME);
	aSurfIdIntWall->SetNumberOfTuples(numPolys);
	for (vtkIdType i = 0; i < numPolys; i++)
	{
		aSurfIdIntWall->SetValue(i, INT_WALL_SURF_ID);
	}
	intAAASurface->GetCellData()->SetActiveScalars(SURF_ID_NAME);
	intAAASurface->GetCellData()->SetScalars(aSurfIdIntWall);

	numPolys = outAAASurface->GetNumberOfPolys();
	vtkSmartPointer<vtkIntArray> aSurfIdExtWall = vtkSmartPointer<vtkIntArray>::New();
	aSurfIdExtWall->SetNumberOfValues(1);
	aSurfIdExtWall->SetName(SURF_ID_NAME);
	aSurfIdExtWall->SetNumberOfTuples(numPolys);
	for (vtkIdType i = 0; i < numPolys; i++)
	{
		aSurfIdExtWall->SetValue(i, EXT_WALL_SURF_ID);
	}
	outAAASurface->GetCellData()->SetActiveScalars(SURF_ID_NAME);
	outAAASurface->GetCellData()->SetScalars(aSurfIdExtWall);

	// ------------------------------------------- //
	// Merge all surfaces and add caps 
	// ------------------------------------------- //
	std::cout << "Creating ILT and wall volumes ..." << std::endl;
	vtkSmartPointer<vtkCubeSource> pCubeSource = vtkSmartPointer<vtkCubeSource>::New();
	boundsData[0] -= 10*EPS;
	boundsData[1] += 10*EPS;
	boundsData[2] -= 10*EPS;
	boundsData[3] += 10*EPS;
	boundsData[4] += EPS;
	boundsData[5] -= EPS;
	pCubeSource->SetBounds(boundsData);

	triaFilter->RemoveAllInputs();
	triaFilter->SetInputConnection(pCubeSource->GetOutputPort());
	triaFilter->Update();

	vtkSmartPointer<vtkPolyData> pTriangulatedCube = vtkSmartPointer<vtkPolyData>::New();
	pTriangulatedCube->DeepCopy(triaFilter->GetOutput());

	vtkIdType numCubePoints = pTriangulatedCube->GetNumberOfPoints();
	vtkSmartPointer<vtkDoubleArray> thicknessCube = vtkSmartPointer<vtkDoubleArray>::New();
	thicknessCube->SetNumberOfValues(1);
	thicknessCube->SetName(THICKNESS_NAME);
	thicknessCube->SetNumberOfTuples(numCubePoints);
	for (vtkIdType i = 0; i < numCubePoints; i++)
	{
		thicknessCube->SetValue(i, dmin_ILT_thickness);
	}
	pTriangulatedCube->GetPointData()->SetActiveScalars(THICKNESS_NAME);
	pTriangulatedCube->GetPointData()->SetScalars(thicknessCube);

	numPolys = pTriangulatedCube->GetNumberOfPolys();
	vtkSmartPointer<vtkIntArray> aCapsSurfId = vtkSmartPointer<vtkIntArray>::New();
	aCapsSurfId->SetNumberOfValues(1);
	aCapsSurfId->SetName(SURF_ID_NAME);
	aCapsSurfId->SetNumberOfTuples(numPolys);
	for (vtkIdType i = 0; i < numPolys; i++)
	{
		aCapsSurfId->SetValue(i, ILT_CAPS_SURF_ID);
	}
	pTriangulatedCube->GetCellData()->SetActiveScalars(SURF_ID_NAME);
	pTriangulatedCube->GetCellData()->SetScalars(aCapsSurfId);

	merger->SetOperationToIntersection();
	merger->RemoveAllInputs();
	merger->SetInputData( 1, pTriangulatedCube );
	
	vtkSmartPointer<vtkPolyData> pILTVolume = vtkSmartPointer<vtkPolyData>::New();

	vtkSmartPointer<vtkAppendPolyData> pAppender = vtkSmartPointer<vtkAppendPolyData>::New();
	merger->SetInputConnection(0, pAppender->GetOutputPort());
	cleaner->SetAbsoluteTolerance(EPS/100);
	//cleaner->ConvertLinesToPointsOff();
	//cleaner->ConvertPolysToLinesOff();
	//cleaner->ConvertStripsToPolysOff();
	connectivityFilter->SetExtractionModeToLargestRegion();
	if (boCreateILT)
	{
		pReverser->SetInputData(SurfaceILT);
		pReverser->Update();
		pAppender->AddInputData(pReverser->GetOutput());
		pAppender->AddInputData(intAAASurface);
		pAppender->Update();
		merger->Update();
		cleaner->RemoveAllInputs();
		cleaner->SetInputData(merger->GetOutput());
		cleaner->Update();
		triaFilter->RemoveAllInputs();
		triaFilter->SetInputData(cleaner->GetOutput());
		triaFilter->Update();
		connectivityFilter->RemoveAllInputs();
		connectivityFilter->SetInputData(triaFilter->GetOutput());
		connectivityFilter->Update();
		pILTVolume->DeepCopy(connectivityFilter->GetOutput());
	}

	for (vtkIdType i = 0; i < numPolys; i++)
	{
		aCapsSurfId->SetValue(i, WALL_CAPS_SURF_ID);
	}
	pTriangulatedCube->GetCellData()->SetActiveScalars(SURF_ID_NAME);
	pTriangulatedCube->GetCellData()->SetScalars(aCapsSurfId);

	pReverser->RemoveAllInputs();
	pReverser->SetInputData(intAAASurface);
	pReverser->Update();
	pAppender->RemoveAllInputs();
	pAppender->AddInputData(pReverser->GetOutput());
	pAppender->AddInputData(outAAASurface);
	pAppender->Update();
	merger->Update();
	cleaner->RemoveAllInputs();
	cleaner->SetInputData(merger->GetOutput());
	cleaner->Update();
	triaFilter->RemoveAllInputs();
	triaFilter->SetInputData(cleaner->GetOutput());
	triaFilter->Update();
	connectivityFilter->RemoveAllInputs();
	connectivityFilter->SetInputData(triaFilter->GetOutput());
	connectivityFilter->Update();
	vtkSmartPointer<vtkPolyData> pAAAWall = vtkSmartPointer<vtkPolyData>::New();
	pAAAWall->DeepCopy(connectivityFilter->GetOutput());

	pAAAWall->GetCellData()->RemoveArray("Distance");
	pAAAWall->GetCellData()->RemoveArray("CellSource");
	pAAAWall->GetPointData()->RemoveArray("Distance");
	pAAAWall->GetPointData()->RemoveArray("PointSource");

	std::cout << "Writing .vtp surfaces ..." << std::endl;
	// Write output
	vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	pdWriter->SetFileName(WallSurface.c_str() );
	pdWriter->SetDataModeToBinary();
	pdWriter->SetCompressorTypeToZLib();
	pdWriter->SetInputData(pAAAWall);
	if (!pdWriter->Write())
	{
		std::cerr << "Failed to write wall surface to " << WallSurface.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	if (boCreateILT)
	{
		pILTVolume->GetCellData()->RemoveArray("Distance");
		pILTVolume->GetCellData()->RemoveArray("CellSource");
		pILTVolume->GetPointData()->RemoveArray("Distance");
		pILTVolume->GetPointData()->RemoveArray("PointSource");
		pdWriter->SetFileName(ILTSurface.c_str() );
		pdWriter->SetInputData(pILTVolume);
		if (!pdWriter->Write())
		{
			std::cerr << "Failed to write ILT surface to " << ILTSurface.c_str() << std::endl;
			return EXIT_FAILURE;
		}
	}

	std::cout << "Creating .stl surfaces ..." << std::endl;
	vtkSmartPointer<vtkThreshold> selector = vtkSmartPointer<vtkThreshold>::New();
	selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, SURF_ID_NAME);
	// extract caps
	selector->ThresholdByLower(INT_ILT_SURF_ID);
	selector->SetInputData(pILTVolume);
	selector->Update();

	vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	geometryFilter->SetInputConnection(selector->GetOutputPort());
	geometryFilter->Update();

	if (boCreateILT)
	{
		// merge the 2 volumes
		pAppender->RemoveAllInputs();
		pAppender->AddInputData(geometryFilter->GetOutput());
		pAppender->AddInputData(pAAAWall);
		pAppender->Update();
		cleaner->SetInputData(pAppender->GetOutput());
	}
	else
	{
		cleaner->SetInputData(pAAAWall);
	}

	cleaner->Update();
	smoother->SetInputData(cleaner->GetOutput());
	smoother->SetNumberOfIterations(1);
	smoother->Update();
	vtkSmartPointer<vtkPolyData> pAAAMerged = vtkSmartPointer<vtkPolyData>::New();
	pAAAMerged->DeepCopy(smoother->GetOutput());

	fileName = dirOutputs + "\\AAA_Merged.vtp";
	iWriteOutput(fileName.c_str(), pAAAMerged);

	// ------------------------------------------- //
	// Create STL files and save them
	// ------------------------------------------- //

	selector->SetInputData(pAAAMerged);
	selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, SURF_ID_NAME);
	// extract caps
	selector->ThresholdBetween(WALL_CAPS_SURF_ID, WALL_CAPS_SURF_ID);
	selector->Update();
	geometryFilter->Update();

	vtkSmartPointer<vtkPolyData> pWallCap = vtkSmartPointer<vtkPolyData>::New();
	pWallCap->DeepCopy(geometryFilter->GetOutput());

	fileName = dirOutputs + "\\" + WALL_CAP_NAME + ".stl";
	vtkSmartPointer<vtkSTLWriter> pdSTLWriter = vtkSmartPointer<vtkSTLWriter>::New();
	pdSTLWriter->SetFileName(fileName.c_str());
	pdSTLWriter->SetFileTypeToBinary();
	pdSTLWriter->SetInputData(pWallCap);
	if (!pdSTLWriter->Write())
	{
		std::cerr << "Failed to write AAA wall cap surfaces to " << fileName.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	// extract exterior surface
	selector->ThresholdBetween(EXT_WALL_SURF_ID, EXT_WALL_SURF_ID);
	selector->Update();
	geometryFilter->Update();

	vtkSmartPointer<vtkPolyData> pWallExtSurf = vtkSmartPointer<vtkPolyData>::New();
	pWallExtSurf->DeepCopy(geometryFilter->GetOutput());

	fileName = dirOutputs + "\\" + EXT_WALL_SURF_NAME + ".stl";
	pdSTLWriter->SetFileName(fileName.c_str());
	pdSTLWriter->SetInputData(pWallExtSurf);
	if (!pdSTLWriter->Write())
	{
		std::cerr << "Failed to write external wall surface to " << fileName.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	// extract interior surface
	selector->ThresholdBetween(INT_WALL_SURF_ID, INT_WALL_SURF_ID);
	selector->Update();
	geometryFilter->Update();

	vtkSmartPointer<vtkPolyData> pWallIntSurf = vtkSmartPointer<vtkPolyData>::New();
	pWallIntSurf->DeepCopy(geometryFilter->GetOutput());

	fileName = dirOutputs + "\\" + INT_WALL_SURF_NAME + ".stl";
	pdSTLWriter->SetFileName(fileName.c_str());
	pdSTLWriter->SetInputData(pWallIntSurf);
	if (!pdSTLWriter->Write())
	{
		std::cerr << "Failed to write internal wall surface to " << fileName.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	vtkSmartPointer<vtkPolyData> pILTCap = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> pILTIntSurface = vtkSmartPointer<vtkPolyData>::New();
	
	if (boCreateILT)
	{
		// extract ILT caps
		selector->ThresholdBetween(ILT_CAPS_SURF_ID, ILT_CAPS_SURF_ID);
		selector->Update();
		geometryFilter->Update();

		pILTCap->DeepCopy(geometryFilter->GetOutput());
		fileName = dirOutputs + "\\" + ILT_CAP_NAME + ".stl";
		pdSTLWriter->SetFileName(fileName.c_str());
		pdSTLWriter->SetInputData(pILTCap);
		if (!pdSTLWriter->Write())
		{
			std::cerr << "Failed to write ILT cap surfaces to " << fileName.c_str() << std::endl;
			return EXIT_FAILURE;
		}

		// extract interior ILT surface
		selector->ThresholdBetween(INT_ILT_SURF_ID, INT_ILT_SURF_ID);
		selector->Update();
		geometryFilter->Update();
		pILTIntSurface->DeepCopy(geometryFilter->GetOutput());
		
		fileName = dirOutputs + "\\" + INT_ILT_SURF_NAME + ".stl";
		pdSTLWriter->SetFileName(fileName.c_str());
		pdSTLWriter->SetInputData(pILTIntSurface);
		if (!pdSTLWriter->Write())
		{
			std::cerr << "Failed to write internal ILT surface to " << fileName.c_str() << std::endl;
			return EXIT_FAILURE;
		}
	}

	// ------------------------------------------- //
	// Create MSH file
	// ------------------------------------------- //
	std::cout << "Writing .msh surfaces ..." << std::endl;
	vtkSmartPointer<vtkPolyData> pEdgesExtSurface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> pEdgesIntSurface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> pEdgesIltSurface = vtkSmartPointer<vtkPolyData>::New();

	// append all entities that go into the msh file
	vtkSmartPointer<vtkFeatureEdges> edgeExtractor = vtkSmartPointer<vtkFeatureEdges>::New();
	edgeExtractor->BoundaryEdgesOn();
	edgeExtractor->FeatureEdgesOff();
	edgeExtractor->ManifoldEdgesOff();
	edgeExtractor->NonManifoldEdgesOff();
	edgeExtractor->SetInputData(pWallExtSurf);
	edgeExtractor->Update();
	pEdgesExtSurface->DeepCopy(edgeExtractor->GetOutput());

	edgeExtractor->RemoveAllInputs();
	edgeExtractor->SetInputData(pWallIntSurf);
	edgeExtractor->Update();
	pEdgesIntSurface->DeepCopy(edgeExtractor->GetOutput());

	if (boCreateILT)
	{
		edgeExtractor->RemoveAllInputs();
		edgeExtractor->SetInputData(pILTIntSurface);
		edgeExtractor->Update();
		pEdgesIltSurface->DeepCopy(edgeExtractor->GetOutput());
	}

	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	appendFilter->RemoveAllInputs();
	appendFilter->AddInputData(pWallExtSurf);
	appendFilter->AddInputData(pWallIntSurf);
	appendFilter->AddInputData(pWallCap);
	if (boCreateILT)
	{
		appendFilter->AddInputData(pILTIntSurface);
		appendFilter->AddInputData(pILTCap);
		appendFilter->AddInputData(pEdgesIltSurface);
	}
	appendFilter->AddInputData(pEdgesExtSurface);
	appendFilter->AddInputData(pEdgesIntSurface);
	appendFilter->Update();

	cleaner->RemoveAllInputs();
	cleaner->SetAbsoluteTolerance(EPS/1000);
	cleaner->ConvertLinesToPointsOff();
	cleaner->ConvertPolysToLinesOff();
	cleaner->ConvertStripsToPolysOff();
	cleaner->SetInputData(appendFilter->GetOutput());
	cleaner->Update();

	// all nodes merged, create a locator for nodes
	vtkSmartPointer<vtkKdTreePointLocator> locatorAAANodes = vtkSmartPointer<vtkKdTreePointLocator>::New();
	locatorAAANodes->SetDataSet(cleaner->GetOutput());
	locatorAAANodes->BuildLocator();

	fileName = dirOutputs + "\\AAA.msh";
	pf = pfCreateMSHFile(fileName.c_str(), cleaner->GetOutput()->GetPoints());
	fprintf(pf, "$Elements\n");
	vtkIdType numMeshElements = pWallExtSurf->GetNumberOfPolys() + pWallCap->GetNumberOfPolys() +
		pWallIntSurf->GetNumberOfPolys() + pILTIntSurface->GetNumberOfPolys() +
		pILTCap->GetNumberOfPolys() + pEdgesIltSurface->GetNumberOfLines() +
		pEdgesExtSurface->GetNumberOfLines() + pEdgesIntSurface->GetNumberOfLines();
	fprintf(pf, "%d\n", numMeshElements);
	vtkIdType nextIndex = nAddMSHElements(pf, 1, 0, 1, pWallExtSurf, locatorAAANodes, VTK_TRIANGLE);
	nextIndex = nAddMSHElements(pf, nextIndex, 0, 2, pWallCap, locatorAAANodes, VTK_TRIANGLE);
	nextIndex = nAddMSHElements(pf, nextIndex, 0, 3, pWallIntSurf, locatorAAANodes, VTK_TRIANGLE);
	if (boCreateILT)
	{
		nextIndex = nAddMSHElements(pf, nextIndex, 0, 4, pILTIntSurface, locatorAAANodes, VTK_TRIANGLE);
		nextIndex = nAddMSHElements(pf, nextIndex, 0, 5, pILTCap, locatorAAANodes, VTK_TRIANGLE);
		nextIndex = nAddMSHElements(pf, nextIndex, 0, 6, pEdgesIltSurface, locatorAAANodes, VTK_LINE);
	}
	nextIndex = nAddMSHElements(pf, nextIndex, 0, 7, pEdgesIntSurface, locatorAAANodes, VTK_LINE);
	nextIndex = nAddMSHElements(pf, nextIndex, 0, 8, pEdgesExtSurface, locatorAAANodes, VTK_LINE);
	fprintf(pf, "$EndElements\n");
	fclose(pf);
	std::cout << "Done!" << std::endl;

	return EXIT_SUCCESS;
}
