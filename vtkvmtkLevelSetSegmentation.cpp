#include "vtkvmtkLevelSetSegmentation.h"

//VTK-Include
#include <vtkSimpleImageToImageFilter.h>
#include <vtkCallbackCommand.h>
#include <vtkImageMathematics.h>
#include <vtkMarchingCubes.h>
#include <vtkImageCast.h>

//STD

//VMTK-Src
#include <vtkvmtkGeodesicActiveContourLevelSetImageFilter.h>
#include <vtkvmtkCurvesLevelSetImageFilter.h>
#include <vtkvmtkThresholdSegmentationLevelSetImageFilter.h>
#include <vtkvmtkLaplacianSegmentationLevelSetImageFilter.h>

//VMTK-New
#include "vtkvmtkImageFeature.h"

//Windows
#include "WindowsAPI.h"

vtkStandardNewMacro(vtkvmtkLevelSetSegmentation);

vtkvmtkLevelSetSegmentation::vtkvmtkLevelSetSegmentation()
{
	this->Image = nullptr;
	this->vmtkRenderer = nullptr;
	this->SurfaceViewer = nullptr;
	this->OwnRenderer = false;
	this->DeepCopyImage = false;
	/*this->InitialLevelSets = vtkImageData::New();
	this->InitializationImage = vtkImageData::New();
	this->FeatureImage = vtkImageData::New();
	this->LevelSetsInput = vtkImageData::New();
	this->LevelSetsOutput = vtkImageData::New();
	this->LevelSets = vtkImageData::New();;*/
	this->InitialLevelSets = nullptr;
	this->InitializationImage = nullptr;
	this->FeatureImage = nullptr;
	this->LevelSetsInput = nullptr;
	this->LevelSetsOutput = nullptr;
	this->LevelSets = nullptr;
	this->UpperThreshold = 0.0;
	this->LowerThreshold = 0.0;
	this->NumberOfIterations = 0;
	this->PropagationScaling = 0.0;
	this->CurvatureScaling = 0.0;
	this->AdvectionScaling = 1.0;
	this->IsoSurfaceValue = 0.0;
	this->MaximumRMSError = 1E-20;
	this->DerivativeSigma = 0.0;
	this->FeatureDerivativeSigma = 0.0;
	this->NegateForInitialization = false;
	this->SigmoidRemapping = false;
	this->SetLevelSetsType("geodesic");
	this->SetFeatureImageType("gradient");
	this->UpwindFactor = 1.0;
	this->FWHMRadius[0] = 1; this->FWHMRadius[1] = 1; this->FWHMRadius[2] = 1;
	this->FWHMBackgroundValue = 0.0;
	this->EdgeWeight = 0.0;
	this->SmoothingIterations = 5;
	this->SmoothingTimeStep = 0.1;
	this->SmoothingConductance = 0.8;

	this->ImageSeeder = nullptr;
	this->vmtkImageInitialization = nullptr;
}

vtkvmtkLevelSetSegmentation::~vtkvmtkLevelSetSegmentation()
{
	/*if (this->Image != nullptr && this->DeepCopyImage == true)
	{
		this->Image->Delete();
		this->Image = nullptr;
	}*/
	if (this->vmtkRenderer != nullptr && this->InnervmtkRenderer == true)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = nullptr;
	}
	/*if (this->InitialLevelSets)
	{
		this->InitialLevelSets->Delete();
		this->InitialLevelSets = nullptr;
	}
	if (this->InitializationImage)
	{
		this->InitializationImage->Delete();
		this->InitializationImage = nullptr;
	} 
	if (this->FeatureImage)
	{
		this->FeatureImage->Delete();
		this->FeatureImage = nullptr;
	} 
	if (this->LevelSetsInput)
	{
		this->LevelSetsInput->Delete();
		this->LevelSetsInput = nullptr;
	} 
	if (this->LevelSetsOutput)
	{
		this->LevelSetsOutput->Delete();
		this->LevelSetsOutput = nullptr;
	}
	if (this->LevelSets)
	{
		this->LevelSets->Delete();
		this->LevelSets = nullptr;
	}*/
	if (this->SurfaceViewer)
	{
		this->SurfaceViewer->Delete();
		this->SurfaceViewer = nullptr;
	}
	delete[] this->LevelSetsType;
	this->LevelSetsType = nullptr;
	delete[] this->FeatureImageType;
	this->FeatureImageType = nullptr;

	/*if (this->ImageSeeder)
	{
		this->ImageSeeder->Delete();
		this->ImageSeeder = nullptr;
	}*/
}


