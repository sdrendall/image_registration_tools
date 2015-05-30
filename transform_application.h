#include "itkBSplineInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"


#ifndef TRANSFORM_APPLICATION
#define TRANSFORM_APPLICATION

template<typename IMAGE_TYPE, typename TRANSFORM_TYPE>
typename IMAGE_TYPE::Pointer apply_transform(typename IMAGE_TYPE::Pointer input_image, TRANSFORM_TYPE transform) {
    // Applys a transform object to the given image using bspline interpolation and returns the result

    typedef itk::BSplineInterpolateImageFunction<IMAGE_TYPE, double, double> InterpolatorType;
    typedef itk::ResampleImageFilter<IMAGE_TYPE, IMAGE_TYPE> ImageResamplerType;

    typename ImageResamplerType::Pointer resampler = ImageResamplerType::New();
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

    resampler->SetTransform(transform);
    resampler->SetInput(input_image);
    resampler->SetSize(input_image->GetLargestPossibleRegion().GetSize());
    resampler->SetOutputOrigin(input_image->GetOrigin());
    resampler->SetOutputSpacing(input_image->GetSpacing());
    resampler->SetOutputDirection(input_image->GetDirection());
    resampler->SetDefaultPixelValue(0);

    resampler->Update();
    return resampler->GetOutput();
}

#endif
