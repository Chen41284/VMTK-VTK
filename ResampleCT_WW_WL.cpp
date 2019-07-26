#include "ResampleCT_WW_WL.h"

//VTK
#include "vtkVersion.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkImageActor.h"
#include "vtkImageMapper3D.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkJPEGWriter.h"
#include "vtkImageCast.h"
#include <vtkSliderWidget.h>
#include <vtkCommand.h>
#include <vtkSliderRepresentation.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkSphereSource.h>

#include <cstdlib>
#include <iostream>

#include "itkImageToVTKImageFilter.h"

class WWvtkSliderCallback : public vtkCommand
{
public:
	typedef itk::Image<float, 2>  ImageType;
	typedef itk::IntensityWindowingImageFilter<
		ImageType,
		ImageType >  IntensityFilterType;
	typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
	IntensityFilterType::Pointer intensityWindowing;
	ConnectorType::Pointer rescaledConnector;
	vtkSmartPointer<vtkRenderWindow> renderWindow;
	vtkSmartPointer<vtkImageActor> actor;
	static WWvtkSliderCallback *New()
	{
		return new WWvtkSliderCallback;
	}
	virtual void Execute(vtkObject *caller, unsigned long, void*)
	{
		vtkSliderWidget *sliderWidget =
			reinterpret_cast<vtkSliderWidget*>(caller);
		int windowWidth = static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
		int level = intensityWindowing->GetLevel();
		intensityWindowing->SetWindowLevel(windowWidth, level);
		intensityWindowing->Modified();
		std::cout << "窗宽: " << windowWidth << "  窗位: " << level << std::endl;
		rescaledConnector->Update();
		actor->Update();
		renderWindow->Render();
	}
	WWvtkSliderCallback() :intensityWindowing(0), rescaledConnector(0), renderWindow(0), actor(0) {}
};

class WLvtkSliderCallback : public vtkCommand
{
public:
	typedef itk::Image<float, 2>  ImageType;
	typedef itk::IntensityWindowingImageFilter<
		ImageType,
		ImageType >  IntensityFilterType;
	typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
	IntensityFilterType::Pointer intensityWindowing;
	ConnectorType::Pointer rescaledConnector;
	vtkSmartPointer<vtkRenderWindow> renderWindow;
	vtkSmartPointer<vtkImageActor> actor;
	static WLvtkSliderCallback *New()
	{
		return new WLvtkSliderCallback;
	}
	virtual void Execute(vtkObject *caller, unsigned long, void*)
	{
		vtkSliderWidget *sliderWidget =
			reinterpret_cast<vtkSliderWidget*>(caller);
		int level = static_cast<vtkSliderRepresentation *>(sliderWidget->GetRepresentation())->GetValue();
		int windowWidth = intensityWindowing->GetWindow();
		intensityWindowing->SetWindowLevel(windowWidth, level);
		intensityWindowing->Modified();
		std::cout << "窗宽: " << windowWidth << "  窗位: " << level << std::endl;
		rescaledConnector->Update();
		actor->Update();
		renderWindow->Render();
	}
	WLvtkSliderCallback() :intensityWindowing(0), rescaledConnector(0), renderWindow(0), actor(0){}
};

