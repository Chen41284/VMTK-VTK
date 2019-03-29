#include "vtkvmtkImageWriter.h"

//VTK Includes
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkTransform.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkTransformFilter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageShiftScale.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkImageCast.h>

//STD Includes
#include <cmath>
#include <cstdlib>
#include <map>
#include <fstream>
#include <vector>
#include <string.h>

//VMTK Source Code
#include <vtkvmtkITKImageWriter.h>

vtkStandardNewMacro(vtkvmtkImageWriter);

vtkvmtkImageWriter::vtkvmtkImageWriter()
{
	this->Format = NULL;
	this->GuessFormat = true;
	this->UseITKIO = true;
	this->ApplyTransform = false;
	this->OutputFileName = NULL;
	this->OutputRawFileName = NULL;
	this->OutputDirectoryName = NULL;
	this->PixelRepresentation = NULL;
	this->Image = NULL;
	this->WindowLevel[0] = 1.0; this->WindowLevel[1] = 0.0;
	this->RasToIjkMatrixCoefficients = vtkMatrix4x4::New();
	this->RasToIjkMatrixCoefficients->Identity();
	this->Image = vtkImageData::New();
}

vtkvmtkImageWriter::~vtkvmtkImageWriter()
{
	if (this->Format != NULL)
	{
		delete[] this->Format;
		this->Format = NULL;
	}
	if (this->OutputFileName != NULL)
	{
		delete[] this->OutputFileName;
		this->OutputFileName = NULL;
	}
	if (this->OutputRawFileName != NULL)
	{
		delete[] this->OutputRawFileName;
		this->OutputRawFileName = NULL;
	}
	if (this->OutputDirectoryName != NULL)
	{
		delete[] this->OutputDirectoryName;
		this->OutputDirectoryName = NULL;
	}
	if (this->PixelRepresentation != NULL)
	{
		delete[] this->PixelRepresentation;
		this->PixelRepresentation = NULL;
	}
	if (this->RasToIjkMatrixCoefficients != NULL)
	{
		this->RasToIjkMatrixCoefficients->Delete();
		this->RasToIjkMatrixCoefficients = NULL;
	}
	if (this->Image != NULL)
	{
		this->Image->Delete();
		this->Image = NULL;
	}
}

void vtkvmtkImageWriter::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Writer\n";
}

void vtkvmtkImageWriter::WriteVTKImageFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing VTK Image file." << std::endl;
	vtkSmartPointer<vtkStructuredPointsWriter> writer =
		vtkSmartPointer <vtkStructuredPointsWriter>::New();
	writer->SetInputData(this->Image);
	writer->SetFileName(this->OutputFileName);
	writer->Write();
}

void vtkvmtkImageWriter::WriteVTSXMLVolumeFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing VTS XML grid file." << std::endl;
	vtkSmartPointer<vtkMatrix4x4> matrix =
		vtkSmartPointer<vtkMatrix4x4>::New();
	if (this->ApplyTransform == false)
	{
		double *origin = this->Image->GetOrigin();
		double *spacing = this->Image->GetSpacing();
		double elements[16] = {
			1 / spacing[0], 0, 0, -origin[0] / spacing[0],
			0, 1 / spacing[1], 0, -origin[1] / spacing[1],
			0, 0, 1 / spacing[2], -origin[2] / spacing[2],
			0, 0, 0, 1
		};
	}
	else
	{
		matrix->DeepCopy(this->RasToIjkMatrixCoefficients);
	}
	vtkSmartPointer<vtkTransform> trans = 
		vtkSmartPointer<vtkTransform>::New();
	trans->SetMatrix(matrix);
	vtkSmartPointer<vtkTransformFilter> trans_filt =
		vtkSmartPointer<vtkTransformFilter>::New();
	trans_filt->SetTransform(trans);
	trans_filt->SetInputData(this->Image);
	trans_filt->Update();
	vtkSmartPointer<vtkXMLStructuredGridWriter> writer =
		vtkSmartPointer<vtkXMLStructuredGridWriter>::New();
	writer->SetInputConnection(trans_filt->GetOutputPort());
	writer->SetFileName(this->OutputFileName);
	writer->Write();
}

void vtkvmtkImageWriter::WriteVTKXMLImageFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing VTK XML image file." << std::endl;
	vtkSmartPointer<vtkXMLImageDataWriter> writer =
		vtkSmartPointer <vtkXMLImageDataWriter>::New();
	writer->SetInputData(this->Image);
	writer->SetFileName(this->OutputFileName);
	writer->Write();
}

void vtkvmtkImageWriter::WriteMetaImageFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing meta image file." << std::endl;
	vtkSmartPointer<vtkMetaImageWriter> writer =
		vtkSmartPointer <vtkMetaImageWriter>::New();
	writer->SetInputData(this->Image);
	writer->SetFileName(this->OutputFileName);
	if (this->OutputRawFileName != NULL)
		writer->SetRAWFileName(this->OutputRawFileName);
	writer->Write();
}

