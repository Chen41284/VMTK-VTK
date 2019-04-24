/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageSeeder v $
Language:  C++
Date:      $Date: 2019/04/16 17:35:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimageseeder.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __vtkvmtkImageSeeder_H
#define __vtkvmtkImageSeeder_H

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
#include <vtkCellPicker.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <set>

//VMTK-NEW
#include "vtkvmtkRenderer.h"

//VMTK-Src
#include <vtkvmtkImagePlaneWidget.h>

//Static Call Method;
void CtrlSelectSeed(vtkObject * caller, long unsigned int evId, void* clientData, void* callData);

class  vtkvmtkImageSeeder : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageSeeder *New();
	vtkTypeMacro(vtkvmtkImageSeeder, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//vtkImageData
	//浅拷贝
	//void SetImage(vtkImageData* data) { this->Image = data; };
	//vtkImageData* GetImage() { return this->Image; };
	void SetImage(vtkSmartPointer<vtkImageData> data) { this->Image = data; };
	vtkSmartPointer<vtkImageData> GetImage() { return this->Image; };

	//external renderer
	void SetRenderer(vtkvmtkRenderer* renderer) { this->vmtkRenderer = renderer; };
	vtkvmtkRenderer* GetRenderer(void) { return this->vmtkRenderer; };

	//name of the array to display
	vtkSetStringMacro(ArrayName);

	//toggle rendering
	vtkSetMacro(Display, bool);

	//toggle avoid removal of seeds from renderer
	vtkSetMacro(KeepSeeds, bool);

	//toggle interpolation of graylevels on image planes
	vtkSetMacro(TextureInterpolation, bool);

	//获取Seeds，主要用于静态函数的调用
	//vtkPolyData* GetSeeds() { return this->Seeds; };
	vtkSmartPointer<vtkPolyData> GetSeeds() { return this->Seeds; };

	void BuildView();
	void Execute();
	void InitializeSeeds();

	void WidgetsOn();
	void WidgetsOff();

protected:
	vtkvmtkImageSeeder();
	~vtkvmtkImageSeeder();

	//vtkImageData *Image;
	vtkvmtkRenderer *vmtkRenderer;
	vtkSmartPointer<vtkImageData> Image;
	bool InnervmtkRenderer;
	bool Display;
	char *ArrayName = nullptr;
	//vtkCellPicker *Picker;
	//vtkvmtkImagePlaneWidget *PlaneWidgetX;
	//vtkvmtkImagePlaneWidget *PlaneWidgetY;
	//vtkvmtkImagePlaneWidget *PlaneWidgetZ;
	//vtkActor *SeedActor;
	//vtkPolyData *Seeds;
	//vtkPoints *SeedPoints;

	vtkSmartPointer<vtkCellPicker> Picker;
	vtkSmartPointer<vtkvmtkImagePlaneWidget> PlaneWidgetX;
	vtkSmartPointer<vtkvmtkImagePlaneWidget> PlaneWidgetY;
	vtkSmartPointer<vtkvmtkImagePlaneWidget> PlaneWidgetZ;
	vtkSmartPointer<vtkActor> SeedActor;
	vtkSmartPointer<vtkPolyData> Seeds;
	vtkSmartPointer<vtkPoints> SeedPoints;
	bool TextureInterpolation;
	bool KeepSeeds;

	//Inner Method
	
private:
	vtkvmtkImageSeeder(const vtkvmtkImageSeeder&) = delete;
	void operator== (const vtkvmtkImageSeeder&) = delete;
};

#endif
