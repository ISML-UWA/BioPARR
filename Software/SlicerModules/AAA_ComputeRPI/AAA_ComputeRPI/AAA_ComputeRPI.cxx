#include <stdio.h>

// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkSmartPointer.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkCleanPolyData.h>
#include <vtkKdTreePointLocator.h>
#include <vtkThreshold.h>
#include <vtkGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCardinalSpline.h>
#include <vtkClipPolyData.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataWriter.h>

#include "AAA_ComputeRPICLP.h"
#include "AAA_defines.h"

#define EPS 1.0

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

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

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
	PARSE_ARGS;
	vtkDebugLeaks::SetExitError(true);

	// Check inputs
	if (wallAAA.size() == 0)
	{
		std::cerr << "ERROR: no AAA wall specified!" << std::endl;
		return EXIT_FAILURE;
	}
	if (ILTSurface.size() == 0) 
	{
		std::cerr << "ERROR: no ILT surface specified!" << std::endl;
		return EXIT_FAILURE;
	}
	if (AAASurface.size() == 0)
	{
		std::cerr << "ERROR: no output surface specified!" << std::endl;
		return EXIT_FAILURE;
	}
	bool boVolumeInput = true;
	std::string::size_type pos = wallAAA.find_last_of(".");
	if (pos == std::string::npos) {
		std::cerr << "WARNING: extension of the AAA wall input missing! Will try to read anyway." << std::endl;
	} else {
		std::string inputFileExt = wallAAA.substr(pos+1);
		if (inputFileExt == "vtp") boVolumeInput = false; 
	}

	std::cout << "Reading inputs ..." << std::endl;
	// Read inputs
	vtkSmartPointer<vtkXMLPolyDataReader> pdReaderILT = vtkSmartPointer<vtkXMLPolyDataReader>::New();
	pdReaderILT->SetFileName(ILTSurface.c_str() );
	pdReaderILT->Update();
	if( pdReaderILT->GetErrorCode() != 0 )
    {
		std::cerr << "ERROR: Failed to read ILT surface from " << ILTSurface.c_str() << std::endl;
		return EXIT_FAILURE;
    }

	vtkSmartPointer<vtkPolyData> SurfaceAAA = vtkSmartPointer<vtkPolyData>::New();

	if (boVolumeInput) {
		vtkSmartPointer<vtkUnstructuredGridReader> pdReaderAAAStress = vtkSmartPointer<vtkUnstructuredGridReader>::New();
		pdReaderAAAStress->SetFileName(wallAAA.c_str() );
		pdReaderAAAStress->ReadAllScalarsOn();
		pdReaderAAAStress->Update();
		if( pdReaderAAAStress->GetErrorCode() != 0 )
		{
			std::cerr << "ERROR: Failed to read AAA wall as volume from " << wallAAA.c_str() << std::endl;
			boVolumeInput = false;
		}
		std::cout << "Extracting surfaces ..." << std::endl;
		// Extract AAA wall surface
		vtkSmartPointer<vtkDataSetSurfaceFilter> surfFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
		surfFilter->SetInputConnection(pdReaderAAAStress->GetOutputPort());
		surfFilter->Update();
		// trim surface to eliminate end effects

		// two clipping filters are used to remove the caps at the upper and lower ends of the input surface
		vtkSmartPointer<vtkClipPolyData> upperClipper = vtkSmartPointer<vtkClipPolyData>::New();
		vtkSmartPointer<vtkClipPolyData> lowerClipper = vtkSmartPointer<vtkClipPolyData>::New();

		// find the bounds of data
		double boundsData[6];
		surfFilter->GetOutput()->ComputeBounds();
		surfFilter->GetOutput()->GetBounds(boundsData);
		double zMin, zMax, dz;
		zMin = boundsData[4];
		zMax = boundsData[5];
		dz = 3;

		vtkSmartPointer<vtkPlane> upperPlane = vtkSmartPointer<vtkPlane>::New();
		upperPlane->SetNormal(0, 0, -1);
		upperClipper->SetInputConnection(surfFilter->GetOutputPort());
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

		vtkSmartPointer<vtkCleanPolyData> cleaner1 = vtkSmartPointer<vtkCleanPolyData>::New();
		cleaner1->SetInputConnection(lowerClipper->GetOutputPort());
		cleaner1->Update();

		SurfaceAAA->DeepCopy(cleaner1->GetOutput());
	}
	// not a volume input or error reading volume input; try surface input
	if (boVolumeInput == false) {
		vtkSmartPointer<vtkXMLPolyDataReader> pdReaderAAAStress = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		pdReaderAAAStress->SetFileName(wallAAA.c_str() );
		pdReaderAAAStress->Update();
		if( pdReaderAAAStress->GetErrorCode() != 0 )
		{
			std::cerr << "ERROR: Failed to read AAA wall surface from " << wallAAA.c_str() << std::endl;
			return EXIT_FAILURE;
		}
		SurfaceAAA->DeepCopy(pdReaderAAAStress->GetOutput());
	}

	// Extract interior ILT surface
	vtkSmartPointer<vtkThreshold> selector = vtkSmartPointer<vtkThreshold>::New();
	selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, SURF_ID_NAME);
	selector->ThresholdBetween(INT_ILT_SURF_ID, INT_ILT_SURF_ID);
	selector->SetInputData(pdReaderILT->GetOutput());
	selector->Update();

	vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
	geometryFilter->SetInputConnection(selector->GetOutputPort());
	geometryFilter->Update();

	vtkSmartPointer<vtkPolyData> interiorILTSurface = vtkSmartPointer<vtkPolyData>::New();
	interiorILTSurface->DeepCopy(geometryFilter->GetOutput());

	std::cout << "Computing diameters ..." << std::endl;
	vtkSmartPointer<vtkCutter> zCutter = vtkSmartPointer<vtkCutter>::New();
	vtkSmartPointer<vtkPlane> zPlane = vtkSmartPointer<vtkPlane>::New();
	vtkSmartPointer<vtkCardinalSpline> DiameterSpline = vtkSmartPointer<vtkCardinalSpline>::New();
	zCutter->SetCutFunction(zPlane);
	zCutter->SetInputData(pdReaderILT->GetOutput());
	zPlane->SetNormal(0,0,1);
	double boundsData[6];
	double z, zMin, zMax;
	double d;
	SurfaceAAA->ComputeBounds();
	SurfaceAAA->GetBounds(boundsData);
	zMin = boundsData[4];
	zMax = boundsData[5];
	z = zMax-1;
	while (z > (zMin + 1))
	{
		// compute diameter of a cut through AAA
		zPlane->SetOrigin(0,0,z);
		zCutter->Update();
		zCutter->GetOutput()->ComputeBounds();
		zCutter->GetOutput()->GetBounds(boundsData);
		d = (boundsData[1] - boundsData[0] + boundsData[3] - boundsData[2])/2;
		DiameterSpline->AddPoint(z, d);
		z -= 1;
	}
	d = DiameterSpline->Evaluate(z+1);
	DiameterSpline->AddPoint(zMin, d);
	d = DiameterSpline->Evaluate(zMax-1);
	DiameterSpline->AddPoint(zMax, d);
	double diamAAA = d;

	std::cout << "Computing RPI ..." << std::endl;
	vtkIdType numPoints = SurfaceAAA->GetNumberOfPoints();
	vtkSmartPointer<vtkDoubleArray> NORD = vtkSmartPointer<vtkDoubleArray>::New();
    NORD->SetName("NORD");
	NORD->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> ILT = vtkSmartPointer<vtkDoubleArray>::New();
    ILT->SetName("ILT"); // thickness of ILT
	ILT->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> strengthMnoH = vtkSmartPointer<vtkDoubleArray>::New();
    strengthMnoH->SetName("strengthMnoH"); // strength for a male with no history
	strengthMnoH->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> strengthMH = vtkSmartPointer<vtkDoubleArray>::New();
    strengthMH->SetName("strengthMwithH"); // strength for a male with history
	strengthMH->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> strengthFnoH = vtkSmartPointer<vtkDoubleArray>::New();
    strengthFnoH->SetName("strengthFnoH"); // strength for a female with no history
	strengthFnoH->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> strengthFH = vtkSmartPointer<vtkDoubleArray>::New();
    strengthFH->SetName("strengthFwithH"); // strength for a female with history
	strengthFH->SetNumberOfValues(numPoints);

	vtkSmartPointer<vtkDoubleArray> RPIMnoH = vtkSmartPointer<vtkDoubleArray>::New();
    RPIMnoH->SetName("RPIMnoH"); // RPI for a male with no history
	RPIMnoH->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> RPIMH = vtkSmartPointer<vtkDoubleArray>::New();
    RPIMH->SetName("RPIMwithH"); // RPI for a male with history
	RPIMH->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> RPIFnoH = vtkSmartPointer<vtkDoubleArray>::New();
    RPIFnoH->SetName("RPIFnoH"); // RPI for a female with no history
	RPIFnoH->SetNumberOfValues(numPoints);
	vtkSmartPointer<vtkDoubleArray> RPIFH = vtkSmartPointer<vtkDoubleArray>::New();
    RPIFH->SetName("RPIFwithH"); // RPI for a female with history
	RPIFH->SetNumberOfValues(numPoints);

	// build locator for closest point on internal ILT surface
	vtkSmartPointer<vtkKdTreePointLocator> kDTree = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTree->SetDataSet(interiorILTSurface);
	kDTree->BuildLocator();
	vtkSmartPointer<vtkDoubleArray> arrayThickness = vtkDoubleArray::SafeDownCast(interiorILTSurface->GetPointData()->GetArray(THICKNESS_NAME));
	if( arrayThickness == NULL )
	{
		std::cerr << "ERROR: Missing ILT thickness data!" << std::endl;
		return EXIT_FAILURE;
	}
	vtkSmartPointer<vtkDoubleArray> arrayStress = vtkDoubleArray::SafeDownCast(SurfaceAAA->GetPointData()->GetArray("S:MaxPrincipal"));
	if( arrayStress == NULL )
	{
		std::cerr << "ERROR: Missing Maximum Principal Stress!" << std::endl;
		return EXIT_FAILURE;
	}
	double pPoint[3];
	for (vtkIdType i = 0; i < numPoints; i++)
	{
		SurfaceAAA->GetPoint(i, pPoint);
		double zP = pPoint[2];
		d = DiameterSpline->Evaluate(zP);
		double nordia = d/diamAAA;
		NORD->SetValue(i, nordia);
		vtkIdType k = kDTree->FindClosestPoint(pPoint);
		double t = arrayThickness->GetValue(k);
		ILT->SetValue(i, t);
		double stress = arrayStress->GetValue(i);
		if (nordia > 3.9) nordia = 3.9; // constrain values
		if (t > 3.6) t = 3.6;
		double strength = 71.9 - 37.9*(sqrt(t)-0.81)-15.6*(d/diamAAA-2.46);
		double sex = 0.5; // male
		double hist = 0.5; // with history
		strengthMH->SetValue(i, (strength - 21.3*hist + 19.3*sex)/100);
		hist = -0.5; // no history
		strengthMnoH->SetValue(i, (strength - 21.3*hist + 19.3*sex)/100);
		sex = -0.5; // female
		strengthFnoH->SetValue(i, (strength - 21.3*hist + 19.3*sex)/100);
		hist = 0.5; // with history
		strengthFH->SetValue(i, (strength - 21.3*hist + 19.3*sex)/100);
		RPIMH->SetValue(i, stress/strengthMH->GetValue(i));
		RPIMnoH->SetValue(i, stress/strengthMnoH->GetValue(i));
		RPIFH->SetValue(i, stress/strengthFH->GetValue(i));
		RPIFnoH->SetValue(i, stress/strengthFnoH->GetValue(i));
	}
	// Add point data
	SurfaceAAA->GetPointData()->SetActiveScalars("NORD");
	SurfaceAAA->GetPointData()->SetScalars(NORD);
	SurfaceAAA->GetPointData()->SetActiveScalars("ILT");
	SurfaceAAA->GetPointData()->SetScalars(ILT);

	SurfaceAAA->GetPointData()->SetActiveScalars("strengthMnoH");
	SurfaceAAA->GetPointData()->SetScalars(strengthMnoH);
	SurfaceAAA->GetPointData()->SetActiveScalars("strengthMwithH");
	SurfaceAAA->GetPointData()->SetScalars(strengthMH);
	SurfaceAAA->GetPointData()->SetActiveScalars("strengthFnoH");
	SurfaceAAA->GetPointData()->SetScalars(strengthFnoH);
	SurfaceAAA->GetPointData()->SetActiveScalars("strengthFwithH");
	SurfaceAAA->GetPointData()->SetScalars(strengthFH);

	SurfaceAAA->GetPointData()->SetActiveScalars("RPIMnoH");
	SurfaceAAA->GetPointData()->SetScalars(RPIMnoH);
	SurfaceAAA->GetPointData()->SetActiveScalars("RPIMwithH");
	SurfaceAAA->GetPointData()->SetScalars(RPIMH);
	SurfaceAAA->GetPointData()->SetActiveScalars("RPIFnoH");
	SurfaceAAA->GetPointData()->SetScalars(RPIFnoH);
	SurfaceAAA->GetPointData()->SetActiveScalars("RPIFwithH");
	SurfaceAAA->GetPointData()->SetScalars(RPIFH);

	std::cout << "Writing results ..." << std::endl;
	return iWriteOutput(AAASurface.c_str(), SurfaceAAA);
}