void vtkvmtkImageWriter::WritePNGImageFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing PNG image file." << std::endl;
	vtkImageData* OutputImage = this->Image;
	if (this->Image->GetScalarTypeAsString() != "unsigned char")
	{
		vtkImageShiftScale *shiftScale = vtkImageShiftScale::New();
		shiftScale->SetInputData(this->Image);
		if (this->WindowLevel[0] == 0.0)
		{
			double *scalarRange = this->Image->GetScalarRange();
			shiftScale->SetShift(-scalarRange[0]);
			shiftScale->SetScale(255.0 / (scalarRange[1] - scalarRange[0]));
		}
		else
		{
			shiftScale->SetShift(-(this->WindowLevel[1] - this->WindowLevel[0] / 2.0));
			shiftScale->SetScale(255.0 / this->WindowLevel[0]);
		}
		shiftScale->SetOutputScalarTypeToUnsignedChar();
		shiftScale->ClampOverflowOn();
		shiftScale->Update();
		OutputImage = shiftScale->GetOutput();
	}
	vtkSmartPointer<vtkPNGWriter> writer = 
		vtkSmartPointer<vtkPNGWriter>::New();
	writer->SetInputData(OutputImage);
	if (this->Image->GetDimensions()[2] == 1)
		writer->SetFileName(this->OutputFileName);
	else
	{
		writer->SetFilePrefix(this->OutputFileName);
		writer->SetFilePattern("%s%04d.png");
	}
	writer->Write();
}

void vtkvmtkImageWriter::WriteTIFFImageFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing TIFF image file." << std::endl;
	vtkImageData* OutputImage = this->Image;
	if (this->Image->GetScalarTypeAsString() != "unsigned char")
	{
		vtkImageShiftScale *shiftScale = vtkImageShiftScale::New();
		shiftScale->SetInputData(this->Image);
		if (this->WindowLevel[0] == 0.0)
		{
			double *scalarRange = this->Image->GetScalarRange();
			shiftScale->SetShift(-scalarRange[0]);
			shiftScale->SetScale(255.0 / (scalarRange[1] - scalarRange[0]));
		}
		else
		{
			shiftScale->SetShift(-(this->WindowLevel[1] - this->WindowLevel[0] / 2.0));
			shiftScale->SetScale(255.0 / this->WindowLevel[0]);
		}
		shiftScale->SetOutputScalarTypeToUnsignedChar();
		shiftScale->ClampOverflowOn();
		shiftScale->Update();
		OutputImage = shiftScale->GetOutput();
	}
	vtkSmartPointer<vtkTIFFWriter> writer =
		vtkSmartPointer<vtkTIFFWriter>::New();
	writer->SetInputData(OutputImage);
	if (this->Image->GetDimensions()[2] == 1)
		writer->SetFileName(this->OutputFileName);
	else
	{
		writer->SetFilePrefix(this->OutputFileName);
		writer->SetFilePattern("%s%04d.tif");
	}
	writer->Write();
}

void vtkvmtkImageWriter::WritePointDataImageFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing PointData file." << std::endl;
	FILE* fp = NULL;
	fp = fopen(this->OutputFileName, "w");
	// Header Information
	std::string line = "X Y Z";
	std::vector<std::string> arrayNames;
	if (this->Image->GetPointData()->GetScalars()->GetName() == NULL ||
		this->Image->GetPointData()->GetScalars()->GetName() == '\0')
	{
		this->Image->GetPointData()->GetScalars()->SetName("__Scalars");
	}
	int numberArray = this->Image->GetPointData()->GetNumberOfArrays();
	for (int i = 0; i < numberArray; i++)
	{
		vtkDataArray *array = this->Image->GetPointData()->GetArray(i);
		std::string arrayName = array->GetName();
		if (arrayName.empty())
			continue;
		if (arrayName.back() == '_')
			continue;
		arrayNames.push_back(arrayName);
		if (array->GetNumberOfComponents() == 1)
			line = line + ' ' + arrayName; 
		else
		{
			for (int j = 0; j < array->GetNumberOfComponents(); j++)
				line = line + ' ' + arrayName + std::to_string(j);
		}
	}
	line = line + '\n';
	fprintf(fp, "%s", line);
	line.clear();
	//The Image Data point
	int numberPoints = this->Image->GetNumberOfPoints();
	for (int i = 0; i < numberPoints; i++)
	{
		double *point = this->Image->GetPoint(i);
		line = std::to_string(point[0]) + ' ' + std::to_string(point[1]) + ' ' + std::to_string(point[2]);
		for (auto arrayName : arrayNames)
		{
			vtkDataArray *array = this->Image->GetPointData()->GetArray(arrayName.c_str());
			for (int j = 0; j < array->GetNumberOfComponents(); j++)
				line = line + ' ' + std::to_string(array->GetComponent(i, j));
		}
		line = line + '\n';
		fprintf(fp, "%s", line);
		line.clear();
	}
	fclose(fp);
	fp = NULL;
}

