/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkSurfaceViewer,v $
Language:  C++
Date:      $Date: 2019/03/31 16:38:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtksurfaceviewer.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkSurfaceViewer_H
#define __vtkvmtkSurfaceViewer_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkActor.h>
#include <vtkScalarBarActor.h>
#include <vtkActor2D.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <set>

//VMTK-NEW
#include "vtkvmtkRenderer.h"

class  vtkvmtkSurfaceViewer;

// Define interaction style
class KeyPressInteractorStyleSurface : public vtkInteractorStyleTrackballCamera
{
public:
	static KeyPressInteractorStyleSurface* New();
	vtkTypeMacro(KeyPressInteractorStyleSurface, vtkInteractorStyleTrackballCamera);

	virtual void OnKeyPress() override;

	void SetSurfaceViewer(vtkvmtkSurfaceViewer* surfaceViewer)
	{
		this->surfaceViewer = surfaceViewer;
	}

protected:
	vtkvmtkSurfaceViewer* surfaceViewer;
};

class  vtkvmtkSurfaceViewer : public vtkImageAlgorithm
{
public:
	static vtkvmtkSurfaceViewer *New();
	vtkTypeMacro(vtkvmtkSurfaceViewer, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//Set Input Attributes
	//toggle rendering
	vtkSetMacro(Display, bool);

	//["surface","wireframe","edges"]','change surface representation
	vtkSetStringMacro(Representation);

	//'(0.0,1.0)','object opacity in the scene'
	vtkSetMacro(Opacity, double);

	//'name of the array where the scalars to be displayed are stored'
	vtkSetStringMacro(ArrayName);

	//'range of the scalar map'
	vtkSetVector2Macro(ScalarRange, double);

	//["rainbow","blackbody","cooltowarm","grayscale"]','choose the color map
	vtkSetStringMacro(ColorMap);

	//number of colors in the color map
	vtkSetMacro(NumberOfColors, int);

	//toggle scalar bar
	vtkSetMacro(Legend, bool);

	//toggle flat or shaded surface display
	vtkSetMacro(FlatInterpolation, bool);

	//toggle display of point or cell data
	vtkSetMacro(DisplayCellData, bool);

	//toggle rendering of tag
	vtkSetMacro(DisplayTag, bool);

	//name of the array where the tags to be displayed are stored
	vtkSetStringMacro(RegionTagArrayName);

	//RGB color of the object in the scene
	vtkSetVector3Macro(Color, double);

	//width of line objects in the scene
	vtkSetMacro(LineWidth, int);

	//title of the scalar bar
	vtkSetStringMacro(LegendTitle);

	//Set the Input Image
	void SetSurface(vtkPolyData* data){ this->Surface = data ; }
	//DeepCoyp Facedata
	void SetInputSurface(vtkPolyData* data)
	{
		this->Surface = vtkPolyData::New();
		this->Surface->DeepCopy(data);
		this->InnerSurface = true;
	}

	//external renderer
	void SetRenderer(vtkvmtkRenderer* renderer) { this->vmtkRenderer = renderer; };
	vtkvmtkRenderer* GetRenderer(void) { return this->vmtkRenderer; };

	//Execute
	void Execute();

	//Outer Method
	void SetSurfaceRepresentation(const char* representation);
	void RepresentationCallback();

protected:
	vtkvmtkSurfaceViewer();
	~vtkvmtkSurfaceViewer();
	//Input Arguments
	vtkPolyData *Surface;
	vtkvmtkRenderer *vmtkRenderer;
	bool InnervmtkRenderer;
	bool OwnRenderer;
	bool InnerSurface;
	bool Display;
	double Opacity;
	char *ArrayName = nullptr;
	double ScalarRange[2];
	char *ColorMap = nullptr;
	int NumberOfColors;
	bool Legend;
	char *LegendTitle;
	bool Grayscale;
	bool FlatInterpolation;
	bool DisplayCellData;
	double Color[3];
	int LineWidth;
	char *Representation = nullptr;
	bool DisplayTag;
	char *RegionTagArrayName = nullptr;
	int NumberOfRegions;
	std::set<double> TagSet;
	vtkActor *Actor;
	vtkActor2D *labelsActor;
	vtkScalarBarActor *ScalarBarActor;

	//Inner Method
	void BuildView();
	void BuildViewWithTag();

private:
	vtkvmtkSurfaceViewer(const vtkvmtkSurfaceViewer&) = delete;
	void operator== (const vtkvmtkSurfaceViewer&) = delete;
};

#endif