void vtkvmtkLevelSetSegmentation::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image LevelSet Segmentation\n";
}


//��֤�������ֵ����Ч��
int vtkvmtkLevelSetSegmentation::ThresholdValidator(const char* text)
{
	if (!strcmp(text, "n"))  //������
		return 1;
	if (!strcmp(text, "i")) //����
	{
		this->vmtkRenderer->Render();
		return 0;
	}
	if (abs(atof(text) - 0.0) < 1e-06)
	{
		std::cout << "Input text is error!" << std::endl;
		return 0;
	}
	return 1;
}

//��֤������ַ����Ƿ���"Y"��"N"
int vtkvmtkLevelSetSegmentation::YesNoValidator(const char* text)
{
	if (!strcmp(text, "n") || !strcmp(text, "y"))
		return 1;
	return 0;
}

//pypescript�еĵ��ñ�׼�������������ݲ���֤��Ч��
//�����Ϊ����KeyPressInteractorStyle�ļ�����Ӧ��ȡ���벢��֤�����������Ч��
//validatorΪ��֤�����������Ч�Եĺ�������Ч����1����Ч����0
const char* vtkvmtkLevelSetSegmentation::InputText(const char*queryString, 
	int(vtkvmtkLevelSetSegmentation::*validator)(const char* text))
{
	//��̬����vmtkRenderer�е�Keypress�Ľ������ȡ�û��Ӽ������������
	const char* InputString = this->vmtkRenderer->PromptAsync(queryString);
	//����������֤�����ǿգ���ʹ�ô˺�����֤������ַ�������Ч��
	if (validator != nullptr)
	{
		while ((this->*validator)(InputString) == 0) //������ַ���Ч����������
			InputString = this->vmtkRenderer->PromptAsync(queryString);
	}
	return InputString;
}

//��ȡ�������ֵ
double vtkvmtkLevelSetSegmentation::ThresholdInput(const char* queryString)
{
	//�������еĳ���Ա������������ǰ׺
	const char* thresholdString = this->InputText(queryString, &vtkvmtkLevelSetSegmentation::ThresholdValidator);

	double threshold = 0.0;
	if (strcmp(thresholdString, "n")) //not equal != 
		threshold = atof(thresholdString);

	return threshold;
}

