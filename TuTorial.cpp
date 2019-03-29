#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <string.h>
#include <vtkAutoInit.h>
#include "itkMetaImageIOFactory.h"
#include "itkPNGImageIOFactory.h"
#include <itkGDCMImageIOFactory.h>
#include <itkJPEGImageIOFactory.h>

//VTK-VMTK
#include "vtkvmtkImageReader.h"
#include "vtkvmtkImageRender.h"
#include "vtkvmtkImageViewer.h"
#include "vtkvmtkImageWriter.h"


VTK_MODULE_INIT(vtkRenderingOpenGL);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

//Tuturial Method
void ShowImage(const char* fileName);
void DicomToVti(const char* InputFileName, const char* OutputFileName);


int main()
{
	const char* InputFileName = "C:\\Users\\chenjiaxing\\Desktop\\Python\\body.vti";
	const char* OutputFileName = "C:\\Users\\chenjiaxing\\Desktop\\Python\\PNG\\body";
	ShowImage(InputFileName);
	//DicomToVti(InputFileName, OutputFileName);
	getchar();
	return 0;
}

//Reading and displaying images
void ShowImage(const char* fileName)
{
	//itk::MetaImageIOFactory::RegisterOneFactory();
	itk::GDCMImageIOFactory::RegisterOneFactory();
	//itk::JPEGImageIOFactory::RegisterOneFactory();
	//itk::PNGImageIOFactory::RegisterOneFactory();
	vtkSmartPointer<vtkvmtkImageReader> reader =
		vtkSmartPointer<vtkvmtkImageReader>::New();
	reader->SetInputFileName(fileName);
	reader->SetUseITKIO(true);
	reader->Execute();
	vtkSmartPointer<vtkvmtkImageViewer> viwer =
		vtkSmartPointer<vtkvmtkImageViewer>::New();
	if (reader->GetImage() == NULL)
	{
		std::cout << "no Image " << std::endl;
		return;
	}
	viwer->SetImage(reader->GetImage());
	viwer->Execute();
}

//Image format conversion
void DicomToVti(const char* InputFileName, const char* OutputFileName)
{
	itk::GDCMImageIOFactory::RegisterOneFactory();
	vtkSmartPointer<vtkvmtkImageReader> reader =
		vtkSmartPointer<vtkvmtkImageReader>::New();
	reader->SetInputFileName(InputFileName);
	reader->SetUseITKIO(true);
	reader->Execute();
	vtkvmtkImageWriter *writer = vtkvmtkImageWriter::New();
	if (reader->GetImage() == NULL)
	{
		std::cout << "no Image " << std::endl;
		return;
	}
	writer->SetImage(reader->GetImage());
	writer->SetOutputFileName(OutputFileName);
	writer->SetFormat("png");
	writer->Execute();
	std::cout << "Success Write the File" << std::endl;
}