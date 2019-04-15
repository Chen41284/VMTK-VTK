#include "vtkvmtkImageFeature.h"

//VTK-Include
#include <vtkImageCast.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkImageMathematics.h>
#include <vtkPointData.h>


//VMTK-Include
#include <vtkvmtkFWHMFeatureImageFilter.h>
#include <vtkvmtkUpwindGradientMagnitudeImageFilter.h>
#include <vtkvmtkBoundedReciprocalImageFilter.h>
#include <vtkvmtkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <vtkvmtkGradientMagnitudeImageFilter.h>
#include <vtkvmtkSigmoidImageFilter.h>

//STD

vtkStandardNewMacro(vtkvmtkImageFeature);

vtkvmtkImageFeature::vtkvmtkImageFeature()
{
	this->Image = nullptr;
	this->FeatureImage = vtkImageData::New();
	this->Dimensionality = 3;
	this->DerivativeSigma = 0.0;
	this->SigmoidRemapping = false;
	this->SetFeatureImageType("gradient");
	this->UpwindFactor = 1.0;
	this->FWHMRadius[0] = 3; this->FWHMRadius[1] = 3; this->FWHMRadius[2] = 3;
	this->FWHMBackgroundValue = 0.0;
}

vtkvmtkImageFeature::~vtkvmtkImageFeature()
{
	if (this->FeatureImage)
	{
		this->FeatureImage->Delete();
		this->FeatureImage = nullptr;
	}
	if (this->FeatureImageType)
	{
		delete[] this->FeatureImageType;
		this->FeatureImageType = nullptr;
	}
}

void vtkvmtkImageFeature::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
	os << indent << "vtk ITK FeatureImage\n";
}

void vtkvmtkImageFeature::BuildVTKGradientBasedFeatureImage()
{
	vtkSmartPointer<vtkImageCast> cast =
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(this->Image);
	cast->SetOutputScalarTypeToFloat();
	cast->Update();

	vtkSmartPointer<vtkImageGradientMagnitude> gradientMagnitude =
		vtkSmartPointer<vtkImageGradientMagnitude>::New();
	gradientMagnitude->SetInputConnection(cast->GetOutputPort());
	gradientMagnitude->SetDimensionality(this->Dimensionality);
	gradientMagnitude->Update();

	vtkSmartPointer<vtkImageMathematics> imageAdd = 
		vtkSmartPointer<vtkImageMathematics>::New();
	imageAdd->SetInputConnection(gradientMagnitude->GetOutputPort());
	imageAdd->SetOperationToAddConstant();
	imageAdd->SetConstantC(1.0);
	imageAdd->Update();

	vtkSmartPointer<vtkImageMathematics> imageInvert = 
		vtkSmartPointer<vtkImageMathematics>::New();
	imageInvert->SetInputConnection(imageAdd->GetOutputPort());
	imageInvert->SetOperationToInvert();
	imageInvert->SetConstantC(1e20);
	imageInvert->DivideByZeroToCOn();
	imageInvert->Update();

	this->FeatureImage->DeepCopy(imageInvert->GetOutput());
}

void vtkvmtkImageFeature::BuildFWHMBasedFeatureImage()
{
	vtkSmartPointer<vtkImageCast> cast =
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(this->Image);
	cast->SetOutputScalarTypeToFloat();
	cast->Update();

	vtkSmartPointer<vtkvmtkFWHMFeatureImageFilter> fwhmFeatureImageFilter = 
		vtkSmartPointer<vtkvmtkFWHMFeatureImageFilter>::New();
	fwhmFeatureImageFilter->SetInputConnection(cast->GetOutputPort());
	fwhmFeatureImageFilter->SetRadius(this->FWHMRadius);
	fwhmFeatureImageFilter->SetBackgroundValue(this->FWHMBackgroundValue);
	fwhmFeatureImageFilter->Update();

	this->FeatureImage->DeepCopy(fwhmFeatureImageFilter->GetOutput());
}