//����ͼ���ˮƽ��,����ͼ���ˮƽ��
void vtkvmtkLevelSetSegmentation::LevelSetEvolution()
{
	if (!strcmp(this->LevelSetsType, "geodesic"))
	{
		vtkSmartPointer<vtkvmtkGeodesicActiveContourLevelSetImageFilter> levelSetsGeodesic =
			vtkSmartPointer<vtkvmtkGeodesicActiveContourLevelSetImageFilter>::New();
		levelSetsGeodesic->SetFeatureImage(this->FeatureImage);
		levelSetsGeodesic->SetDerivativeSigma(this->FeatureDerivativeSigma);
		levelSetsGeodesic->SetAutoGenerateSpeedAdvection(1);
		levelSetsGeodesic->SetPropagationScaling(this->PropagationScaling);
		levelSetsGeodesic->SetCurvatureScaling(this->CurvatureScaling);
		levelSetsGeodesic->SetAdvectionScaling(this->AdvectionScaling);
		levelSetsGeodesic->SetInputData(this->LevelSetsInput);
		levelSetsGeodesic->SetNumberOfIterations(this->NumberOfIterations);
		levelSetsGeodesic->SetIsoSurfaceValue(this->IsoSurfaceValue);
		levelSetsGeodesic->SetMaximumRMSError(this->MaximumRMSError);
		levelSetsGeodesic->SetInterpolateSurfaceLocation(1);
		levelSetsGeodesic->SetUseImageSpacing(1);
		vtkSmartPointer<vtkCallbackCommand> ProgressCallCommand =
			vtkSmartPointer<vtkCallbackCommand>::New();
		// Set the callback to the function we created.
		ProgressCallCommand->SetCallback(LevelSetCallbackFunction);
		levelSetsGeodesic->AddObserver(vtkCommand::ProgressEvent, ProgressCallCommand);
		levelSetsGeodesic->Update();

		//this->LevelSetsOutput = vtkImageData::New(); No need
		this->LevelSetsOutput = vtkSmartPointer<vtkImageData>::New();
		this->LevelSetsOutput->DeepCopy(levelSetsGeodesic->GetOutput());
	}
	else if (!strcmp(this->LevelSetsType, "curves"))
	{
		vtkSmartPointer<vtkvmtkCurvesLevelSetImageFilter> levelSetsCurves =
			vtkSmartPointer<vtkvmtkCurvesLevelSetImageFilter>::New();
		levelSetsCurves->SetFeatureImage(this->FeatureImage);
		levelSetsCurves->SetDerivativeSigma(this->FeatureDerivativeSigma);
		levelSetsCurves->SetAutoGenerateSpeedAdvection(1);
		levelSetsCurves->SetPropagationScaling(this->PropagationScaling);
		levelSetsCurves->SetCurvatureScaling(this->CurvatureScaling);
		levelSetsCurves->SetAdvectionScaling(this->AdvectionScaling);
		levelSetsCurves->SetInputData(this->LevelSetsInput);
		levelSetsCurves->SetNumberOfIterations(this->NumberOfIterations);
		levelSetsCurves->SetIsoSurfaceValue(this->IsoSurfaceValue);
		levelSetsCurves->SetMaximumRMSError(this->MaximumRMSError);
		levelSetsCurves->SetInterpolateSurfaceLocation(1);
		levelSetsCurves->SetUseImageSpacing(1);
		vtkSmartPointer<vtkCallbackCommand> ProgressCallCommand =
			vtkSmartPointer<vtkCallbackCommand>::New();
		// Set the callback to the function we created.
		ProgressCallCommand->SetCallback(LevelSetCallbackFunction);
		levelSetsCurves->AddObserver(vtkCommand::ProgressEvent, ProgressCallCommand);
		levelSetsCurves->Update();

		this->LevelSetsOutput = vtkSmartPointer<vtkImageData>::New();
		this->LevelSetsOutput->DeepCopy(levelSetsCurves->GetOutput());
	}
	else if (!strcmp(this->LevelSetsType, "threshold"))
	{
		vtkSmartPointer<vtkvmtkThresholdSegmentationLevelSetImageFilter> levelSetsThreshold =
			vtkSmartPointer<vtkvmtkThresholdSegmentationLevelSetImageFilter>::New();
		levelSetsThreshold->SetFeatureImage(this->Image);
		const char* queryString = "Please input lower threshold (\'n\' for none): ";
		this->LowerThreshold = this->ThresholdInput(queryString);
		queryString = "Please input upper threshold (\'n\' for none): ";
		this->UpperThreshold = this->ThresholdInput(queryString);
		double *scalarRange = this->Image->GetScalarRange();
		if (fabs(this->LowerThreshold - 0.0) > 1e-6)  //��ֵ�����޲�Ϊ��ʱ��������ֵ������
			levelSetsThreshold->SetLowerThreshold(this->LowerThreshold);
		else
			levelSetsThreshold->SetLowerThreshold(scalarRange[0] - 1.0);
		if (fabs(this->UpperThreshold - 0.0) > 1e-6)  //��ֵ�����޲�Ϊ��ʱ��������ֵ������
			levelSetsThreshold->SetUpperThreshold(this->UpperThreshold);
		else
			levelSetsThreshold->SetUpperThreshold(scalarRange[1] + 1.0);
		levelSetsThreshold->SetEdgeWeight(this->EdgeWeight);
		levelSetsThreshold->SetSmoothingIterations(this->SmoothingIterations);
		levelSetsThreshold->SetSmoothingTimeStep(this->SmoothingTimeStep);
		levelSetsThreshold->SetSmoothingConductance(this->SmoothingConductance);
		levelSetsThreshold->SetPropagationScaling(this->PropagationScaling);
		levelSetsThreshold->SetCurvatureScaling(this->CurvatureScaling);
		levelSetsThreshold->SetInputData(this->LevelSetsInput);
		levelSetsThreshold->SetNumberOfIterations(this->NumberOfIterations);
		levelSetsThreshold->SetIsoSurfaceValue(this->IsoSurfaceValue);
		levelSetsThreshold->SetMaximumRMSError(this->MaximumRMSError);
		levelSetsThreshold->SetInterpolateSurfaceLocation(1);
		levelSetsThreshold->SetUseImageSpacing(1);
		vtkSmartPointer<vtkCallbackCommand> ProgressCallCommand =
			vtkSmartPointer<vtkCallbackCommand>::New();
		// Set the callback to the function we created.
		ProgressCallCommand->SetCallback(LevelSetCallbackFunction);
		levelSetsThreshold->AddObserver(vtkCommand::ProgressEvent, ProgressCallCommand);
		levelSetsThreshold->Update();

		this->LevelSetsOutput = vtkSmartPointer<vtkImageData>::New();
		this->LevelSetsOutput->DeepCopy(levelSetsThreshold->GetOutput());
	}
	else if (!strcmp(this->LevelSetsType, "laplacian"))
	{
		vtkSmartPointer<vtkvmtkLaplacianSegmentationLevelSetImageFilter> levelSetsLaplacian =
			vtkSmartPointer<vtkvmtkLaplacianSegmentationLevelSetImageFilter>::New();
		levelSetsLaplacian->SetFeatureImage(this->Image);
		levelSetsLaplacian->SetPropagationScaling(0.0 - this->PropagationScaling);
		levelSetsLaplacian->SetCurvatureScaling(this->CurvatureScaling);
		levelSetsLaplacian->SetInputData(this->LevelSetsInput);
		levelSetsLaplacian->SetNumberOfIterations(this->NumberOfIterations);
		levelSetsLaplacian->SetIsoSurfaceValue(this->IsoSurfaceValue);
		levelSetsLaplacian->SetMaximumRMSError(this->MaximumRMSError);
		levelSetsLaplacian->SetInterpolateSurfaceLocation(1);
		levelSetsLaplacian->SetUseImageSpacing(1);
		vtkSmartPointer<vtkCallbackCommand> ProgressCallCommand =
			vtkSmartPointer<vtkCallbackCommand>::New();
		// Set the callback to the function we created.
		ProgressCallCommand->SetCallback(LevelSetCallbackFunction);
		levelSetsLaplacian->AddObserver(vtkCommand::ProgressEvent, ProgressCallCommand);
		levelSetsLaplacian->Update();

		this->LevelSetsOutput = vtkSmartPointer<vtkImageData>::New();
		this->LevelSetsOutput->DeepCopy(levelSetsLaplacian->GetOutput());
	}
	else
	{
		std::cout << "Unsupported LevelSetsType" << std::endl;
		return;
	}

}

