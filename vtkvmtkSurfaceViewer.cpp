#include "vtkvmtkSurfaceViewer.h"

//VTK-Include
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkScalarsToColors.h>
#include <vtkColorTransferFunction.h>
#include <vtkTextProperty.h>
#include <vtkPoints.h>
#include <vtkCellCenters.h>
#include <vtkCellArray.h>
#include <vtkLabeledDataMapper.h>


//STD

vtkStandardNewMacro(vtkvmtkSurfaceViewer);
vtkStandardNewMacro(KeyPressInteractorStyleSurface);

void KeyPressInteractorStyleSurface::OnKeyPress()
{
	// Get the keypress
	vtkRenderWindowInteractor *rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	if (key == "Escape")
	{
		bool flag = this->surfaceViewer->GetRenderer()->GetTextInputMode();
		if (flag)
			this->surfaceViewer->GetRenderer()->SetTextInputMode(false);
		else
			this->surfaceViewer->GetRenderer()->SetTextInputMode(true);
	}

	if (this->surfaceViewer->GetRenderer()->GetTextInputMode())
	{
		if (key == "Return" || key == "Enter")
		{
			this->surfaceViewer->GetRenderer()->ExitTextInputMode();
			return;
		}

		if (key.size() > 3)
		{
			if (!strcmp(key.substr(0, 3).c_str(), "KP_"))
				key.erase(0, 3);
		}

		if (key == "space")
			key = " ";
		else if (key == "minus" || key == "Subtract")
			key = "-";
		else if (key == "period" || key == "Decimal")
			key = ".";
		else if (key.size() > 1 && key != "Backspace" && key != "BackSpace")
			key = ""; //the other KeyPress is ignored

		//delete one character
		if (key == "Backspace" && key == "BackSpace")
		{
			std::string textInput = this->surfaceViewer->GetRenderer()->GetCurrentTextInput();
			textInput.pop_back(); //delete the last one character
			const char* textDelete = textInput.c_str();
			this->surfaceViewer->GetRenderer()->SetCurrentTextInput(textDelete);
		}
		else //Add one character
		{
			std::string textInput = "";
			if (this->surfaceViewer->GetRenderer()->GetCurrentTextInput() != NULL)
				textInput += this->surfaceViewer->GetRenderer()->GetCurrentTextInput();
			textInput.append(key);
			const char* textAdd = textInput.c_str();
			this->surfaceViewer->GetRenderer()->SetCurrentTextInput(textAdd);
		}
		this->surfaceViewer->GetRenderer()->UpdateTextInput();
		return;
	}

	// Reset the Camera
	if (key == "r")
	{
		this->surfaceViewer->GetRenderer()->ResetCameraCallback();
	}

	// Screenshot and Save the Image
	if (key == "x")
	{
		this->surfaceViewer->GetRenderer()->ScreenshotCallback();
	}

	// QuitRenderer
	if (key == "q")
	{
		this->surfaceViewer->GetRenderer()->QuitRendererCallback();
	}

	//Change the Surface Representation
	if (key == "c")
	{
		this->surfaceViewer->RepresentationCallback();
	}

	// Forward events
	vtkInteractorStyleTrackballCamera::OnKeyPress();
}

vtkvmtkSurfaceViewer::vtkvmtkSurfaceViewer()
{
	this->Surface = nullptr;
	this->vmtkRenderer = nullptr;
	this->OwnRenderer = false;
	this->InnerSurface = false;
	this->Display = true;
	this->Opacity = 1.0;
	this->ArrayName = nullptr;
	this->ScalarRange[0] = 0.0; this->ScalarRange[1] = 0.0;
	this->ColorMap = new char[strlen("cooltowarm")+1];
	strcpy_s(this->ColorMap, strlen("cooltowarm")+1, "cooltowarm");
	this->NumberOfColors = 256;
	this->Legend = false;
	this->InnervmtkRenderer = false;
	this->LegendTitle = nullptr;
	this->Grayscale = false;
	this->FlatInterpolation = false;
	this->DisplayCellData = false;
	this->Color[0] = -1.0; this->Color[1] = -1.0; this->Color[2] = -1.0;
	this->LineWidth = 1;
	this->Representation = new char[strlen("surface")+1];
	strcpy_s(this->Representation, strlen("surface") + 1, "surface");
	this->DisplayTag = false;
	this->RegionTagArrayName = new char[strlen("RegionTagArray")+1];
	strcpy_s(this->RegionTagArrayName, strlen("RegionTagArray") + 1, "RegionTagArray");
	this->NumberOfRegions = 0;
	this->Actor = nullptr;
	this->labelsActor = nullptr;
	this->ScalarBarActor = nullptr;
}

