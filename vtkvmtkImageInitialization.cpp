#include "vtkvmtkImageInitialization.h"

//STD
#include <functional>
#include <map>

//VTK
#include <vtkImageMathematics.h>
#include <vtkIdList.h>
#include <vtkImageThreshold.h>
#include <vtkImageShiftScale.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkImageDilateErode3D.h>
#include <vtkMarchingCubes.h>
#include <vtkImageCast.h>

//VMTK-Src
#include <vtkvmtkFastMarchingUpwindGradientImageFilter.h>
#include <vtkvmtkCollidingFrontsImageFilter.h>

//VMTK-New
#include "vtkvmtkImageSeeder.h"

//Windows
#include "WindowsAPI.h"

vtkStandardNewMacro(vtkvmtkImageInitialization);

vtkvmtkImageInitialization::vtkvmtkImageInitialization()
{
	this->vmtkRenderer = nullptr;
	this->Image = nullptr;
	this->InnervmtkRenderer = false;
	this->InitialLevelSets = nullptr;
	this->Surface = nullptr;
	this->Interactive = true;
	this->SetMethod("collidingfronts");
	this->UpperThreshold = nullptr;
	this->LowerThreshold = nullptr;
	this->NegateImage = false;
	this->IsoSurfaceValue = 0.0;
	this->ImageSeeder = nullptr;
	this->SurfaceViewer = nullptr;
	this->MergedInitialLevelSets = nullptr;
}

vtkvmtkImageInitialization::~vtkvmtkImageInitialization()
{
	if (this->vmtkRenderer != nullptr && this->InnervmtkRenderer == true)
	{
		this->vmtkRenderer->Delete();
		this->vmtkRenderer = nullptr;
	}
	delete this->UpperThreshold;
	delete this->LowerThreshold;
	/*if (this->InitialLevelSets)  //智能指针不用手动释放
	{
		this->InitialLevelSets->Delete();
		this->InitialLevelSets = nullptr;
	}
	if (this->Surface)
	{
		this->Surface->Delete();
		this->Surface = nullptr;
	}
	if (this->ImageSeeder)
	{
		this->ImageSeeder->Delete();
		this->ImageSeeder = nullptr;
	}
	if (this->SurfaceViewer)
	{
		this->SurfaceViewer->Delete();
		this->SurfaceViewer = nullptr;
	}
	if (this->MergedInitialLevelSets)
	{
		this->MergedInitialLevelSets->Delete();
		this->MergedInitialLevelSets = nullptr;
	}*/
}

void vtkvmtkImageInitialization::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK Image Initialization\n";
}

////种子点的输入
//vtkPolyData* vtkvmtkImageInitialization::SeedInput(const char *queryString, int numberOfSeeds)
vtkSmartPointer<vtkPolyData> vtkvmtkImageInitialization::SeedInput(const char* queryString, int numberOfSeeds)
{
	bool invalid = true;
	while (invalid)
	{
		invalid = false;
		std::string Info(queryString);
		Info += " (click on the image while pressing Ctrl).\n";
		this->ImageSeeder->InitializeSeeds();
		this->vmtkRenderer->SetPromptMessage(Info.c_str());
		this->vmtkRenderer->Render();
		if (numberOfSeeds > 0)
			if (this->ImageSeeder->GetSeeds()->GetNumberOfPoints() != numberOfSeeds)
			{
				std::string InvalidInfo = "Invalid selection. Please place exactly \'" +
					std::to_string(numberOfSeeds) + "\' seeds.\n";
				invalid = true;
				continue;   //继续循环输入，指导应输入的种子点数达到要求
			}
	}

	//vtkPolyData *seeds = vtkPolyData::New();
	vtkSmartPointer<vtkPolyData> seeds = vtkSmartPointer<vtkPolyData>::New();
	seeds->DeepCopy(this->ImageSeeder->GetSeeds());

	return seeds;
}

//验证输入的字符串的有效性
int vtkvmtkImageInitialization::ThresholdValidator(const char* text)
{
	if (!strcmp(text, "n"))  //无输入
		return 1;
	if (!strcmp(text, "i")) //交互
	{
		this->vmtkRenderer->Render();
		return 0;
	}
	if (abs(atof(text) - 0.0) < 1e-06) //atof返回0.0代表输入的字符串无效
	{
		std::cout << "Input text is error!" << std::endl;
		return 0;
	}
	return 1;
}

