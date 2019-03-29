/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vmtkImageWriter,v $
Language:  C++
Date:      $Date: 2019/03/28 17:25:47 $
Version:   $Revision: 1.0 $

Modified: changed from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimagevoiselector.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkImageVOISelector_H
#define __vtkvmtkImageVOISelector_H

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkCubeSource.h>
#include <vtkActor.h>
#include <vtkBoxWidget.h>
#include <vtkSetGet.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

//VMTK-Src
#include <vtkvmtkImagePlaneWidget.h>

//VTK-VMTK
#include "vtkvmtkImageRender.h"

class  vtkvmtkImageVOISelector : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageVOISelector *New();
	vtkTypeMacro(vtkvmtkImageVOISelector, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//Set Or Get the toggle interactivity
	vtkSetMacro(Interactive, bool);
	vtkGetMacro(Interactive, bool);

	//bounds of the cubical region of interest
	vtkSetVector6Macro(BoxBounds, double);

	//The input image
	void SetImage(vtkImageData* data){ this->Image->DeepCopy(data); }
	//Get Output Cropped Image
	vtkImageData* GetImage() { return this->Image; }
	//external renderer
	void SetvmtkRenderer(vtkvmtkImageRenderer *renderer) { this->vmtkRenderer = renderer; }

	//Execute
	void Execute();

	//the input image
	//vtkSetObjectMacro(Image, vtkImageData);
	//vtkGetObjectMacro(Image, vtkImageData);
	vtkImageData* GetImage() { return this->Image; };


protected:
	vtkvmtkImageVOISelector();
	~vtkvmtkImageVOISelector();
	//Input Arguments
	vtkCubeSource *CubeSource;
	vtkActor *CubeActor;
	bool BoxActive;
	bool Interactive;
	double BoxBounds[6];
	vtkImageData *CroppedImage;
	vtkvmtkImageRenderer *vmtkRenderer;
	bool OwnRenderer;
	vtkvmtkImagePlaneWidget *PlaneWidgetX;
	vtkvmtkImagePlaneWidget *PlaneWidgetY;
	vtkvmtkImagePlaneWidget *PlaneWidgetZ;
	vtkBoxWidget *BoxWidget;
	vtkImageData *Image;

	//The BoxActor call Method
	void HideCube(vtkObject * caller, long unsigned int evId, void* clientData, void* callData);
	void UpdateCube(vtkObject * caller, long unsigned int evId, void* clientData, void* callData);

	//Inner Method
	void Display();
	void ExtractVOI();

private:
	vtkvmtkImageVOISelector(const vtkvmtkImageVOISelector&) = delete;
	void operator== (const vtkvmtkImageVOISelector&) = delete;
};

#endif