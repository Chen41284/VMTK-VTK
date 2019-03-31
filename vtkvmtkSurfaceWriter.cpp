#include "vtkvmtkSurfaceWriter.h"

//VTK Includes
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkSTLWriter.h>
#include <vtkPLYWriter.h>
#include <vtkPointData.h>
#include <vtkObject.h>
#include <vtkDataSetAttributes.h>
#include <vtkTriangleFilter.h>
#include <vtkIdList.h>

//STD
#include <map>

vtkStandardNewMacro(vtkvmtkSurfaceWriter);

vtkvmtkSurfaceWriter::vtkvmtkSurfaceWriter()
{
	this->Format = NULL;
	this->GuessFormat = true;
	this->OutputFileName = NULL;
	this->Surface = vtkPolyData::New();
	this->CellData = false;
	this->Mode = new char[10];
	strcpy_s(this->Mode, 10, "binary");
}

vtkvmtkSurfaceWriter::~vtkvmtkSurfaceWriter()
{
	if (this->Format != NULL)
	{
		delete this->Format;
		this->Format = NULL;
	}
	if (this->OutputFileName != NULL)
	{
		delete this->OutputFileName;
		this->OutputFileName = NULL;
	}
	this->Surface->Delete();
	this->Surface = NULL;
	delete[] this->Mode;
	this->Mode = NULL;
}

void vtkvmtkSurfaceWriter::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Surface MarchingCubes\n";
}

void vtkvmtkSurfaceWriter::WriteVTKSurfaceFile()
{
	if (this->OutputFileName == NULL)
		std::cerr << "Error: no OutputFileName." << std::endl;
	std::cout << "Writing VTK surface file." << std::endl;
	vtkSmartPointer<vtkPolyDataWriter> writer = 
		vtkSmartPointer<vtkPolyDataWriter>::New();
	writer->SetInputData(this->Surface);
	writer->SetFileName(this->OutputFileName);
	if (this->Mode == "binary")
		writer->SetFileTypeToBinary();
	else if (this->Mode == "ascii" )
		writer->SetFileTypeToASCII();
	writer->Write();
}

void vtkvmtkSurfaceWriter::WriteVTKXMLSurfaceFile()
{
	if (this->OutputFileName == NULL)
		std::cerr << "Error: no OutputFileName." << std::endl;
	std::cout << "Writing VTK XML surface file." << std::endl;
	vtkSmartPointer<vtkXMLPolyDataWriter> writer =
		vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	writer->SetInputData(this->Surface);
	writer->SetFileName(this->OutputFileName);
	if (this->Mode == "binary")
		writer->SetDataModeToBinary();
	else if (this->Mode == "ascii")
		writer->SetDataModeToAscii();
	writer->Write();
}

void vtkvmtkSurfaceWriter::WriteSTLSurfaceFile()
{
	if (this->OutputFileName == NULL)
		std::cerr << "Error: no OutputFileName." << std::endl;
	std::cout << "Writing STL surface file." << std::endl;
	vtkSmartPointer<vtkSTLWriter> writer =
		vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetInputData(this->Surface);
	writer->SetFileName(this->OutputFileName);
	if (this->Mode == "binary")
		writer->SetFileTypeToBinary();
	else if (this->Mode == "ascii")
		writer->SetFileTypeToASCII();
	writer->Write();
}

void vtkvmtkSurfaceWriter::WritePLYSurfaceFile()
{
	if (this->OutputFileName == NULL)
		std::cerr << "Error: no OutputFileName." << std::endl;
	std::cout << "Writing PLY surface file." << std::endl;
	vtkSmartPointer<vtkPLYWriter> writer =
		vtkSmartPointer<vtkPLYWriter>::New();
	writer->SetInputData(this->Surface);
	writer->SetFileName(this->OutputFileName);
	if (this->Mode == "binary")
		writer->SetFileTypeToBinary();
	else if (this->Mode == "ascii")
		writer->SetFileTypeToASCII();
	writer->Write();
}

