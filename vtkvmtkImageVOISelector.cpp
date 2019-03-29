#include "vtkvmtkImageVOISelector.h"

// VTK includes
#include <vtkPolyData.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

//VMTK-Src

//VTK-VMTK

vtkStandardNewMacro(vtkvmtkImageVOISelector);

vtkvmtkImageVOISelector::vtkvmtkImageVOISelector()
{
	this->CubeSource = vtkCubeSource::New();
	this->CubeActor = vtkActor::New();
	this->BoxActive = false;
	for (int i = 0; i < 6; i++)
		this->BoxBounds[i] = 0.0;
	this->CroppedImage = vtkImageData::New();
	this->vmtkRenderer = NULL;
	this->OwnRenderer = false;
	this->PlaneWidgetX = NULL;
	this->PlaneWidgetY = NULL;
	this->PlaneWidgetZ = NULL;
	this->BoxWidget = NULL;
	this->Image = vtkImageData::New();
	this->Interactive = true;
}

vtkvmtkImageVOISelector::~vtkvmtkImageVOISelector()
{
	this->CubeSource->Delete();
	this->CubeSource = NULL;
	this->CubeActor->Delete();
	this->CubeActor = NULL;
	this->CroppedImage->Delete();
	this->CroppedImage = NULL;
	this->Image->Delete();
	this->Image = NULL;
	if (this->PlaneWidgetX != NULL)
	{
		this->PlaneWidgetX->Delete();
		this->PlaneWidgetX = NULL;
	}
	if (this->PlaneWidgetY != NULL)
	{
		this->PlaneWidgetY->Delete();
		this->PlaneWidgetY = NULL;
	}
	if (this->PlaneWidgetZ != NULL)
	{
		this->PlaneWidgetZ->Delete();
		this->PlaneWidgetZ = NULL;
	}
}

void vtkvmtkImageVOISelector::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Selector\n";
}

void vtkvmtkImageVOISelector::HideCube(vtkObject * caller, long unsigned int evId, void* clientData, void* callData)
{
	this->CubeActor->VisibilityOff();
}

void vtkvmtkImageVOISelector::UpdateCube(vtkObject * caller, long unsigned int evId, void* clientData, void* callData)
{
	vtkSmartPointer<vtkPolyData> polyData =
		vtkSmartPointer<vtkPolyData>::New();
	vtkBoxWidget *object = static_cast<vtkBoxWidget*>(caller);
	object->GetPolyData(polyData);
	polyData->ComputeBounds();
	this->CubeSource->SetBounds(polyData->GetBounds());
	this->CubeSource->Modified();
	this->CubeActor->VisibilityOn();
}