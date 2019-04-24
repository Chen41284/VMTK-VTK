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
	/*if (this->InitialLevelSets)  //����ָ�벻���ֶ��ͷ�
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

////���ӵ������
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
				continue;   //����ѭ�����룬ָ��Ӧ��������ӵ����ﵽҪ��
			}
	}

	//vtkPolyData *seeds = vtkPolyData::New();
	vtkSmartPointer<vtkPolyData> seeds = vtkSmartPointer<vtkPolyData>::New();
	seeds->DeepCopy(this->ImageSeeder->GetSeeds());

	return seeds;
}

//��֤������ַ�������Ч��
int vtkvmtkImageInitialization::ThresholdValidator(const char* text)
{
	if (!strcmp(text, "n"))  //������
		return 1;
	if (!strcmp(text, "i")) //����
	{
		this->vmtkRenderer->Render();
		return 0;
	}
	if (abs(atof(text) - 0.0) < 1e-06) //atof����0.0����������ַ�����Ч
	{
		std::cout << "Input text is error!" << std::endl;
		return 0;
	}
	return 1;
}

//������ֵ
double vtkvmtkImageInitialization::ThresholdInput(const char* queryString)
{
	//�������еĳ���Ա������������ǰ׺
	const char* thresholdString = this->InputText(queryString, &vtkvmtkImageInitialization::ThresholdValidator);

	double threshold = 0.0;
	if (strcmp(thresholdString, "n")) //not equal != 
		threshold = atof(thresholdString);

	return threshold;
}

//pypescript�еĵ��ñ�׼�������������ݲ���֤��Ч��
//�����Ϊ����KeyPressInteractorStyle�ļ�����Ӧ��ȡ���벢��֤�����������Ч��
//validatorΪ��֤�����������Ч�Եĺ�������Ч����1����Ч����0
const char* vtkvmtkImageInitialization::InputText(const char*queryString,
	int(vtkvmtkImageInitialization::*validator)(const char* text))
{
	//��̬����vmtkRenderer�е�Keypress�Ľ������ȡ�û��Ӽ������������
	const char* InputString = this->vmtkRenderer->PromptAsync(queryString);
	std::cout << "InputString  " << InputString << std::endl;
	std::cout << "(this->*validator)(InputString)  " << (this->*validator)(InputString) << std::endl;
	//����������֤�����ǿգ���ʹ�ô˺�����֤������ַ�������Ч��
	int count = 0;
	if (validator != nullptr)
	{
		while ((this->*validator)(InputString) == 0) //������ַ���Ч����������
		{
			++count;
			std::cout << "Error Input times: " << count << std::endl;
			InputString = this->vmtkRenderer->PromptAsync(queryString);
		}			
	}
	return InputString;
}

//��֤������ַ����Ƿ���"Y"��"N"
int vtkvmtkImageInitialization::YesNoValidator(const char* text)
{
	if (!strcmp(text, "n") || !strcmp(text, "y"))
		return 1;
	return 0;
}

//��֤������ַ����Ƿ���{0, 1, 2, 3, 4}�е�һ��
int vtkvmtkImageInitialization::InitializationTypeValidator(const char* text)
{
	std::string numberString = "01234";
	if (strlen(text) < 1)  //���ַ�
		return 0;
	if (strlen(text) > 1) //����һ�����֣�����һ�����ֵĿ϶�������
		return 0;

	std::size_t found = numberString.find(text);
	if (found != std::string::npos) //�ҵ��Ļ�
		return 1;

	return 0; //������� 5��6��7��
}

//��ֵ��ʼ��
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
	this->IsoSurfaceValue = 0.0;   //����
}

//������ֵ�ĳ�ʼ��
void vtkvmtkImageInitialization::ThresholdInitialize()
{
	std::cout << "Threshold initialization." << std::endl;

	if (this->Interactive)
	{
		const char *queryString = "Please input lower threshold (\'n\' for none): ";
		double LowerThreshold = this->ThresholdInput(queryString);
		this->SetLowThreshold(LowerThreshold);  //Ҫ����new double

		queryString = "Please input upper threshold (\'n\' for none): ";
		double UpperThreshold = this->ThresholdInput(queryString);
		this->SetUpperThreshold(UpperThreshold);
	}

	double *scalarRange = this->Image->GetScalarRange();
	//thresholdedImage = self.Image;
	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->IsoSurfaceValue = 0.0;
}

//���ٵ����
void vtkvmtkImageInitialization::FastMarchingInitialize()
{
	std::cout << "Fast marching initialization." << std::endl;

	vtkSmartPointer<vtkIdList> sourceSeedIds = 
		vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> targetSeedIds = 
		vtkSmartPointer<vtkIdList>::New();

	//����ʽ�������ӵ�
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

		//�ͷ��ڴ�
		//sourceSeeds->Delete();
		//targetSeeds->Delete();
	} //end if
	else //���û�ָ������
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
	bool flagThreshold = false; //�ж��Ƿ��������ֵ��
	//��������������ֵ
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
		flagThreshold = true;  //�Ѿ���ֵ��
	}

	double scale = 1.0;
	if (scalarRange[1] - scalarRange[0] > 0.0)
		scale = 1.0 / (scalarRange[1] - scalarRange[0]);

	//ƫ�Ƴ߶�
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

//��ײǰ�߳�ʼ��
void vtkvmtkImageInitialization::CollidingFrontsInitialize()
{
	std::cout << "Colliding fronts initialization." << std::endl;
	showMemoryInfo();
	vtkSmartPointer<vtkIdList> seedIds1 = 
		vtkSmartPointer<vtkIdList>::New();
	vtkSmartPointer<vtkIdList> seedIds2 = 
		vtkSmartPointer<vtkIdList>::New();
	//����ʽ�������ӵ�
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
		
		//�ͷ��ڴ�
		//seeds->Delete();
	} //end if
	else //���û�ָ������
	{
		int SourcePointsIJK[3] = { this->SourcePoints[0], this->SourcePoints[1], this->SourcePoints[2] };
		seedIds1->InsertNextId(this->Image->ComputePointId(SourcePointsIJK));
		int TargetPointsIJK[3] = { this->TargetPoints[0], this->TargetPoints[1], this->TargetPoints[2] };
		seedIds2->InsertNextId(this->Image->ComputePointId(TargetPointsIJK));
	}

	double *scalarRange = this->Image->GetScalarRange();
	vtkSmartPointer<vtkImageData> thresholdedImage =
		vtkSmartPointer<vtkImageData>::New();
	bool flagThreshold = false; //�ж��Ƿ��������ֵ��
	//��������������ֵ
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
		flagThreshold = true;  //�Ѿ���ֵ��
	}

	double scale = 1.0;
	if (scalarRange[1] - scalarRange[0] > 0.0)
		scale = 1.0 / (scalarRange[1] - scalarRange[0]);

	//ƫ�Ƴ߶�
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

	//���������ڴ�Ĳ�������ҳ�ļ��ڴ棨�����ڴ棩������
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

	//ʹ��DeepCopy�����ڽ����ú������ú󣬽��������ڵ�����ڴ����
	this->InitialLevelSets = vtkSmartPointer<vtkImageData>::New();
	this->InitialLevelSets->DeepCopy(subtract->GetOutput());
	this->IsoSurfaceValue = 0.0;

}

//���ӵ�ĳ�ʼ��
void vtkvmtkImageInitialization::SeedInitialize()
{
	std::cout << "Seed initialization." << std::endl;
	vtkSmartPointer<vtkIdList> seedIds = 
		vtkSmartPointer<vtkIdList>::New();
	if (this->Interactive)  //����ʽ����
	{
		const char* queryString = "Please place seeds";
		//vtkPolyData *seeds = this->SeedInput(queryString, 0);
		vtkSmartPointer<vtkPolyData> seeds = this->SeedInput(queryString, 0);
		for (int i = 0; i < seeds->GetNumberOfPoints(); ++i)
		{
			seedIds->InsertNextId(this->Image->FindPoint(seeds->GetPoint(i)));
		}		
	}
	else   //���û�ָ��
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

	//vtkImageDilateErode3D����չһ��ֵ����ʴ��һ��ֵ�� 
	//��ʹ����Բ���㼣����������ֵ�ı߽��Ͻ�����ʴ/���š�
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

//��ʾˮƽ������
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

//�ϲ�ˮƽ��
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

//ִ�е�����
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
			//�����ʼ������
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
				//InitializationTypeValidator���ѽ�0,1,2,3,4֮�������ų�
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
	else  //�ǽ����������
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