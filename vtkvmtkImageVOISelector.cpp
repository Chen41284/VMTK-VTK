#include "vtkvmtkImageVOISelector.h"

// VTK includes
#include <vtkPolyData.h>
#include <vtkCellPicker.h>
#include <vtkCallbackCommand.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkExtractVOI.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>

//VMTK-Src
#include <vtkvmtkImagePlaneWidget.h>

//VTK-VMTK
#include "vtkvmtkImageRender.h"

vtkStandardNewMacro(vtkvmtkImageVOISelector);

void HideCubeStatic(vtkObject * caller, long unsigned int evId, void* clientData, void* callData)
{
	vtkvmtkImageVOISelector *selector = static_cast<vtkvmtkImageVOISelector*>(clientData);
	selector->HideCube();
}

void UpdateCubeStatic(vtkObject * caller, long unsigned int evId, void* clientData, void* callData)
{
	vtkBoxWidget *boxWidget = static_cast<vtkBoxWidget*>(caller);
	vtkvmtkImageVOISelector *selector = static_cast<vtkvmtkImageVOISelector*>(clientData);
	selector->UpdateCube(boxWidget);
}

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

void vtkvmtkImageVOISelector::HideCube()
{
	this->CubeActor->VisibilityOff();
}

void vtkvmtkImageVOISelector::UpdateCube(vtkBoxWidget* caller)
{
	vtkSmartPointer<vtkPolyData> polyData =
		vtkSmartPointer<vtkPolyData>::New();
	vtkBoxWidget *object = caller;
	object->GetPolyData(polyData);
	polyData->ComputeBounds();
	this->CubeSource->SetBounds(polyData->GetBounds());
	this->CubeSource->Modified();
	this->CubeActor->VisibilityOn();
}


