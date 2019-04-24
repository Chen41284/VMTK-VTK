/*=========================================================================

Program:   VMTK
Module:    $RCSfile: vtkvmtkLevelSetSegmentation,v $
Language:  C++
Date:      $Date: 2019/04/47 20:44:47 $
Version:   $Revision: 1.0 $

Modified: adapted from https://github.com/vmtk/vmtk/blob/master/vmtkScripts/vmtklevelsetsegmentation.py by @Chen41284
  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm
  for details.

	 This software is distributed WITHOUT ANY WARRANTY; without even
	 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	 PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __vtkvmtkLevelSetSegmentation_H
#define __vtkvmtkLevelSetSegmentation_H

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
#include <vtkActor2D.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>
#include <set>

//VMTK-NEW
#include "vtkvmtkRenderer.h"
#include "vtkvmtkSurfaceViewer.h"
#include "vtkvmtkImageSeeder.h"
#include "vtkvmtkImageInitialization.h"

static void LevelSetCallbackFunction(vtkObject* caller, long unsigned int eventId,
	void* clientData, void* callData);


class  vtkvmtkLevelSetSegmentation : public vtkImageAlgorithm
{
public:
	static vtkvmtkLevelSetSegmentation *New();
	vtkTypeMacro(vtkvmtkLevelSetSegmentation, vtkImageAlgorithm);
	void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

	//vtkImageData
	//ǳ����
	//void SetImage(vtkImageData* data) { this->Image = data; };
	void SetImage(vtkSmartPointer<vtkImageData> data) { this->Image = data; };

	//featureimage
	//void SetFeatureImage(vtkImageData* data) { this->FeatureImage->DeepCopy(data); };
	void SetFeatureImage(vtkSmartPointer<vtkImageData> data) { this->FeatureImage = data; };

	//initializationimage
	//void SetInitializationImage(vtkImageData* data) { this->InitializationImage->DeepCopy(data); };
	void SetInitializationImage(vtkSmartPointer<vtkImageData> data) { this->InitializationImage = data; };

	//initiallevelsets
	//void SetInitialLevelSets(vtkImageData* data) { this->InitialLevelSets->DeepCopy(data); };
	void SetInitialLevelSets(vtkSmartPointer<vtkImageData> data) { this->InitialLevelSets = data; };

	//levelsets
	//void SetLevelSets(vtkImageData* data) { this->LevelSets->DeepCopy(data); };
	void SetLevelSets(vtkSmartPointer<vtkImageData> data) { this->LevelSets = data; };

	//levelsetstype ["geodesic","curves","threshold","laplacian"]
	vtkSetStringMacro(LevelSetsType);

	//featureimagetype,["vtkgradient","gradient","upwind","fwhm"]
	vtkSetStringMacro(FeatureImageType);

	//negate
	vtkSetMacro(NegateForInitialization, bool);

	//sigmoid
	vtkSetMacro(SigmoidRemapping, bool);

	//isosurfacevalue
	vtkSetMacro(IsoSurfaceValue, double);

	//derivativesigma
	vtkSetMacro(DerivativeSigma, double);

	//featurederivativesigma
	vtkSetMacro(FeatureDerivativeSigma, double);

	//upwindfactor
	vtkSetMacro(UpwindFactor, double);

	//fwhmradius
	vtkSetVector3Macro(FWHMRadius, int);

	//fwhmbackgroundvalue
	vtkSetMacro(FWHMBackgroundValue, double);

	//iterations
	vtkSetMacro(NumberOfIterations, int);

	//propagation
	vtkSetMacro(PropagationScaling, double);

	//curvature
	vtkSetMacro(CurvatureScaling, double);

	//advection
	vtkSetMacro(AdvectionScaling, double);

	//edgeweight
	vtkSetMacro(EdgeWeight, double);

	//smoothingiterations
	vtkSetMacro(SmoothingIterations, int);

	//smoothingtimestep
	vtkSetMacro(SmoothingTimeStep, double);

	//smoothingconductance
	vtkSetMacro(SmoothingConductance, double);

	//External vmtkRenderer
	void SetvmtkRenderer(vtkvmtkRenderer* renderer) { this->vmtkRenderer = renderer; };

	//Execute
	void Execute();

protected:
	vtkvmtkLevelSetSegmentation();
	~vtkvmtkLevelSetSegmentation();
	//Input Arguments
	//vtkImageData *Image;
	vtkSmartPointer<vtkImageData> Image;
	//û��ָ��Ļ�, vmtkRenderer���Զ����ù��캯��
	vtkvmtkRenderer *vmtkRenderer; 
	//����������ʾ
	vtkvmtkSurfaceViewer *SurfaceViewer;
	bool OwnRenderer;
	bool InnervmtkRenderer;
	bool DeepCopyImage;
	//vtkImageData *InitialLevelSets;
	//vtkImageData *InitializationImage;
	//vtkImageData *FeatureImage;
	//vtkImageData *LevelSetsInput;
	//vtkImageData *LevelSetsOutput;
	//vtkImageData *LevelSets;
	//����ָ��
	vtkSmartPointer<vtkImageData> InitialLevelSets;
	vtkSmartPointer<vtkImageData> InitializationImage;
	vtkSmartPointer<vtkImageData> FeatureImage;
	vtkSmartPointer<vtkImageData> LevelSetsInput;
	vtkSmartPointer<vtkImageData> LevelSetsOutput;
	vtkSmartPointer<vtkImageData> LevelSets;
	double UpperThreshold;
	double LowerThreshold;
	int NumberOfIterations;
	double PropagationScaling;
	double CurvatureScaling;
	double AdvectionScaling;
	double IsoSurfaceValue;
	double MaximumRMSError;
	double DerivativeSigma;
	double FeatureDerivativeSigma;
	bool NegateForInitialization;
	bool SigmoidRemapping;
	char* LevelSetsType = nullptr;
	char* FeatureImageType = nullptr;
	double UpwindFactor;
	int FWHMRadius[3];
	double FWHMBackgroundValue;
	double EdgeWeight;
	int SmoothingIterations;
	double SmoothingTimeStep;
	double SmoothingConductance;

	//vtkvmtkImageSeeder *ImageSeeder;
	vtkSmartPointer<vtkvmtkImageSeeder> ImageSeeder;
	vtkSmartPointer<vtkvmtkImageInitialization> vmtkImageInitialization;

	const char* InputText(const char*queryString, int (vtkvmtkLevelSetSegmentation::*validator)(const char* text)=nullptr);

	//Inner Method
	int ThresholdValidator(const char* text);
	double ThresholdInput(const char* queryString);
	void LevelSetEvolution();
	void MergeLevelSet();
	void DisplayLevelSetSurface(vtkImageData *levelSets, double value = 0.0);
	int YesNoValidator(const char *text);
	int EvolutionParametersValidator(const char* text);

	//String process
	std::string strip(const std::string &str, char ch = ' ');
	std::vector<std::string> split(const std::string &str, std::string ch = " ");

private:
	vtkvmtkLevelSetSegmentation(const vtkvmtkLevelSetSegmentation&) = delete;
	void operator== (const vtkvmtkLevelSetSegmentation&) = delete;
};

#endif
