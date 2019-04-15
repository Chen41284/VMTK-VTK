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

vtkStandardNewMacro(vtkvmtkLevelSetSegmentation);

vtkvmtkLevelSetSegmentation::vtkvmtkLevelSetSegmentation()
{
	this->Image = nullptr;
	this->vmtkRenderer = nullptr;
	this->SurfaceViewer = nullptr;
	this->OwnRenderer = false;
	this->DeepCopyImage = false;
	this->InitialLevelSets = vtkImageData::New();
	this->InitializationImage = vtkImageData::New();
	this->FeatureImage = vtkImageData::New();
	this->LevelSetsInput = vtkImageData::New();
	this->LevelSetsOutput = vtkImageData::New();
	this->LevelSets = vtkImageData::New();;
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
	this->FWHMRadius[0] = 1.0; this->FWHMRadius[1] = 1.0; this->FWHMRadius[2] = 1.0;
	this->FWHMBackgroundValue = 0.0;
	this->EdgeWeight = 0.0;
	this->SmoothingIterations = 5;
	this->SmoothingTimeStep = 0.1;
	this->SmoothingConductance = 0.8;
}

vtkvmtkLevelSetSegmentation::~vtkvmtkLevelSetSegmentation()
{
	if (this->Image != nullptr && this->DeepCopyImage == true)
	{
		this->Image->Delete();
		this->Image = nullptr;
	}
	if (this->vmtkRenderer != nullptr && this->InnervmtkRenderer == true)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = nullptr;
	}
	if (this->InitialLevelSets)
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
	}
	if (this->SurfaceViewer)
	{
		this->SurfaceViewer->Delete();
		this->SurfaceViewer = nullptr;
	}
	delete[] this->LevelSetsType;
	this->LevelSetsType = nullptr;
	delete[] this->FeatureImageType;
	this->FeatureImageType = nullptr;
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
	if (atoi(text) == 0)
	{
		std::cout << "Input text is error!" << std::endl;
		return 0;
	}
	if (atoi(text) == -1)
	{
		std::cout << "The Input text is too large!" << std::endl;
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
int vtkvmtkLevelSetSegmentation::ThresholdInput(const char* queryString)
{
	//�������еĳ���Ա������������ǰ׺
	const char* thresholdString = this->InputText(queryString, &vtkvmtkLevelSetSegmentation::ThresholdValidator);

	int threshold = 0;
	if (strcmp(thresholdString, "n")) //not equal
		threshold = atoi(thresholdString);

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
	if (this->LevelSets->GetNumberOfPoints() == 0)
		this->LevelSets->DeepCopy(this->LevelSetsOutput);
	else
	{
		vtkSmartPointer<vtkImageMathematics> minFilter = 
			vtkSmartPointer<vtkImageMathematics>::New();
		minFilter->SetOperationToMin();
		minFilter->SetInput1Data(this->LevelSets);
		minFilter->SetInput2Data(this->LevelSetsOutput);
		minFilter->Update();
		this->LevelSets->DeepCopy(minFilter->GetOutput());
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
	this->SurfaceViewer->SetInputSurface(marchingCubes->GetOutput());
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
	cast->SetOutputScalarTypeToFloat();
	cast->Update();
	this->Image = vtkImageData::New();
	this->Image->DeepCopy(cast->GetOutput());
	this->DeepCopyImage = true;

	if (this->InitializationImage->GetNumberOfPoints() == 0)
		this->InitializationImage->DeepCopy(this->Image);

	if (this->FeatureImage->GetNumberOfPoints() == 0)
	{
		if(!strcmp(this->LevelSetsType, "geodesic") ||
			!strcmp(this->LevelSetsType, "curves"))
		{

		}
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