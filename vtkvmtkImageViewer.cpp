#include "vtkvmtkImageViewer.h"

//VTK Includes
#include <vtkObjectFactory.h>
#include <vtkTextProperty.h>
#include <vtkPointData.h>
//STD Includes
#include <math.h>
#include <time.h>

//VMTK Includes
#include <vtkvmtkImagePlaneWidget.h>

vtkStandardNewMacro(vtkvmtkImageViewer);

vtkvmtkImageViewer::vtkvmtkImageViewer()
{
	this->Image = NULL;
	this->vmtkRenderer = NULL;
	this->OwnRenderer = false;
	this->Display = true;
	this->ArrayName = NULL;
	this->Picker = NULL;
	this->PlaneWidgetX = NULL;
	this->PlaneWidgetY = NULL;
	this->PlaneWidgetZ = NULL;
	this->Margins = false;
	this->TextureInterpolation = true;
	this->ContinuousCursor = false;
	this->WindowLevel[0] = 0.0; this->WindowLevel[1] = 0.0;
}

vtkvmtkImageViewer::~vtkvmtkImageViewer()
{
	if (this->Image != NULL)
	{
		this->Image->Delete();
		this->Image = NULL;
	}
	if (this->vmtkRenderer != NULL)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = NULL;
	}
	if (this->ArrayName != NULL)
	{
		delete[] this->ArrayName;
		this->ArrayName = NULL;
	}
	if (this->Picker != NULL)
	{
		this->Picker->Delete();
		this->Picker = NULL;
	}
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

void vtkvmtkImageViewer::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Viewer\n";
}

void vtkvmtkImageViewer::Execute()
{
	if ((this->Image == NULL) && (this->Display == true))
	{
		std::cerr << "no Image" << std::endl;
		return;
	}
	this->BuildView();
}

void vtkvmtkImageViewer::BuildView()
{
	if (this->vmtkRenderer == NULL)
	{
		this->vmtkRenderer = vtkvmtkImageRenderer::New();
		this->vmtkRenderer->Initialize();
		this->OwnRenderer = true;
	}
	if (this->ArrayName != NULL)
		this->Image->GetPointData()->SetActiveScalars(this->ArrayName);
	int *wholeExtent = this->Image->GetExtent(); //Six number
	if (this->Picker == NULL)
		this->Picker = vtkCellPicker::New();
	if (this->PlaneWidgetX == NULL)
		this->PlaneWidgetX = vtkvmtkImagePlaneWidget::New();
	if (this->PlaneWidgetY == NULL)
		this->PlaneWidgetY = vtkvmtkImagePlaneWidget::New();
	if (this->PlaneWidgetZ == NULL)
		this->PlaneWidgetZ = vtkvmtkImagePlaneWidget::New();
	this->Picker->SetTolerance(0.005);
	this->PlaneWidgetX->SetResliceInterpolateToLinear();
	this->PlaneWidgetX->SetTextureInterpolate(this->TextureInterpolation);
	this->PlaneWidgetX->SetUseContinuousCursor(this->ContinuousCursor);
	this->PlaneWidgetX->SetInputData(this->Image);
	this->PlaneWidgetX->SetPlaneOrientationToXAxes();
	this->PlaneWidgetX->SetSliceIndex(wholeExtent[0]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetX->DisplayTextOn();
	else
		this->PlaneWidgetX->DisplayTextOff();
	this->PlaneWidgetX->SetPicker(this->Picker);
	this->PlaneWidgetX->KeyPressActivationOff();
	this->PlaneWidgetX->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
	if (this->Margins)
	{
		this->PlaneWidgetX->SetMarginSizeX(0.05);
		this->PlaneWidgetX->SetMarginSizeY(0.05);
	}
	else
	{
		this->PlaneWidgetX->SetMarginSizeX(0.0);
		this->PlaneWidgetX->SetMarginSizeY(0.0);
	}
	if (this->WindowLevel[0] != 0.0)
		this->PlaneWidgetX->SetWindowLevel(this->WindowLevel[0], this->WindowLevel[1]);
	this->PlaneWidgetX->On();

	this->PlaneWidgetY->SetResliceInterpolateToLinear();
	this->PlaneWidgetY->SetTextureInterpolate(this->TextureInterpolation);
	this->PlaneWidgetY->SetUseContinuousCursor(this->ContinuousCursor);
	this->PlaneWidgetY->SetInputData(this->Image);
	this->PlaneWidgetY->SetPlaneOrientationToYAxes();
	this->PlaneWidgetY->SetSliceIndex(wholeExtent[2]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetY->DisplayTextOn();
	else
		this->PlaneWidgetY->DisplayTextOff();
	this->PlaneWidgetY->SetPicker(this->Picker);
	this->PlaneWidgetY->KeyPressActivationOff();
	this->PlaneWidgetY->SetLookupTable(this->PlaneWidgetX->GetLookupTable());
	this->PlaneWidgetY->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
	if (this->Margins)
	{
		this->PlaneWidgetY->SetMarginSizeX(0.05);
		this->PlaneWidgetY->SetMarginSizeY(0.05);
	}
	else
	{
		this->PlaneWidgetY->SetMarginSizeX(0.0);
		this->PlaneWidgetY->SetMarginSizeY(0.0);
	}
	if (this->WindowLevel[0] != 0.0)
		this->PlaneWidgetY->SetWindowLevel(this->WindowLevel[0], this->WindowLevel[1]);
	this->PlaneWidgetY->On();

	this->PlaneWidgetZ->SetResliceInterpolateToLinear();
	this->PlaneWidgetZ->SetTextureInterpolate(this->TextureInterpolation);
	this->PlaneWidgetZ->SetUseContinuousCursor(this->ContinuousCursor);
	this->PlaneWidgetZ->SetInputData(this->Image);
	this->PlaneWidgetZ->SetPlaneOrientationToZAxes();
	this->PlaneWidgetZ->SetSliceIndex(wholeExtent[4]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetZ->DisplayTextOn();
	else
		this->PlaneWidgetZ->DisplayTextOff();
	this->PlaneWidgetZ->SetPicker(this->Picker);
	this->PlaneWidgetZ->KeyPressActivationOff();
	this->PlaneWidgetZ->SetLookupTable(this->PlaneWidgetX->GetLookupTable());
	this->PlaneWidgetZ->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
	if (this->Margins)
	{
		this->PlaneWidgetZ->SetMarginSizeX(0.05);
		this->PlaneWidgetZ->SetMarginSizeY(0.05);
	}
	else
	{
		this->PlaneWidgetZ->SetMarginSizeX(0.0);
		this->PlaneWidgetZ->SetMarginSizeY(0.0);
	}
	if (this->WindowLevel[0] != 0.0)
		this->PlaneWidgetZ->SetWindowLevel(this->WindowLevel[0], this->WindowLevel[1]);
	this->PlaneWidgetZ->On();

	if (this->Display == 1)
		this->vmtkRenderer->Render();
}
