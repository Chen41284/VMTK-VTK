/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageRender,v $
Language:  C++
Date:      $Date: 2019/03/23 17:55:47 $
Version:   $Revision: 1.0 $

Modified: changed from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkrenderer.py by @chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __vtkvmtkImageRender_H
#define __vtkvmtkImageRender_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkCamera.h>
#include <vtkObject.h>
#include <vtkTextActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <map>

class vtkvmtkImageRenderer;
class KeyPressInteractorStyle;

class vmtkRendererInputStream : public vtkImageAlgorithm
{
public:
	static vmtkRendererInputStream* New();
	vtkTypeMacro(vmtkRendererInputStream, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;


	void SetvmtkRender(vtkvmtkImageRenderer* imageRender)
	{
		this->renderer = imageRender;
	}

	char* ReadLine();
	void promt(const char* text);

	vmtkRendererInputStream(vtkvmtkImageRenderer* render);
protected:
	vtkvmtkImageRenderer* renderer;
	vmtkRendererInputStream();
	~vmtkRendererInputStream();
};


// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
	static KeyPressInteractorStyle* New();
	vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);

	virtual void OnKeyPress() override;

	void SetvmtkRender(vtkvmtkImageRenderer* imageRender)
	{
		this->renderer = imageRender;
	}

protected:
	vtkvmtkImageRenderer* renderer;
};

//renderer used to make several viewers use the same rendering window
class  vtkvmtkImageRenderer : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageRenderer *New();
	vtkTypeMacro(vtkvmtkImageRenderer, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//toggle rendering of annotations superimposed to the renderer
	vtkSetMacro(Annotations, bool);
	vtkGetMacro(Annotations, bool);

	//size of the rendering window in pixels
	vtkSetVector2Macro(WindowSize, int);
	vtkGetVector2Macro(WindowSize, int);

	//position of the rendering window (top left pixel)
	vtkSetVector2Macro(WindowPosition, int);
	vtkGetVector2Macro(WindowPosition, int);

	//toggle rendering smooth points
	vtkSetMacro(PointSmoothing, bool);
	vtkGetMacro(PointSmoothing, bool);

	//toggle rendering smooth lines
	vtkSetMacro(LineSmoothing, bool);
	vtkGetMacro(LineSmoothing, bool);

	//toggle rendering smooth polygons
	vtkSetMacro(PolygonSmoothing, bool);
	vtkGetMacro(PolygonSmoothing, bool);

	//toggle rendering of annotations superimposed to the renderer
	vtkSetMacro(TextInputMode, bool);
	vtkGetMacro(TextInputMode, bool);

	//background color of the rendering window
	vtkSetVector3Macro(Background, double);
	vtkGetVector3Macro(Background, double);

	//UseRendererInputStream
	vtkSetMacro(UseRendererInputStream, bool);
	vtkGetMacro(UseRendererInputStream, bool);

	//magnification to apply to the rendering window when taking a screenshot
	vtkSetMacro(ScreenshotMagnification, int);
	vtkGetMacro(ScreenshotMagnification, int);

	//change the CurrentInput Text
	vtkSetStringMacro(CurrentTextInput);
	vtkGetStringMacro(CurrentTextInput);

	//change the Query Text
	vtkSetStringMacro(TextInputQuery);
	vtkGetStringMacro(TextInputQuery);

	//Set or get the RenderWindowInteractor
	//vtkSetObjectMacro(RenderWindowInteractor, vtkRenderWindowInteractor);
	//vtkGetObjectMacro(RenderWindowInteractor, vtkRenderWindowInteractor);
	void SetRenderWindowInteractor(vtkRenderWindowInteractor* inter) { this->RenderWindowInteractor = inter; };
	vtkRenderWindowInteractor *GetRenderWindowInteractor(){ return this->RenderWindowInteractor; };

	//Get the Renderer
	vtkRenderer* GetRenderer() { return this->Renderer; };

	//Get the RenderWindow
	vtkRenderWindow* GetRenderWindow() { return this->RenderWindow; };

	//Execute
	void Execute();
	//Read Image Method
	int _GetScreenFontSize();
	void UpdateTextInput();
	void ResetCameraCallback();
	void ScreenshotCallback();
	void QuitRendererCallback();
	void EnterTextInputMode(bool interactive = 1);
	void ExitTextInputMode();
	void Render(bool interactive = 1);
	void Initialize();
	void Close();

	//Call by Another Class
	//callback is methond, the argument is not known
	void PromptAsync(const char* queryText /*,callback*/);

protected:
	vtkvmtkImageRenderer();
	~vtkvmtkImageRenderer();
	vtkRenderer *Renderer;
	vtkRenderWindow *RenderWindow;
	vtkRenderWindowInteractor *RenderWindowInteractor;
	vtkCamera *Camera;

	//Input Arguments
	int WindowSize[2];
	int WindowPosition[2];
	double Background[3];
	bool Annotations;
	bool PointSmoothing;
	bool LineSmoothing;
	bool PolygonSmoothing;
	bool TextInputMode;
	bool ExitAfterTextInputMode;
	vtkObject *ExitTextInputCallback;
	vtkTextActor *TextInputActor;
	char *TextInputQuery;
	char *CurrentTextInput;
	double InputPosition[2];
	vtkTextActor *TextActor;
	double Position[2];
	int ScreenshotMagnification;
	bool UseRendererInputStream;

private:
	vtkvmtkImageRenderer(const vtkvmtkImageRenderer&) = delete;
	void operator== (const vtkvmtkImageRenderer&) = delete;
};

#endif
