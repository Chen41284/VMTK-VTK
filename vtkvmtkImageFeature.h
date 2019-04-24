/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkImageFeature,v $
Language:  C++
Date:      $Date: 2019/04/15 17:38:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtkimagefeatures.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef __vtkvmtkImageFeature_H
#define __vtkvmtkImageFeature_H

// VTK includes
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkSetGet.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkActor.h>
#include <vtkScalarBarActor.h>
#include <vtkActor2D.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <set>

//VMTK-NEW
#include "vtkvmtkRenderer.h"


class  vtkvmtkImageFeature : public vtkImageAlgorithm
{
public:
	static vtkvmtkImageFeature *New();
	vtkTypeMacro(vtkvmtkImageFeature, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//vtkImageData
	//浅拷贝即可，该类的输出由FeatureImage输出
	void SetImage(vtkImageData* data) { this->Image = data; };

	//设置featureImage
	void SetFeatureImage(vtkImageData* data) { this->FeatureImage->DeepCopy(data); };

	//获取featureImage
	vtkImageData* GetFeatureImage(void) { return this->FeatureImage; };

	//featureimagetype,["vtkgradient","gradient","upwind","fwhm"]
	vtkSetStringMacro(FeatureImageType);

	//'dimensionality','int',1,'(2,3,1)'
	vtkSetMacro(Dimensionality, int);

	//sigmoid
	vtkSetMacro(SigmoidRemapping, bool);

	//derivativesigma
	vtkSetMacro(DerivativeSigma, double);

	//'upwindfactor',(0.0,1.0)'
	vtkSetMacro(UpwindFactor, double);

	//FWHMRadius
	vtkSetVector3Macro(FWHMRadius, int);

	//FWHMBackgroundValue
	vtkSetMacro(FWHMBackgroundValue, double);

	void Execute();

protected:
	vtkvmtkImageFeature();
	~vtkvmtkImageFeature();
	
	//vtkImageData *Image;
	//vtkImageData *FeatureImage;

	//智能指针
	vtkSmartPointer<vtkImageData> Image;
	vtkSmartPointer<vtkImageData> FeatureImage;

	//Inner Method
	void BuildVTKGradientBasedFeatureImage();
	void BuildFWHMBasedFeatureImage();
	void BuildUpwindGradientBasedFeatureImage();
	void BuildGradientBasedFeatureImage();

	int Dimensionality;
	double DerivativeSigma;
	bool SigmoidRemapping;
	char *FeatureImageType = nullptr;
	double UpwindFactor;
	int FWHMRadius[3];
	double FWHMBackgroundValue;
private:
	vtkvmtkImageFeature(const vtkvmtkImageFeature&) = delete;
	void operator== (const vtkvmtkImageFeature&) = delete;
};

#endif