void ResampleCT_WW_WL(const char* InputFileName, const char* OutputFileName, int windowWidth, int windowLevel)
{
	itk::MetaImageIOFactory::RegisterOneFactory();
	itk::GDCMImageIOFactory::RegisterOneFactory();
	itk::JPEGImageIOFactory::RegisterOneFactory();
	itk::PNGImageIOFactory::RegisterOneFactory();
	//计算阈值的范围
	int lower = windowLevel - windowWidth / 2;
	int upper = windowLevel + windowWidth / 2;
	//我们现在明确选择要处理的输入图像的像素类型和尺寸，
    //以及我们打算在平滑和重采样期间用于内部计算的像素类型。
	const     unsigned int    InDimension = 3;

	typedef   unsigned short  InputPixelType;
	typedef   float           InternalPixelType;

	typedef itk::Image< InputPixelType, InDimension >   InputImageType;
	typedef itk::Image< InternalPixelType, InDimension >   InternalImageType;
	
	typedef itk::ImageFileReader< InputImageType  >  ReaderType;

	ReaderType::Pointer reader = ReaderType::New();

	reader->SetFileName(InputFileName);

	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught!" << std::endl;
		std::cerr << excep << std::endl;
	}

	typedef itk::IntensityWindowingImageFilter<
		InputImageType,
		InternalImageType >  IntensityFilterType;

	IntensityFilterType::Pointer intensityWindowing = IntensityFilterType::New();

	//调窗直接设置窗宽与窗位
	//intensityWindowing->SetWindowLevel(180, 90);

	intensityWindowing->SetWindowMinimum(lower);
	intensityWindowing->SetWindowMaximum(upper);

	intensityWindowing->SetOutputMinimum(0.0);
	intensityWindowing->SetOutputMaximum(255.0); // floats but in the range of chars.

	intensityWindowing->SetInput(reader->GetOutput());

	typedef itk::RecursiveGaussianImageFilter<
		InternalImageType,
		InternalImageType > GaussianFilterType;
	
	GaussianFilterType::Pointer smootherX = GaussianFilterType::New();
	GaussianFilterType::Pointer smootherY = GaussianFilterType::New();

	smootherX->SetInput(intensityWindowing->GetOutput());
	smootherY->SetInput(smootherX->GetOutput());
	
	InputImageType::ConstPointer inputImage = reader->GetOutput();

	const InputImageType::SpacingType& inputSpacing = inputImage->GetSpacing();

	const double isoSpacing = std::sqrt(inputSpacing[2] * inputSpacing[0]);

	smootherX->SetSigma(isoSpacing);
	smootherY->SetSigma(isoSpacing);

	smootherX->SetDirection(0);
	smootherY->SetDirection(1);
	
	typedef   unsigned char   OutputPixelType;
	const unsigned int OutDimension = 3;

	typedef itk::Image< OutputPixelType, OutDimension >   OutputImageType;

	typedef itk::ResampleImageFilter<
		InternalImageType, OutputImageType >  ResampleFilterType;

	ResampleFilterType::Pointer resampler = ResampleFilterType::New();
	
	typedef itk::IdentityTransform< double, OutDimension >  TransformType;

	TransformType::Pointer transform = TransformType::New();
	transform->SetIdentity();

	resampler->SetTransform(transform);
	
	typedef itk::LinearInterpolateImageFunction<
		InternalImageType, double >  InterpolatorType;

	InterpolatorType::Pointer interpolator = InterpolatorType::New();

	resampler->SetInterpolator(interpolator);
	
	resampler->SetDefaultPixelValue(255); // highlight regions without source

	OutputImageType::SpacingType spacing;

	spacing[0] = isoSpacing;
	spacing[1] = isoSpacing;
	spacing[2] = isoSpacing;


	resampler->SetOutputSpacing(spacing);
	resampler->SetOutputOrigin(inputImage->GetOrigin());
	resampler->SetOutputDirection(inputImage->GetDirection());
	InputImageType::SizeType   inputSize =
		inputImage->GetLargestPossibleRegion().GetSize();

	typedef InputImageType::SizeType::SizeValueType SizeValueType;

	const double dx = inputSize[0] * inputSpacing[0] / isoSpacing;
	const double dy = inputSize[1] * inputSpacing[1] / isoSpacing;
	const double dz = (inputSize[2] - 1) * inputSpacing[2] / isoSpacing;
	InputImageType::SizeType   size;

	size[0] = static_cast<SizeValueType>(dx);
	size[1] = static_cast<SizeValueType>(dy);
	size[2] = static_cast<SizeValueType>(dz);

	resampler->SetSize(size);
	resampler->SetInput(smootherY->GetOutput());
	resampler->Update();

	typedef itk::ImageFileWriter< OutputImageType >  WriterType;

	WriterType::Pointer writer = WriterType::New();

	writer->SetFileName(OutputFileName);
	writer->SetInput(resampler->GetOutput());

	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
	}
}

void WriteImage(const char* FileName)
{
	itk::BMPImageIOFactory::RegisterOneFactory();
	itk::MetaImageIOFactory::RegisterOneFactory();
	itk::GDCMImageIOFactory::RegisterOneFactory();
	itk::JPEGImageIOFactory::RegisterOneFactory();
	itk::PNGImageIOFactory::RegisterOneFactory();
	typedef unsigned char     PixelType;
	const     unsigned int    Dimension = 2;
	typedef itk::Image< PixelType, Dimension >  ImageType;

	ImageType::RegionType region;
	ImageType::IndexType start;
	start[0] = 0;
	start[1] = 0;

	ImageType::SizeType size;
	size[0] = 200;
	size[1] = 300;

	region.SetSize(size);
	region.SetIndex(start);

	ImageType::Pointer image = ImageType::New();
	image->SetRegions(region);
	image->Allocate();

	ImageType::IndexType ind;
	ind[0] = 10;
	ind[1] = 10;

	typedef  itk::ImageFileWriter< ImageType  > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName(FileName);
	writer->SetInput(image);
	writer->Update();

}

