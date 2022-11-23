#ifndef OPENCV_MAIN_HPP
#define OPENCV_MAIN_HPP

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/tracking/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>


class OpenCV_App {
    public:
        OpenCV_App();
        ~OpenCV_App();

        int angle( cv::Point a, cv::Point b, cv::Point c );
        void find_squares(cv::Mat& image, std::vector<std::vector<cv::Point> >* squares);
        cv::Mat debugSquares( std::vector<std::vector<cv::Point> > squares, cv::Mat image, int& carre_choosed);
        bool findROI(cv::Mat& frame, cv::Mat& vierge, std::vector<cv::Point> carre);
        void trackingSymbols(cv::Mat image, cv::Ptr<cv::Tracker>& tracker);
        
        bool is_Capture_Open();
        cv::Mat& getCap(void);

    private:
        cv::Point ROI0Coord;
        cv::Point ROI1Coord;
        cv::Point ROI2Coord;
        cv::Point ROI3Coord;
        cv::Point ROI0Size;
        cv::Point ROI1Size;
        cv::Point ROI2Size;
        cv::Point ROI3Size;

        cv::Mat ROI0;
        cv::Mat ROI1;
        cv::Mat ROI2;
        cv::Mat ROI3;

        cv::VideoCapture capture;
        cv::Mat current_frame;
};

#endif