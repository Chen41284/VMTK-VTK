#include "vtkvmtkImageReader.h"

//VTK Includes
#include <vtkObjectFactory.h>
#include <vtkImageFlip.h>
#include <vtkXMLImageDataReader.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>
#include <vtkImageReader.h>
#include <vtkMetaImageReader.h>
#include <vtkTIFFReader.h>
#include <vtkPNGReader.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkvmtkITKArchetypeImageSeriesScalarReader.h>

//STD Includes
#include <cmath>
#include <cstdlib>
#include <map>


vtkStandardNewMacro(vtkvmtkImageReader);

vtkvmtkImageReader::vtkvmtkImageReader()
{
	this->Format = nullptr;
	this->GuessFormat = 1;
	this->UseITKIO = 1;
	this->InputFileName = nullptr;
	this->InputFilePrefix = nullptr;
	this->InputFilePattern = nullptr;
	this->Image = 0;
	this->Output = 0;
	for (int i = 0; i < 6; i++)
		this->DataExtent[i] = -1;
	for (int i = 0; i < 3; i++)
	{
		this->DataSpacing[i] = 1.0;
		this->DataOrigin[i] = 0.0;
		this->Flip[i] = 0;
	}
	this->DataByteOrder = (char *)"littleendian";
	this->DataScalarType = (char *)"float";
	this->DesiredOrientation = (char *)"native";
	this->HeaderSize = 0;
	this->FileDimensionality = 3;
	this->AutoOrientDICOMImage = 1;
	this->RasToIjkMatrixCoefficients = vtkMatrix4x4::New();
	this->RasToIjkMatrixCoefficients->Identity();
	this->XyzToRasMatrixCoefficients = vtkMatrix4x4::New();
	this->XyzToRasMatrixCoefficients->Identity();
}

vtkvmtkImageReader::~vtkvmtkImageReader()
{
	if (this->Format)
	{
		delete[] this->Format;
		this->Format = nullptr;
	}
	if (this->InputFileName)
	{
		delete[] this->InputFileName;
		this->InputFileName = nullptr;
	}
	if (this->InputFilePrefix)
	{
		delete[] this->InputFilePrefix;
		this->InputFilePrefix = nullptr;
	}
	if (this->InputFilePattern)
	{
		delete[] this->InputFilePattern;
		this->InputFilePattern = nullptr;
	}
	if (this->RasToIjkMatrixCoefficients)
	{
		RasToIjkMatrixCoefficients->Delete();
		this->RasToIjkMatrixCoefficients = nullptr;
	}
	if (this->XyzToRasMatrixCoefficients)
	{
		XyzToRasMatrixCoefficients->Delete();
		this->XyzToRasMatrixCoefficients = nullptr;
	}
}

void vtkvmtkImageReader::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Reader\n";
}