//输入阈值
double vtkvmtkImageInitialization::ThresholdInput(const char* queryString)
{
	//调用类中的程序员函数，加类名前缀
	const char* thresholdString = this->InputText(queryString, &vtkvmtkImageInitialization::ThresholdValidator);

	double threshold = 0.0;
	if (strcmp(thresholdString, "n")) //not equal != 
		threshold = atof(thresholdString);

	return threshold;
}

//pypescript中的调用标准输入流输入数据并验证有效性
//这里改为调用KeyPressInteractorStyle的键盘相应获取输入并验证输入的数据有效性
//validator为验证输入的数据有效性的函数，有效返回1，无效返回0
const char* vtkvmtkImageInitialization::InputText(const char*queryString,
	int(vtkvmtkImageInitialization::*validator)(const char* text))
{
	//动态调用vmtkRenderer中的Keypress的交互类获取用户从键盘输入的数据
	const char* InputString = this->vmtkRenderer->PromptAsync(queryString);
	std::cout << "InputString  " << InputString << std::endl;
	std::cout << "(this->*validator)(InputString)  " << (this->*validator)(InputString) << std::endl;
	//如果传入的验证函数非空，则使用此函数验证输入的字符串的有效性
	int count = 0;
	if (validator != nullptr)
	{
		while ((this->*validator)(InputString) == 0) //输入的字符无效，重新输入
		{
			++count;
			std::cout << "Error Input times: " << count << std::endl;
			InputString = this->vmtkRenderer->PromptAsync(queryString);
		}			
	}
	return InputString;
}

//验证输入的字符串是否是"Y"或"N"
int vtkvmtkImageInitialization::YesNoValidator(const char* text)
{
	if (!strcmp(text, "n") || !strcmp(text, "y"))
		return 1;
	return 0;
}

//验证输入的字符串是否是{0, 1, 2, 3, 4}中的一个
int vtkvmtkImageInitialization::InitializationTypeValidator(const char* text)
{
	std::string numberString = "01234";
	if (strlen(text) < 1)  //空字符
		return 0;
	if (strlen(text) > 1) //仅有一个数字，多于一个数字的肯定不符合
		return 0;

	std::size_t found = numberString.find(text);
	if (found != std::string::npos) //找到的话
		return 1;

	return 0; //其他情况 5，6，7等
}

//阈值初始化
void vtkvmtkImageInitialization::IsosurfaceInitialize()
{
	std::cout << "Isosurface initialization." << std::endl;

	if (this->Interactive)
	{
		const char* queryString = "Please input isosurface level (\'n\' for none): ";
		this->IsoSurfaceValue = this->ThresholdInput(queryString);
	}
	vtkSmartPointer<vtkImageMathematics> imageMathematics =
		vtkSmartPointer<vtkImageMathematics>::New();
	imageMathematics->SetInputData(this->Image);
	imageMathematics->SetConstantK(-1.0);
	imageMathematics->SetOperationToMultiplyByK();
	imageMathematics->Update();

	vtkSmartPointer<vtkImageMathematics> subtract = 
		vtkSmartPointer<vtkImageMathematics>::New();
	subtract->SetInputConnection(imageMathematics->GetOutputPort());
	subtract->SetOperationToAddConstant();
	subtract->SetConstantC(this->IsoSurfaceValue);
	subtract->Update();

	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->InitialLevelSets->DeepCopy(subtract->GetOutput());
	this->IsoSurfaceValue = 0.0;   //重置
}

//上下阈值的初始化
void vtkvmtkImageInitialization::ThresholdInitialize()
{
	std::cout << "Threshold initialization." << std::endl;

	if (this->Interactive)
	{
		const char *queryString = "Please input lower threshold (\'n\' for none): ";
		double LowerThreshold = this->ThresholdInput(queryString);
		this->SetLowThreshold(LowerThreshold);  //要创建new double

		queryString = "Please input upper threshold (\'n\' for none): ";
		double UpperThreshold = this->ThresholdInput(queryString);
		this->SetUpperThreshold(UpperThreshold);
	}

	double *scalarRange = this->Image->GetScalarRange();
	//thresholdedImage = self.Image;
	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->IsoSurfaceValue = 0.0;
}

