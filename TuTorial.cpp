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
#include "vtkvmtkRenderer.h"
#include "vtkvmtkImageViewer.h"
#include "vtkvmtkImageWriter.h"
#include "vtkvmtkImageVOISelector.h"
#include "vtkvmtkMarchingCubes.h"
#include "vtkvmtkSurfaceWriter.h"
#include "vtkvmtkSurfaceViewer.h"
#include "vtkvmtkLevelSetSegmentation.h"

//Windows
#include "WindowsAPI.h"


VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);



//Tuturial Method
void ShowImage(const char* fileName);
void DicomToVti(const char* InputFileName, const char* OutputFileName);
void VolumeExtraction(const char* InputFileName, const char* OutputFileName);
void MarchingCubes(const char* InputFileName, int level, const char* OutputFile = NULL);
void ImageAndSurfaceView(const char* InputFileName, int level);
void LevelSetSegmentation(const char* InputFileName, const char* OutputFileName);

int main()
{
	const char* InputFileName = "C:\\Users\\chenjiaxing\\Desktop\\Python\\body.vti";
	const char* OutputFileName = "C:\\Users\\chenjiaxing\\Desktop\\Python\\PNG\\body_mc_surface.ply";
	LevelSetSegmentation(InputFileName, OutputFileName);
	//ImageAndSurfaceView(InputFileName, 500);
	//MarchingCubes(InputFileName, 500, NULL);
	//VolumeExtraction(InputFileName, OutputFileName);
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
	vtkSmartPointer<vtkvmtkImageViewer> viewer =
		vtkSmartPointer<vtkvmtkImageViewer>::New();
	if (reader->GetImage() == NULL)
	{
		std::cout << "no Image " << std::endl;
		return;
	}
	viewer->SetImage(reader->GetImage());
	viewer->Execute();
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
	writer->Delete();
}

//Volume of interest (VOI) extraction
void VolumeExtraction(const char* InputFileName, const char* OutFileName)
{
	itk::GDCMImageIOFactory::RegisterOneFactory();
	vtkSmartPointer<vtkvmtkImageReader> reader =
		vtkSmartPointer<vtkvmtkImageReader>::New();
	reader->SetInputFileName(InputFileName);
	reader->SetUseITKIO(true);
	reader->Execute();
	vtkvmtkImageVOISelector *selector = vtkvmtkImageVOISelector::New();
	selector->SetImage(reader->GetImage());
	selector->Execute();
	vtkSmartPointer<vtkvmtkImageWriter> writer =
		vtkSmartPointer<vtkvmtkImageWriter>::New();
	writer->SetImage(selector->GetImage());
	writer->SetOutputFileName(OutFileName);
	writer->SetFormat("vtkxml");
	writer->Execute();
	std::cout << "Success Write the ExtractionVOI File" << std::endl;
	selector->Delete();
}

//MarchingCubes
void MarchingCubes(const char* InputFileName, int level, const char* OutputFileName)
{
	itk::GDCMImageIOFactory::RegisterOneFactory();
	itk::MetaImageIOFactory::RegisterOneFactory();
	vtkSmartPointer<vtkvmtkImageReader> reader =
		vtkSmartPointer<vtkvmtkImageReader>::New();
	reader->SetInputFileName(InputFileName);
	reader->SetUseITKIO(true);
	reader->Execute();
	vtkvmtkMarchingCubes *marchingCubes = vtkvmtkMarchingCubes::New();
	marchingCubes->SetImage(reader->GetImage());
	marchingCubes->SetLevel(level);
	marchingCubes->SetConnectivity(true);
	marchingCubes->Execute();
	//文件名非空则输出文件
	if (OutputFileName != NULL)
	{
		vtkSmartPointer<vtkvmtkSurfaceWriter> writer =
			vtkSmartPointer<vtkvmtkSurfaceWriter>::New();
		writer->SetSurface(marchingCubes->GetSurface());
		writer->SetOutputFileName(OutputFileName);
		writer->Execute();
		std::cout << "Success Write the ExtractionVOI File" << std::endl;
	}
	vtkSmartPointer<vtkvmtkSurfaceViewer> surfaceViewer =
		vtkSmartPointer<vtkvmtkSurfaceViewer>::New();
	surfaceViewer->SetSurface(marchingCubes->GetSurface());
	surfaceViewer->SetLegend(true);
	surfaceViewer->SetColorMap("blackbody");
	surfaceViewer->Execute();
	marchingCubes->Delete();
}

//Show Image And Surface
void ImageAndSurfaceView(const char* InputFileName, int level)
{
	itk::GDCMImageIOFactory::RegisterOneFactory();
	itk::MetaImageIOFactory::RegisterOneFactory();
	vtkSmartPointer<vtkvmtkImageReader> reader =
		vtkSmartPointer<vtkvmtkImageReader>::New();
	reader->SetInputFileName(InputFileName);
	reader->SetUseITKIO(true);
	reader->Execute();
	vtkvmtkMarchingCubes *marchingCubes = vtkvmtkMarchingCubes::New();
	marchingCubes->SetImage(reader->GetImage());
	marchingCubes->SetLevel(level);
	marchingCubes->SetConnectivity(true);
	marchingCubes->Execute();
	//使ImageView和SurfaceView使用同一个Render
	vtkSmartPointer<vtkvmtkRenderer> renderer = 
		vtkSmartPointer<vtkvmtkRenderer>::New();
	renderer->Initialize();
	vtkSmartPointer<vtkvmtkImageViewer> ImageViewer =
		vtkSmartPointer<vtkvmtkImageViewer>::New();
	if (reader->GetImage() == NULL)
	{
		std::cout << "no Image " << std::endl;
		return;
	}
	ImageViewer->SetImage(reader->GetImage());
	ImageViewer->SetvmtkRenderer(renderer);
	ImageViewer->SetDisplay(false); //设置为false，防止图像提前显示
	ImageViewer->Execute();
	vtkSmartPointer<vtkvmtkSurfaceViewer> surfaceViewer =
		vtkSmartPointer<vtkvmtkSurfaceViewer>::New();
	surfaceViewer->SetRenderer(renderer);
	surfaceViewer->SetSurface(marchingCubes->GetSurface());
	surfaceViewer->SetLegend(true);
	surfaceViewer->SetColorMap("blackbody");
	surfaceViewer->SetDisplay(false); //设置为false，防止三维的网格提前显示
	surfaceViewer->Execute();
	//将图像和重建的三维网格一起显示
	//Add Promt Information
	const char* text = "\n  \'c\',\'Change surface representation.\'";
	renderer->Render(1, text);
	marchingCubes->Delete();
}

//水平集的分割
void LevelSetSegmentation(const char* InputFileName, const char* OutputFileName)
{
	vtkObject::GlobalWarningDisplayOff();
	itk::GDCMImageIOFactory::RegisterOneFactory();
	itk::MetaImageIOFactory::RegisterOneFactory();
	std::cout << "reader begin: " << std::endl;
	showMemoryInfo();
	vtkSmartPointer<vtkvmtkImageReader> reader =
		vtkSmartPointer<vtkvmtkImageReader>::New();
	reader->SetInputFileName(InputFileName);
	reader->SetUseITKIO(true);
	reader->Execute();
	std::cout << "reader->Execute: " << std::endl;
	showMemoryInfo();
	vtkSmartPointer<vtkvmtkLevelSetSegmentation> levelSet =
		vtkSmartPointer<vtkvmtkLevelSetSegmentation>::New();
	levelSet->SetImage(reader->GetImage());
	levelSet->Execute();
}