//Ñ­»·Ö´ÐÐ
void vtkvmtkImageVOISelector::Display()
{
	int *wholeExtent = this->Image->GetExtent();
	vtkSmartPointer<vtkCellPicker> picker = 
		vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);
	this->PlaneWidgetX->SetInputData(this->Image);
	this->PlaneWidgetX->SetPlaneOrientationToXAxes();
	this->PlaneWidgetX->SetSliceIndex(wholeExtent[0]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetX->DisplayTextOn();
	else
		this->PlaneWidgetX->DisplayTextOff();
	this->PlaneWidgetX->SetPicker(picker);
	this->PlaneWidgetX->KeyPressActivationOff();
	this->PlaneWidgetX->On();

	this->PlaneWidgetY->SetInputData(this->Image);
	this->PlaneWidgetY->SetPlaneOrientationToYAxes();
	this->PlaneWidgetY->SetSliceIndex(wholeExtent[2]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetY->DisplayTextOn();
	else
		this->PlaneWidgetY->DisplayTextOff();
	this->PlaneWidgetY->SetPicker(picker);
	this->PlaneWidgetY->KeyPressActivationOff();
	this->PlaneWidgetY->SetLookupTable(this->PlaneWidgetX->GetLookupTable());
	this->PlaneWidgetY->On();

	this->PlaneWidgetZ->SetInputData(this->Image);
	this->PlaneWidgetZ->SetPlaneOrientationToYAxes();
	this->PlaneWidgetZ->SetSliceIndex(wholeExtent[4]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetZ->DisplayTextOn();
	else
		this->PlaneWidgetZ->DisplayTextOff();
	this->PlaneWidgetZ->SetPicker(picker);
	this->PlaneWidgetZ->KeyPressActivationOff();
	this->PlaneWidgetZ->SetLookupTable(this->PlaneWidgetX->GetLookupTable());
	this->PlaneWidgetZ->On();

	this->BoxWidget->SetPriority(1.0);
	this->BoxWidget->SetHandleSize(5E-3);
	this->BoxWidget->SetInputData(this->Image);
	this->BoxWidget->PlaceWidget();
	this->BoxWidget->RotationEnabledOff();
	vtkSmartPointer<vtkCallbackCommand> StartEventCallback =
		vtkSmartPointer<vtkCallbackCommand>::New();
	StartEventCallback->SetCallback(HideCubeStatic);
	StartEventCallback->SetClientData(this);
	vtkSmartPointer<vtkCallbackCommand> EndEventCallback =
		vtkSmartPointer<vtkCallbackCommand>::New();
	EndEventCallback->SetCallback(UpdateCubeStatic);
	EndEventCallback->SetClientData(this);
	vtkSmartPointer<vtkCallbackCommand> EnableEventCallback =
		vtkSmartPointer<vtkCallbackCommand>::New();
	EnableEventCallback->SetCallback(UpdateCubeStatic);
	EnableEventCallback->SetClientData(this);
	vtkSmartPointer<vtkCallbackCommand> DisableEventCallback =
		vtkSmartPointer<vtkCallbackCommand>::New();
	DisableEventCallback->SetCallback(HideCubeStatic);
	DisableEventCallback->SetClientData(this);

	this->BoxWidget->AddObserver(vtkCommand::StartInteractionEvent, StartEventCallback);
	this->BoxWidget->AddObserver(vtkCommand::EndInteractionEvent, EndEventCallback);
	this->BoxWidget->AddObserver(vtkCommand::EnableEvent, EnableEventCallback);
	this->BoxWidget->AddObserver(vtkCommand::DisableEvent, DisableEventCallback);

	vtkSmartPointer<vtkPolyData> polyData = 
		vtkSmartPointer<vtkPolyData>::New();
	this->BoxWidget->GetPolyData(polyData);
	polyData->ComputeBounds();
	this->CubeSource->SetBounds(polyData->GetBounds());
	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	cubeMapper->SetInputConnection(this->CubeSource->GetOutputPort());
	this->CubeActor->SetMapper(cubeMapper);
	this->CubeActor->GetProperty()->SetColor(0.6, 0.6, 0.2);
	this->CubeActor->GetProperty()->SetOpacity(0.25);
	this->CubeActor->VisibilityOff();
	this->vmtkRenderer->GetRenderer()->AddActor(this->CubeActor);
	this->vmtkRenderer->GetRenderer()->ResetCamera();
	this->vmtkRenderer->Render();
	this->vmtkRenderer->GetRenderer()->RemoveActor(this->CubeActor);
	this->BoxActive = false;
	if (this->BoxWidget->GetEnabled() == 1)
	{
		vtkSmartPointer<vtkPolyData> polyData2 = 
			vtkSmartPointer<vtkPolyData>::New();
		this->BoxWidget->GetPolyData(polyData2);
		polyData2->ComputeBounds();
		double *bounds = polyData2->GetBounds();
		this->BoxBounds[0] = bounds[0];
		this->BoxBounds[1] = bounds[1];
		this->BoxBounds[2] = bounds[2];
		this->BoxBounds[3] = bounds[3];
		this->BoxBounds[4] = bounds[4];
		this->BoxBounds[5] = bounds[5];
		this->BoxActive = true;
	}
	this->BoxWidget->Off();
}

void vtkvmtkImageVOISelector::ExtractVOI()
{
	int *wholeExtent = this->Image->GetExtent();
	double *origin = this->Image->GetOrigin();
	double *spacing = this->Image->GetSpacing();

	int newVOI[6] = { 0, 0, 0, 0, 0, 0 };
	newVOI[0] = std::max(wholeExtent[0], int(ceil((this->BoxBounds[0] - origin[0]) / spacing[0])));
	newVOI[1] = std::min(wholeExtent[1], int(floor((this->BoxBounds[1] - origin[0]) / spacing[0])));
	newVOI[2] = std::max(wholeExtent[2], int(ceil((this->BoxBounds[2] - origin[1]) / spacing[1])));
	newVOI[3] = std::min(wholeExtent[3], int(floor((this->BoxBounds[3] - origin[1]) / spacing[1])));
	newVOI[4] = std::max(wholeExtent[4], int(ceil((this->BoxBounds[4] - origin[2]) / spacing[2])));
	newVOI[5] = std::min(wholeExtent[5], int(floor((this->BoxBounds[5] - origin[2]) / spacing[2])));

	vtkSmartPointer<vtkExtractVOI> extractVOI =
		vtkSmartPointer<vtkExtractVOI>::New();
	extractVOI->SetInputData(this->CroppedImage);
	extractVOI->SetVOI(newVOI);
	extractVOI->Update();
	this->CroppedImage->DeepCopy(extractVOI->GetOutput());
}

void vtkvmtkImageVOISelector::Execute()
{
	//Not Input Image, All Dimension is zero
	if (this->Image->GetDimensions()[0] == 0 &&
		this->Image->GetDimensions()[1] == 0 &&
		this->Image->GetDimensions()[2] == 0)
	{
		std::cerr << "Error: no Image." << std::endl;
	}
	this->CroppedImage->DeepCopy(this->Image);
	if (this->Interactive == 1)
	{
		if (this->vmtkRenderer == NULL)
		{
			std::cout << "vmtkRenderer Initial" << std::endl;
			this->vmtkRenderer = vtkvmtkImageRenderer::New();
			this->vmtkRenderer->Initialize();
			this->OwnRenderer = true;
		}
		this->PlaneWidgetX = vtkvmtkImagePlaneWidget::New();
		this->PlaneWidgetX->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
		this->PlaneWidgetY = vtkvmtkImagePlaneWidget::New();
		this->PlaneWidgetY->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
		this->PlaneWidgetZ = vtkvmtkImagePlaneWidget::New();
		this->PlaneWidgetZ->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
		this->BoxWidget = vtkBoxWidget::New();
		this->BoxWidget->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());

		this->vmtkRenderer->SetPromptMessage("Press 'i' to activate interactor");

		this->Display();
		while (this->BoxActive == 1)
		{
			this->ExtractVOI();
			this->Image = this->CroppedImage;
			this->Display();
		}
	}
	else
		this->ExtractVOI();

	this->Image = this->CroppedImage;
}