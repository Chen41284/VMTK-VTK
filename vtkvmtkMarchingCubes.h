/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkMachingCubes,v $
Language:  C++
Date:      $Date: 2019/03/30 16:47:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkmarchingcubes.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __vtkvmtkMarchingCubes_H
#define __vtkvmtkMarchingCubes_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

class  vtkvmtkMarchingCubes : public vtkImageAlgorithm
{
public:
	static vtkvmtkMarchingCubes *New();
	vtkTypeMacro(vtkvmtkMarchingCubes, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//name of the array to work with
	vtkSetStringMacro(ArrayName);

	//graylevel to generate the isosurface at
	vtkSetMacro(Level, double);

	//only output the largest connected region of the isosurface
	vtkSetMacro(Connectivity, bool);

	//Execute
	void Execute();

	//the input image
	//vtkSetObjectMacro(Image, vtkImageData);
	//vtkGetObjectMacro(Image, vtkImageData);
	//void SetImage(vtkImageData* data) { this->Image->DeepCopy(data); }
	void SetImage(vtkImageData* data) { this->Image = data; };
	vtkImageData* GetImage() { return this->Image; }

	//The Output data
	vtkPolyData *GetSurface() { return this->Surface; };

protected:
	vtkvmtkMarchingCubes();
	~vtkvmtkMarchingCubes();
	//Input Arguments
	vtkImageData *Image;
	bool Connectivity;
	double Level;
	char* ArrayName;
	vtkPolyData* Surface;

private:
	vtkvmtkMarchingCubes(const vtkvmtkMarchingCubes&) = delete;
	void operator== (const vtkvmtkMarchingCubes&) = delete;
};

#endif
