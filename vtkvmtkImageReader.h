/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vmtkimagereader,v $
Language:  C++
Date:      $Date: 2019/03/21 16:47:47 $
Version:   $Revision: 1.0 $

Modified: changed from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimagereader.py by @chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __vtkvmtkImageReader_H
#define __vtkvmtkImageReader_H

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

class  vtkvmtkImageReader : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageReader *New();
	vtkTypeMacro(vtkvmtkImageReader, vtkImageAlgorithm);
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

	//size of the image header - raw only
	vtkSetMacro(HeaderSize, int);
	vtkGetMacro(HeaderSize, int);

	//input image file name
	vtkSetStringMacro(InputFileName);
	vtkGetStringMacro(InputFileName);

	//input file prefix (e.g. foo_)
	vtkSetStringMacro(InputFilePrefix);
	vtkGetStringMacro(InputFilePrefix);

	//input file pattern (e.g. %s%04d.png)
	vtkSetStringMacro(InputFilePattern);
	vtkGetStringMacro(InputFilePattern);

	//spacing of the image - raw, tiff, png, itk
	vtkSetVector3Macro(DataSpacing, double);
	vtkGetVector3Macro(DataSpacing, double);

	//spacing of the image - raw, tiff, png, itk
	vtkSetVector3Macro(DataOrigin, double);
	vtkGetVector3Macro(DataOrigin, double);

	//["native","axial","coronal","sagittal"]','desired data orientation - itk only
	vtkSetStringMacro(DesiredOrientation);
	vtkGetStringMacro(DesiredOrientation);

	//["littleendian","bigendian"]','byte ordering - raw only
	vtkSetStringMacro(DataByteOrder);
	vtkGetStringMacro(DataByteOrder);

	//["float","double","int","short","ushort","uchar"]','scalar type - raw only
	vtkSetStringMacro(DataScalarType);
	vtkGetStringMacro(DataScalarType);

	//3D extent of the image - raw and png
	vtkSetVector6Macro(DataExtent, int);
	vtkGetVector6Macro(DataExtent, int);

	//dimensionality of the file to read - raw only
	vtkSetMacro(FileDimensionality, int);
	vtkGetMacro(FileDimensionality, int);

	//toggle flipping of the corresponding axis
	vtkSetVector3Macro(Flip, bool);
	vtkGetVector3Macro(Flip, bool);

	//flip a dicom stack in order to have a left-to-right, 
	//posterio-to-anterior, inferior-to-superior image; 
	//this is based on the \"image orientation (patient)\" field in the dicom header
	vtkSetMacro(AutoOrientDICOMImage, bool);
	vtkGetMacro(AutoOrientDICOMImage, bool);

	//Execute
	void Execute();

	//the input image
	//vtkSetObjectMacro(Image, vtkImageData);
	//vtkGetObjectMacro(Image, vtkImageData);
	void SetImage(vtkImageData* data) { this->Image = data; };
	vtkImageData* GetImage() { return this->Image; };

protected:
	vtkvmtkImageReader();
	~vtkvmtkImageReader();
	//Input Arguments
	char *Format;
	bool GuessFormat;
	bool UseITKIO;
	char* InputFileName;
	char* InputFilePrefix;
	char* InputFilePattern;
	vtkImageData *Image;
	int DataExtent[6];
	double DataSpacing[3];
	double DataOrigin[3];
	char* DesiredOrientation;
	char* DataByteOrder;
	char* DataScalarType;
	int HeaderSize = 0;
	int FileDimensionality;
	bool Flip[3];
	bool AutoOrientDICOMImage;

	//OutputArguments
	vtkImageData *Output;
	vtkMatrix4x4* RasToIjkMatrixCoefficients;
	vtkMatrix4x4* XyzToRasMatrixCoefficients;

	//Read Image Method
	void ReadVTKXMLImageFile();
	void ReadVTKImageFile();
	void ReadRawImageFile();
	void ReadMetaImageFile();
	void ReadTIFFImageFile();
	void ReadPNGImageFile();
	void ReadITKIO();

private:
	vtkvmtkImageReader(const vtkvmtkImageReader&) = delete;
	void operator== (const vtkvmtkImageReader&) = delete;
};

#endif
