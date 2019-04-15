#include "vtkvmtkRenderer.h"

//VTK Includes
#include <vtkObjectFactory.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkTextProperty.h>
//STD Includes
#include <math.h>
#include <time.h>

vtkStandardNewMacro(KeyPressInteractorStyle);
vtkStandardNewMacro(vtkvmtkRenderer);
vtkStandardNewMacro(vmtkRendererInputStream);
std::string getTimeRenderer();

vmtkRendererInputStream::vmtkRendererInputStream(vtkvmtkRenderer *renderer)
{
	this->renderer = renderer;
}

vmtkRendererInputStream::vmtkRendererInputStream()
{
	this->renderer = NULL;
}

vmtkRendererInputStream::~vmtkRendererInputStream()
{

}

void vmtkRendererInputStream::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image InputStream Text\n";
}

char* vmtkRendererInputStream::ReadLine()
{
	this->renderer->EnterTextInputMode();

	return this->renderer->GetCurrentTextInput();
}

void vmtkRendererInputStream::promt(const char* text)
{
	this->renderer->SetTextInputQuery(text);
	char* empty = '\0';
	this->renderer->SetCurrentTextInput(empty);
	this->renderer->UpdateTextInput();
}

void KeyPressInteractorStyle::OnKeyPress()
{
	// Get the keypress
	vtkRenderWindowInteractor *rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	if (key == "Escape")
	{
		bool flag = this->renderer->GetTextInputMode();
		if (flag)
			this->renderer->SetTextInputMode(false);
		else
			this->renderer->SetTextInputMode(true);
	}

	if (this->renderer->GetTextInputMode())
	{
		if (key == "Return" || key == "Enter")
		{
			this->renderer->ExitTextInputMode();
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
			std::string textInput = this->renderer->GetCurrentTextInput();
			textInput.pop_back(); //delete the last one character
			const char* textDelete = textInput.c_str();
			this->renderer->SetCurrentTextInput(textDelete);
		}
		else //Add one character
		{
			std::string textInput = "";
			if (this->renderer->GetCurrentTextInput() != NULL)
				textInput += this->renderer->GetCurrentTextInput();
			textInput.append(key);
			const char* textAdd = textInput.c_str();
			this->renderer->SetCurrentTextInput(textAdd);
		}
		this->renderer->UpdateTextInput();
		return;
	}

	// Reset the Camera
	if (key == "r")
	{
		this->renderer->ResetCameraCallback();
	}

	// Screenshot and Save the Image
	if (key == "x")
	{
		this->renderer->ScreenshotCallback();
	}

	// QuitRenderer
	if (key == "q")
	{
		this->renderer->QuitRendererCallback();
	}

	// Forward events
	vtkInteractorStyleTrackballCamera::OnKeyPress();
}

vtkvmtkRenderer::vtkvmtkRenderer()
{
	this->Renderer = nullptr;
	this->RenderWindow = nullptr;
	this->RenderWindowInteractor = nullptr;
	this->Camera = nullptr;
	this->WindowSize[0] = 1200; this->WindowSize[1] = 900;
	this->WindowPosition[0] = 50; this->WindowPosition[1] = 50;
	this->Background[0] = 0.1; this->Background[1] = 0.1; this->Background[2] = 0.2;
	this->Annotations = 1;
	this->PointSmoothing = 1;
	this->LineSmoothing = 1;
	this->PolygonSmoothing = 0;
	this->TextInputMode = false;
	this->ExitAfterTextInputMode = true;
	this->ExitTextInputCallback = nullptr;
	this->TextInputActor = nullptr;
	this->TextInputQuery = nullptr;
	this->CurrentTextInput = nullptr;
	this->InputPosition[0] = 0.35; this->InputPosition[1] = 0.1;
	this->TextActor = nullptr;
	this->Position[0] = 0.001; this->Position[1] = 0.05;
	this->ScreenshotMagnification = 4;
	this->UseRendererInputStream = true;
}

