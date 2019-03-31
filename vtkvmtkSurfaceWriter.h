/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageWriter,v $
Language:  C++
Date:      $Date: 2019/03/30 17:56:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtksurfacewriter.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkSurfaceWriter_H
#define __vtkvmtkSurfaceWriter_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

class  vtkvmtkSurfaceWriter : public vtkImageAlgorithm
{
public:
	static vtkvmtkSurfaceWriter *New();
	vtkTypeMacro(vtkvmtkSurfaceWriter, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//"vtkxml","vtk","dicom","raw","meta","tiff","png"]','file format'
	vtkSetStringMacro(Format);
	vtkGetStringMacro(Format);

	//guess file format from extension
	vtkSetMacro(GuessFormat, bool);
	vtkGetMacro(GuessFormat, bool);

	//use ITKIO mechanism
	vtkSetMacro(UseITKIO, bool);
	vtkGetMacro(UseITKIO, bool);

	//'["ascii","binary"]','write files in ASCII or binary mode'
	vtkSetStringMacro(Mode);

	//output file name
	vtkSetStringMacro(OutputFileName);

	//Set the Input Image
	void SetSurface(vtkPolyData* data)
	{
		this->Surface->DeepCopy(data);
	}

	//Execute
	void Execute();

	//Output File Methods
	void WriteVTKSurfaceFile();
	void WriteVTKXMLSurfaceFile();
	void WriteSTLSurfaceFile();
	void WritePLYSurfaceFile();
	void WritePointDataSurfaceFile();
	void WriteTecplotSurfaceFile();

protected:
	vtkvmtkSurfaceWriter();
	~vtkvmtkSurfaceWriter();
	//Input Arguments
	char *Format;
	bool GuessFormat;
	bool UseITKIO;
	bool CellData;
	char* OutputFileName;
	char* Mode;
	vtkPolyData* Surface;

private:
	vtkvmtkSurfaceWriter(const vtkvmtkSurfaceWriter&) = delete;
	void operator== (const vtkvmtkSurfaceWriter&) = delete;
};

#endif