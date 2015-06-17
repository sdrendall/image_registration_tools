#ifndef IMAGE_TO_IMAGE_REGISTRATION
#define IMAGE_TO_IMAGE_REGISTRATION

#include <iostream>
#include <vector>
#include <string>
#include "optionparser.h"

#include "itkImage.h"
#include "itkMultiThreader.h"

#include "itkImageRegistrationMethodv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkMattesMutualInformationImageToImageMetricv4.h"
#include "itkLBFGSBOptimizerv4.h"
#include "itkResampleImageFilter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"

#include "itkBSplineTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "itkCompositeTransform.h"

#include "itkBSplineTransformInitializer.h"
#include "itkCenteredTransformInitializer.h"

#include "itkTimeProbesCollectorBase.h"
#include "itkMemoryProbesCollectorBase.h"
#include "itkCommand.h"

#include "apply_transform.h"
#include "image_io.h"
#include "string_splitting.h"

// For the time being, this works on 2d images
const int IMAGE_DIMENSIONS = 2;
const int BSPLINE_ORDER = 3;

typedef itk::Image<float, IMAGE_DIMENSIONS> IMAGE_TYPE;
typedef itk::BSplineTransform<double, IMAGE_DIMENSIONS, BSPLINE_ORDER> BSPLINE_TRANSFORM_TYPE;
typedef itk::CenteredRigid2DTransform<double> RIGID_TRANSFORM_TYPE;
typedef itk::CompositeTransform<double, IMAGE_DIMENSIONS> COMPOSITE_TRANSFORM_TYPE;

void configure_optimizer(itk::LBFGSBOptimizerv4::Pointer optimizer, unsigned int num_params);
IMAGE_TYPE::Pointer load_image(const char* image_path);
RIGID_TRANSFORM_TYPE::Pointer compute_rigid_transform(IMAGE_TYPE::Pointer fixed_image, IMAGE_TYPE::Pointer moving_image);
BSPLINE_TRANSFORM_TYPE::Pointer compute_bSpline_transform(IMAGE_TYPE::Pointer fixed_image, IMAGE_TYPE::Pointer moving_image);
COMPOSITE_TRANSFORM_TYPE::Pointer compose_transforms(RIGID_TRANSFORM_TYPE::Pointer rigid_transform, BSPLINE_TRANSFORM_TYPE::Pointer bspline_transform);

#endif