vtkvmtkRenderer::~vtkvmtkRenderer()
{
	if (this->Renderer != nullptr)
	{
		this->Renderer->Delete();
		this->Renderer = nullptr;
	}
	if (this->RenderWindow != nullptr)
	{
		this->RenderWindow->Delete();
		this->RenderWindow = nullptr;
	}
	if (this->RenderWindowInteractor != nullptr)
	{
		this->RenderWindowInteractor->Delete();
		this->RenderWindowInteractor = nullptr;
	}
	if (this->Camera != nullptr)
	{
		this->Camera->Delete();
		this->Camera = nullptr;
	}
	if (this->ExitTextInputCallback != nullptr)
	{
		this->ExitTextInputCallback->Delete();
		this->ExitTextInputCallback = nullptr;
	}
	if (this->TextInputActor != nullptr)
	{
		this->TextInputActor->Delete();
		this->TextInputActor = nullptr;
	}
	if (this->TextInputQuery != nullptr)
	{
		delete[] TextInputQuery;
		this->TextInputQuery = nullptr;
	}
	if (this->CurrentTextInput != nullptr)
	{
		delete[] CurrentTextInput;
		this->CurrentTextInput;
	}
	if (this->TextActor != nullptr)
	{
		this->TextActor->Delete();
		this->TextActor = nullptr;
	}
}

void vtkvmtkRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Render\n";
}

int vtkvmtkRenderer::_GetScreenFontSize()
{
	vtkSmartPointer<vtkRenderWindow> tempRenderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	int userScreenWidth = tempRenderWindow->GetScreenSize()[0];
	int userScreenHeight = tempRenderWindow->GetScreenSize()[1];
	// Number of pixels across the width of the calibration monitor
	int	baseScreenWidth = 3360;
	// Number of pixels across the heigh of the calibration monitor
	int baseScreenHeight = 2100;
	// Font size appropriate for the calibration monitor of size baseScreenWidth x baseScreenHeight
	int	baseFontSize = 24;
	int baseFontScalePerPixelWidth = floor(baseScreenWidth / baseFontSize);
	int	scaledFontSize = ceil(userScreenWidth / baseFontScalePerPixelWidth);

	//make sure that the font size won't be set too low or high for low/high screen resolutions. 
	if (scaledFontSize < 8)
		scaledFontSize = 8;

	if (scaledFontSize > 50)
		scaledFontSize = 50;

	return scaledFontSize;
}

void vtkvmtkRenderer::ResetCameraCallback()
{
	printf("Reset Camera\n");
	this->Renderer->ResetCamera();
	this->RenderWindow->Render();
}

void vtkvmtkRenderer::QuitRendererCallback()
{
	printf("Quit renderer\n");
	this->Renderer->RemoveActor(this->TextActor);
	this->RenderWindowInteractor->ExitCallback();
	// Close the window
	//this->RenderWindow->Finalize();
	// Stop the interactor
	//this->RenderWindowInteractor->TerminateApp();
}

void vtkvmtkRenderer::ScreenshotCallback()
{
	printf("Take a Screenshot\n");
	std::string  filePrefix = "ScreenShot\\vmtk-";
	std::string fileName = filePrefix + getTimeRenderer() + ".png";
	vtkSmartPointer<vtkWindowToImageFilter> windowToImage =
		vtkSmartPointer<vtkWindowToImageFilter>::New();
	windowToImage->SetInput(this->RenderWindow);
	//windowToImage->SetMagnification(this->ScreenshotMagnification);
	windowToImage->Update();
	this->RenderWindow->Render();
	vtkSmartPointer<vtkPNGWriter> writer =
		vtkSmartPointer<vtkPNGWriter>::New();
	writer->SetInputConnection(windowToImage->GetOutputPort());
	writer->SetFileName(fileName.c_str());
	std::cout << fileName << std::endl;
	writer->Write();
}

void vtkvmtkRenderer::UpdateTextInput()
{
	if (this->TextInputQuery != NULL)
	{
		if (this->CurrentTextInput != NULL || this->CurrentTextInput == "")
		{
			std::string text = this->TextInputQuery;
			text += this->CurrentTextInput; text += "_";
			this->TextInputActor->SetInput(text.c_str());
		}
		else
			this->TextInputActor->SetInput(this->GetTextInputQuery());
		this->Renderer->AddActor(this->TextInputActor);
	}
	else
	{
		this->Renderer->RemoveActor(this->TextInputActor);
	}
	this->RenderWindow->Render();
}

void vtkvmtkRenderer::Close()
{
	return;
}

