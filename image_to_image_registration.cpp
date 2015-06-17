#include "image_to_image_registration.h"

using namespace std;

// Observer class, to output the progress of the registration
// This was copied directly from a rigid registration example, without modification
class CommandIterationUpdate : public itk::Command {
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>   Pointer;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

public:
  typedef itk::LBFGSBOptimizerv4 OptimizerType;
  typedef   const OptimizerType *                  OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event) {
    Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event) {
    OptimizerPointer optimizer =
      dynamic_cast< OptimizerPointer >( object );
    if (! itk::IterationEvent().CheckEvent( &event )) {
        return;
    }

    cout << optimizer->GetCurrentIteration() << "   ";
    cout << optimizer->GetValue() << "   ";
    cout << optimizer->GetCurrentPosition() << endl;
    }
};

// Observer for the BSplineTransform Computation.
class BsplineTransformIterationUpdater : public itk::Command {
public:
    typedef BsplineTransformIterationUpdater Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);

private:
    fstream outputFile;
    const char * outputFilePath;

protected:
    BsplineTransformIterationUpdater() {};

public:
    typedef itk::LBFGSBOptimizerv4 OptimizerType;
    typedef const OptimizerType * OptimizerPointer;

    void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE {
        Execute( (const itk::Object *)caller, event);
    }
  
    void Execute(const itk::Object * object, const itk::EventObject & event) ITK_OVERRIDE {
        OptimizerPointer optimizer = static_cast< OptimizerPointer >( object );
        if( !(itk::IterationEvent().CheckEvent( &event )) ){
            return;
        }

        cout << optimizer->GetCurrentIteration() << "\t";
        cout << optimizer->GetCurrentMetricValue() << endl;
    }
};


// option parsing
struct Arg: public option::Arg {
   static void printError(const char* msg1, const option::Option& opt, const char* msg2) {
     fprintf(stderr, "ERROR: %s", msg1);
     fwrite(opt.name, opt.namelen, 1, stderr);
     fprintf(stderr, "%s", msg2);
   }

   static option::ArgStatus Unknown(const option::Option& option, bool msg) {
     if (msg) printError("Unknown option '", option, "'\n");
     return option::ARG_ILLEGAL;
   }

   static option::ArgStatus Required(const option::Option& option, bool msg) {
     if (option.arg != 0)
       return option::ARG_OK;

     if (msg) printError("Option '", option, "' requires an argument\n"); return option::ARG_ILLEGAL;
   }

   static option::ArgStatus Numeric(const option::Option& option, bool msg) {
     char* endptr = 0;
     if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
     if (endptr != option.arg && *endptr == 0)
       return option::ARG_OK;

     if (msg) printError("Option '", option, "' requires a numeric argument\n");
     return option::ARG_ILLEGAL;
   }
 };


enum optionIndex {
    UNKNOWN,
    HELP,
    FIXED_IMAGE,
    MOVING_IMAGE,
    OUTPUT_PATH,
    TRANSFORM_PATH,
    APPLICATION_TARGET
};


const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", Arg::Unknown, "USAGE: image_to_image_registration [options]\n\n"
                                       "Options: "},
    {HELP, 0, "h", "help", Arg::None, "--help, -h \tDisplay this help message and exit"},
    {FIXED_IMAGE, 0, "f", "fixed", Arg::Required, "--fixed, -f path \tPath to the fixed image to register the moving image to."},
    {MOVING_IMAGE, 0, "m", "moving", Arg::Required, "--moving, -m path \tPath to the moving image."},
    {OUTPUT_PATH, 0, "o", "output", Arg::Required, "--output, -o path \tPath to save the output moving image"},
    {TRANSFORM_PATH, 0, "t", "transform", Arg::Required, "--transform, -t path \tPath to save the computed transform"},
    {APPLICATION_TARGET, 0, "a", "apply", Arg::Required, "--apply, -a input_path,output_path \tPaths to additional images to apply the transform to"},
    {0,0,0,0,0,0}
};


