#ifndef ITK_Fuction_CT_H
#define ITK_Fuction_CT_H

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkMetaImageIOFactory.h"
#include "itkPNGImageIOFactory.h"
#include <itkGDCMImageIOFactory.h>
#include <itkJPEGImageIOFactory.h>
#include <itkBMPImageIOFactory.h>
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"


//调成CT的窗宽与窗位
void ResampleCT_WW_WL(const char* InputFileName, const char* OutputFileName, int windowWidth, int windowLevel);
void WriteImage(const char *FileName);
int RescaleIntensityImageFilter(const char* InputFileName, const char* OutputFileName);

#endif