void vtkvmtkImageReader::Execute()
{
	std::map<std::string, const char*> extensionFormats;
	extensionFormats["vti"] = "vtkxml";
	extensionFormats["vtk"] = "vtk";
	extensionFormats["dcm"] = "dicom";
	extensionFormats["raw"] = "raw";
	extensionFormats["mhd"] = "meta";
	extensionFormats["mha"] = "meta";
	extensionFormats["tif"] = "tiff";
	extensionFormats["png"] = "png";

	if (this->GuessFormat && this->InputFileName != nullptr && this->Format == nullptr)
	{
		std::string fileName = this->InputFileName;
		size_t index = fileName.find_first_of(".");
		if (index != std::string::npos)
		{
			std::string extension = fileName.substr(++index);
			std::map<std::string, const char*>::iterator it;
			it = extensionFormats.find(extension);
			if (it != extensionFormats.end())
				this->SetFormat(extensionFormats[extension]);
				//this->Format = (char*)extensionFormats[extension];  
			// strcmp(this->Format, "vtkxml")
			//Set Method use the string
		}
	}
	//Use ITKIO the Format in not [vtkxml, vtk, raw]
	if (this->UseITKIO == true &&
		this->InputFileName != nullptr &&
		strcmp(this->Format, "vtkxml") && strcmp(this->Format, "vtk") &&
		strcmp(this->Format, "raw")) 
	{
		this->ReadITKIO();
	}
	else
	{
		if (!strcmp(Format, "vtkxml"))
			this->ReadVTKXMLImageFile();
		else if (!strcmp(Format, "vtk"))
			this->ReadVTKImageFile();
		else if (!strcmp(Format, "raw"))
			this->ReadRawImageFile();
		else if (!strcmp(Format, "meta"))
			this->ReadMetaImageFile();
		else if (!strcmp(Format, "png"))
			this->ReadPNGImageFile();
		else if (!strcmp(Format, "tiff"))
			this->ReadTIFFImageFile();
		else if (!strcmp(Format, "dicom"))
			std::cerr << "Error: please enable parameter UseITKIO in order to read dicom files." << std::endl;
		else
			std::cerr << "Error: unsupported format: " + std::string(Format) + '.' << std::endl;
	}

	if (this->Flip[0] == 1 || this->Flip[1] == 1 || this->Flip[2] == 1)
	{
		vtkImageData *temp0 = this->Image;
		if (this->Flip[0] == 1)
		{
			vtkImageFlip *flipFilter = vtkImageFlip::New();
			flipFilter->SetInputData(this->Image);
			flipFilter->SetFilteredAxis(0);
			flipFilter->Update();
			temp0 = flipFilter->GetOutput();
		}

		vtkImageData* temp1 = temp0;
		if (this->Flip[1] == 1)
		{
			vtkImageFlip *flipFilter = vtkImageFlip::New();
			flipFilter->SetInputData(temp0);
			flipFilter->SetFilteredAxis(1);
			flipFilter->Update();
			temp1 = flipFilter->GetOutput();
		}

		vtkImageData* temp2 = temp1;
		if (this->Flip[2] == 1)
		{
			vtkImageFlip *flipFilter = vtkImageFlip::New();
			flipFilter->SetInputData(temp1);
			flipFilter->SetFilteredAxis(2);
			flipFilter->Update();
			temp2 = flipFilter->GetOutput();
		}
		this->Image = temp2;

		printf("Spacing %f %f %f" , this->Image->GetSpacing());
		printf("Origin %f %f %f" , this->Image->GetOrigin());
		printf("Dimensions %d %d %d" , this->Image->GetDimensions());
	}
	this->Output = this->Image;
}

void vtkvmtkImageReader::ReadVTKXMLImageFile()
{
	if (this->InputFileName == nullptr)
		std::cerr << "Error: no InputFileName." << std::endl;
	std::cout << "Reading VTK XML image file." << std::endl;
	vtkXMLImageDataReader *reader = vtkXMLImageDataReader::New();
	reader->SetFileName(this->InputFileName);
	reader->Update();
	this->Image = reader->GetOutput();
}

void vtkvmtkImageReader::ReadVTKImageFile()
{
	if (this->InputFileName == nullptr)
		std::cerr << "Error: no InputFileName." << std::endl;
	std::cout << "Reading VTK image file." << std::endl;
	vtkStructuredPointsReader *reader = vtkStructuredPointsReader::New();
	reader->SetFileName(this->InputFileName);
	reader->Update();
	this->Image = reader->GetOutput();
}