//快速的面绘
void vtkvmtkImageInitialization::FastMarchingInitialize()
{
	std::cout << "Fast marching initialization." << std::endl;

	vtkSmartPointer<vtkIdList> sourceSeedIds = 
		vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> targetSeedIds = 
		vtkSmartPointer<vtkIdList>::New();

	//交互式输入种子点
	if (this->Interactive)
	{
		const char *queryString = "Please input lower threshold (\'n\' for none): ";
		double LowerThreshold = this->ThresholdInput(queryString);
		this->SetLowThreshold(LowerThreshold);

		queryString = "Please input upper threshold (\'n\' for none): ";
		double UpperThreshold = this->ThresholdInput(queryString);
		this->SetUpperThreshold(UpperThreshold);

		queryString = "Please place source seeds";
		//vtkPolyData *sourceSeeds = this->SeedInput(queryString, 0);
		vtkSmartPointer<vtkPolyData> sourceSeeds = this->SeedInput(queryString, 0);

		queryString = "Please place target seeds";
		//vtkPolyData *targetSeeds = this->SeedInput(queryString, 0);
		vtkSmartPointer<vtkPolyData> targetSeeds = this->SeedInput(queryString, 0);

		for (int i = 0; i < sourceSeeds->GetNumberOfPoints(); ++i)
			sourceSeedIds->InsertNextId(this->Image->FindPoint(sourceSeeds->GetPoint(i)));

		for (int i = 0; i < targetSeeds->GetNumberOfPoints(); ++i)
			targetSeedIds->InsertNextId(this->Image->FindPoint(targetSeeds->GetPoint(i)));

		//释放内存
		//sourceSeeds->Delete();
		//targetSeeds->Delete();
	} //end if
	else //由用户指定输入
	{
		int SourcePointsLength = int(this->SourcePoints.size() / 3);
		for (int i = 0; i < SourcePointsLength; ++i)
		{
			int IJK[3] = { this->SourcePoints[3 * i + 0],this->SourcePoints[3 * i + 1], this->SourcePoints[3 * i + 2] };
			sourceSeedIds->InsertNextId(this->Image->ComputePointId(IJK));
		}

		int TargetPointsLength = int(this->TargetPoints.size() / 3);
		for (int i = 0; i < TargetPointsLength; ++i)
		{
			int IJK[3] = { this->TargetPoints[3 * i + 0], this->TargetPoints[3 * i + 1], this->TargetPoints[3 * i + 2] };
			targetSeedIds->InsertNextId(this->Image->ComputePointId(IJK));
		}			
	}// end else

	double *scalarRange = this->Image->GetScalarRange();
	vtkSmartPointer<vtkImageData> thresholdedImage =
		vtkSmartPointer<vtkImageData>::New();
	bool flagThreshold = false; //判断是否进行了阈值化
	//设置面绘的上下阈值
	if ( (this->LowerThreshold != nullptr) || (this->UpperThreshold != nullptr) )
	{
		vtkSmartPointer<vtkImageThreshold> threshold =
			vtkSmartPointer<vtkImageThreshold>::New();
		threshold->SetInputData(this->Image);
		if ((this->LowerThreshold != nullptr) && (this->UpperThreshold != nullptr))
			threshold->ThresholdBetween(*(this->LowerThreshold), *(this->UpperThreshold));
		else if (this->LowerThreshold != nullptr)
			threshold->ThresholdByUpper(*(this->LowerThreshold));
		else if (this->UpperThreshold != nullptr)
			threshold->ThresholdByLower(*(this->UpperThreshold));
		threshold->ReplaceInOff();
		threshold->ReplaceOutOn();
		threshold->SetOutValue(scalarRange[0] - scalarRange[1]);
		threshold->Update();
		scalarRange = threshold->GetOutput()->GetScalarRange();
		thresholdedImage->DeepCopy(threshold->GetOutput());
		flagThreshold = true;  //已经阈值化
	}

	double scale = 1.0;
	if (scalarRange[1] - scalarRange[0] > 0.0)
		scale = 1.0 / (scalarRange[1] - scalarRange[0]);

	//偏移尺度
	vtkSmartPointer<vtkImageShiftScale> shiftScale = 
		vtkSmartPointer<vtkImageShiftScale>::New();
	if (flagThreshold)
		shiftScale->SetInputData(thresholdedImage);
	else
		shiftScale->SetInputData(this->Image);
	shiftScale->SetShift(-scalarRange[0]);
	shiftScale->SetScale(scale);
	shiftScale->SetOutputScalarTypeToFloat();
	shiftScale->Update();

	//speedImage = shiftScale.GetOutput()
	vtkSmartPointer<vtkvmtkFastMarchingUpwindGradientImageFilter> fastMarching =
		vtkSmartPointer<vtkvmtkFastMarchingUpwindGradientImageFilter>::New();
	fastMarching->SetInputData(shiftScale->GetOutput());
	fastMarching->SetSeeds(sourceSeedIds);
	fastMarching->GenerateGradientImageOff();
	fastMarching->SetTargetOffset(100.0);
	fastMarching->SetTargets(targetSeedIds);
	if (targetSeedIds->GetNumberOfIds() > 0)
		fastMarching->SetTargetReachedModeToOneTarget();
	else
		fastMarching->SetTargetReachedModeToNoTargets();
	fastMarching->Update();
	vtkSmartPointer<vtkImageMathematics> subtract =
		vtkSmartPointer<vtkImageMathematics>::New();
	subtract->SetInputConnection(fastMarching->GetOutputPort());
	subtract->SetOperationToAddConstant();
	subtract->SetConstantC(-fastMarching->GetTargetValue());
	subtract->Update();
	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->InitialLevelSets->DeepCopy(subtract->GetOutput());
	this->IsoSurfaceValue = 0.0;
}