int main(int argc, char** argv) {
    // Parses the input arguments using the lean mean option parser
    argv += (argc > 0);
    argc -= (argc > 0);

    option::Stats stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    option::Option* buffer = new option::Option[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (options[HELP]) {
        option::printUsage(cout, usage);
        return 1;
    }

    // Load images
    IMAGE_TYPE::Pointer fixed_image = load_image<IMAGE_TYPE>(options[FIXED_IMAGE].arg);
    IMAGE_TYPE::Pointer moving_image = load_image<IMAGE_TYPE>(options[MOVING_IMAGE].arg);

    // Compute transform
    RIGID_TRANSFORM_TYPE::Pointer rigid_transform = compute_rigid_transform(fixed_image, moving_image);    
    moving_image = apply_transform<IMAGE_TYPE, RIGID_TRANSFORM_TYPE>(moving_image, rigid_transform);
    BSPLINE_TRANSFORM_TYPE::Pointer bspline_transform = compute_bSpline_transform(fixed_image, moving_image);

    // Apply tranform
    IMAGE_TYPE::Pointer output_image = apply_transform<IMAGE_TYPE, BSPLINE_TRANSFORM_TYPE>(moving_image, bspline_transform);
    write_image<IMAGE_TYPE>(output_image, options[OUTPUT_PATH].arg);

    // Optionally save transform
    if (options[TRANSFORM_PATH]) {
       COMPOSITE_TRANSFORM_TYPE::Pointer composite_transform = compose_transforms(rigid_transform, bspline_transform);
       write_transform<COMPOSITE_TRANSFORM_TYPE>(composite_transform, options[TRANSFORM_PATH].arg);
    }

    // Apply images to additional images
    string arg_str;
    vector<string> io_paths;
    for (option::Option* opt = options[APPLICATION_TARGET]; opt; opt = opt->next()) {
        arg_str = opt->arg;
        io_paths = split(arg_str, ',');
        moving_image = load_image<IMAGE_TYPE>(io_paths[0].c_str());
        moving_image = apply_transform<IMAGE_TYPE, RIGID_TRANSFORM_TYPE>(moving_image, rigid_transform);
        moving_image = apply_transform<IMAGE_TYPE, BSPLINE_TRANSFORM_TYPE>(moving_image, bspline_transform);
        write_image<IMAGE_TYPE>(moving_image, io_paths[1].c_str());
    }

    return 0;
}


RIGID_TRANSFORM_TYPE::Pointer compute_rigid_transform(IMAGE_TYPE::Pointer fixed_image, IMAGE_TYPE::Pointer moving_image) {
    typedef itk::ImageRegistrationMethodv4<IMAGE_TYPE, IMAGE_TYPE> RegistrationType;
    typedef itk::LBFGSBOptimizerv4 OptimizerType;
    typedef itk::MeanSquaresImageToImageMetricv4<IMAGE_TYPE, IMAGE_TYPE> MetricType;
    typedef itk::LinearInterpolateImageFunction<IMAGE_TYPE, double> InterpolatorType;
    typedef itk::CenteredTransformInitializer<RIGID_TRANSFORM_TYPE, IMAGE_TYPE, IMAGE_TYPE> TransformInitializerType;

    // Instantiate the metric, optimizer, interpolator and registration objects
    MetricType::Pointer           metric        = MetricType::New();
    OptimizerType::Pointer        optimizer     = OptimizerType::New();
    InterpolatorType::Pointer     interpolator  = InterpolatorType::New();
    RIGID_TRANSFORM_TYPE::Pointer   transform     = RIGID_TRANSFORM_TYPE::New();
    RegistrationType::Pointer     registration  = RegistrationType::New();

    // Set up the registration
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetInitialTransform(transform);

    // Set the inputs
    registration->SetFixedImage(fixed_image);
    registration->SetMovingImage(moving_image);

    // Initialize the transform using center of mass
    TransformInitializerType::Pointer initializer = TransformInitializerType::New();
    initializer->SetFixedImage(fixed_image);
    initializer->SetMovingImage(moving_image);
    initializer->SetTransform(transform);

    initializer->MomentsOn();  // MomentsOn() sets the initializer to center mass mode

    initializer->InitializeTransform();
    transform->SetAngle(0.0);

    // Configure the optimizer
    const unsigned int num_params = transform->GetNumberOfParameters();
    configure_optimizer(optimizer, num_params);

    // The command observer will report on the registrations progress at each iteration
    //  The class definition is below, and taken directly from the ImageRegistration6.cxx example
    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    // Begin Registration by calling Update()
    try {
        registration->Update();
        cout << "Optimizer stop condition: "
                  << registration->GetOptimizer()->GetStopConditionDescription()
                  << endl;
    } catch ( itk::ExceptionObject & err ) {
        cerr << "ExceptionObject caught !" << endl;
        cerr << err << endl;
        throw -1;
    }

    return transform;
}


BSPLINE_TRANSFORM_TYPE::Pointer compute_bSpline_transform(IMAGE_TYPE::Pointer fixed_image, IMAGE_TYPE::Pointer moving_image){
    typedef itk::LBFGSBOptimizerv4 OptimizerType;
    typedef itk::MattesMutualInformationImageToImageMetricv4<IMAGE_TYPE, IMAGE_TYPE> MetricType;
    typedef itk::ImageRegistrationMethodv4<IMAGE_TYPE, IMAGE_TYPE, BSPLINE_TRANSFORM_TYPE> RegistrationType;
    typedef itk::BSplineTransformInitializer<BSPLINE_TRANSFORM_TYPE, IMAGE_TYPE> BSplineTransformInitializerType;

    // Instantiate the metric, optimizer, interpolator, transform and registration objects
    MetricType::Pointer metric = MetricType::New();
    OptimizerType::Pointer optimizer = OptimizerType::New();
    RegistrationType::Pointer registration = RegistrationType::New();
    BSPLINE_TRANSFORM_TYPE::Pointer transform = BSPLINE_TRANSFORM_TYPE::New();
    BSplineTransformInitializerType::Pointer transform_initializer = BSplineTransformInitializerType::New();

    // Calculate image physical dimensions and mesh_size
    BSPLINE_TRANSFORM_TYPE::PhysicalDimensionsType fixed_image_physical_dimensions;
    BSPLINE_TRANSFORM_TYPE::MeshSizeType mesh_size;
    IMAGE_TYPE::SizeType fixed_image_size = fixed_image->GetLargestPossibleRegion().GetSize();
    unsigned int number_of_grid_nodes_in_one_dimension = 8;

    for (int i = 0; i < 2; i++) {
        fixed_image_physical_dimensions[i] =  (fixed_image_size[i] - 1) * fixed_image->GetSpacing()[i];
    }
    mesh_size.Fill(number_of_grid_nodes_in_one_dimension - BSPLINE_ORDER);

    // Initialize the transform
    transform_initializer->SetTransform(transform);
    transform_initializer->SetImage(fixed_image);
    transform_initializer->SetTransformDomainMeshSize(mesh_size);
    transform_initializer->InitializeTransform();

    transform->SetIdentity();

    // Set Metic Parameters
    metric->SetNumberOfHistogramBins(64);

    // Specify the optimizer parameters
    const unsigned int num_params = transform->GetNumberOfParameters();
    configure_optimizer(optimizer, num_params);

    // Add an observer to the optimizer
    BsplineTransformIterationUpdater::Pointer observer = BsplineTransformIterationUpdater::New();
    optimizer->AddObserver(itk::IterationEvent(), observer);

    // Connect everything to the registration object
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetInitialTransform(transform);

    // Set the inputs for the registration object
    registration->SetFixedImage(fixed_image);
    registration->SetMovingImage(moving_image);

    // Set Multi-Resolution Options
    // The shrink factor denotes to the factor by which the image will be downsized
    // The smoothing sigma determines the width of the gaussian kernel used to smooth the downsampled image
    const unsigned int number_of_levels = 3;

    RegistrationType::ShrinkFactorsArrayType shrink_factor_per_level;
    shrink_factor_per_level.SetSize(number_of_levels);
    shrink_factor_per_level[0] = 4;
    shrink_factor_per_level[1] = 2;
    shrink_factor_per_level[2] = 1;

    RegistrationType::SmoothingSigmasArrayType sigma_per_level;
    sigma_per_level.SetSize(number_of_levels);
    sigma_per_level[0] = 4;
    sigma_per_level[1] = 2;
    sigma_per_level[2] = 0;

    registration->SetNumberOfLevels(number_of_levels);
    registration->SetShrinkFactorsPerLevel(shrink_factor_per_level);
    registration->SetSmoothingSigmasPerLevel(sigma_per_level);

    // Start Registration
    try {
        registration->Update();

        cout << "Optimizer stop condition = "
                  << registration->GetOptimizer()->GetStopConditionDescription()
                  << endl;
    } catch ( itk::ExceptionObject & err ) {
        cerr << "ExceptionObject caught !" << endl;
        cerr << err << endl;
        throw -1;
    }

    return transform;
}

void configure_optimizer(itk::LBFGSBOptimizerv4::Pointer optimizer, unsigned int num_params) {
    typedef itk::LBFGSBOptimizerv4 OptimizerType;
    OptimizerType::BoundSelectionType boundSelect(num_params);
    OptimizerType::BoundValueType upperBound(num_params);
    OptimizerType::BoundValueType lowerBound(num_params);

    boundSelect.Fill(0);
    upperBound.Fill(0.0);
    lowerBound.Fill(0.0);

    optimizer->SetBoundSelection(boundSelect);
    optimizer->SetUpperBound(upperBound);
    optimizer->SetLowerBound(lowerBound);

    optimizer->SetCostFunctionConvergenceFactor(1.e7);
    optimizer->SetGradientConvergenceTolerance(1e-35);
    optimizer->SetNumberOfIterations(200);
    optimizer->SetMaximumNumberOfFunctionEvaluations(200);
    optimizer->SetMaximumNumberOfCorrections(7);
}


COMPOSITE_TRANSFORM_TYPE::Pointer compose_transforms(RIGID_TRANSFORM_TYPE::Pointer rigid_transform, BSPLINE_TRANSFORM_TYPE::Pointer bspline_transform) {
    typedef itk::TransformFileWriterTemplate<double> TransformWriterType;

    COMPOSITE_TRANSFORM_TYPE::Pointer composite_transform = COMPOSITE_TRANSFORM_TYPE::New();
    composite_transform->AddTransform(rigid_transform);
    composite_transform->AddTransform(bspline_transform);
    return composite_transform;
}
