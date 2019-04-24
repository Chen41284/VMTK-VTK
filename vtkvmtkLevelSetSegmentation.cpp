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


//验证输入的阈值的有效性
int vtkvmtkLevelSetSegmentation::ThresholdValidator(const char* text)
{
	if (!strcmp(text, "n"))  //无输入
		return 1;
	if (!strcmp(text, "i")) //交互
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

//验证输入的字符串是否是"Y"或"N"
int vtkvmtkLevelSetSegmentation::YesNoValidator(const char* text)
{
	if (!strcmp(text, "n") || !strcmp(text, "y"))
		return 1;
	return 0;
}

//pypescript中的调用标准输入流输入数据并验证有效性
//这里改为调用KeyPressInteractorStyle的键盘相应获取输入并验证输入的数据有效性
//validator为验证输入的数据有效性的函数，有效返回1，无效返回0
const char* vtkvmtkLevelSetSegmentation::InputText(const char*queryString, 
	int(vtkvmtkLevelSetSegmentation::*validator)(const char* text))
{
	//动态调用vmtkRenderer中的Keypress的交互类获取用户从键盘输入的数据
	const char* InputString = this->vmtkRenderer->PromptAsync(queryString);
	//如果传入的验证函数非空，则使用此函数验证输入的字符串的有效性
	if (validator != nullptr)
	{
		while ((this->*validator)(InputString) == 0) //输入的字符无效，重新输入
			InputString = this->vmtkRenderer->PromptAsync(queryString);
	}
	return InputString;
}

//获取输入的阈值
double vtkvmtkLevelSetSegmentation::ThresholdInput(const char* queryString)
{
	//调用类中的程序员函数，加类名前缀
	const char* thresholdString = this->InputText(queryString, &vtkvmtkLevelSetSegmentation::ThresholdValidator);

	double threshold = 0.0;
	if (strcmp(thresholdString, "n")) //not equal != 
		threshold = atof(thresholdString);

	return threshold;
}

//设置图像的水平集,生成图像的水平集
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
		if (fabs(this->LowerThreshold - 0.0) > 1e-6)  //阈值的下限不为零时，设置阈值的下限
			levelSetsThreshold->SetLowerThreshold(this->LowerThreshold);
		else
			levelSetsThreshold->SetLowerThreshold(scalarRange[0] - 1.0);
		if (fabs(this->UpperThreshold - 0.0) > 1e-6)  //阈值的上限不为零时，设置阈值的上限
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

//合并不同的水平集
void vtkvmtkLevelSetSegmentation::MergeLevelSet()
{
	//初始时点的数目为零
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

//显示水平集的面绘表面
void vtkvmtkLevelSetSegmentation::DisplayLevelSetSurface(vtkImageData *levelSets, double value)
{
	vtkSmartPointer<vtkMarchingCubes> marchingCubes = 
		vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputData(levelSets);
	marchingCubes->SetValue(0, value);
	marchingCubes->Update();

	std::cout << "Displaying.\n" << std::endl;

	//SurfaceViewer在本类（LevelSet)的Exe中定义
	//this->SurfaceViewer->SetInputSurface(marchingCubes->GetOutput());
	this->SurfaceViewer->SetSurface(marchingCubes->GetOutput());
	this->SurfaceViewer->SetDisplay(false);
	this->SurfaceViewer->SetOpacity(0.5);
	//通过调用Execute来调用BuildView,BuilView为类的保护方法
	this->SurfaceViewer->Execute();
}

//NumberOfIterations, PropagationScaling, CurvatureScaling, AdvectionScaling
//判断输入的参数的有效性
int vtkvmtkLevelSetSegmentation::EvolutionParametersValidator(const char* text)
{
	if (text != nullptr)
		return 1;
	if (!strcmp(text, "q") || !strcmp(text, "e"))
		return 1;
	std::string TextSource = this->strip(text);  //清除起始和结尾的空格
	std::vector<std::string> SplitText = this->split(TextSource);  //默认以空格分割字符串
	if (SplitText.size() < 1 || SplitText.size() > 4)
		return 0;
	int first = atoi(SplitText[0].c_str());
	//可用try..catch, stoi, stod的结合
	if (first == 0 || first == - 1)
	{
		std::cout << "Error Input" << std::endl;
		return 0;
	}
	//size的大小为1 或是4
	if (SplitText.size() == 4)
	{
		double second = atof(SplitText[1].c_str());
		double third = atof(SplitText[2].c_str());
		double four = atof(SplitText[3].c_str());
		//输入的字符串转为double时出错
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

//该类执行的主体
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
	cast->SetOutputScalarTypeToFloat(); //DICOM图像内存占用翻了三倍
	cast->Update();

	//this->Image = vtkImageData::New();
	//this->Image->DeepCopy(cast->GetOutput());
	//this->DeepCopyImage = true;

	this->Image = cast->GetOutput();


	//if (this->InitializationImage->GetNumberOfPoints() == 0)
	//	this->InitializationImage->DeepCopy(this->Image);
	if (this->InitializationImage == nullptr)
		this->InitializationImage = this->Image;

	//如果特征图像为空
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

	//水平集非空
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
				this->strip(inputString); //去除前后空格
				std::vector<std::string> splitInputString = this->split(inputString, " ");  //以空格分割字符串
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
			//; //pass，什么也不做
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


//调用此函数，输出水平集对象的生成的进度
void LevelSetCallbackFunction(vtkObject* caller, long unsigned int eventId,
	void* clientData, void* callData)
{
	//调用者均为vtkSimpleImageToImageFilter的子类，可进行安全的上行转换
	vtkSimpleImageToImageFilter* filter = static_cast<vtkSimpleImageToImageFilter*>(caller);
	static double ProgressPre = 0.0; //记录上一次的进度 0.0 - 1.0
	int percentStep = 10;   //每一步的进度在10%以上才输出
	int ProgressCurrent = filter->GetProgress();
	//如果和上一次的进度相比在10%以内则不输出进度
	if (int(ProgressCurrent * 100) / percentStep == int(ProgressPre * 100) / percentStep)
		return;
	ProgressPre = ProgressCurrent;
	std::cout << "Progress: " << std::to_string(int(100 * ProgressCurrent)) << "%" << std::endl;
}

//去除字符序列起始和结尾的空白字符
std::string vtkvmtkLevelSetSegmentation::strip(const std::string &str, char ch)
{
	int i = 0;
	while (str[i] == ch)// 头部ch字符个数是 i
		i++;
	int j = str.size() - 1;
	while (str[j] == ch) //
		j--;
	return str.substr(i, j + 1 - i);

}

//按字符ch分割字符串，默认以空格分割，返回分割后的字符vector
std::vector<std::string> vtkvmtkLevelSetSegmentation::split(const std::string &str, std::string ch)
{
	//以 ch 为分割字符，把 cstr 分割为多个元素存到vector
	std::vector<std::string> ret;
	int pos = 0;
	int start = 0;
	while ((pos = str.find(ch, start)) != std::string::npos)
	{
		if (pos > start)
			ret.push_back(str.substr(start, pos - start));
		start = pos + ch.size(); //如果是空格，则每一次增加1
	}
	if (str.size() > start)
		ret.push_back(str.substr(start));
	return ret;
}