//�ϲ���ͬ��ˮƽ��
void vtkvmtkLevelSetSegmentation::MergeLevelSet()
{
	//��ʼʱ�����ĿΪ��
	//if (this->LevelSets->GetNumberOfPoints() == 0)
	//	this->LevelSets->DeepCopy(this->LevelSetsOutput);
	if (this->LevelSets == nullptr)
		this->LevelSets = this->LevelSetsOutput;
	else
	{
		vtkSmartPointer<vtkImageMathematics> minFilter = 
			vtkSmartPointer<vtkImageMathematics>::New();
		minFilter->SetOperationToMin();
		minFilter->SetInput1Data(this->LevelSets);
		minFilter->SetInput2Data(this->LevelSetsOutput);
		minFilter->Update();
		//this->LevelSets->DeepCopy(minFilter->GetOutput());
		this->LevelSets = minFilter->GetOutput();
	}	
}

//��ʾˮƽ����������
void vtkvmtkLevelSetSegmentation::DisplayLevelSetSurface(vtkImageData *levelSets, double value)
{
	vtkSmartPointer<vtkMarchingCubes> marchingCubes = 
		vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputData(levelSets);
	marchingCubes->SetValue(0, value);
	marchingCubes->Update();

	std::cout << "Displaying.\n" << std::endl;

	//SurfaceViewer�ڱ��ࣨLevelSet)��Exe�ж���
	//this->SurfaceViewer->SetInputSurface(marchingCubes->GetOutput());
	this->SurfaceViewer->SetSurface(marchingCubes->GetOutput());
	this->SurfaceViewer->SetDisplay(false);
	this->SurfaceViewer->SetOpacity(0.5);
	//ͨ������Execute������BuildView,BuilViewΪ��ı�������
	this->SurfaceViewer->Execute();
}