vtkvmtkSurfaceViewer::~vtkvmtkSurfaceViewer()
{
	delete[] this->ColorMap;
	this->ColorMap = nullptr;
	delete[] this->Representation;
	this->Representation = nullptr;
	delete[] this->RegionTagArrayName;
	this->RegionTagArrayName = nullptr;
	if (this->ArrayName != nullptr)
	{
		delete[] this->ArrayName;
		this->ArrayName = nullptr;
	}
	if (this->LegendTitle != nullptr)
	{
		delete[] this->LegendTitle;
		this->LegendTitle = nullptr;
	}
	if (this->labelsActor != nullptr)
	{
		this->labelsActor->Delete();
		this->labelsActor = nullptr;
	}
	if (this->Actor != nullptr)
	{
		this->Actor->Delete();
		this->Actor = nullptr;
	}
	if (this->ScalarBarActor != nullptr)
	{
		this->ScalarBarActor->Delete();
		this->ScalarBarActor = nullptr;
	}
	//内部的vmtkRenderer，则回收该内存空间
	if (this->vmtkRenderer != nullptr && this->InnervmtkRenderer == true)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = nullptr;
	}
	//内部的Surface polydata，则回收该内存空间
	if (this->Surface != nullptr && this->InnerSurface == true)
	{
		this->Surface->Delete();
		this->Surface = nullptr;
	}
}

void vtkvmtkSurfaceViewer::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Surface Viewer\n";
}

void vtkvmtkSurfaceViewer::SetSurfaceRepresentation(const char* representation)
{
	std::cout << "representation: " << representation << std::endl;
	if (representation == "surface")
	{
		this->Actor->GetProperty()->SetRepresentationToSurface();
		this->Actor->GetProperty()->EdgeVisibilityOff();
	}
	else if (representation == "edges")
	{
		this->Actor->GetProperty()->SetRepresentationToSurface();
		this->Actor->GetProperty()->EdgeVisibilityOn();
	}	
	else if (representation == "wireframe")
	{
		this->Actor->GetProperty()->SetRepresentationToWireframe();
		this->Actor->GetProperty()->EdgeVisibilityOff();
	}
	this->SetRepresentation(representation);
}

void vtkvmtkSurfaceViewer::RepresentationCallback()
{
	if (this->Actor == nullptr)
		return;

	if (!strcmp(this->Representation, "surface"))
		this->SetSurfaceRepresentation("edges");
	else if (!strcmp(this->Representation, "edges"))
		this->SetSurfaceRepresentation("wireframe");
	else if (!strcmp(this->Representation, "wireframe"))
		this->SetSurfaceRepresentation("surface");

	this->vmtkRenderer->GetRenderWindow()->Render();
}