void vtkvmtkImageWriter::WriteITKIO()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	vtkSmartPointer<vtkvmtkITKImageWriter> writer = 
		vtkSmartPointer<vtkvmtkITKImageWriter>::New();
	writer->SetInputData(this->Image);
	writer->SetFileName(this->OutputFileName);
	writer->SetUseCompression(1);
	vtkSmartPointer<vtkMatrix4x4> matrix =
		vtkSmartPointer<vtkMatrix4x4>::New();
	if (this->ApplyTransform == false)
	{
		double *origin = this->Image->GetOrigin();
		double *spacing = this->Image->GetSpacing();
		double elements[16] = {
			1 / spacing[0], 0, 0, -origin[0] / spacing[0],
			0, 1 / spacing[1], 0, -origin[1] / spacing[1],
			0, 0, 1 / spacing[2], -origin[2] / spacing[2],
			0, 0, 0, 1
		};
	}
	else
	{
		matrix->DeepCopy(this->RasToIjkMatrixCoefficients);
	}
	writer->SetRasToIJKMatrix(matrix);
	writer->Write();
}

void vtkvmtkImageWriter::Execute()
{
	//Not Input Image, All Dimension is zero
	if (this->Image->GetDimensions()[0] == 0 &&
		this->Image->GetDimensions()[1] == 0 &&
		this->Image->GetDimensions()[2] == 0)
	{
		std::cerr << "Error: no Image." << std::endl;
	}
	std::map<std::string, const char*> extensionFormats;
	extensionFormats["vti"] = "vtkxml";
	extensionFormats["vtk"] = "vtk";
	extensionFormats["dcm"] = "dicom";
	extensionFormats["raw"] = "raw";
	extensionFormats["mhd"] = "meta";
	extensionFormats["mha"] = "meta";
	extensionFormats["tif"] = "tiff";
	extensionFormats["png"] = "png";

	if (this->GuessFormat && this->OutputFileName != NULL && this->Format == NULL)
	{
		std::string fileName = this->OutputFileName;
		size_t index = fileName.find_first_of(".");
		if (index != std::string::npos)
		{
			std::string extension = fileName.substr(++index);
			std::map<std::string, const char*>::iterator it;
			it = extensionFormats.find(extension);
			if (it != extensionFormats.end())
				this->SetFormat(extensionFormats[extension]);
				//this->Format = (char*)extensionFormats[extension];
			//this->SetFormat(extensionFormats[extension]); strcmp(this->Format, "vtkxml")
		}
	}
	if (this->PixelRepresentation != NULL)
	{
		vtkImageCast *cast = vtkImageCast::New();
		cast->SetInputData(this->Image);
		if (this->PixelRepresentation == "double")
			cast->SetOutputScalarTypeToDouble();
		else if (this->PixelRepresentation == "float")
			cast->SetOutputScalarTypeToFloat();
		else if (this->PixelRepresentation == "short")
			cast->SetOutputScalarTypeToShort();
		else
			std::cerr << "Error: unsupported pixel representation" << this->PixelRepresentation << "." << std::endl;
		cast->Update();
		this->Image->DeepCopy(cast->GetOutput());
	}
	//Use ITKIO the Format in not [vtkxml, vtk, raw]
	if (this->UseITKIO == true &&
		this->OutputFileName != NULL &&
		strcmp(this->Format, "vtkxml") && strcmp(this->Format, "vtk") &&
		strcmp(this->Format, "tiff") && strcmp(this->Format, "png") &&
		strcmp(this->Format, "dat") && strcmp(this->Format, "vtsxml"))
	{
		this->WriteITKIO();
	}
	else
	{
		if (!strcmp(Format, "vtkxml"))
			this->WriteVTKXMLImageFile();
		else if (!strcmp(Format, "vtk"))
			this->WriteVTKImageFile();
		else if (!strcmp(Format, "meta"))
			this->WriteMetaImageFile();
		else if (!strcmp(Format, "png"))
			this->WritePNGImageFile();
		else if (!strcmp(Format, "tiff"))
			this->WriteTIFFImageFile();
		else if (!strcmp(Format, "pointdata"))
			this->WritePointDataImageFile();
		else if (!strcmp(Format, "vtsxml"))
			this->WriteVTSXMLVolumeFile();
		else
			std::cerr << "Error: unsupported format: " + std::string(Format) + '.' << std::endl;
	}
}