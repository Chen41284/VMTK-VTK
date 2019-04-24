#include "vtkvmtkImageSeeder.h"

//VTK
#include <vtkPointData.h>
#include <vtkGlyph3D.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCellPicker.h>
#include <vtkCallbackCommand.h>

//STD

vtkStandardNewMacro(vtkvmtkImageSeeder);

void CtrlSelectSeed(vtkObject* caller, long unsigned int evId, void* clientData, void* callData)
{
	//ClientData是Seeder类
	//caller 是vtkvmtkImagePlaneWidget的类
	vtkvmtkImagePlaneWidget *planeWidget = static_cast<vtkvmtkImagePlaneWidget*>(caller);
	vtkvmtkImageSeeder *Seeder = static_cast<vtkvmtkImageSeeder*>(clientData);

	if (Seeder->GetRenderer()->GetRenderWindowInteractor()->GetControlKey() == 0)
		return;
	double cursorData[4] = { 0.0, 0.0, 0.0, 0.0 };
	planeWidget->GetCursorData(cursorData);
	double *spacing = Seeder->GetImage()->GetSpacing();
	double *origin = Seeder->GetImage()->GetOrigin();
	int *extent = Seeder->GetImage()->GetExtent();
	double point[3] = { 0.0, 0.0, 0.0 };
	point[0] = cursorData[0] * spacing[0] + origin[0];
	point[1] = cursorData[1] * spacing[1] + origin[1];
	point[2] = cursorData[2] * spacing[2] + origin[2];
	Seeder->GetSeeds()->GetPoints()->InsertNextPoint(point);
	Seeder->GetSeeds()->Modified();
	Seeder->GetRenderer()->GetRenderWindow()->Render();
}

vtkvmtkImageSeeder::vtkvmtkImageSeeder()
{
	this->Image = nullptr;
	this->vmtkRenderer = nullptr;
	this->InnervmtkRenderer = false;
	this->Display = true;
	this->SetArrayName("");
	this->Picker = nullptr;
	this->PlaneWidgetX = nullptr;
	this->PlaneWidgetY = nullptr;
	this->PlaneWidgetZ = nullptr;
	this->SeedActor = nullptr;
	this->Seeds = nullptr;
	this->TextureInterpolation = true;
	this->KeepSeeds = false;
	//this->SeedPoints = vtkPoints::New();
}

vtkvmtkImageSeeder::~vtkvmtkImageSeeder()
{
	delete[] this->ArrayName;
	this->ArrayName = nullptr;
	if (this->vmtkRenderer != nullptr && this->InnervmtkRenderer == true)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = nullptr;
	}
	/*if (this->PlaneWidgetX != nullptr)
	{
		this->PlaneWidgetX->Delete();
		this->PlaneWidgetX = nullptr;
	}
	if (this->PlaneWidgetY != nullptr)
	{
		this->PlaneWidgetY->Delete();
		this->PlaneWidgetY = nullptr;
	}
	if (this->PlaneWidgetZ != nullptr)
	{
		this->PlaneWidgetZ->Delete();
		this->PlaneWidgetZ = nullptr;
	}
	if (this->SeedActor != nullptr)
	{
		this->SeedActor->Delete();
		this->SeedActor = nullptr;
	}
	if (this->Seeds != nullptr)
	{
		this->Seeds->Delete();
		this->Seeds = nullptr;
	}
	if (this->SeedPoints)
	{
		this->SeedPoints->Delete();
		this->SeedPoints = nullptr;
	}*/
}

void vtkvmtkImageSeeder::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Seeder\n";
}

void vtkvmtkImageSeeder::WidgetsOn()
{
	this->PlaneWidgetX->On();
	this->PlaneWidgetY->On();
	this->PlaneWidgetZ->On();
}

void vtkvmtkImageSeeder::WidgetsOff()
{
	this->PlaneWidgetX->Off();
	this->PlaneWidgetY->Off();
	this->PlaneWidgetZ->Off();
}

void vtkvmtkImageSeeder::InitializeSeeds()
{
	this->Seeds->Initialize();
	this->SeedPoints = vtkSmartPointer<vtkPoints>::New();
	this->Seeds->SetPoints(this->SeedPoints);
}