void vtkvmtkSurfaceViewer::BuildView()
{
	if (this->vmtkRenderer == nullptr)
	{
		this->vmtkRenderer = vtkvmtkRenderer::New();
		this->vmtkRenderer->Initialize();
		this->OwnRenderer = true;
		this->InnervmtkRenderer = true;
	}
	if (this->Actor != nullptr)
		this->vmtkRenderer->GetRenderer()->RemoveActor(this->Actor);
	if (this->ScalarBarActor != nullptr)
		this->vmtkRenderer->GetRenderer()->RemoveActor(this->ScalarBarActor);
	if (this->Surface != nullptr)
	{
		vtkSmartPointer<vtkPolyDataMapper> mapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(this->Surface);
		vtkDataArray* array = nullptr;
		if (this->ArrayName != nullptr)
		{
			if (this->DisplayCellData == false)
			{
				this->Surface->GetPointData()->SetActiveScalars(this->ArrayName);
				array = this->Surface->GetPointData()->GetScalars();
			}
			else //this->DisplayCellData == true
			{
				this->Surface->GetCellData()->SetActiveScalars(this->ArrayName);
				array = this->Surface->GetCellData()->GetScalars();
				mapper->SetScalarModeToUseCellData();
			}
			if (this->ScalarRange[1] > this->ScalarRange[0])
				mapper->SetScalarRange(this->ScalarRange);
			else if (array != nullptr)
				mapper->SetScalarRange(array->GetRange(0));
			if (this->Grayscale)
			{
				vtkSmartPointer<vtkLookupTable> lut =
					vtkSmartPointer<vtkLookupTable>::New();
				lut->SetValueRange(0.0, 1.0);
				lut->SetSaturationRange(0.0, 0.0);
				mapper->SetLookupTable(lut);
			}
		} //this->ArrayName != nullptr
		else
			mapper->ScalarVisibilityOff();

		//Setup the LooukupTable for ColorMap
		if (!strcmp(this->ColorMap, "grayscale")) //this->ColorMap == "grayscale"
		{
			std::cout << "ColorMap:grayscale " << std::endl;
			vtkLookupTable *lut = vtkLookupTable::SafeDownCast(mapper->GetLookupTable());
			lut->SetNumberOfTableValues(this->NumberOfColors);
			lut->SetValueRange(0.0, 1.0);
			lut->SetSaturationRange(0.0, 0.0);
			lut->Build();
			mapper->SetLookupTable(lut);
		}
		else if (!strcmp(this->ColorMap, "rainbow"))
		{
			std::cout << "ColorMap:rainbow " << std::endl;
			vtkLookupTable *lut = vtkLookupTable::SafeDownCast(mapper->GetLookupTable());
			lut->SetHueRange(0.666667, 0.0);
			lut->SetSaturationRange(0.75, 0.75);
			lut->SetValueRange(1.0, 1.0);
			lut->SetAlphaRange(1.0, 1.0);
			lut->SetNumberOfColors(this->NumberOfColors);
			lut->Build();
			mapper->SetLookupTable(lut);
		}
		else if (!strcmp(this->ColorMap, "blackbody"))
		{
			std::cout << "ColorMap:blackbody " << std::endl;
			vtkLookupTable *lut = vtkLookupTable::SafeDownCast(mapper->GetLookupTable());
			lut->SetNumberOfTableValues(this->NumberOfColors);
			vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = 
				vtkSmartPointer<vtkColorTransferFunction>::New();
			colorTransferFunction->SetColorSpaceToRGB();
			colorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
			colorTransferFunction->AddRGBPoint(0.4, 0.901961, 0.0, 0.0);
			colorTransferFunction->AddRGBPoint(0.8, 0.901961, 0.901961, 0.0);
			colorTransferFunction->AddRGBPoint(1.0, 1.0, 1.0, 1.0);
			for (int ii = 0; ii < this->NumberOfColors; ii++)
			{
				double ss = double(ii) / double(this->NumberOfColors);
				double *cc = colorTransferFunction->GetColor(ss);
				lut->SetTableValue(ii, cc[0], cc[1], cc[2], 1.0);
			}
			lut->Build();
			mapper->SetLookupTable(lut);
		}
		else if (!strcmp(this->ColorMap, "cooltowarm"))
		{
			std::cout << "ColorMap:cooltowarm " << std::endl;
			vtkLookupTable *lut = vtkLookupTable::SafeDownCast(mapper->GetLookupTable());
			lut->SetNumberOfTableValues(this->NumberOfColors);
			vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction =
				vtkSmartPointer<vtkColorTransferFunction>::New();
			colorTransferFunction->SetColorSpaceToRGB();
			colorTransferFunction->AddRGBPoint(0, 0.231373, 0.298039, 0.752941);
			colorTransferFunction->AddRGBPoint(0.5, 0.865003, 0.865003, 0.865003);
			colorTransferFunction->AddRGBPoint(1.0, 0.705882, 0.0156863, 0.14902);
			for (int ii = 0; ii < this->NumberOfColors; ii++)
			{
				double ss = double(ii) / double(this->NumberOfColors);
				double *cc = colorTransferFunction->GetColor(ss);
				lut->SetTableValue(ii, cc[0], cc[1], cc[2], 1.0);
			}
			lut->Build();
			mapper->SetLookupTable(lut);
		}
		else
		{
			std::cerr << "Error:InValid ColorMap:" << this->ColorMap << std::endl;
			return;
		}
		this->Actor = vtkActor::New();
		this->Actor->SetMapper(mapper);
		if (this->Color[0] >= 0.0)
			this->Actor->GetProperty()->SetColor(this->Color);
		this->Actor->GetProperty()->SetOpacity(this->Opacity);
		this->Actor->GetProperty()->SetLineWidth(this->LineWidth);
		if (this->FlatInterpolation)
			this->Actor->GetProperty()->SetInterpolationToFlat();
		this->SetSurfaceRepresentation(this->Representation);
		this->vmtkRenderer->GetRenderer()->AddActor(this->Actor);
		//Add KeyPressed monitor
		vtkSmartPointer<KeyPressInteractorStyleSurface> style =
			vtkSmartPointer<KeyPressInteractorStyleSurface>::New();
		style->SetSurfaceViewer(this);
		this->vmtkRenderer->GetRenderWindowInteractor()->SetInteractorStyle(style);
	}
	if (this->Legend == true && this->Actor != nullptr)
	{
		this->ScalarBarActor = vtkScalarBarActor::New();
		this->ScalarBarActor->SetLookupTable(this->Actor->GetMapper()->GetLookupTable());
		this->ScalarBarActor->GetLabelTextProperty()->ItalicOff();
		this->ScalarBarActor->GetLabelTextProperty()->BoldOff();
		this->ScalarBarActor->GetLabelTextProperty()->ShadowOff();
		this->ScalarBarActor->SetLabelFormat("%.2f");
		this->ScalarBarActor->SetTitle(this->LegendTitle);
		this->vmtkRenderer->GetRenderer()->AddActor(this->ScalarBarActor);
	}
	if (this->Display)
	{
		//Add Promt Information
		const char* text = "\n  \'c\',\'Change surface representation.\'";
		this->vmtkRenderer->Render(1, text);
	}
}

