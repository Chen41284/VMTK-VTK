#include "vtkvmtkMarchingCubes.h"

//VTK 
#include <vtkImageTranslateExtent.h>
#include <vtkPointData.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataConnectivityFilter.h>

vtkStandardNewMacro(vtkvmtkMarchingCubes);

vtkvmtkMarchingCubes::vtkvmtkMarchingCubes()
{
	this->Image = vtkImageData::New();
	this->Surface = vtkPolyData::New();
	this->ArrayName = NULL;
	this->Level = 0.0;
	this->Connectivity = false;
}

vtkvmtkMarchingCubes::~vtkvmtkMarchingCubes()
{
	this->Image->Delete();
	this->Image = NULL;
	this->Surface->Delete();
	this->Surface = NULL;
	if (this->ArrayName != NULL)
	{
		delete this->ArrayName;
		this->ArrayName = NULL;
	}
}

void vtkvmtkMarchingCubes::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image MarchingCubes\n";
}

void vtkvmtkMarchingCubes::Execute()
{
	//Not Input Image, All Dimension is zero
	if (this->Image->GetDimensions()[0] == 0 &&
		this->Image->GetDimensions()[1] == 0 &&
		this->Image->GetDimensions()[2] == 0)
	{
		std::cerr << "Error: no Image." << std::endl;
	}
	int *extent = this->Image->GetExtent();
	vtkSmartPointer<vtkImageTranslateExtent> translateExtent = 
		vtkSmartPointer<vtkImageTranslateExtent>::New();
	translateExtent->SetInputData(this->Image);
	translateExtent->SetTranslation(-extent[0], -extent[2], -extent[4]);
	translateExtent->Update();

	if (this->ArrayName != NULL)
		translateExtent->GetOutput()->GetPointData()->SetActiveScalars(this->ArrayName);

	vtkSmartPointer<vtkMarchingCubes> marchingCubes = 
		vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputConnection(translateExtent->GetOutputPort());
	marchingCubes->SetValue(0, this->Level);
	marchingCubes->Update();

	this->Surface->DeepCopy(marchingCubes->GetOutput());
	if (this->Connectivity == 1)
	{
		vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = 
			vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
		connectivityFilter->SetInputData(this->Surface);
		connectivityFilter->SetExtractionModeToLargestRegion();
		connectivityFilter->Update();
		this->Surface->DeepCopy(connectivityFilter->GetOutput());
	}	
}