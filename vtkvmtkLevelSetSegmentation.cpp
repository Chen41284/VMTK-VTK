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
int vtkvmtkLevelSetSegmentation::ThresholdInput(const char* queryString)
{
	//调用类中的程序员函数，加类名前缀
	const char* thresholdString = this->InputText(queryString, &vtkvmtkLevelSetSegmentation::ThresholdValidator);

	int threshold = 0;
	if (strcmp(thresholdString, "n")) //not equal
		threshold = atoi(thresholdString);

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

//合并不同的水平集
void vtkvmtkLevelSetSegmentation::MergeLevelSet()
{
	//初始时点的数目为零
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
	this->SurfaceViewer->SetInputSurface(marchingCubes->GetOutput());
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