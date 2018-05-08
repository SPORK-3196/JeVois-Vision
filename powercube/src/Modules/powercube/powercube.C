#include <jevois/Core/Module.H>
#include <jevois/Image/RawImageOps.H>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

/**
 * Parameters
 * ----------
 * Parameters are used to allow calibration of the Module through Serial input.
**/
static jevois::ParameterCategory const GeneralParameters("General PowerCube Module Parameters");
static jevois::ParameterCategory const FilterParameters("Color filtering parameters");
static jevois::ParameterCategory const EdgeDetectParameters("Edge Detection Options");

JEVOIS_DECLARE_PARAMETER(displayLevel, int, "What step of processing should be output as feed", 2, jevois::Range<int>(0,2), GeneralParameters);

JEVOIS_DECLARE_PARAMETER(min_r, int, "Minimum R threshold for color filtering", 127, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(min_g, int, "Minimum G threshold for color filtering", 127, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(min_b, int, "Minimum B threshold for color filtering", 20,  jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(max_r, int, "Maximum R threshold for color filtering", 255, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(max_g, int, "Maximum G threshold for color filtering", 255, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(max_b, int, "Maximum B threshold for color filtering", 150, jevois::Range<int>(0, 255), FilterParameters);

JEVOIS_DECLARE_PARAMETER(thresh1, double, "First threshold for hysteresis", 50.0, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(thresh2, double, "Second threshold for hysteresis", 150.0, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(aperture, int, "Aperture size for the Sobel operator", 3, jevois::Range<int>(3, 53), EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER_WITH_CALLBACK(l2grad, bool, "Use more accurate L2 gradient norm if true, L1 if false", false, EdgeDetectParameters);



/**
 *  PowerCube
 *  ---------
 *  Detects Power cubes and models their 3d orientation
 *
 *  Input images are RGB thesholded to exaggerate the presence of yellow Power
 *  Cubes, and then run through a Canny edge detection function to generate
 *  skeletal outlines of Power Cubes. Hough Line Transforms are then used to
 *  get a geometric profile of the Cube, which is then extrapolated to 3D space
 *  to infer an orientation and position for the Cube.
**/
class powercube : public jevois::Module,
                public jevois::Parameter
                    <displayLevel,                              // General
                    min_r, min_g, min_b, max_r, max_g, max_b,   // Color
                    thresh1, thresh2, aperture, l2grad>         // Canny
{
public:
    // Default base class constructor ok
    using jevois::Module::Module;

    // Virtual destructor for safe inheritance
    virtual ~powercube() { }

    // Processing function
    virtual void process(jevois::InputFrame && p_inframe, jevois::OutputFrame && p_outframe) override
    {
        // Get the RawImage from the InputFrame (InputFrame is the memory block
        // filled by the camera, 'inimg' is owned by the module)
        jevois::RawImage inimg = p_inframe.get();
    
        // Release the InputFrame to give the memory block back to the camera
        p_inframe.done();



        // Get a RawImage reference to the OutputFrame, and paint it black
        jevois::RawImage outimg = p_outframe.get();
        outimg.require("output", inimg.width, inimg.height+40, inimg.fmt);
        jevois::rawimage::drawFilledRect(outimg, 0, 0, outimg.width, outimg.height, jevois::yuyv::Black);


        
        // Convert input image into an OpenCV image for processing
        //cv::Mat proc_img = jevois::rawimage::convertToCvRGB(inimg);   //NOTE: When adding color threshold back in, uncomment this and delete the below
        cv::Mat proc_img = jevois::rawimage::convertToCvGray(inimg);

        if (displayLevel::get() == 0)  // If display level is set to raw input
            jevois::rawimage::pasteRGBtoYUYV(proc_img, outimg, 0, 20);

        // RGB Thresholding function
        /*cv::inRange(
            proc_img,           // Input Image
            cv::Scalar(         // Minimum RGB values
                min_r::get(),   //   r
                min_g::get(),   //   g
                min_b::get()),  //   b
            cv::Scalar(         // Maximum RGB values
                max_r::get(),   //   r
                max_g::get(),   //   g
                max_b::get()),  //   b
            proc_img);          // Output Image*/

        if (displayLevel::get() == 1)  // If disaply level is set to threshold
            jevois::rawimage::pasteGreyToYUYV(proc_img, outimg, 0, 20);

        // Edge detection function
        cv::Canny(
            proc_img,           // Input Image
            proc_img,           // Output Image
            thresh1::get(),     //
            thresh2::get(),     //
            aperture::get(),    //
            l2grad::get());     //

        if (displayLevel::get() == 2)  // If disaply level is set to Canny
            jevois::rawimage::pasteGreyToYUYV(proc_img, outimg, 0, 20);

        // Hough Line Transform function

        


        // Insert camera feed (grayscale)
        // Write header text
        jevois::rawimage::writeText(outimg, "SPORK - 3196 | Power Cube Detection Module", 0, 0, jevois::yuyv::White);

        // Send the output image with our processing results to the host over USB:
        p_outframe.send();
    }

    // Callback function for parameter l2grad
    void onParamChange(l2grad const & param, bool const & newval) override
    {
        LINFO("you changed l2grad to be " + std::to_string(newval));
    }
};

// Allow the module to be loaded as a shared object (.so) file:
JEVOIS_REGISTER_MODULE(powercube);