int RescaleIntensityImageFilter(const char* InputFileName, 
	const char* OutputFileName)
{
	itk::BMPImageIOFactory::RegisterOneFactory();
	itk::MetaImageIOFactory::RegisterOneFactory();
	itk::GDCMImageIOFactory::RegisterOneFactory();
	typedef itk::Image<float, 2>  ImageType;

	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(InputFileName);

	typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
	ConnectorType::Pointer originalConnector = ConnectorType::New();
	originalConnector->SetInput(reader->GetOutput());
	vtkSmartPointer<vtkImageActor> originalActor =
		vtkSmartPointer<vtkImageActor>::New();
#if VTK_MAJOR_VERSION <= 5
	originalActor->SetInput(originalConnector->GetOutput());
#else
	originalConnector->Update();
	originalActor->GetMapper()->SetInputData(originalConnector->GetOutput());
#endif

	//typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescaleFilterType;
	//RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
	//rescaleFilter->SetInput(reader->GetOutput());
	//rescaleFilter->SetOutputMinimum(0);
	//rescaleFilter->SetOutputMaximum(255);

	typedef itk::IntensityWindowingImageFilter<
		ImageType,
		ImageType >  IntensityFilterType;

	IntensityFilterType::Pointer intensityWindowing = IntensityFilterType::New();

	intensityWindowing->SetWindowLevel(800, 800);

	intensityWindowing->SetOutputMinimum(0.0);
	intensityWindowing->SetOutputMaximum(255.0);

	intensityWindowing->SetInput(reader->GetOutput());

	ConnectorType::Pointer rescaledConnector = ConnectorType::New();
	//rescaledConnector->SetInput(rescaleFilter->GetOutput());
	rescaledConnector->SetInput(intensityWindowing->GetOutput());

	vtkSmartPointer<vtkImageActor> rescaledActor =
		vtkSmartPointer<vtkImageActor>::New();
#if VTK_MAJOR_VERSION <= 5
	rescaledActor->SetInput(rescaledConnector->GetOutput());
#else
	rescaledConnector->Update();
	rescaledActor->GetMapper()->SetInputData(rescaledConnector->GetOutput());
#endif

	// There will be one render window
	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->SetSize(900, 300);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	// Define viewport ranges
	// (xmin, ymin, xmax, ymax)
	double leftViewport[4] = { 0.0, 0.0, 0.5, 1.0 };
	double rightViewport[4] = { 0.5, 0.0, 1.0, 1.0 };

	// Setup both renderers
	vtkSmartPointer<vtkRenderer> leftRenderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderWindow->AddRenderer(leftRenderer);
	leftRenderer->SetViewport(leftViewport);
	leftRenderer->SetBackground(.6, .5, .4);

	vtkSmartPointer<vtkRenderer> rightRenderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderWindow->AddRenderer(rightRenderer);
	rightRenderer->SetViewport(rightViewport);
	rightRenderer->SetBackground(.4, .5, .6);

	// Add the sphere to the left and the cube to the right
	leftRenderer->AddActor(originalActor);
	rightRenderer->AddActor(rescaledActor);

	vtkSmartPointer<vtkCamera> camera =
		vtkSmartPointer<vtkCamera>::New();
	leftRenderer->SetActiveCamera(camera);
	rightRenderer->SetActiveCamera(camera);

	leftRenderer->ResetCamera();

	renderWindow->Render();


	//添加调节窗位的滑条
	vtkSmartPointer<vtkSliderRepresentation2D> sliderRepWL =
		vtkSmartPointer<vtkSliderRepresentation2D>::New();

	sliderRepWL->SetMinimumValue(-2048);
	sliderRepWL->SetMaximumValue(4096);
	sliderRepWL->SetValue(40);
	sliderRepWL->SetTitleText("Window level");
	sliderRepWL->GetSliderProperty()->SetColor(1, 0, 0);//red
	sliderRepWL->GetTitleProperty()->SetColor(1, 0, 0);//red
	sliderRepWL->GetLabelProperty()->SetColor(1, 0, 0);//red
	sliderRepWL->GetSelectedProperty()->SetColor(0, 1, 0);//green
	sliderRepWL->GetTubeProperty()->SetColor(1, 1, 0);//yellow
	sliderRepWL->GetCapProperty()->SetColor(1, 1, 0);//yellow

	//sliderRepWL->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
	//sliderRepWL->GetPoint1Coordinate()->SetValue(40, 80);
	//sliderRepWL->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
	//sliderRepWL->GetPoint2Coordinate()->SetValue(200, 80);
	sliderRepWL->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
	sliderRepWL->GetPoint1Coordinate()->SetValue(0.6, 0.1);
	sliderRepWL->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
	sliderRepWL->GetPoint2Coordinate()->SetValue(0.8, 0.1);

	vtkSmartPointer<vtkSliderWidget> sliderWidgetWL =
		vtkSmartPointer<vtkSliderWidget>::New();
	sliderWidgetWL->SetInteractor(interactor);
	sliderWidgetWL->SetRepresentation(sliderRepWL);
	sliderWidgetWL->SetAnimationModeToAnimate();
	sliderWidgetWL->EnabledOn();

	vtkSmartPointer<WLvtkSliderCallback> callbackWL =
		vtkSmartPointer<WLvtkSliderCallback>::New();
	callbackWL->intensityWindowing = intensityWindowing;
	callbackWL->rescaledConnector = rescaledConnector;
	callbackWL->renderWindow = renderWindow;
	callbackWL->actor = rescaledActor;

	sliderWidgetWL->AddObserver(vtkCommand::InteractionEvent, callbackWL);

	//添加调节窗宽的滑条
	vtkSmartPointer<vtkSliderRepresentation2D> sliderRepWW =
		vtkSmartPointer<vtkSliderRepresentation2D>::New();

	sliderRepWW->SetMinimumValue(1);
	sliderRepWW->SetMaximumValue(10000);
	sliderRepWW->SetValue(800);
	sliderRepWW->SetTitleText("Window width");
	sliderRepWW->GetSliderProperty()->SetColor(1, 0, 0);//red
	sliderRepWW->GetTitleProperty()->SetColor(1, 0, 0);//red
	sliderRepWW->GetLabelProperty()->SetColor(1, 0, 0);//red
	sliderRepWW->GetSelectedProperty()->SetColor(0, 1, 0);//green
	sliderRepWW->GetTubeProperty()->SetColor(1, 1, 0);//yellow
	sliderRepWW->GetCapProperty()->SetColor(1, 1, 0);//yellow

	//sliderRepWW->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
	//sliderRepWW->GetPoint1Coordinate()->SetValue(40, 80);
	//sliderRepWW->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
	//sliderRepWW->GetPoint2Coordinate()->SetValue(200, 80);
	sliderRepWW->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedViewport();
	sliderRepWW->GetPoint1Coordinate()->SetValue(0.1, 0.1);
	sliderRepWW->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedViewport();
	sliderRepWW->GetPoint2Coordinate()->SetValue(0.3, 0.1);

	vtkSmartPointer<vtkSliderWidget> sliderWidgetWW =
		vtkSmartPointer<vtkSliderWidget>::New();
	sliderWidgetWW->SetInteractor(interactor);
	sliderWidgetWW->SetRepresentation(sliderRepWW);
	sliderWidgetWW->SetAnimationModeToAnimate();
	sliderWidgetWW->EnabledOn();

	vtkSmartPointer<WWvtkSliderCallback> callbackWW =
		vtkSmartPointer<WWvtkSliderCallback>::New();
	callbackWW->intensityWindowing = intensityWindowing;
	callbackWW->rescaledConnector = rescaledConnector;
	callbackWW->renderWindow = renderWindow;
	callbackWW->actor = rescaledActor;

	sliderWidgetWW->AddObserver(vtkCommand::InteractionEvent, callbackWW);


	vtkSmartPointer<vtkInteractorStyleImage> style =
		vtkSmartPointer<vtkInteractorStyleImage>::New();

	interactor->SetInteractorStyle(style);

	interactor->Start();

	std::cout << "修改后的结果 " << std::endl;
	std::cout << intensityWindowing->GetWindow() << " " << intensityWindowing->GetLevel() << std::endl;

	//输出JPEG图像
	vtkSmartPointer<vtkImageCast> cast =
		vtkSmartPointer<vtkImageCast>::New();
	cast->SetInputData(rescaledConnector->GetOutput());
	cast->SetOutputScalarTypeToUnsignedChar();
	cast->Update();
	vtkSmartPointer<vtkJPEGWriter> writer = 
		vtkSmartPointer<vtkJPEGWriter>::New();
	writer->SetFileName(OutputFileName);
	writer->SetInputData(cast->GetOutput());
	writer->Write();

	return EXIT_SUCCESS;
}