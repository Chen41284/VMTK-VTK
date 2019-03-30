/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageWriter,v $
Language:  C++
Date:      $Date: 2019/03/27 16:30:47 $
Version:   $Revision: 1.0 $

Modified: changed from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimagewriter.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkImageWriter_H
#define __vtkvmtkImageWriter_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
class vtkMatrix4x4;

// STD includes
#include <algorithm>
#include <string>
#include <vector>

class  vtkvmtkImageWriter : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageWriter *New();
	vtkTypeMacro(vtkvmtkImageWriter, vtkImageAlgorithm);
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

	//apply transform on writing - ITKIO only
	vtkSetMacro(ApplyTransform, bool);
	vtkGetMacro(ApplyTransform, bool);

	//output file name
	vtkSetStringMacro(OutputFileName);

	//name of the output raw file - meta image only
	vtkSetStringMacro(OutputRawFileName);

	//output directory name - png, tiff
	vtkSetStringMacro(OutputDirectoryName);

	//["double","float","short"]','output scalar type
	vtkSetStringMacro(PixelRepresentation);

	//window and level for mapping graylevels to 0-255 before writing - png, tiff
	vtkSetVector2Macro(WindowLevel, double);

	//Set RasToIjkMatrixCoefficients
	void SetRasToIjkMatrixCoefficients(vtkMatrix4x4 *matrix)
	{
		this->RasToIjkMatrixCoefficients->DeepCopy(matrix);
	}

	//Set the Input Image
	void SetImage(vtkImageData* data)
	{
		this->Image->DeepCopy(data);
	}

	//Execute
	void Execute();

	//the input image
	//vtkSetObjectMacro(Image, vtkImageData);
	//vtkGetObjectMacro(Image, vtkImageData);
	vtkImageData* GetImage() { return this->Image; };

	//Output File Methods
	void WriteVTKImageFile();
	void WriteVTSXMLVolumeFile();
	void WriteVTKXMLImageFile();
	void WriteMetaImageFile();
	void WritePNGImageFile();
	void WriteTIFFImageFile();
	void WritePointDataImageFile();
	void WriteITKIO();

protected:
	vtkvmtkImageWriter();
	~vtkvmtkImageWriter();
	//Input Arguments
	char *Format;
	bool GuessFormat;
	bool UseITKIO;
	bool ApplyTransform;
	char* OutputFileName;
	char* OutputRawFileName;
	char* OutputDirectoryName;
	char* PixelRepresentation;
	vtkImageData *Image;
	vtkMatrix4x4* RasToIjkMatrixCoefficients;
	double WindowLevel[2];

private:
	vtkvmtkImageWriter(const vtkvmtkImageWriter&) = delete;
	void operator== (const vtkvmtkImageWriter&) = delete;
};

#endif