void vtkvmtkRenderer::EnterTextInputMode(bool interactive)
{
	this->SetCurrentTextInput("");
	this->Renderer->AddActor(this->TextInputActor);
	this->Renderer->RemoveActor(this->TextActor);
	this->UpdateTextInput();
	this->TextInputMode = 1;
	this->Render(interactive);
}

void vtkvmtkRenderer::ExitTextInputMode()
{
	this->Renderer->RemoveActor(this->TextInputActor);
	this->Renderer->AddActor(this->TextActor);
	this->RenderWindow->Render();
	this->TextInputMode = 0;
	if (this->ExitTextInputCallback != NULL)
	{
		//PromptAsync
		//this->ExitTextInputCallback(this->CurrentTextInput);
		this->ExitTextInputCallback = NULL;
	}
	if (this->ExitAfterTextInputMode)
		this->RenderWindowInteractor->ExitCallback();
}

void vtkvmtkRenderer::Render(bool interactive, const char* Promt)
{
	//std::cout << interactive << std::endl;
	if (interactive)
		this->RenderWindowInteractor->Initialize();
	this->RenderWindow->SetWindowName("vmtk - the Vascular Modeling Toolkit");

	std::string keyText = "\n  \'x\', \'Take screenshot.\'";
	keyText += "\n  \'r\', \'Reset camera.\'";
	keyText += "\n  \'q\', \'Quit renderer/proceed.\'";
	if (Promt != nullptr)
		keyText += Promt;
	std::cout << keyText << std::endl;

	this->TextActor->SetInput(keyText.c_str());
	this->Renderer->AddActor(this->TextActor);
	this->RenderWindow->Render();

	if (interactive)
	{
		this->RenderWindowInteractor->Start();
	}

}

void vtkvmtkRenderer::Initialize()
{
	if (this->Renderer == NULL)
	{
		this->Renderer = vtkRenderer::New();
		this->Renderer->SetBackground(this->Background);
		this->RenderWindow = vtkRenderWindow::New();
		this->RenderWindow->AddRenderer(this->Renderer);
		this->RenderWindow->SetSize(this->WindowSize[0], this->WindowSize[1]);
		this->RenderWindow->SetPosition(this->WindowPosition[0], this->WindowPosition[1]);
		this->RenderWindow->SetPointSmoothing(this->PointSmoothing);
		this->RenderWindow->SetLineSmoothing(this->LineSmoothing);
		this->RenderWindow->SetPolygonSmoothing(this->PolygonSmoothing);
		this->RenderWindowInteractor = vtkRenderWindowInteractor::New();
		this->RenderWindow->SetInteractor(this->RenderWindowInteractor);

		//Add KeyPressed monitor
		vtkSmartPointer<KeyPressInteractorStyle> style =
			vtkSmartPointer<KeyPressInteractorStyle>::New();
		style->SetvmtkRender(this);
		this->RenderWindowInteractor->SetInteractorStyle(style);

		int fontSize = this->_GetScreenFontSize();
		this->TextActor = vtkTextActor::New();
		this->TextActor->GetTextProperty()->SetFontSize(fontSize);
		this->TextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
		this->TextActor->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
		this->TextActor->SetPosition(this->Position);
		this->Renderer->AddActor(this->TextActor);

		this->TextInputActor = vtkTextActor::New();
		this->TextInputActor->GetTextProperty()->SetFontSize(fontSize);
		this->TextInputActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
		this->TextInputActor->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
		this->TextInputActor->SetPosition(this->InputPosition);
	}
}

void vtkvmtkRenderer::Execute()
{
	this->Initialize();
}

//Add Prompt Message And Ask the Customer to Input
//return the Input text
const char* vtkvmtkRenderer::PromptAsync(const char* queryText /*,callback*/)
{
	this->SetTextInputQuery(queryText);
	char* empty = '\0';
	this->SetCurrentTextInput(empty);
	//self.ExitTextInputCallback = callback;
	this->UpdateTextInput();
	this->EnterTextInputMode(0);
	return this->GetCurrentTextInput();
}

//Only Add the Prompt Message
void vtkvmtkRenderer::SetPromptMessage(const char* Info)
{
	this->SetTextInputQuery(Info);
	char* empty = '\0';
	this->SetCurrentTextInput(empty);
	this->UpdateTextInput();
}

std::string getTimeRenderer()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H%M%S", localtime(&timep));
	return tmp;
}