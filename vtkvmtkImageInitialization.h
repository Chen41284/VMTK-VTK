/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageInitialization,v $
Language:  C++
Date:      $Date: 2019/04/17 20:54:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimageinitialization.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkImageInitialization_H
#define __vtkvmtkImageInitialization_H

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkCubeSource.h>
#include <vtkActor.h>
#include <vtkBoxWidget.h>
#include <vtkSetGet.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

//VMTK-Src
#include <vtkvmtkImagePlaneWidget.h>

//VTK-VMTK
#include "vtkvmtkRenderer.h"
#include "vtkvmtkSurfaceViewer.h"
#include "vtkvmtkImageSeeder.h"


class  vtkvmtkImageInitialization : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageInitialization *New();
	vtkTypeMacro(vtkvmtkImageInitialization, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//The Input Image
	//浅拷贝
	//void SetImage(vtkImageData* data) { this->Image = data; };
	void SetImage(vtkSmartPointer<vtkImageData> data) { this->Image = data; };
	//external renderer
	void SetvmtkRenderer(vtkvmtkRenderer *renderer) { this->vmtkRenderer = renderer; }

	//输出初始化后的图像数据集
	vtkSmartPointer<vtkImageData> GetInitialLevelSets()
	{
		return this->InitialLevelSets; 
	};

	//获取初始化后的重建表面的数据
	//vtkPolyData* GetSurface() { return this->Surface; };
	vtkSmartPointer<vtkPolyData> GetSurface() { return this->Surface; };

	//设定初始水平集
	//void SetInitialLevelSets(vtkImageData *data) { this->InitialLevelSets = data; };
	void SetInitialLevelSets(vtkSmartPointer<vtkImageData> data) { this->InitialLevelSets = data; };

	//设置交互
	vtkSetMacro(Interactive, bool);

	//["isosurface","threshold","collidingfronts","fastmarching","seeds"]
	//设置初始化的方法
	vtkSetStringMacro(Method);

	//the value of the upper threshold to use for threshold, collidingfronts and fastmarching
	//vtkSetMacro(UpperThreshold, double*);
	void SetUpperThreshold(double upperValue)
	{ 
		this->UpperThreshold = new double(0.0);
		*(this->UpperThreshold) = upperValue;
	};

	//the value of the upper threshold to use for threshold, collidingfronts and fastmarching
	//vtkSetMacro(LowerThreshold, double*);
	void SetLowThreshold(double lowerValue)
	{ 
		this->LowerThreshold = new double(0.0);
		*(this->LowerThreshold) = lowerValue; 
	};

	//list of source point IJK coordinates
	void SetSourcePoints(const std::vector<int> &src) { this->SourcePoints = src; };

	//list of target point IJK coordinates
	void SetTargetPoints(const std::vector<int> &target) { this->TargetPoints = target; };

	//negate image values before initializing
	vtkSetMacro(NegateImage, bool);

	//the isosurface value to adopt as the level set surface
	vtkSetMacro(IsoSurfaceValue, double);
	vtkGetMacro(IsoSurfaceValue, double);

	//vtkPolyData* SeedInput(const char *queryString, int numberOfSeeds);
	vtkSmartPointer<vtkPolyData> SeedInput(const char* queryString, int numberOfSeeds);

	//设置外部的种子点
	//void SetImageSeeder(vtkvmtkImageSeeder* seed) { this->ImageSeeder = seed; };
	void SetImageSeeder(vtkSmartPointer<vtkvmtkImageSeeder> seed) { this->ImageSeeder = seed; };

	//设置外部的SurfaceViewer
	//void SetSurfaceViewer(vtkvmtkSurfaceViewer *surfaceViewer) { this->SurfaceViewer = surfaceViewer; };
	void SetSurfaceViewer(vtkSmartPointer<vtkvmtkSurfaceViewer> surfaceViewer) { this->SurfaceViewer = surfaceViewer; };

	//Execute
	void Execute();

protected:
	vtkvmtkImageInitialization();
	~vtkvmtkImageInitialization();
	//Input Arguments
	vtkvmtkRenderer *vmtkRenderer;
	bool InnervmtkRenderer;
	//vtkImageData *InitialLevelSets;
	//vtkPolyData *Surface;
	//vtkvmtkImageSeeder *ImageSeeder;
	//vtkImageData *MergedInitialLevelSets;
	//vtkvmtkSurfaceViewer *SurfaceViewer;
	//vtkImageData *Image;

	//置换成智能指针方便动态释放内存空间
	vtkSmartPointer<vtkImageData> InitialLevelSets;
	vtkSmartPointer<vtkPolyData> Surface;
	vtkSmartPointer<vtkvmtkImageSeeder> ImageSeeder;
	vtkSmartPointer<vtkImageData> MergedInitialLevelSets;
	vtkSmartPointer<vtkvmtkSurfaceViewer> SurfaceViewer;
	vtkSmartPointer<vtkImageData> Image;
	//Inner Data
	bool Interactive;
	char *Method = nullptr;
	double *UpperThreshold;
	double *LowerThreshold;
	std::vector<int> SourcePoints;
	std::vector<int> TargetPoints;
	bool NegateImage;
	double IsoSurfaceValue;

	//Inner Method
	int ThresholdValidator(const char* text);
	double ThresholdInput(const char* queryString);
	int YesNoValidator(const char *text);
	int InitializationTypeValidator(const char* text);

	void IsosurfaceInitialize(void);
	void ThresholdInitialize(void);
	void FastMarchingInitialize(void);
	void CollidingFrontsInitialize(void);
	void SeedInitialize(void);

	//void DisplayLevelSetSurface(vtkImageData *levelSets);
	void DisplayLevelSetSurface(vtkSmartPointer<vtkImageData> levelSets);
	void MergeLevelSets();

	const char* InputText(const char*queryString, int (vtkvmtkImageInitialization::*validator)(const char* text) = nullptr);

private:
	vtkvmtkImageInitialization(const vtkvmtkImageInitialization&) = delete;
	void operator== (const vtkvmtkImageInitialization&) = delete;
};

#endif