//碰撞前线初始化
void vtkvmtkImageInitialization::CollidingFrontsInitialize()
{
	std::cout << "Colliding fronts initialization." << std::endl;
	showMemoryInfo();
	vtkSmartPointer<vtkIdList> seedIds1 = 
		vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> seedIds2 = 
		vtkSmartPointer<vtkIdList>::New();
	//交互式输入种子点
	if (this->Interactive)
	{
		const char *queryString = "Please input lower threshold (\'n\' for none): ";
		double LowerThreshold = this->ThresholdInput(queryString);
		this->SetLowThreshold(LowerThreshold);

		queryString = "Please input upper threshold (\'n\' for none): ";
		double UpperThreshold = this->ThresholdInput(queryString);
		this->SetUpperThreshold(UpperThreshold);

		queryString = "Please place two seeds";
		//vtkPolyData *seeds = this->SeedInput(queryString, 2);
		vtkSmartPointer<vtkPolyData> seeds = this->SeedInput(queryString, 2);

		seedIds1->InsertNextId(this->Image->FindPoint(seeds->GetPoint(0)));
		seedIds2->InsertNextId(this->Image->FindPoint(seeds->GetPoint(1)));
		
		//释放内存
		//seeds->Delete();
	} //end if
	else //由用户指定输入
	{
		int SourcePointsIJK[3] = { this->SourcePoints[0], this->SourcePoints[1], this->SourcePoints[2] };
		seedIds1->InsertNextId(this->Image->ComputePointId(SourcePointsIJK));
		int TargetPointsIJK[3] = { this->TargetPoints[0], this->TargetPoints[1], this->TargetPoints[2] };
		seedIds2->InsertNextId(this->Image->ComputePointId(TargetPointsIJK));
	}

	double *scalarRange = this->Image->GetScalarRange();
	vtkSmartPointer<vtkImageData> thresholdedImage =
		vtkSmartPointer<vtkImageData>::New();
	bool flagThreshold = false; //判断是否进行了阈值化
	//设置面绘的上下阈值
	if ((this->LowerThreshold != nullptr) || (this->UpperThreshold != nullptr))
	{
		vtkSmartPointer<vtkImageThreshold> threshold =
			vtkSmartPointer<vtkImageThreshold>::New();
		threshold->SetInputData(this->Image);
		if ((this->LowerThreshold != nullptr) && (this->UpperThreshold != nullptr))
			threshold->ThresholdBetween(*(this->LowerThreshold), *(this->UpperThreshold));
		else if (this->LowerThreshold != nullptr)
			threshold->ThresholdByUpper(*(this->LowerThreshold));
		else if (this->UpperThreshold != nullptr)
			threshold->ThresholdByLower(*(this->UpperThreshold));
		threshold->ReplaceInOff();
		threshold->ReplaceOutOn();
		threshold->SetOutValue(scalarRange[0] - scalarRange[1]);
		threshold->Update();
		scalarRange = threshold->GetOutput()->GetScalarRange();
		thresholdedImage->DeepCopy(threshold->GetOutput());
		flagThreshold = true;  //已经阈值化
	}

	double scale = 1.0;
	if (scalarRange[1] - scalarRange[0] > 0.0)
		scale = 1.0 / (scalarRange[1] - scalarRange[0]);

	//偏移尺度
	vtkSmartPointer<vtkImageShiftScale> shiftScale =
		vtkSmartPointer<vtkImageShiftScale>::New();
	if (flagThreshold)
		shiftScale->SetInputData(thresholdedImage);
	else
		shiftScale->SetInputData(this->Image);
	shiftScale->SetShift(-scalarRange[0]);
	shiftScale->SetScale(scale);
	shiftScale->SetOutputScalarTypeToFloat();
	shiftScale->Update();

	//speedImage = shiftScale.GetOutput()

	//大量消耗内存的操作，（页文件内存（虚拟内存）翻倍）
	vtkSmartPointer<vtkvmtkCollidingFrontsImageFilter> collidingFronts = 
		vtkSmartPointer<vtkvmtkCollidingFrontsImageFilter>::New();
	collidingFronts->SetInputData(shiftScale->GetOutput());
	collidingFronts->SetSeeds1(seedIds1);
	collidingFronts->SetSeeds2(seedIds2);
	collidingFronts->ApplyConnectivityOn();
	collidingFronts->StopOnTargetsOn();
	collidingFronts->Update();

	vtkSmartPointer<vtkImageMathematics> subtract = 
		vtkSmartPointer<vtkImageMathematics>::New();
	subtract->SetInputConnection(collidingFronts->GetOutputPort());
	subtract->SetOperationToAddConstant();
	subtract->SetConstantC(-10.0 * collidingFronts->GetNegativeEpsilon());
	subtract->Update();

	//使用DeepCopy可以在结束该函数调用后，将本函数内的类的内存回收
	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->InitialLevelSets->DeepCopy(subtract->GetOutput());
	this->IsoSurfaceValue = 0.0;

}

