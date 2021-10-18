// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkSmartPointer.h>

#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPolyData.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkPointData.h>
#include <vtkFeatureEdges.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkFeatureEdges.h>
#include <vtkIdTypeArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkKdTreePointLocator.h>
#include <vtkPolygon.h>

#include <vtkCleanPolyData.h>


#include "AAA_ExtractSurfacesCLP.h"
#include "AAA_defines.h"

int iWriteSurfToFile(const char* fileName, vtkDataObject *pvData)
{
	// Write resulting surface
	vtkSmartPointer<vtkXMLPolyDataWriter> pdWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	pdWriter->SetFileName(fileName);
	pdWriter->SetDataModeToBinary();
	pdWriter->SetCompressorTypeToZLib();
	pdWriter->SetInputData(pvData);
	if (!pdWriter->Write())
	{
		std::cerr << "Failed to write interior surface to " << fileName << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void vExtendSurface(vtkSmartPointer<vtkPolyData> pSurface, float fLength)
{
	// Detect edges
	vtkSmartPointer<vtkFeatureEdges> edgeExtractor = vtkSmartPointer<vtkFeatureEdges>::New();
	edgeExtractor->BoundaryEdgesOn();
	edgeExtractor->FeatureEdgesOff();
	edgeExtractor->ManifoldEdgesOff();
	edgeExtractor->NonManifoldEdgesOff();
	edgeExtractor->SetColoring(0);
	edgeExtractor->SetInputData(pSurface);
	edgeExtractor->Update();

	vtkSmartPointer<vtkPolyData> boundaryEdges = vtkSmartPointer<vtkPolyData>::New();
	boundaryEdges = edgeExtractor->GetOutput();

	// Find center of the surface
	double boundsData[6];
	pSurface->ComputeBounds();
	pSurface->GetBounds(boundsData);
	double zC = (boundsData[4] + boundsData[5])/2;
	
	// Number of nodes and segments on the boundary
	vtkIdType numPoints = boundaryEdges->GetNumberOfPoints();
	vtkIdType numEdges = boundaryEdges->GetNumberOfCells();

	// Create new points and map them to the edge points
	vtkSmartPointer<vtkIdTypeArray> aIdxNewPoints = vtkSmartPointer<vtkIdTypeArray>::New();
	aIdxNewPoints->SetNumberOfValues(numPoints);
	for (vtkIdType i = 0; i < numPoints; i++)
	{
		double *pBoundaryPoint = boundaryEdges->GetPoint(i);
		vtkIdType idxPoint;
		if (pBoundaryPoint[2] > zC)	{
			idxPoint = pSurface->GetPoints()->InsertNextPoint(pBoundaryPoint[0], pBoundaryPoint[1], pBoundaryPoint[2]+fLength);
		} else {
			idxPoint = pSurface->GetPoints()->InsertNextPoint(pBoundaryPoint[0], pBoundaryPoint[1], pBoundaryPoint[2]-fLength);
		}
		aIdxNewPoints->SetValue(i, idxPoint);
	}

	vtkSmartPointer<vtkKdTreePointLocator> kDTreeLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTreeLocator->SetDataSet(pSurface);
	kDTreeLocator->BuildLocator();
	// add rectangles on the boundary
	for (vtkIdType i = 0; i < numEdges; i++)
	{
		double pP0[3], pP1[3];
		vtkIdType idxP0, idxP1;
		vtkIdType idxSurfP0, idxSurfP1;
		idxP0 = boundaryEdges->GetCell(i)->GetPointId(0);
		idxP1 = boundaryEdges->GetCell(i)->GetPointId(1);
		// identify corresponding points in the original surface
		boundaryEdges->GetPoint(idxP0, pP0);
		boundaryEdges->GetPoint(idxP1, pP1);
		idxSurfP0 = kDTreeLocator->FindClosestPoint(pP0);
		idxSurfP1 = kDTreeLocator->FindClosestPoint(pP1);
		// create a new rectangular cell
		// Create the rectangle
		vtkSmartPointer<vtkPolygon> rectangle = vtkSmartPointer<vtkPolygon>::New();
		rectangle->GetPointIds()->SetNumberOfIds(4); //make a quad
		rectangle->GetPointIds()->SetId(0, idxSurfP1);
		rectangle->GetPointIds()->SetId(1, idxSurfP0);
		rectangle->GetPointIds()->SetId(2, aIdxNewPoints->GetValue(idxP0));
		rectangle->GetPointIds()->SetId(3, aIdxNewPoints->GetValue(idxP1)); 
		pSurface->GetPolys()->InsertNextCell(rectangle);
	}
}

int main( int argc, char * argv[] )
{
	PARSE_ARGS;
	vtkDebugLeaks::SetExitError(true);

	// Check inputs
	if (AAA_Surface_model.size() == 0)
	{
		std::cerr << "ERROR: no input surface specified!" << std::endl;
		return EXIT_FAILURE;
	}

	if (bloodSurface.size() == 0) 
	{
		std::cerr << "ERROR: no blood surface name specified!" << std::endl;
		return EXIT_FAILURE;
	}

	if (externalSurface.size() == 0) 
	{
		std::cerr << "ERROR: no external surface name specified!" << std::endl;
		return EXIT_FAILURE;
	}
	
	// Read inputs
	vtkSmartPointer<vtkXMLPolyDataReader> pdReader1 = vtkSmartPointer<vtkXMLPolyDataReader>::New();
	pdReader1->SetFileName(AAA_Surface_model.c_str() );
	pdReader1->Update();
	if( pdReader1->GetErrorCode() != 0 )
    {
		std::cerr << "ERROR: Failed to read input surface from " << AAA_Surface_model.c_str() << std::endl;
		return EXIT_FAILURE;
    }

	if (pdReader1->GetOutput()->GetNumberOfPoints() == 0)
	{
		std::cerr << "ERROR: No points read from " << AAA_Surface_model.c_str() << std::endl;
		return EXIT_FAILURE;
    }

	// remove normals
	pdReader1->GetOutput()->GetPointData()->RemoveArray("Normals");
	
	// two clipping filters are used to remove the caps at the upper and lower ends of the input surface
	vtkSmartPointer<vtkClipPolyData> upperClipper = vtkSmartPointer<vtkClipPolyData>::New();
	vtkSmartPointer<vtkClipPolyData> lowerClipper = vtkSmartPointer<vtkClipPolyData>::New();

	// find the bounds of data
	double boundsData[6];
	pdReader1->GetOutput()->ComputeBounds();
	pdReader1->GetOutput()->GetBounds(boundsData);
	double zMin, zMax, dz;
	zMin = boundsData[4];
	zMax = boundsData[5];
	dz = 2;
	
	vtkSmartPointer<vtkPlane> upperPlane = vtkSmartPointer<vtkPlane>::New();
	upperPlane->SetNormal(0,0,-1);
	upperClipper->SetInputConnection(pdReader1->GetOutputPort());
	upperClipper->SetClipFunction(upperPlane);
	upperClipper->InsideOutOff();
	upperClipper->GenerateClippedOutputOn();
	upperPlane->SetOrigin(0,0,zMax - dz);
	upperClipper->Update();

	// remove lower cap
	vtkSmartPointer<vtkPlane> lowerPlane = vtkSmartPointer<vtkPlane>::New();
	lowerPlane->SetNormal(0,0,1);
	lowerClipper->SetInputConnection(upperClipper->GetOutputPort());
	lowerClipper->SetClipFunction(lowerPlane);
	lowerClipper->InsideOutOff();
	lowerClipper->GenerateClippedOutputOn();
	lowerPlane->SetOrigin(0,0,zMin + dz);
	lowerClipper->Update();

	// Separate the 2 surfaces
	vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter =
		vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
	connectivityFilter->SetExtractionModeToAllRegions();
	connectivityFilter->ColorRegionsOn();
	connectivityFilter->SetInputConnection(lowerClipper->GetOutputPort());
	connectivityFilter->SetExtractionModeToSpecifiedRegions();
	connectivityFilter->AddSpecifiedRegion(0);
	connectivityFilter->ColorRegionsOff();
	connectivityFilter->Update();

	int iNumSurfs = connectivityFilter->GetNumberOfExtractedRegions();

	vtkSmartPointer<vtkCleanPolyData> cleaner1 = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner1->SetInputConnection(connectivityFilter->GetOutputPort());
	cleaner1->Update();
	
	vtkSmartPointer<vtkPolyData> Surface1 = vtkSmartPointer<vtkPolyData>::New();
	Surface1->DeepCopy(cleaner1->GetOutput());
	vExtendSurface(Surface1, 3);

	vtkSmartPointer<vtkCleanPolyData> cleaner2 = vtkSmartPointer<vtkCleanPolyData>::New();
	vtkSmartPointer<vtkPolyData> Surface2 = vtkSmartPointer<vtkPolyData>::New();
	int res = EXIT_SUCCESS;
	if (iNumSurfs > 1)
	{
		connectivityFilter->DeleteSpecifiedRegion(0);
		connectivityFilter->AddSpecifiedRegion(1);
		connectivityFilter->Update();

		cleaner2->SetInputConnection(connectivityFilter->GetOutputPort());
		cleaner2->Update();

		Surface2->DeepCopy(cleaner2->GetOutput());
		vExtendSurface(Surface2, 3);

		// check which one is the interior surface
		double boundsData1[6];
		Surface1->ComputeBounds();
		Surface1->GetBounds(boundsData);

		Surface2->ComputeBounds();
		Surface2->GetBounds(boundsData1);

		if ((boundsData[1] - boundsData[0]) < boundsData1[1] - boundsData1[0]) 
		{
			res = iWriteSurfToFile(bloodSurface.c_str(), Surface1);
			if (res != EXIT_SUCCESS) return res;
			res = iWriteSurfToFile(externalSurface.c_str(), Surface2);
			return res;
		}
		else
		{
			res = iWriteSurfToFile(bloodSurface.c_str(), Surface2);
			if (res != EXIT_SUCCESS) return res;
			res = iWriteSurfToFile(externalSurface.c_str(), Surface1);
			return res;
		}
	}
	else
	{
		res = iWriteSurfToFile(externalSurface.c_str(), Surface1);
		return res;
	}
	return EXIT_SUCCESS;
}