// RAW File:the current Information defined by the user
// Set the:
//     FilePrefix, FilePattern, FileDimension, 
//     DataExtent, DataSpacing, DataOrigin
//	   HeaderSize, DataScalarType
void vtkvmtkImageReader::ReadRawImageFile()
{
	if (this->InputFileName == nullptr && this->InputFilePrefix == nullptr)
	{
		std::cerr << "Error: no InputFileName or InputFilePrefix." << std::endl;
	}
	std::cout << "Reading RAW image file." << std::endl;
	vtkImageReader * reader = vtkImageReader::New();
	if (this->InputFileName != nullptr)
		reader->SetFileName(this->InputFileName);
	else // Not InputFileName, but have FilePrefix or FilePattern
	{
		reader->SetFilePrefix(this->InputFilePrefix);
		if (this->InputFilePattern != nullptr)
			reader->SetFilePattern(this->InputFilePattern);
		else
			reader->SetFilePattern("%s%04d.png");
	}
	reader->SetFileDimensionality(this->FileDimensionality);
	if (DataByteOrder == "littleendian")
		reader->SetDataByteOrderToLittleEndian();
	else if (DataByteOrder == "bigendian")
		reader->SetDataByteOrderToBigEndian();
	reader->SetDataExtent(this->DataExtent);
	reader->SetDataSpacing(this->DataSpacing);
	reader->SetDataOrigin(this->DataOrigin);
	reader->SetHeaderSize(this->HeaderSize);
	if (this->DataScalarType == "float")
		reader->SetDataScalarTypeToFloat();
	else if (this->DataScalarType == "double")
		reader->SetDataScalarTypeToDouble();
	else if (this->DataScalarType == "int")
		reader->SetDataScalarTypeToInt();
	else if (this->DataScalarType == "short")
		reader->SetDataScalarTypeToShort();
	else if (this->DataScalarType == "ushort")
		reader->SetDataScalarTypeToUnsignedShort();
	else if (this->DataScalarType == "uchar")
		reader->SetDataScalarTypeToUnsignedChar();
	reader->Update();
	this->Image = reader->GetOutput();
}

void vtkvmtkImageReader::ReadMetaImageFile()
{
	if (this->InputFileName == nullptr)
		std::cerr << "Error: no InputFileName." << std::endl;
	std::cout << "Reading meta image file." << std::endl;
	vtkMetaImageReader *reader = vtkMetaImageReader::New();
	reader->SetFileName(this->InputFileName);
	reader->Update();
	this->Image = reader->GetOutput();
}


// if not input the InputFileName, the user should defined the following arguments
//	  FilePrefix, FilePattern, 
//	  DataExtent, DataSpacing, DataOrigin
//
void vtkvmtkImageReader::ReadTIFFImageFile()
{
	if (this->InputFileName == nullptr && this->InputFilePrefix == nullptr)
	{
		std::cerr << "Error: no InputFileName or InputFilePrefix." << std::endl;
	}
	std::cout << "Reading TIFF image file." << std::endl;
	vtkTIFFReader *reader = vtkTIFFReader::New();
	if (this->InputFileName != nullptr)
		reader->SetFileName(this->InputFileName);
	else // Not InputFileName, but have FilePrefix or FilePattern
	{
		reader->SetFilePrefix(this->InputFilePrefix);
		if (this->InputFilePattern != nullptr)
			reader->SetFilePattern(this->InputFilePattern);
		else
			reader->SetFilePattern("%s%04d.png");
		reader->SetDataExtent(this->DataExtent);
		reader->SetDataSpacing(this->DataSpacing);
		reader->SetDataOrigin(this->DataOrigin);
	}
	reader->Update();
	this->Image = reader->GetOutput();
}

//
// if not input the InputFileName, the user should defined the following arguments
//	  FilePrefix, FilePattern,
//	  DataExtent, DataSpacing, DataOrigin
//
void vtkvmtkImageReader::ReadPNGImageFile()
{
	if (this->InputFileName == nullptr && this->InputFilePrefix == nullptr)
	{
		std::cerr << "Error: no InputFileName or InputFilePrefix." << std::endl;
	}
	std::cout << "Reading PNG image file." << std::endl;
	vtkPNGReader *reader = vtkPNGReader::New();
	if (this->InputFileName != nullptr)
		reader->SetFileName(this->InputFileName);
	else // Not InputFileName, but have FilePrefix or FilePattern
	{
		reader->SetFilePrefix(this->InputFilePrefix);
		if (this->InputFilePattern != nullptr)
			reader->SetFilePattern(this->InputFilePattern);
		else
			reader->SetFilePattern("%s%04d.png");
		reader->SetDataExtent(this->DataExtent);
		reader->SetDataSpacing(this->DataSpacing);
		reader->SetDataOrigin(this->DataOrigin);
	}
	reader->Update();
	this->Image = reader->GetOutput();
}