//NumberOfIterations, PropagationScaling, CurvatureScaling, AdvectionScaling
//�ж�����Ĳ�������Ч��
int vtkvmtkLevelSetSegmentation::EvolutionParametersValidator(const char* text)
{
	if (text != nullptr)
		return 1;
	if (!strcmp(text, "q") || !strcmp(text, "e"))
		return 1;
	std::string TextSource = this->strip(text);  //�����ʼ�ͽ�β�Ŀո�
	std::vector<std::string> SplitText = this->split(TextSource);  //Ĭ���Կո�ָ��ַ���
	if (SplitText.size() < 1 || SplitText.size() > 4)
		return 0;
	int first = atoi(SplitText[0].c_str());
	//����try..catch, stoi, stod�Ľ��
	if (first == 0 || first == - 1)
	{
		std::cout << "Error Input" << std::endl;
		return 0;
	}
	//size�Ĵ�СΪ1 ����4
	if (SplitText.size() == 4)
	{
		double second = atof(SplitText[1].c_str());
		double third = atof(SplitText[2].c_str());
		double four = atof(SplitText[3].c_str());
		//������ַ���תΪdoubleʱ����
		if (abs(second - 0.0) < 1e-06 ||
			abs(third - 0.0) < 1e-06 ||
			abs(four - 0.0) < 1e-06)
		{
			std::cout << "Error Input" << std::endl;
			return 0;
		}
	}
	return 1;
}