void vtkvmtkImageSeeder::BuildView()
{
	//this->ArrayName != ""；相等的话strcmp为0
	if (strcmp(this->ArrayName, ""))
		this->Image->GetPointData()->SetActiveScalars(this->ArrayName);
	int *wholeExtent = this->Image->GetExtent();

	this->PlaneWidgetX->SetResliceInterpolateToLinear();
	this->PlaneWidgetX->SetTextureInterpolate(this->TextureInterpolation);
	this->PlaneWidgetX->SetInputData(this->Image);
	this->PlaneWidgetX->SetPlaneOrientationToXAxes();
	this->PlaneWidgetX->SetSliceIndex(wholeExtent[0]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetX->DisplayTextOn();
	else
		this->PlaneWidgetX->DisplayTextOff();
	this->PlaneWidgetX->KeyPressActivationOff();

	this->PlaneWidgetY->SetResliceInterpolateToLinear();
	this->PlaneWidgetY->SetTextureInterpolate(this->TextureInterpolation);
	this->PlaneWidgetY->SetInputData(this->Image);
	this->PlaneWidgetY->SetPlaneOrientationToYAxes();
	this->PlaneWidgetY->SetSliceIndex(wholeExtent[2]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetY->DisplayTextOn();
	else
		this->PlaneWidgetY->DisplayTextOff();
	this->PlaneWidgetY->KeyPressActivationOff();
	this->PlaneWidgetY->SetLookupTable(this->PlaneWidgetX->GetLookupTable());

	this->PlaneWidgetZ->SetResliceInterpolateToLinear();
	this->PlaneWidgetZ->SetTextureInterpolate(this->TextureInterpolation);
	this->PlaneWidgetZ->SetInputData(this->Image);
	this->PlaneWidgetZ->SetPlaneOrientationToZAxes();
	this->PlaneWidgetZ->SetSliceIndex(wholeExtent[4]);
	if (this->vmtkRenderer->GetAnnotations())
		this->PlaneWidgetZ->DisplayTextOn();
	else
		this->PlaneWidgetZ->DisplayTextOff();
	this->PlaneWidgetZ->KeyPressActivationOff();
	this->PlaneWidgetZ->SetLookupTable(this->PlaneWidgetX->GetLookupTable());

	vtkSmartPointer<vtkGlyph3D> glyphs = 
		vtkSmartPointer<vtkGlyph3D>::New();
	vtkSmartPointer<vtkSphereSource> glyphSource =
		vtkSmartPointer<vtkSphereSource>::New();
	glyphs->SetInputData(this->Seeds);
	glyphs->SetSourceConnection(glyphSource->GetOutputPort());
	glyphs->SetScaleModeToDataScalingOff();
	glyphs->SetScaleFactor(this->Image->GetLength()*0.01);
	vtkSmartPointer<vtkPolyDataMapper> glyphMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	glyphMapper->SetInputConnection(glyphs->GetOutputPort());
	this->SeedActor = vtkSmartPointer<vtkActor>::New();
	this->SeedActor->SetMapper(glyphMapper);
	this->SeedActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
	this->vmtkRenderer->GetRenderer()->AddActor(this->SeedActor);
	this->WidgetsOn();

	//开启渲染
	if (this->Display == true)
	{
		this->vmtkRenderer->Render(1, "\n  \'Ctrl\', \'Add Seed.\' ");
	}
}

void vtkvmtkImageSeeder::Execute()
{
	if ((this->Image == nullptr) && (this->Display == true))
		std::cout << "Error: no Image." << std::endl;

	if (this->vmtkRenderer == nullptr)
	{
		this->vmtkRenderer = vtkvmtkRenderer::New();
		this->vmtkRenderer->Initialize();
		this->InnervmtkRenderer = true;
	}
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
	this->Picker->SetTolerance(0.005);

	this->PlaneWidgetX = vtkSmartPointer<vtkvmtkImagePlaneWidget>::New();
	this->PlaneWidgetX->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
	vtkSmartPointer<vtkCallbackCommand> CtrlSelectCallback =
		vtkSmartPointer<vtkCallbackCommand>::New();
	CtrlSelectCallback->SetCallback(CtrlSelectSeed);
	CtrlSelectCallback->SetClientData(this);
	this->PlaneWidgetX->AddObserver(vtkCommand::StartInteractionEvent, CtrlSelectCallback);
	this->PlaneWidgetX->SetPicker(this->Picker);
	this->PlaneWidgetY = vtkSmartPointer<vtkvmtkImagePlaneWidget>::New();
	this->PlaneWidgetY->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
	this->PlaneWidgetY->AddObserver(vtkCommand::StartInteractionEvent, CtrlSelectCallback);
	this->PlaneWidgetY->SetPicker(this->Picker);
	this->PlaneWidgetZ = vtkSmartPointer<vtkvmtkImagePlaneWidget>::New();
	this->PlaneWidgetZ->SetInteractor(this->vmtkRenderer->GetRenderWindowInteractor());
	this->PlaneWidgetZ->AddObserver(vtkCommand::StartInteractionEvent, CtrlSelectCallback);
	this->PlaneWidgetZ->SetPicker(this->Picker);

	this->Seeds = vtkSmartPointer<vtkPolyData>::New();
	this->InitializeSeeds();
	this->BuildView();
	this->WidgetsOff();

	if (this->KeepSeeds)
		this->vmtkRenderer->GetRenderer()->RemoveActor(this->SeedActor);
}