void vtkvmtkSurfaceViewer::BuildViewWithTag()
{
	if (this->vmtkRenderer == nullptr)
	{
		this->vmtkRenderer = vtkvmtkRenderer::New();
		this->vmtkRenderer->Initialize();
		this->OwnRenderer = true;
	}
	if (this->Actor != nullptr)
		this->vmtkRenderer->GetRenderer()->RemoveActor(this->Actor);
	if (this->ScalarBarActor != nullptr)
		this->vmtkRenderer->GetRenderer()->RemoveActor(this->ScalarBarActor);
	//Judge PointData Or CellData which is not NULL
	vtkSmartPointer<vtkPoints> labelPoints = vtkSmartPointer<vtkPoints>::New();
	if (this->Surface->GetPointData()->GetArray(this->RegionTagArrayName) == NULL &&
		this->Surface->GetCellData()->GetArray(this->RegionTagArrayName) == NULL)
	{
		std::cerr << "Error: no regiontagarray with name specified" << std::endl;
		return;
	}
	else if (this->Surface->GetPointData()->GetArray(this->RegionTagArrayName) != NULL)
	{
		vtkDataArray *regionTagArray = this->Surface->GetPointData()->GetArray(this->RegionTagArrayName);
		int numberOfPoints = this->Surface->GetNumberOfPoints();
		for (int j = 0; j < numberOfPoints; j++)
		{
			TagSet.insert(regionTagArray->GetTuple1(j));
		}
		this->NumberOfRegions = TagSet.size();
		std::set<double> tagSetCopy = TagSet;
		labelPoints->SetNumberOfPoints(TagSet.size());
		double point[3] = { 0.0, 0.0, 0.0 };
		for (int j = 0; j < numberOfPoints; j++)
		{
			double item = regionTagArray->GetTuple1(j);
			std::set<double>::iterator it;
			it = tagSetCopy.find(item);
			if (it != tagSetCopy.end())
			{
				this->Surface->GetPoint(j, point);
				it = TagSet.find(item);  //the tagSetCopy maychanged
				labelPoints->SetPoint(std::distance(it, TagSet.begin()), point);
				tagSetCopy.erase(item);
			}
		}
		this->Surface->GetPointData()->SetActiveScalars(this->RegionTagArrayName);
	}
	else if (this->Surface->GetCellData()->GetArray(this->RegionTagArrayName) != NULL)
	{
		vtkDataArray *regionTagArray = this->Surface->GetCellData()->GetArray(this->RegionTagArrayName);
		int numberOfCells = this->Surface->GetNumberOfCells();
		for (int j = 0; j < numberOfCells; j++)
		{
			TagSet.insert(regionTagArray->GetTuple1(j));
		}
		this->NumberOfRegions = TagSet.size();
		std::set<double> tagSetCopy = TagSet;
		labelPoints->SetNumberOfPoints(TagSet.size());
		double point[3] = { 0.0, 0.0, 0.0 };
		vtkSmartPointer<vtkCellCenters> cellCenters = 
			vtkSmartPointer<vtkCellCenters>::New();
		cellCenters->SetInputData(this->Surface);
		cellCenters->Update();

		vtkDataArray *regionTagArrayCenters = cellCenters->GetOutput()->GetPointData()->GetArray(this->RegionTagArrayName);
		int numberOfPoints = cellCenters->GetOutput()->GetNumberOfPoints();
		for (int j = 0; j < numberOfPoints; j++)
		{
			double item = regionTagArray->GetTuple1(j);
			std::set<double>::iterator it;
			it = tagSetCopy.find(item);
			if (it != tagSetCopy.end())
			{
				cellCenters->GetOutput()->GetPoint(j, point);
				it = TagSet.find(item);  //the tagSetCopy maychanged
				labelPoints->SetPoint(std::distance(it, TagSet.begin()), point);
				tagSetCopy.erase(item);
			}
		}
		this->Surface->GetCellData()->SetActiveScalars(this->RegionTagArrayName);
	}
	vtkSmartPointer<vtkPolyData> labelPolyData = 
		vtkSmartPointer<vtkPolyData>::New();
	labelPolyData->SetPoints(labelPoints);

	vtkSmartPointer<vtkIntArray> labelArray = 
		vtkSmartPointer<vtkIntArray>::New();
	labelArray->SetNumberOfComponents(1);
	labelArray->SetNumberOfTuples(this->NumberOfRegions);
	labelArray->SetName("label");
	labelArray->FillComponent(0, 0);

	labelPolyData->GetPointData()->AddArray(labelArray);

	int indexOfItem = 0;
	for (std::set<double>::iterator it = TagSet.begin(); it != TagSet.end(); it++)
	{
		labelArray->SetTuple1(indexOfItem, *it);
		indexOfItem++;
	}
	labelPolyData->GetPointData()->SetActiveScalars("label");
	vtkSmartPointer<vtkLabeledDataMapper> labelsMapper = 
		vtkSmartPointer<vtkLabeledDataMapper>::New();
	labelsMapper->SetInputData(labelPolyData);
	labelsMapper->SetLabelModeToLabelScalars();
	labelsMapper->GetLabelTextProperty()->SetColor(1, 1, 1);
	labelsMapper->GetLabelTextProperty()->SetFontSize(14);
	this->labelsActor = vtkActor2D::New();
	this->labelsActor->SetMapper(labelsMapper);
	this->vmtkRenderer->GetRenderer()->AddActor(this->labelsActor);

	vtkSmartPointer<vtkPolyDataMapper> surfaceMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	surfaceMapper->SetInputData(this->Surface);
	surfaceMapper->ScalarVisibilityOn();
	surfaceMapper->SetScalarRange(*TagSet.begin(), *(TagSet.end()--));
	this->Actor = vtkActor::New();
	this->Actor->SetMapper(surfaceMapper);
	this->Actor->GetProperty()->SetOpacity(this->Opacity);
	this->Actor->GetProperty()->SetLineWidth(this->LineWidth);
	if (this->FlatInterpolation)
		this->Actor->GetProperty()->SetInterpolationToFlat();
	this->SetSurfaceRepresentation(this->Representation);
	this->vmtkRenderer->GetRenderer()->AddActor(this->Actor);
	//Add KeyPressed monitor
	vtkSmartPointer<KeyPressInteractorStyleSurface> style =
		vtkSmartPointer<KeyPressInteractorStyleSurface>::New();
	style->SetSurfaceViewer(this);
	this->vmtkRenderer->GetRenderWindowInteractor()->SetInteractorStyle(style);
	if (this->Legend == true && this->Actor != nullptr)
	{
		this->ScalarBarActor = vtkScalarBarActor::New();
		this->ScalarBarActor->SetLookupTable(this->Actor->GetMapper()->GetLookupTable());
		this->ScalarBarActor->GetLabelTextProperty()->ItalicOff();
		this->ScalarBarActor->GetLabelTextProperty()->BoldOff();
		this->ScalarBarActor->GetLabelTextProperty()->ShadowOff();
		this->ScalarBarActor->SetLabelFormat("%.2f");
		this->ScalarBarActor->SetTitle(this->LegendTitle);
		this->vmtkRenderer->GetRenderer()->AddActor(this->ScalarBarActor);
	}
	if (this->Display)
	{
		//Add Promt Information
		const char* text = "\n  \'c\',\'Change surface representation.\'";
		this->vmtkRenderer->Render(1, text);
	}
}

void vtkvmtkSurfaceViewer::Execute()
{
	if (this->Surface == nullptr && this->Display == false)
		std::cerr << "Error: no Surface." << std::endl;
	if (this->DisplayTag == false)
		this->BuildView();
	else
		this->BuildViewWithTag();
}