void vtkvmtkImageReader::ReadITKIO()
{
	if (this->InputFileName == nullptr)
		std::cout << "Error: no InputFileName." << std::endl;
	vtkvmtkITKArchetypeImageSeriesScalarReader *reader = vtkvmtkITKArchetypeImageSeriesScalarReader::New();
	reader->SetArchetype(this->InputFileName);
	reader->SetDefaultDataSpacing(this->DataSpacing);
	reader->SetDefaultDataOrigin(this->DataOrigin);
	reader->SetOutputScalarTypeToNative();
	if (DesiredOrientation == "native")
		reader->SetDesiredCoordinateOrientationToNative();
	else if (DesiredOrientation == "axial")
		reader->SetDesiredCoordinateOrientationToAxial();
	else if (DesiredOrientation == "coronal")
		reader->SetDesiredCoordinateOrientationToCoronal();
	else if (DesiredOrientation == "sagittal")
		reader->SetDesiredCoordinateOrientationToSagittal();
	reader->SetSingleFile(0);
	reader->Update();
	this->Image = vtkImageData::New();
	this->Image->DeepCopy(reader->GetOutput());
	vtkMatrix4x4 *matrix = reader->GetRasToIjkMatrix();
	const double elements[16] = {
		   matrix->GetElement(0, 0), matrix->GetElement(0, 1), matrix->GetElement(0, 2), matrix->GetElement(0, 3),
		   matrix->GetElement(1, 0), matrix->GetElement(1, 1), matrix->GetElement(1, 2), matrix->GetElement(1, 3),
		   matrix->GetElement(2, 0), matrix->GetElement(2, 1), matrix->GetElement(2, 2), matrix->GetElement(2, 3),
		   matrix->GetElement(3, 0), matrix->GetElement(3, 1), matrix->GetElement(3, 2), matrix->GetElement(3, 3) };
	this->RasToIjkMatrixCoefficients->DeepCopy(elements);
	matrix->Invert();
	double origin[3] = { matrix->GetElement(0, 3), matrix->GetElement(1, 3), matrix->GetElement(2, 3) };
	double translationToOrigin[3] = { -origin[0], -origin[1], -origin[2] };
	for (int i = 0; i < 3; i++)
	{
		double direction[3] = { matrix->GetElement(0, i), matrix->GetElement(1, i), matrix->GetElement(2, i) };
		vtkMath::Normalize(direction);
		matrix->SetElement(0, i, direction[0]);
		matrix->SetElement(1, i, direction[1]);
		matrix->SetElement(2, i, direction[2]);
	}				
	matrix->SetElement(0, 3, 0.0);
	matrix->SetElement(1, 3, 0.0);
	matrix->SetElement(2, 3, 0.0);

	vtkTransform *transform = vtkTransform::New();
	transform->PostMultiply();
	transform->Translate(translationToOrigin);
	transform->Concatenate(matrix);
	transform->Translate(origin);

	matrix = transform->GetMatrix();
	const double elements2[16] = {
		   matrix->GetElement(0, 0), matrix->GetElement(0, 1), matrix->GetElement(0, 2), matrix->GetElement(0, 3),
		   matrix->GetElement(1, 0), matrix->GetElement(1, 1), matrix->GetElement(1, 2), matrix->GetElement(1, 3),
		   matrix->GetElement(2, 0), matrix->GetElement(2, 1), matrix->GetElement(2, 2), matrix->GetElement(2, 3),
		   matrix->GetElement(3, 0), matrix->GetElement(3, 1), matrix->GetElement(3, 2), matrix->GetElement(3, 3) };
	this->XyzToRasMatrixCoefficients->DeepCopy(elements2);
}