//种子点的初始化
void vtkvmtkImageInitialization::SeedInitialize()
{
	std::cout << "Seed initialization." << std::endl;
	vtkSmartPointer<vtkIdList> seedIds = 
		vtkSmartPointer<vtkIdList>::New();
	if (this->Interactive)  //交互式输入
	{
		const char* queryString = "Please place seeds";
		//vtkPolyData *seeds = this->SeedInput(queryString, 0);
		vtkSmartPointer<vtkPolyData> seeds = this->SeedInput(queryString, 0);
		for (int i = 0; i < seeds->GetNumberOfPoints(); ++i)
		{
			seedIds->InsertNextId(this->Image->FindPoint(seeds->GetPoint(i)));
		}		
	}
	else   //由用户指定
	{
		int SourcePointsLength = int(this->SourcePoints.size() / 3);
		for (int i = 0; i < SourcePointsLength; ++i)
		{
			int IJK[3] = { this->SourcePoints[i * 3 + 0], this->SourcePoints[i * 3 + 1], this->SourcePoints[i * 3 + 2] };
			seedIds->InsertNextId(this->Image->ComputePointId(IJK));
		}
		int TargetPointsLength = int(this->TargetPoints.size() / 3);
		for (int i = 0; i < TargetPointsLength; ++i)
		{
			int IJK[3] = { this->TargetPoints[i * 3 + 0], this->TargetPoints[i * 3 + 1], this->TargetPoints[i * 3 + 2] };
			seedIds->InsertNextId(this->Image->ComputePointId(IJK));
		}
	}

	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->InitialLevelSets->DeepCopy(this->Image);

	vtkDataArray* levelSetsInputScalars = this->InitialLevelSets->GetPointData()->GetScalars();
	levelSetsInputScalars->FillComponent(0, 1.0);
	int *dimensions = this->Image->GetDimensions();
	for (int i = 0; i < seedIds->GetNumberOfIds(); ++i)
		levelSetsInputScalars->SetComponent(seedIds->GetId(i), 0, -1.0);

	//vtkImageDilateErode3D将扩展一个值并侵蚀另一个值。 
	//它使用椭圆形足迹，仅在两个值的边界上进行侵蚀/扩张。
	vtkSmartPointer<vtkImageDilateErode3D> dilateErode = 
		vtkSmartPointer<vtkImageDilateErode3D>::New();
	dilateErode->SetInputData(this->InitialLevelSets);
	dilateErode->SetDilateValue(-1.0);
	dilateErode->SetErodeValue(1.0);
	dilateErode->SetKernelSize(3, 3, 3);
	dilateErode->Update();
	this->InitialLevelSets->DeepCopy(dilateErode->GetOutput());
	this->IsoSurfaceValue = 0.0;
}