void vtkvmtkImageFeature::BuildUpwindGradientBasedFeatureImage()
{
	vtkSmartPointer<vtkImageCast> cast =
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(this->Image);
	cast->SetOutputScalarTypeToFloat();
	cast->Update();

	vtkSmartPointer<vtkvmtkUpwindGradientMagnitudeImageFilter> gradientMagnitude = 
		vtkSmartPointer<vtkvmtkUpwindGradientMagnitudeImageFilter>::New();
	gradientMagnitude->SetInputConnection(cast->GetOutputPort());
	gradientMagnitude->SetUpwindFactor(this->UpwindFactor);
	gradientMagnitude->Update();

	if (this->SigmoidRemapping == true)
	{
		double *scalarRange = gradientMagnitude->GetOutput()->GetPointData()->GetScalars()->GetRange();
		double inputMinimum = scalarRange[0];
		double inputMaximum = scalarRange[1];
		double alpha = -(inputMaximum - inputMinimum) / 6.0;
		double beta = (inputMaximum + inputMinimum) / 2.0;
		vtkSmartPointer<vtkvmtkSigmoidImageFilter> sigmoid = 
			vtkSmartPointer<vtkvmtkSigmoidImageFilter>::New();
		sigmoid->SetInputConnection(gradientMagnitude->GetOutputPort());
		sigmoid->SetAlpha(alpha);
		sigmoid->SetBeta(beta);
		sigmoid->SetOutputMinimum(0.0);
		sigmoid->SetOutputMaximum(1.0);
		sigmoid->Update();
		this->FeatureImage->DeepCopy(sigmoid->GetOutput());
	}
	else
	{
		vtkSmartPointer<vtkvmtkBoundedReciprocalImageFilter> boundedReciprocal = 
			vtkSmartPointer<vtkvmtkBoundedReciprocalImageFilter>::New();
		boundedReciprocal->SetInputConnection(gradientMagnitude->GetOutputPort());
		boundedReciprocal->Update();
		this->FeatureImage->DeepCopy(boundedReciprocal->GetOutput());
	}
}

void vtkvmtkImageFeature::BuildGradientBasedFeatureImage()
{
	vtkSmartPointer<vtkImageCast> cast =
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(this->Image);
	cast->SetOutputScalarTypeToFloat();
	cast->Update();

	double scalarRange[2];
	vtkImageData *gradientImageData = vtkImageData::New();
	if (this->DerivativeSigma > 0.0)
	{
		vtkSmartPointer<vtkvmtkGradientMagnitudeRecursiveGaussianImageFilter> gradientMagnitude = 
			vtkSmartPointer<vtkvmtkGradientMagnitudeRecursiveGaussianImageFilter>::New();
		gradientMagnitude->SetInputConnection(cast->GetOutputPort());
		gradientMagnitude->SetSigma(this->DerivativeSigma);
		gradientMagnitude->SetNormalizeAcrossScale(0);
		gradientMagnitude->Update();
		gradientImageData->DeepCopy(gradientMagnitude->GetOutput());
		double *range  = gradientMagnitude->GetOutput()->GetPointData()->GetScalars()->GetRange();
		scalarRange[0] = range[0]; scalarRange[1] = range[1];
	}
	else
	{
		vtkSmartPointer<vtkvmtkGradientMagnitudeImageFilter> gradientMagnitude = 
			vtkvmtkGradientMagnitudeImageFilter::New();
		gradientMagnitude->SetInputConnection(cast->GetOutputPort());
		gradientMagnitude->Update();
		gradientImageData->DeepCopy(gradientMagnitude->GetOutput());
		double *range = gradientMagnitude->GetOutput()->GetPointData()->GetScalars()->GetRange();
		scalarRange[0] = range[0]; scalarRange[1] = range[1];
	}
	
	if (this->SigmoidRemapping == true)
	{
		double inputMinimum = scalarRange[0];
		double inputMaximum = scalarRange[1];
		double alpha = -(inputMaximum - inputMinimum) / 6.0;
		double beta = (inputMaximum + inputMinimum) / 2.0;
		vtkSmartPointer<vtkvmtkSigmoidImageFilter> sigmoid = 
			vtkSmartPointer<vtkvmtkSigmoidImageFilter>::New();
		sigmoid->SetInputData(gradientImageData);
		sigmoid->SetAlpha(alpha);
		sigmoid->SetBeta(beta);
		sigmoid->SetOutputMinimum(0.0);
		sigmoid->SetOutputMaximum(1.0);
		sigmoid->Update();
		this->FeatureImage->DeepCopy(sigmoid->GetOutput());
	}
	else
	{
		vtkSmartPointer<vtkvmtkBoundedReciprocalImageFilter> boundedReciprocal = 
			vtkSmartPointer<vtkvmtkBoundedReciprocalImageFilter>::New();
		boundedReciprocal->SetInputData(gradientImageData);
		boundedReciprocal->Update();
		this->FeatureImage->DeepCopy(boundedReciprocal->GetOutput());
	}
}

void vtkvmtkImageFeature::Execute()
{
	if (this->Image == nullptr)
		std::cout << "Error: No input image." << std::endl;

	if (!strcmp(this->FeatureImageType, "vtkgradient"))
		this->BuildVTKGradientBasedFeatureImage();
	else if (!strcmp(this->FeatureImageType, "gradient"))
		this->BuildGradientBasedFeatureImage();
	else if (!strcmp(this->FeatureImageType, "upwind"))
		this->BuildUpwindGradientBasedFeatureImage();
	else if (!strcmp(this->FeatureImageType, "fwhm"))
		this->BuildFWHMBasedFeatureImage();
	else
		std::cout << "Error: unsupported feature image type" << std::endl;
}