//����ִ�е�����
void vtkvmtkLevelSetSegmentation::Execute()
{
	if (this->Image == nullptr)
	{
		std::cout << "No Input the Image!" << std::endl;
		return;
	}
	vtkSmartPointer<vtkImageCast> cast = 
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(this->Image);
	cast->SetOutputScalarTypeToFloat(); //DICOMͼ���ڴ�ռ�÷�������
	cast->Update();

	//this->Image = vtkImageData::New();
	//this->Image->DeepCopy(cast->GetOutput());
	//this->DeepCopyImage = true;

	this->Image = cast->GetOutput();


	//if (this->InitializationImage->GetNumberOfPoints() == 0)
	//	this->InitializationImage->DeepCopy(this->Image);
	if (this->InitializationImage == nullptr)
		this->InitializationImage = this->Image;

	//�������ͼ��Ϊ��
	//if (this->FeatureImage->GetNumberOfPoints() == 0)
	if (this->FeatureImage == nullptr)
	{
		if (!strcmp(this->LevelSetsType, "geodesic") ||
			!strcmp(this->LevelSetsType, "curves"))
		{
			vtkSmartPointer<vtkvmtkImageFeature> imageFeatures =
				vtkSmartPointer<vtkvmtkImageFeature>::New();
			imageFeatures->SetImage(this->Image);
			imageFeatures->SetFeatureImageType(this->FeatureImageType);
			imageFeatures->SetSigmoidRemapping(this->SigmoidRemapping);
			imageFeatures->SetDerivativeSigma(this->FeatureDerivativeSigma);
			imageFeatures->SetUpwindFactor(this->UpwindFactor);
			imageFeatures->SetFWHMRadius(this->FWHMRadius);
			imageFeatures->SetFWHMBackgroundValue(this->FWHMBackgroundValue);
			imageFeatures->Execute();
			//this->FeatureImage->DeepCopy(imageFeatures->GetFeatureImage());
			this->FeatureImage = imageFeatures->GetFeatureImage();
		}
		else if (!strcmp(this->LevelSetsType, "threshold") ||
			!strcmp(this->LevelSetsType, "laplacian"))
			//this->FeatureImage->DeepCopy(this->Image);
			this->FeatureImage = this->Image;
		else
			std::cout << "Unsupported LevelSetsType" << std::endl;
	}

	if (this->NumberOfIterations != 0)
	{
		//this->LevelSetsInput->DeepCopy(this->InitialLevelSets);
		this->LevelSetsInput = this->InitialLevelSets;
		this->LevelSetEvolution();
		this->MergeLevelSet();
		return;
	}

	if (this->vmtkRenderer == nullptr)
	{
		this->vmtkRenderer = vtkvmtkRenderer::New();
		this->vmtkRenderer->Initialize();
		this->InnervmtkRenderer = true;
	}
	
	//this->ImageSeeder = vtkvmtkImageSeeder::New();
	this->ImageSeeder = vtkSmartPointer<vtkvmtkImageSeeder>::New();
	this->ImageSeeder->SetRenderer(this->vmtkRenderer);
	//#this->ImageSeeder->SetImage(this->Image);
	this->ImageSeeder->SetImage(this->InitializationImage);
	this->ImageSeeder->SetDisplay(false);
	this->ImageSeeder->Execute();
	//#this->ImageSeeder->SetDisplay(true);
	this->ImageSeeder->BuildView();

	this->SurfaceViewer = vtkvmtkSurfaceViewer::New();
	this->SurfaceViewer->SetRenderer(this->vmtkRenderer);

	//ˮƽ���ǿ�
	//if (this->LevelSets->GetNumberOfPoints() != 0)
	if (this->LevelSets != nullptr)
		this->DisplayLevelSetSurface(this->LevelSets, 0.0);

	this->vmtkImageInitialization = vtkSmartPointer<vtkvmtkImageInitialization>::New();
	//this->vmtkImageInitialization->SetImage(this->Image);
	this->vmtkImageInitialization->SetImage(this->InitializationImage);
	this->vmtkImageInitialization->SetvmtkRenderer(this->vmtkRenderer);
	this->vmtkImageInitialization->SetImageSeeder(this->ImageSeeder);
	this->vmtkImageInitialization->SetSurfaceViewer(this->SurfaceViewer);
	this->vmtkImageInitialization->SetNegateImage(this->NegateForInitialization);

	bool endSegmentation = false;
	while (!endSegmentation)
	{
		if (this->InitialLevelSets == nullptr)
		//if (this->InitialLevelSets->GetNumberOfPoints() == 0)
		{
			this->vmtkImageInitialization->Execute();
			this->LevelSetsInput = this->vmtkImageInitialization->GetInitialLevelSets();
			//this->IsoSurfaceValue = this->vmtkImageInitialization->GetIsoSurfaceValue();
			this->vmtkImageInitialization->SetInitialLevelSets(nullptr);
			//this->vmtkImageInitialization->SetIsoSurfaceValue(0.0);
			this->IsoSurfaceValue = 0.0;
		}
		else
		{
			this->LevelSetsInput = this->InitialLevelSets;
			this->InitialLevelSets = nullptr;
			this->DisplayLevelSetSurface(this->LevelSetsInput, this->IsoSurfaceValue);
		}

		bool endEvolution = false;
		std::string queryString = "";
		const char *inputString = "";
		while (!endEvolution)
		{
			queryString = "Please input parameters (type return to accept current values, \'e\' to end, \'q\' to quit):\n";
			queryString += "NumberOfIterations(" + std::to_string(this->NumberOfIterations) +  ")\n";
			queryString += "PropagationScaling(" + std::to_string(this->PropagationScaling) +  ")\n";
			queryString += "CurvatureScaling(" + std::to_string(this->CurvatureScaling) + ")\n";
			queryString += "AdvectionScaling(" + std::to_string(this->AdvectionScaling) + ")\n";
			inputString = this->InputText(queryString.c_str(), &vtkvmtkLevelSetSegmentation::EvolutionParametersValidator);
			if (!strcmp(inputString, "q"))
				return;
			else if (!strcmp(inputString, "e"))
				endEvolution = true;
			else if (strcmp(inputString, "")) // !=
			{
				this->strip(inputString); //ȥ��ǰ��ո�
				std::vector<std::string> splitInputString = this->split(inputString, " ");  //�Կո�ָ��ַ���
				if (splitInputString.size() == 1)
					this->NumberOfIterations = atoi(splitInputString[0].c_str());
				else if (splitInputString.size() == 4)
				{
					this->NumberOfIterations = atoi(splitInputString[0].c_str());
					this->PropagationScaling = atof(splitInputString[1].c_str());
					this->CurvatureScaling = atof(splitInputString[2].c_str());
					this->AdvectionScaling = atof(splitInputString[3].c_str());
				}
				else
				{
					std::cout << "Wrong number of parameters." << std::endl;
					continue;
				}
			}
			else
			{
				std::cout << "bug In vtkvmtkLevelSetSegmentation 521 line " << std::endl;
			}
			if (endEvolution)
				break;
			this->LevelSetEvolution();
			this->DisplayLevelSetSurface(this->LevelSetsOutput);

			queryString = "Accept result? (y/n): ";
			inputString = this->InputText(queryString.c_str(), &vtkvmtkLevelSetSegmentation::YesNoValidator);
			if (!strcmp(inputString, "y"))
				endEvolution = true;
			else if (!strcmp(inputString, "n"))
				endEvolution = false;
		}
		queryString = "Merge branch? (y/n): ";
		inputString = this->InputText(queryString.c_str(), &vtkvmtkLevelSetSegmentation::YesNoValidator);
		if (!strcmp(inputString, "y"))
			this->MergeLevelSet();
		//else if (!strcmp(inputString, "n"))
			//; //pass��ʲôҲ����
		if (this->LevelSets != nullptr)
			this->DisplayLevelSetSurface(this->LevelSets);
		queryString = "Segment another branch? (y/n): ";
		inputString = this->InputText(queryString.c_str(), &vtkvmtkLevelSetSegmentation::YesNoValidator);
		if (!strcmp(inputString, "y"))
			endSegmentation = false;
		else if (!strcmp(inputString, "n"))
			endSegmentation = true;
	}
}