//显示水平集表面
//void vtkvmtkImageInitialization::DisplayLevelSetSurface(vtkImageData *levelSets)
void vtkvmtkImageInitialization::DisplayLevelSetSurface(vtkSmartPointer<vtkImageData> levelSets)
{
	double value = 0.0;
	vtkSmartPointer<vtkMarchingCubes> marchingCubes =
		vtkSmartPointer<vtkMarchingCubes>::New();
	marchingCubes->SetInputData(levelSets);
	marchingCubes->SetValue(0, value);
	marchingCubes->Update();
	this->Surface = marchingCubes->GetOutput();

	std::cout << "Displaying.\n" << std::endl;
	this->SurfaceViewer->SetSurface(marchingCubes->GetOutput());
	this->SurfaceViewer->SetDisplay(false);
	this->SurfaceViewer->SetOpacity(0.5);
	//this->SurfaceViewer->BuildView();
	this->SurfaceViewer->Execute();
}

//合并水平集
void vtkvmtkImageInitialization::MergeLevelSets()
{
	static int times = 0;
	if (this->MergedInitialLevelSets == nullptr)
	{
		times++;
		std::cout << "this->MergedInitialLevelSets is nullptr : " << times << std::endl;
		this->MergedInitialLevelSets = vtkSmartPointer<vtkImageData>::New();
		this->MergedInitialLevelSets->DeepCopy(this->InitialLevelSets);
	}
	else
	{
		vtkSmartPointer<vtkImageMathematics> minFilter = 
			vtkSmartPointer<vtkImageMathematics>::New();
		minFilter->SetOperationToMin();
		minFilter->SetInput1Data(this->MergedInitialLevelSets);
		minFilter->SetInput2Data(this->InitialLevelSets);
		minFilter->Update();
		this->MergedInitialLevelSets = minFilter->GetOutput();
		std::cout << "Merge Points: " << this->MergedInitialLevelSets->GetNumberOfPoints() << std::endl;
	}
}

