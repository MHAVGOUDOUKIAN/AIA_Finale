#include <OpenCV_Main.hpp>

using namespace cv;
using namespace std;

OpenCV_App::OpenCV_App() {
    capture = cv::VideoCapture(0);
}

OpenCV_App::~OpenCV_App() {}

int OpenCV_App::angle( cv::Point a, cv::Point b, cv::Point c ) {
    Point ab = { b.x - a.x, b.y - a.y };
    Point cb = { b.x - c.x, b.y - c.y };

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);
    int res = (int) floor(alpha * 180. / M_PI + 0.5);

    return res*(M_PI/180);
}

void OpenCV_App::find_squares(cv::Mat& image, std::vector<std::vector<cv::Point> >* squares) {
// blur will enhance Edge detection
    Mat blurred(image);
    medianBlur(image, blurred, 9);

    Mat gray0(blurred.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    // find squares in every color plane of the image
    for (int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        mixChannels(&blurred, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        const int threshold_level = 20;
        for (int l = 0; l < threshold_level; l++) {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch squares with gradient shading
            if (l == 0) {
                Canny(gray0, gray, 10, 255, 3); // 

                // Dilate helps to remove potential holes between Edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else {
                    gray = gray0 >= (l+1) * 255 / threshold_level;
            }

            // Find contours and store them in a list
            findContours(gray, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

            // Test contours
            vector<Point> approx;
            for (size_t i = 0; i < contours.size(); i++) {
                    // approximate contour with accuracy proportional
                    // to the contour perimeter
                    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                    // Note: absolute value of an area is used because
                    // area may be positive or negative - in accordance with the
                    // contour orientation
                    if (approx.size() == 4 &&
                            fabs(contourArea(Mat(approx))) > 1000 &&
                            isContourConvex(Mat(approx))) {
                            double maxCosine = 0;

                            for (int j = 2; j < 5; j++) {
                                    double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                                    maxCosine = MAX(maxCosine, cosine);
                            }

                            if (maxCosine < 0.3)
                                    squares->push_back(approx);
                    }
            }
        }
    }
}

cv::Mat OpenCV_App::debugSquares( std::vector<std::vector<cv::Point> > squares, cv::Mat image, int& carre_choosed) {
    double max_area =0;
    int  index_max_square=0;
    for ( int i = 0; i< squares.size(); i++ ) {
        // draw contour
        
        double area = cv::contourArea(squares[i]);
        if(area > max_area) {
            max_area = area;
            index_max_square = i;
        }

        carre_choosed = index_max_square;
        // draw bounding rect
        /*cv::Rect rect = boundingRect(cv::Mat(squares[i]));
        cv::rectangle(image, rect.tl(), rect.br(), cv::Scalar(0,255,0), 2, 8, 0);

        // draw rotated rect
        cv::RotatedRect minRect = minAreaRect(cv::Mat(squares[i]));
        cv::Point2f rect_points[4];
        minRect.points( rect_points );
        for ( int j = 0; j < 4; j++ ) {
            cv::line( image, rect_points[j], rect_points[(j+1)%4], cv::Scalar(0,0,255), 1, 8 ); // blue
        }*/
    }

    cv::drawContours(image, squares, index_max_square, cv::Scalar(0,0,255), 3, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
    return image;
}

bool OpenCV_App::findROI(cv::Mat& frame, cv::Mat& vierge, std::vector<cv::Point> carre) {
    cv::Point max_y(0,10000), max_y2(0,10000), max_y3(0,10000), max_y4(0,10000);

    for (int i=0;i<4;i++) {
        if (carre[i].y <= max_y.y){
            max_y4 = max_y3;
            max_y3 = max_y2;
            max_y2 = max_y;
            max_y = carre[i];
        }
        else if (carre[i].y <= max_y2.y){
            max_y4 = max_y3;
            max_y3 = max_y2;
            max_y2 = carre[i];
        }
        else if (carre[i].y <= max_y3.y){
            max_y4 = max_y3;
            max_y3 = carre[i];
        }
        else if (carre[i].y <= max_y4.y){
            max_y2 = carre[i];
        }
    }

    if (max_y.x <= max_y2.x){
        carre[0] = max_y;
        carre[1] = max_y2;
    }
    else {
        carre[0] = max_y2;
        carre[1] = max_y;
    }

    if (max_y3.x <= max_y4.x){
        carre[2] = max_y4;
        carre[3] = max_y3;
    }
    else {
        carre[2] = max_y3;
        carre[3] = max_y4;
    }

    int width = std::max(carre[1].x-carre[0].x, carre[2].y-carre[1].y);

    try {
            ROI0Coord = carre[0]-cv::Point(width*0.2/4, width*0.2/4);
            ROI1Coord = carre[1]-cv::Point(width*0.2*3/4, width*0.2/4);
            ROI2Coord = carre[2]-cv::Point(width*0.2*3/4, width*0.2*3/4);
            ROI3Coord = carre[3]-cv::Point(width*0.2/4, width*0.2*3/4);

            ROI0Size = cv::Point(carre[0].x + width*0.2, carre[0].y + width*0.2)-cv::Point(width*0.2/4, width*0.2/4);
            ROI1Size = cv::Point(carre[1].x + width*0.2, carre[1].y + width*0.2)-cv::Point(width*0.2*3/4, width*0.2/4);
            ROI2Size = cv::Point(carre[2].x + width*0.2, carre[2].y + width*0.2)-cv::Point(width*0.2*3/4, width*0.2*3/4);
            ROI3Size = cv::Point(carre[3].x + width*0.2, carre[3].y + width*0.2)-cv::Point(width*0.2/4, width*0.2*3/4);

            ROI0 = vierge(Rect(ROI0Coord, ROI0Size));
            ROI1 = vierge(Rect(ROI1Coord, ROI1Size));    
            ROI2 = vierge(Rect(ROI2Coord, ROI2Size));
            ROI3 = vierge(Rect(ROI3Coord, ROI3Size));

            imshow("test", frame);
            destroyAllWindows();
    }
    catch (exception& e) {
        std::cout << "Can't build ROI" << std::endl;
        return false;
    }

    //cv::rectangle(frame, carre[0]-cv::Point(width*0.2/4, width*0.2/4), cv::Point(carre[0].x + width*0.2, carre[0].y + width*0.2)-cv::Point(width*0.2/4, width*0.2/4), cv::Scalar(255,0,255),3);
    //cv::rectangle(frame, carre[1]-cv::Point(width*0.2*3/4, width*0.2/4), cv::Point(carre[1].x + width*0.2, carre[1].y + width*0.2)-cv::Point(width*0.2*3/4, width*0.2/4), cv::Scalar(255,0,255),3);
    //cv::rectangle(frame, carre[2]-cv::Point(width*0.2*3/4, width*0.2*3/4), cv::Point(carre[2].x + width*0.2, carre[2].y + width*0.2)-cv::Point(width*0.2*3/4, width*0.2*3/4), cv::Scalar(255,0,255),3);
    //cv::rectangle(frame, carre[3]-cv::Point(width*0.2/4, width*0.2*3/4), cv::Point(carre[3].x + width*0.2, carre[3].y + width*0.2)-cv::Point(width*0.2/4, width*0.2*3/4), cv::Scalar(255,0,255),3);

    std::cout << "salut" << std::endl;

    return true;
}

void OpenCV_App::trackingSymbols(cv::Mat image, cv::Ptr<cv::Tracker>& tracker) {
    cv::Rect rect(ROI0Coord, ROI0Size);
    tracker->update(image, rect);
    std::cout << rect.x << " - "<<  rect.y << std::endl;
}

cv::Mat& OpenCV_App::getCap(void) {
    capture.read(current_frame);
    return current_frame;
}

bool OpenCV_App::is_Capture_Open() {
    return capture.isOpened();
}