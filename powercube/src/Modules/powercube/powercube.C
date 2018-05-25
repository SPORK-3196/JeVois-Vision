#include <vector>
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
static jevois::ParameterCategory const ColorParameters("Color Filtering Parameters");
static jevois::ParameterCategory const EdgeDetectParameters("Edge and Line Detection Parameters");

JEVOIS_DECLARE_PARAMETER(displayLevel, int, "What step of processing should be output as camera feed", 3, jevois::Range<int>(0,3), GeneralParameters);
JEVOIS_DECLARE_PARAMETER(erosionIt, int, "How many iterations of erosion should the thresholded image recieve", 1, jevois::Range<int>(0,8), GeneralParameters);
JEVOIS_DECLARE_PARAMETER(dilationIt, int, "How many iterations of dilation should the thresholded image recieve", 1, jevois::Range<int>(0,8), GeneralParameters);

JEVOIS_DECLARE_PARAMETER(min_h, int, "Minimum Hue threshold for PowerCube color detection", 15, jevois::Range<int>(0, 180), ColorParameters);
JEVOIS_DECLARE_PARAMETER(max_h, int, "Maximum Hue threshold for PowerCube color detection", 45, jevois::Range<int>(0, 180), ColorParameters);
JEVOIS_DECLARE_PARAMETER(min_s, int, "Minimum Saturation threshold for PowerCube color detection", 50, jevois::Range<int>(0, 255), ColorParameters);
JEVOIS_DECLARE_PARAMETER(max_s, int, "Maximum Saturation threshold for PowerCube color detection", 255, jevois::Range<int>(0, 255), ColorParameters);
JEVOIS_DECLARE_PARAMETER(min_v, int, "Minimum Value threshold for PowerCube color detection", 50,  jevois::Range<int>(0, 255), ColorParameters);
JEVOIS_DECLARE_PARAMETER(max_v, int, "Maximum Value threshold for PowerCube color detection", 255, jevois::Range<int>(0, 255), ColorParameters);

JEVOIS_DECLARE_PARAMETER(thresh1, double, "First threshold for hysteresis", 50.0, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(thresh2, double, "Second threshold for hysteresis", 150.0, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(aperture, int, "Aperture size for the Sobel operator", 3, jevois::Range<int>(3, 53), EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(l2grad, bool, "Use more accurate L2 gradient norm if true, L1 if false", false, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(line_thresh, int, "Threshold for Hough Line Transform", 100,  jevois::Range<int>(0, 255), EdgeDetectParameters);



/**
 *  PowerCube
 *  ---------
 *  Detects Power cubes and models their 3d orientation
 *
 *  Input images are converted to HSV color format, thresholded so only specific
 *  ranges of color are present (i.e. the yellow of PowerCubes), and then run
 *  through the Canny edge detection function to generate a wireframe image of
 *  Power Cubes. The Hough Line Transform algorithm is then used to create a 2d
 *  geometric profile of PowerCubes, which are then extrapolated to 3D space
 *  to infer an orientation and position for the PowerCubes.
**/
class powercube : public jevois::Module,
                public jevois::Parameter
                    <displayLevel, erosionIt, dilationIt,               // General
                    min_h, min_s, min_v, max_h, max_s, max_v,           // Color
                    thresh1, thresh2, aperture, l2grad, line_thresh>    // Canny
{
public:
    // Default base class constructor
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
        cv::Mat proc_img = jevois::rawimage::convertToCvRGB(inimg);

        if (displayLevel::get() == 0)  // If display level is set to raw input
            jevois::rawimage::pasteRGBtoYUYV(proc_img, outimg, 0, 20);

        

        // Convert the processing image into HSV color encoding
        cv::cvtColor(
            proc_img,               // Input image
            proc_img,               // Output image
            cv::COLOR_RGB2HSV);     // Encoding

        // HSV Thresholding function, used to remove all but the desired color
        cv::inRange(
            proc_img,               // Input Image
            cv::Scalar(             // Minimum HSV values
                min_h::get(),       //   hue
                min_s::get(),       //   saturation
                min_v::get()),      //   value
            cv::Scalar(             // Maximum HSV values
                max_h::get(),       //   hue
                max_s::get(),       //   saturation
                max_v::get()),      //   value
            proc_img);              // Output Image

        // Erosion and Dilation to clear stray pixels
        cv::erode(
            proc_img,               // Input image
            proc_img,               // Output image
            getStructuringElement(  // Kernel (shape the erosion occurs in)
                cv::MORPH_RECT,     //   Type (RECT, CROSS, ELLIPSE)
                cv::Size(3,3),      //   Size
                cv::Point(-1,-1)),  //   Anchor (centered)
            cv::Point(-1,-1),       // Anchor (centered)
            erosionIt::get());      // Iterations
        cv::dilate(
            proc_img,               // Input image
            proc_img,               // Output image
            getStructuringElement(  // Kernel (shape the dilation occurs in)
                cv::MORPH_ELLIPSE,  //   Type (RECT, CROSS, ELLIPSE)
                cv::Size(3,3),      //   Size
                cv::Point(-1,-1)),  //   Anchor (centered)
            cv::Point(-1,-1),       // Anchor (centered)
            dilationIt::get());     // Iterations

        if (displayLevel::get() == 1)  // If display level is set to threshold
            jevois::rawimage::pasteGreyToYUYV(proc_img, outimg, 0, 20);



        // Canny Edge detection algorithm
        cv::Canny(
            proc_img,               // Input Image
            proc_img,               // Output Image
            thresh1::get(),         //
            thresh2::get(),         //
            aperture::get(),        //
            l2grad::get());         //

        if (displayLevel::get() >= 2)  // If display level is set to edge or above
            jevois::rawimage::pasteGreyToYUYV(proc_img, outimg, 0, 20);



        // Probabilistic Hough Line Transform
        std::vector<cv::Vec4i> lines;
        HoughLinesP(
            proc_img,               // Input Image
            lines,                  // Vector of lines
            1,                      // Resolution of polar coordinate 'r' in pixels
            CV_PI/180,              // Resolution of theta coordinate in pixels
            line_thresh::get(),     // Threshold
            50,                     // 'srn' - Purpose unknown
            10);                    // 'stn' - Purpose unknown
        // Draw the lines on screen, if display level is set to line detect
        if (displayLevel::get() == 3)
            for( size_t i = 0; i < lines.size(); i++ )
            {
                cv::Vec4i l = lines[i];
                //line( cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
                jevois::rawimage::drawLine(outimg, l[0], l[1]+20, l[2], l[3]+20, 2, jevois::rgb565::Red);
            }


        // Write header text
        jevois::rawimage::writeText(outimg, "SPORK - 3196 | Power Cube Detection Module", 0, 0, jevois::yuyv::White);
        jevois::rawimage::writeText(outimg, std::to_string(lines.size()) + " lines detected", 0, 10, jevois::yuyv::White);

        // Send the output image with our processing results to the host over USB:
        p_outframe.send();
    }
};

// Allow the module to be loaded as a shared object (.so) file:
JEVOIS_REGISTER_MODULE(powercube);
