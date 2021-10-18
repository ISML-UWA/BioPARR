// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkSmartPointer.h>

#include <vtkUnstructuredGridReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkPolyData.h>
#include <vtkPlane.h>
#include <vtkUnstructuredGrid.h>
#include <vtkClipPolyData.h>
#include <vtkPointData.h>
#include <vtkFeatureEdges.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkKdTreePointLocator.h>
#include <vtkMath.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkProbeFilter.h>
#include <vtkNew.h>
#include <vtkDoubleArray.h>
#include <vtkVertex.h>
#include <vtkCellArray.h>


#include <vtkCleanPolyData.h>

#include "AAA_AverageStressCLP.h"


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


int main(int argc, char * argv[])
{
	PARSE_ARGS;
	vtkDebugLeaks::SetExitError(true);

	// Check inputs
	if (stressMesh.size() == 0)
	{
		std::cerr << "ERROR: no input volume specified!" << std::endl;
		return EXIT_FAILURE;
	}

	if (stressSurface.size() == 0)
	{
		std::cerr << "ERROR: no output surface name specified!" << std::endl;
		return EXIT_FAILURE;
	}

	// Read inputs
	vtkSmartPointer<vtkUnstructuredGridReader> pdReader1 = vtkSmartPointer<vtkUnstructuredGridReader>::New();
	pdReader1->SetFileName(stressMesh.c_str());
	pdReader1->ReadAllScalarsOn();
	pdReader1->Update();
	if (pdReader1->GetErrorCode() != 0)
	{
		std::cerr << "ERROR: Failed to read input surface from " << stressMesh.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	if (pdReader1->GetOutput()->GetNumberOfPoints() == 0)
	{
		std::cerr << "ERROR: No points read from " << stressMesh.c_str() << std::endl;
		return EXIT_FAILURE;
	}

	// extract surface
	vtkSmartPointer<vtkGeometryFilter> exteriorAAASurface = vtkSmartPointer<vtkGeometryFilter>::New();
	exteriorAAASurface->SetInputConnection(pdReader1->GetOutputPort());
	exteriorAAASurface->Update();

	// two clipping filters are used to remove the caps at the upper and lower ends of the input surface
	vtkSmartPointer<vtkClipPolyData> upperClipper = vtkSmartPointer<vtkClipPolyData>::New();
	vtkSmartPointer<vtkClipPolyData> lowerClipper = vtkSmartPointer<vtkClipPolyData>::New();

	// find the bounds of data
	double boundsData[6];
	exteriorAAASurface->GetOutput()->ComputeBounds();
	exteriorAAASurface->GetOutput()->GetBounds(boundsData);
	double zMin, zMax, dz;
	zMin = boundsData[4];
	zMax = boundsData[5];
	dz = 3;

	vtkSmartPointer<vtkPlane> upperPlane = vtkSmartPointer<vtkPlane>::New();
	upperPlane->SetNormal(0, 0, -1);
	upperClipper->SetInputConnection(exteriorAAASurface->GetOutputPort());
	upperClipper->SetClipFunction(upperPlane);
	upperClipper->InsideOutOff();
	upperClipper->GenerateClippedOutputOn();
	upperPlane->SetOrigin(0, 0, zMax - dz);
	upperClipper->Update();

	// remove lower cap
	vtkSmartPointer<vtkPlane> lowerPlane = vtkSmartPointer<vtkPlane>::New();
	lowerPlane->SetNormal(0, 0, 1);
	lowerClipper->SetInputConnection(upperClipper->GetOutputPort());
	lowerClipper->SetClipFunction(lowerPlane);
	lowerClipper->InsideOutOff();
	lowerClipper->GenerateClippedOutputOn();
	lowerPlane->SetOrigin(0, 0, zMin + dz);
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

	vtkSmartPointer<vtkCleanPolyData> cleaner1 = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner1->SetInputConnection(connectivityFilter->GetOutputPort());
	cleaner1->Update();

	vtkSmartPointer<vtkPolyData> Surface1 = vtkSmartPointer<vtkPolyData>::New();
	Surface1->ShallowCopy(cleaner1->GetOutput());

	vtkSmartPointer<vtkCleanPolyData> cleaner2 = vtkSmartPointer<vtkCleanPolyData>::New();
	vtkSmartPointer<vtkPolyData> Surface2 = vtkSmartPointer<vtkPolyData>::New();

	connectivityFilter->DeleteSpecifiedRegion(0);
	connectivityFilter->AddSpecifiedRegion(1);
	connectivityFilter->Update();

	cleaner2->SetInputConnection(connectivityFilter->GetOutputPort());
	cleaner2->Update();

	Surface2->ShallowCopy(cleaner2->GetOutput());

	// check which one is the interior surface
	double boundsData1[6];
	Surface1->ComputeBounds();
	Surface1->GetBounds(boundsData);

	Surface2->ComputeBounds();
	Surface2->GetBounds(boundsData1);

	vtkSmartPointer<vtkPolyData> OuterSurface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> InnerSurface = vtkSmartPointer<vtkPolyData>::New();

	if ((boundsData[1] - boundsData[0]) < boundsData1[1] - boundsData1[0])
	{
		//outer surface is surface2
		OuterSurface->ShallowCopy(cleaner2->GetOutput());
		InnerSurface->ShallowCopy(cleaner1->GetOutput());
	}
	else
	{
		//outer surface is surface1
		OuterSurface->ShallowCopy(cleaner1->GetOutput());
		InnerSurface->ShallowCopy(cleaner2->GetOutput());
	}

	// Build locator
	vtkSmartPointer<vtkKdTreePointLocator> kDTree1 = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTree1->SetDataSet(InnerSurface);
	kDTree1->BuildLocator();

	// Find closest points on inner surface to test points on outer surface

	double testPoint[3];
	double closestPoint[3];
	double gauss1[3];
	double gauss2[3];
	double Mk;
	double Mk1;
	double t = (1 - sqrt(1.0 / 3)) / 2;
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkIdType numPoints = OuterSurface->GetNumberOfPoints();
	for (vtkIdType i = 0; i < numPoints; i++) //
	{
		OuterSurface->GetPoint(i, testPoint);

		// get ID of closest point
		vtkIdType idClosestPoint = kDTree1->FindClosestPoint(testPoint);
		// get location of closest point
		kDTree1->GetDataSet()->GetPoint(idClosestPoint, closestPoint);

		for (int k = 0; k < iNumSegments; k++)
		{
			for (int coor = 0; coor < 3; coor++)
			{
				Mk = closestPoint[coor]*(double)k/iNumSegments + testPoint[coor]*(1-(double)k/iNumSegments);
				Mk1 = closestPoint[coor]*(k+1.0)/iNumSegments + testPoint[coor]*(1-(k+1.0)/iNumSegments);

				gauss1[coor] = t*Mk1 + (1-t)*Mk;
				gauss2[coor] = t*Mk + (1-t)*Mk1;
			}
			points->InsertNextPoint(gauss1);
			points->InsertNextPoint(gauss2);
		}
	}

	vtkSmartPointer<vtkPolyData> gaussPoly = vtkSmartPointer<vtkPolyData>::New();
	gaussPoly->SetPoints(points);
	
	// Interpolate stress values at Gauss points
	vtkSmartPointer<vtkProbeFilter> prober = vtkSmartPointer<vtkProbeFilter>::New();
	prober->SetSourceConnection(pdReader1->GetOutputPort());
	prober->SetInputData(gaussPoly);
	prober->Update();

	vtkPointData *pPointData = prober->GetOutput()->GetPointData();
	pPointData->SetActiveScalars("S:Mises");
	vtkDoubleArray *pMisses = vtkDoubleArray::SafeDownCast(pPointData->GetScalars());
	if (!pMisses) {
		std::cerr << "ERROR: Scalars S:Misses not found in input file! " << std::endl;
	} else {
	 
		//Perform integration and add to OuterSurface
		vtkSmartPointer<vtkDoubleArray> AverageMissesStress = vtkSmartPointer<vtkDoubleArray>::New();
		AverageMissesStress->SetName("S:MisesAvg");
		AverageMissesStress->SetNumberOfValues(numPoints);

		for (vtkIdType n = 0; n < numPoints; n++)
		{
			double avMissesStress = 0;
			for (int m = 0; m < iNumSegments; m++)
			{
				avMissesStress += pMisses->GetValue(n*iNumSegments*2 + m*2) + pMisses->GetValue(n*iNumSegments*2 + m*2 + 1);
			}
			avMissesStress /= (2.0*iNumSegments);
			AverageMissesStress->SetValue(n, avMissesStress);
		} 
		OuterSurface->GetPointData()->SetActiveScalars("S:MisesAvg");
		OuterSurface->GetPointData()->SetScalars(AverageMissesStress);
	}

	pPointData->SetActiveScalars("S:MaxPrincipal");
	vtkDoubleArray *pPrincipalStress = vtkDoubleArray::SafeDownCast(pPointData->GetScalars());
	if (!pMisses) {
		std::cerr << "ERROR: Scalars S:MaxPrincipal not found in input file! " << std::endl;
		return EXIT_FAILURE;
	}

	vtkSmartPointer<vtkDoubleArray> AveragePrincipalStress = vtkSmartPointer<vtkDoubleArray>::New();
	AveragePrincipalStress->SetName("S:MaxPrincipal");
	AveragePrincipalStress->SetNumberOfValues(numPoints);

	for (vtkIdType n = 0; n < numPoints; n++)
	{
		double avPrincipalStress = 0;
		for (int m = 0; m < iNumSegments; m++)
		{
			avPrincipalStress += pPrincipalStress->GetValue(n*iNumSegments*2 + m*2) + pPrincipalStress->GetValue(n*iNumSegments*2 + m*2 + 1);
		}
		avPrincipalStress /= (2.0*iNumSegments);
		AveragePrincipalStress->SetValue(n, avPrincipalStress);
	} 

	OuterSurface->GetPointData()->SetActiveScalars("S:MaxPrincipal");
	OuterSurface->GetPointData()->SetScalars(AveragePrincipalStress);

	iWriteSurfToFile(stressSurface.c_str(), OuterSurface);

	return 0;
}

