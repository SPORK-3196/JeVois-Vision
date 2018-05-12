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
static jevois::ParameterCategory const FilterParameters("Color Filtering Parameters");
static jevois::ParameterCategory const EdgeDetectParameters("Edge and Line Detection Parameters");

JEVOIS_DECLARE_PARAMETER(displayLevel, int, "What step of processing should be output as feed", 2, jevois::Range<int>(0,2), GeneralParameters);

JEVOIS_DECLARE_PARAMETER(min_r, int, "Minimum R threshold for color filtering", 127, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(min_g, int, "Minimum G threshold for color filtering", 127, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(min_b, int, "Minimum B threshold for color filtering", 127,  jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(max_r, int, "Maximum R threshold for color filtering", 255, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(max_g, int, "Maximum G threshold for color filtering", 255, jevois::Range<int>(0, 255), FilterParameters);
JEVOIS_DECLARE_PARAMETER(max_b, int, "Maximum B threshold for color filtering", 255, jevois::Range<int>(0, 255), FilterParameters);

JEVOIS_DECLARE_PARAMETER(thresh1, double, "First threshold for hysteresis", 50.0, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(thresh2, double, "Second threshold for hysteresis", 150.0, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(aperture, int, "Aperture size for the Sobel operator", 3, jevois::Range<int>(3, 53), EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER_WITH_CALLBACK(l2grad, bool, "Use more accurate L2 gradient norm if true, L1 if false", false, EdgeDetectParameters);
JEVOIS_DECLARE_PARAMETER(line_thresh, int, "Threshold for Hough Line Transform", 100,  jevois::Range<int>(0, 255), EdgeDetectParameters);



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
                    thresh1, thresh2, aperture, l2grad, line_thresh>         // Canny
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
        cv::Mat proc_img = jevois::rawimage::convertToCvRGB(inimg);   //NOTE: When adding color threshold back in, uncomment this and delete the below
        //cv::Mat proc_img = jevois::rawimage::convertToCvGray(inimg);

        if (displayLevel::get() == 0)  // If display level is set to raw input
            jevois::rawimage::pasteRGBtoYUYV(proc_img, outimg, 0, 20);

        // RGB Thresholding function
        cv::inRange(
            proc_img,           // Input Image
            cv::Scalar(         // Minimum RGB values
                min_r::get(),   //   r
                min_g::get(),   //   g
                min_b::get()),  //   b
            cv::Scalar(         // Maximum RGB values
                max_r::get(),   //   r
                max_g::get(),   //   g
                max_b::get()),  //   b
            proc_img);          // Output Image

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




        // Probabilistic Hough Line Transform
        std::vector<cv::Vec4i> lines;
        HoughLinesP(
            proc_img,           // Input Image
            lines,              // Vector of lines
            1,                  // Resolution of polar coordinate 'r' in pixels
            CV_PI/180,          // Resolution of theta coordinate in pixels
            line_thresh::get(), // Threshold
            50,                  // 'srn' - Purpose unknown
            10);                 // 'stn' - Purpose unknown
        // Draw the lines
        for( size_t i = 0; i < lines.size(); i++ )
        {
            cv::Vec4i l = lines[i];
            //line( cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, LINE_AA);
            jevois::rawimage::drawLine(outimg, l[0], l[1]+20, l[2], l[3]+20, 2, jevois::rgb565::Red);
        }
        
        // Standard Hough Line Transform function
        /*std::vector<cv::Vec2f> lines; // will hold the results of the detection
        HoughLines(
            proc_img,           // Input Image
            lines,              // Vector of lines
            1,                  // Resolution of polar coordinate 'r' in pixels
            CV_PI/180,          // Resolution of theta coordinate in pixels
            line_thresh::get(), // Threshold
            0,                  // 'srn' - Purpose unknown
            0);                 // 'stn' - Purpose unknown

        // Draw the lines
        for( size_t i = 0; i < lines.size(); i++ )
        {
            int w = outimg.width,
                h = outimg.height;
            
            float rho = lines[i][0], theta = lines[i][1];
            cv::Point pt1, pt2;
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;  // "origin" of the line
            pt1.x = cvRound(x0 + w/2*(-b));
            pt1.y = cvRound(y0 + h/2*(a));
            pt2.x = cvRound(x0 - w/2*(-b));
            pt2.y = cvRound(y0 - h/2*(a));

            jevois::rawimage::writeText(outimg,
                "(" + std::to_string(pt1.x) + "," + std::to_string(pt1.y) + ") -> (" +
                "(" + std::to_string(pt2.x) + "," + std::to_string(pt2.y) + ")",
                0, 40 + (20*i), jevois::yuyv::White);

            //line( proc_img, pt1, pt2, cv::Scalar(0,0,255), 3, CV_AA);
            jevois::rawimage::drawLine(outimg, pt1.x, pt1.y, pt2.x, pt2.y, 2, jevois::rgb565::Red);
        }*/
        


        // Insert camera feed (grayscale)
        // Write header text
        //jevois::rawimage::writeText(outimg, "SPORK - 3196 | Power Cube Detection Module", 0, 0, jevois::yuyv::White);
        jevois::rawimage::writeText(outimg, std::to_string(lines.size()) + " lines detected", 0, 0, jevois::yuyv::White);

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