//���ô˺��������ˮƽ����������ɵĽ���
void LevelSetCallbackFunction(vtkObject* caller, long unsigned int eventId,
	void* clientData, void* callData)
{
	//�����߾�ΪvtkSimpleImageToImageFilter�����࣬�ɽ��а�ȫ������ת��
	vtkSimpleImageToImageFilter* filter = static_cast<vtkSimpleImageToImageFilter*>(caller);
	static double ProgressPre = 0.0; //��¼��һ�εĽ��� 0.0 - 1.0
	int percentStep = 10;   //ÿһ���Ľ�����10%���ϲ����
	int ProgressCurrent = filter->GetProgress();
	//�������һ�εĽ��������10%�������������
	if (int(ProgressCurrent * 100) / percentStep == int(ProgressPre * 100) / percentStep)
		return;
	ProgressPre = ProgressCurrent;
	std::cout << "Progress: " << std::to_string(int(100 * ProgressCurrent)) << "%" << std::endl;
}

//ȥ���ַ�������ʼ�ͽ�β�Ŀհ��ַ�
std::string vtkvmtkLevelSetSegmentation::strip(const std::string &str, char ch)
{
	int i = 0;
	while (str[i] == ch)// ͷ��ch�ַ������� i
		i++;
	int j = str.size() - 1;
	while (str[j] == ch) //
		j--;
	return str.substr(i, j + 1 - i);

}

//���ַ�ch�ָ��ַ�����Ĭ���Կո�ָ���طָ����ַ�vector
std::vector<std::string> vtkvmtkLevelSetSegmentation::split(const std::string &str, std::string ch)
{
	//�� ch Ϊ�ָ��ַ����� cstr �ָ�Ϊ���Ԫ�ش浽vector
	std::vector<std::string> ret;
	int pos = 0;
	int start = 0;
	while ((pos = str.find(ch, start)) != std::string::npos)
	{
		if (pos > start)
			ret.push_back(str.substr(start, pos - start));
		start = pos + ch.size(); //����ǿո���ÿһ������1
	}
	if (str.size() > start)
		ret.push_back(str.substr(start));
	return ret;
}