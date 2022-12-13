#ifndef OPENCV_MAIN_HPP
#define OPENCV_MAIN_HPP

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/tracking/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <Constant.hpp>

class OpenGL_App;

class OpenCV_App {
    public:
        OpenCV_App(OpenGL_App* tab);
        ~OpenCV_App();

        cv::Mat& update();

        int angle( cv::Point a, cv::Point b, cv::Point c );
        void find_squares(cv::Mat& image, std::vector<std::vector<cv::Point> >* squares);
        cv::Mat debugSquares( std::vector<std::vector<cv::Point> > squares, cv::Mat image, int& carre_choosed);
        bool findROI(cv::Mat& frame, cv::Mat& vierge, std::vector<cv::Point> carre);
        void trackingSymbols(cv::Mat image, cv::Ptr<cv::Tracker>& tracker);
        
        bool is_Capture_Open();
        cv::Mat& getCap(void);

        bool end_init_phase;

    private:
        enum Status{INITIALIZATION, RUNNING};

        cv::Point ROI0Coord;
        cv::Point ROI1Coord;
        cv::Point ROI2Coord;
        cv::Point ROI3Coord;
        cv::Point ROI0Size;
        cv::Point ROI1Size;
        cv::Point ROI2Size;
        cv::Point ROI3Size;

        cv::Ptr<cv::Tracker> tracker0;
        cv::Ptr<cv::Tracker> tracker1;
        cv::Ptr<cv::Tracker> tracker2;
        cv::Ptr<cv::Tracker> tracker3;

        cv::Mat ROI0;
        cv::Mat ROI1;
        cv::Mat ROI2;
        cv::Mat ROI3;

        std::vector<std::vector<cv::Point>> liste_carre;

        cv::VideoCapture capture;
        cv::Mat current_frame;

        int index_carre_choosed;
        int app_status;

        cv::Mat frame;
        cv::Mat vierge;

        OpenGL_App* app;
};

#endif