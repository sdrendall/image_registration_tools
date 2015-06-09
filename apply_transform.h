#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkCompositeTransform.h"

#include "optionparser.h"
#include "image_io.h"

#include <iostream>

#ifndef APPLY_TRANSFORM
#define APPLY_TRANSFORM

template<typename IMAGE_TYPE, typename TRANSFORM_TYPE>
typename IMAGE_TYPE::Pointer apply_transform(typename IMAGE_TYPE::Pointer image, typename TRANSFORM_TYPE::Pointer transform) {
    // Applys the transform to the image using bspline interpolation
    typedef itk::ResampleImageFilter<IMAGE_TYPE, IMAGE_TYPE> ImageResamplerType;
    typedef itk::BSplineInterpolateImageFunction<IMAGE_TYPE, double, double> InterpolatorType;

    typename ImageResamplerType::Pointer resampler = ImageResamplerType::New();
    resampler->SetTransform(transform);
    resampler->SetInput(image);

    // Configure the resampler to apply the BSpline Transform
    resampler->SetSize(image->GetLargestPossibleRegion().GetSize());
    resampler->SetOutputOrigin(image->GetOrigin());
    resampler->SetOutputSpacing(image->GetSpacing());
    resampler->SetOutputDirection(image->GetDirection());
    resampler->SetDefaultPixelValue(0);

    // Link the interpolator to the resamplers
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    resampler->SetInterpolator(interpolator);

    resampler->Update();
    return resampler->GetOutput();
}

#endif