//执行的主体
void vtkvmtkImageInitialization::Execute()
{
	if (this->Image == nullptr)
		std::cout << "Error: no Image." << std::endl;

	vtkSmartPointer<vtkImageCast> cast = 
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(this->Image);
	cast->SetOutputScalarTypeToFloat();
	cast->Update();
	this->Image = cast->GetOutput();
	
	if (this->NegateImage)
	{
		double *scalarRange = this->Image->GetScalarRange();
		vtkSmartPointer<vtkImageMathematics> negate = 
			vtkSmartPointer<vtkImageMathematics>::New();
		negate->SetInputData(this->Image);
		negate->SetOperationToMultiplyByK();
		negate->SetConstantK(-1.0);
		negate->Update();
		vtkSmartPointer<vtkImageShiftScale> shiftScale = 
			vtkSmartPointer<vtkImageShiftScale>::New();
		shiftScale->SetInputConnection(negate->GetOutputPort());
		shiftScale->SetShift(scalarRange[1] + scalarRange[0]);
		shiftScale->SetOutputScalarTypeToFloat();
		shiftScale->Update();
		this->Image = shiftScale->GetOutput();
	}

	if (this->Interactive)
	{
		if (this->vmtkRenderer == nullptr)
		{
			this->vmtkRenderer = vtkvmtkRenderer::New();
			this->vmtkRenderer->Initialize();
			this->InnervmtkRenderer = true;
		}
		if (this->ImageSeeder == nullptr)
		{
			this->ImageSeeder = vtkSmartPointer<vtkvmtkImageSeeder>::New();
			this->ImageSeeder->SetRenderer(this->vmtkRenderer);
			this->ImageSeeder->SetImage(this->Image);
			this->ImageSeeder->SetDisplay(false);
			this->ImageSeeder->Execute();
			//this->ImageSeeder->SetDisplay(true);
			this->ImageSeeder->BuildView();
		}
		if (this->SurfaceViewer == nullptr)
		{
			this->SurfaceViewer = vtkSmartPointer<vtkvmtkSurfaceViewer>::New();
			this->SurfaceViewer->SetRenderer(this->vmtkRenderer);
		}
		bool endInitialization = false;
		while (!endInitialization)
		{
			//输入初始化方法
			const char *queryString = "Please choose initialization type: \n 0: colliding fronts;\n \
				1: fast marching;\n 2: threshold;\n 3: isosurface;\n 4: seed\n ";
			const char *initializationType = this->InputText(queryString, &vtkvmtkImageInitialization::InitializationTypeValidator);
			if (!strcmp(initializationType, "0"))
				this->CollidingFrontsInitialize();
			else if (!strcmp(initializationType, "1"))
				this->FastMarchingInitialize();
			else if (!strcmp(initializationType, "2"))
				this->ThresholdInitialize();
			else if (!strcmp(initializationType, "3"))
				this->IsosurfaceInitialize();
			else if (!strcmp(initializationType, "4"))
				this->SeedInitialize();
			else
			{
				//InitializationTypeValidator中已将0,1,2,3,4之外的情况排除
				std::cout << "initializationType: " << initializationType << std::endl;
				std::cout << "bug In vtkvmtkImageInitialization 611 line" << std::endl;
			}
			this->DisplayLevelSetSurface(this->InitialLevelSets);
			queryString = "Accept initialization? (y/n): ";
			const char* inputString = this->InputText(queryString, &vtkvmtkImageInitialization::YesNoValidator);
			if (!strcmp(inputString, "y"))
			{
				this->MergeLevelSets();
				this->DisplayLevelSetSurface(this->MergedInitialLevelSets);
			}
			queryString = "Initialize another branch? (y/n): ";
			inputString = this->InputText(queryString, &vtkvmtkImageInitialization::YesNoValidator);
			if (!strcmp(inputString, "y"))
				endInitialization = false;
			else if (!strcmp(inputString, "n"))
				endInitialization = true;
		}
		this->InitialLevelSets = this->MergedInitialLevelSets;
		this->MergedInitialLevelSets = nullptr;
	} //end if (this->Interactive)
	else  //非交互的情况下
	{
		if (!strcmp(this->Method, "collidingfronts"))
			this->CollidingFrontsInitialize();
		else if (!strcmp(this->Method, "fastmarching"))
			this->FastMarchingInitialize();
		else if (!strcmp(this->Method, "threshold"))
			this->ThresholdInitialize();
		else if (!strcmp(this->Method, "isosurface"))
			this->IsosurfaceInitialize();
		else if (!strcmp(this->Method, "seeds"))
			this->SeedInitialize();
		else
			std::cout << "bug" << std::endl;
	}

	std::cout << "this->InitialLevelSets->GetNumberOfPoints()" << std::endl;
	std::cout << this->InitialLevelSets->GetNumberOfPoints() << std::endl;
	std::cout << "----------------" << std::endl;
}