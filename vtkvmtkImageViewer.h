/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageViewer,v $
Language:  C++
Date:      $Date: 2019/03/25 17:44:47 $
Version:   $Revision: 1.0 $

Modified: changed from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimageviewer.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __vtkvmtkImageViewer_H
#define __vtkvmtkImageViewer_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkCellPicker.h>
#include <vtkImagePlaneWidget.h>

// VMTK
#include "vtkvmtkImageReader.h"
#include "vtkvmtkImageRender.h"

// STD includes
#include <algorithm>
#include <string>
#include <math.h>

class  vtkvmtkImageViewer : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageViewer *New();
	vtkTypeMacro(vtkvmtkImageViewer, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//the input image
	//vtkSetObjectMacro(Image, vtkImageData);
	//vtkGetObjectMacro(Image, vtkImageData);
	void SetImage(vtkImageData* data) { this->Image = data; };
	vtkImageData* GetImage() { return this->Image; };

	//name of the array to display
	vtkSetStringMacro(ArrayName);

	//external renderer
	//vtkSetObjectMacro(vmtkRenderer, vtkvmtkImageRenderer);
	void SetvmtkRenderer(vtkvmtkImageRenderer* renderer) { this->vmtkRenderer = renderer; };

	//the window/level for displaying the images
	vtkSetVector2Macro(WindowLevel, double);

	//toggle rendering
	vtkSetMacro(Display, bool);

	//toggle margins for tilting image planes
	vtkSetMacro(Margins, bool);

	//toggle interpolation of graylevels on image planes
	vtkSetMacro(TextureInterpolation, bool);

	//toggle use of physical continuous coordinates for the cursor
	vtkSetMacro(ContinuousCursor, bool);

	//the X image plane widget
	//vtkGetObjectMacro(PlaneWidgetX, vtkImagePlaneWidget);
	vtkImagePlaneWidget *GetPlaneWidgetX() { return this->PlaneWidgetX; };

	//the Y image plane widget
	//vtkGetObjectMacro(PlaneWidgetY, vtkImagePlaneWidget);
	vtkImagePlaneWidget *GetPlaneWidgetY() { return this->PlaneWidgetY; };

	//the Z image plane widget
	//vtkGetObjectMacro(PlaneWidgetZ, vtkImagePlaneWidget);
	vtkImagePlaneWidget *GetPlaneWidgetZ() { return this->PlaneWidgetZ; };

	//Execute
	void Execute();

protected:
	vtkvmtkImageViewer();
	~vtkvmtkImageViewer();
	void BuildView();

	vtkImageData *Image;
	vtkvmtkImageRenderer* vmtkRenderer;
	bool OwnRenderer;
	bool Display;
	char *ArrayName;
	vtkCellPicker *Picker;
	vtkImagePlaneWidget *PlaneWidgetX;
	vtkImagePlaneWidget *PlaneWidgetY;
	vtkImagePlaneWidget *PlaneWidgetZ;
	bool Margins;
	bool TextureInterpolation;
	bool ContinuousCursor;
	double WindowLevel[2];
private:
	vtkvmtkImageViewer(const vtkvmtkImageViewer&) = delete;
	void operator== (const vtkvmtkImageViewer&) = delete;
};

#endif