void vtkvmtkSurfaceWriter::WritePointDataSurfaceFile()
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
	//If the CellData is true, Write the CellData
	if (this->CellData)
	{
		vtkCellData *dataArrays = this->Surface->GetCellData();
		int NumberOfArrays = dataArrays->GetNumberOfArrays();
		for (int i = 0; i < NumberOfArrays; i++)
		{
			vtkDataArray *array = dataArrays->GetArray(i);
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
		//The Surface Data, Every Point of Each Cell
		int numberOfCells = this->Surface->GetNumberOfCells();
		for (int i = 0; i < numberOfCells; i++)
		{
			double *point = this->Surface->GetCell(i)->GetPoints()->GetPoint(0);
			line = std::to_string(point[0]) + ' ' + std::to_string(point[1]) + ' ' + std::to_string(point[2]);
			for (auto arrayName : arrayNames)
			{
				vtkDataArray *array = dataArrays->GetArray(arrayName.c_str());
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
	else //else Write the Point Data
	{
		vtkPointData *dataArrays = this->Surface->GetPointData();
		int NumberOfArrays = dataArrays->GetNumberOfArrays();
		for (int i = 0; i < NumberOfArrays; i++)
		{
			vtkDataArray *array = dataArrays->GetArray(i);
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
		//The Surface Data, Every Point of Each Cell
		int numberOfPoints = this->Surface->GetNumberOfPoints();
		for (int i = 0; i < numberOfPoints; i++)
		{
			double *point = this->Surface->GetPoint(0);
			line = std::to_string(point[0]) + ' ' + std::to_string(point[1]) + ' ' + std::to_string(point[2]);
			for (auto arrayName : arrayNames)
			{
				vtkDataArray *array = dataArrays->GetArray(arrayName.c_str());
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
}

void vtkvmtkSurfaceWriter::WriteTecplotSurfaceFile()
{
	if (this->OutputFileName == NULL)
	{
		std::cerr << "Error: no OutputFileName! " << std::endl;
		return;
	}
	std::cout << "Writing Tecplot file." << std::endl;
	vtkSmartPointer<vtkTriangleFilter> triangleFilter = 
		vtkSmartPointer<vtkTriangleFilter>::New();
	triangleFilter->SetInputData(this->Surface);
	triangleFilter->PassVertsOff();
	triangleFilter->PassLinesOff();
	triangleFilter->Update();
	this->Surface = triangleFilter->GetOutput();
	FILE* fp = NULL;
	fp = fopen(this->OutputFileName, "w");
	// Header Information
	std::string line = "VARIABLES = X,Y,Z";
	std::vector<std::string> arrayNames;
	vtkPointData *dataArrays = this->Surface->GetPointData();
	int NumberOfArrays = dataArrays->GetNumberOfArrays();
	for (int i = 0; i < NumberOfArrays; i++)
	{
		vtkDataArray *array = dataArrays->GetArray(i);
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
	line = line + "ZONE " + "N=" + std::to_string(this->Surface->GetNumberOfPoints()) + ',';
	line = line + "E=" + std::to_string(this->Surface->GetNumberOfCells()) + ',' + "F=FEPOINT" + ',' + "ET=TRIANGLE" + '\n';
	fprintf(fp, "%s", line);
	line.clear();
	//The Surface Data, Every Point of Each Cell
	int numberOfPoints = this->Surface->GetNumberOfPoints();
	for (int i = 0; i < numberOfPoints; i++)
	{
		double *point = this->Surface->GetPoint(0);
		line = std::to_string(point[0]) + ' ' + std::to_string(point[1]) + ' ' + std::to_string(point[2]);
		for (auto arrayName : arrayNames)
		{
			vtkDataArray *array = dataArrays->GetArray(arrayName.c_str());
			for (int j = 0; j < array->GetNumberOfComponents(); j++)
				line = line + ' ' + std::to_string(array->GetComponent(i, j));
		}
		line = line + '\n';
		fprintf(fp, "%s", line);
		line.clear();
	}
	int numberOfCells = this->Surface->GetNumberOfCells();
	for (int i = 0; i < numberOfCells; i++)
	{
		vtkIdList *cellPointIds = this->Surface->GetCell(i)->GetPointIds();
		line = '\0';
		int numberOfIds = cellPointIds->GetNumberOfIds();
		for (int j = 0; j < numberOfIds; j++)
		{
			if (j > 0)
				line = line + ' ';
			line = line + std::to_string(cellPointIds->GetId(j) + 1);
		}		
		line = line + '\n';
		fprintf(fp, "%s", line);
		line.clear();
	}	
	fclose(fp);
	fp = NULL;
}

void vtkvmtkSurfaceWriter::Execute()
{
	if (this->Surface->GetNumberOfCells() == 0)
	{
		std::cout << "Error: no Surface." << std::endl;
		return;
	}
	std::map<std::string, const char*> extensionFormats;
	extensionFormats["vtp"] = "vtkxml";
	extensionFormats["vtkxml"] = "vtkxml";
	extensionFormats["vtk"] = "vtk";
	extensionFormats["stl"] = "stl";
	extensionFormats["ply"] = "ply";
	extensionFormats["tec"] = "tecplot";
	extensionFormats["dat"] = "pointdata";

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
		}
	}

	if (!strcmp(Format, "vtk"))
		this->WriteVTKSurfaceFile();
	else if (!strcmp(Format, "vtkxml"))
		this->WriteVTKXMLSurfaceFile();
	else if (!strcmp(Format, "stl"))
		this->WriteSTLSurfaceFile();
	else if (!strcmp(Format, "ply"))
		this->WritePLYSurfaceFile();
	else if (!strcmp(Format, "pointdata"))
		this->WritePointDataSurfaceFile();
	else if (!strcmp(Format, "tecplot"))
		this->WriteTecplotSurfaceFile();
	else
		std::cerr << "Error: unsupported format: " + std::string(Format) + '.' << std::endl;
}