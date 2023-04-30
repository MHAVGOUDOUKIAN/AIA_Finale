#include <OpenCV_Main.hpp>
#include <OpenGL_App.hpp>

using namespace cv;
using namespace std;

OpenCV_App::OpenCV_App(OpenGL_App* tab) {
    capture = cv::VideoCapture(0);
    tracker0 = cv::TrackerKCF::create();
    tracker1 = cv::TrackerKCF::create();
    tracker2 = cv::TrackerKCF::create();
    tracker3 = cv::TrackerKCF::create();

    app=tab;

    index_carre_choosed=0;
    end_init_phase = false;
}

// Update va renvoyer le cv::Mat avec toute les informations en utilisant
// 'current-frame'
cv::Mat& OpenCV_App::update(cv::Mat* glViewMatrix) {
    liste_carre.clear();
    frame = getCap();
    vierge = frame;

    find_squares(frame, &liste_carre);

    if(app_status == Status::INITIALIZATION) {
        find_squares(frame, &liste_carre);
        frame = debugSquares(liste_carre, frame, index_carre_choosed);
        
        if(end_init_phase){
            if(findROI(frame, vierge, liste_carre[index_carre_choosed])) {
                end_init_phase=false; 
                tracker0->init(vierge, Rect(ROI0Coord, ROI0Size));
                tracker1->init(vierge, Rect(ROI1Coord, ROI1Size));
                tracker2->init(vierge, Rect(ROI2Coord, ROI2Size));
                tracker3->init(vierge, Rect(ROI3Coord, ROI3Size));
                app_status = Status::RUNNING;
                std::cout << "Mode: RUNNING" << std::endl;
            }
        }    
        return frame;        
    } else {
        try {
            cv::Rect rect0(0,0,0,0);
            cv::Rect rect1(0,0,0,0);
            cv::Rect rect2(0,0,0,0);
            cv::Rect rect3(0,0,0,0);
            if (tracker0->update(vierge, rect0) && tracker1->update(vierge, rect1) && tracker2->update(vierge, rect2) && tracker3->update(vierge, rect3)){
                cv::Mat resBoundingBox0 = vierge(rect0);
                cv::Mat resBoundingBox1 = vierge(rect1);
                cv::Mat resBoundingBox2 = vierge(rect2);
                cv::Mat resBoundingBox3 = vierge(rect3);
                
                line(vierge, cv::Point2i(rect0.x+rect0.width/2, rect0.y+rect0.height/2), cv::Point2i(rect0.x+rect0.width/2, rect0.y+rect0.height/2), cv::Scalar(255,0,255),5);
                line(vierge, cv::Point2i(rect1.x+rect1.width/2, rect1.y+rect1.height/2), cv::Point2i(rect1.x+rect1.width/2, rect1.y+rect1.height/2), cv::Scalar(255,0,255),5);
                line(vierge, cv::Point2i(rect2.x+rect2.width/2, rect2.y+rect2.height/2), cv::Point2i(rect2.x+rect2.width/2, rect2.y+rect2.height/2), cv::Scalar(255,0,255),5);
                line(vierge, cv::Point2i(rect3.x+rect3.width/2, rect3.y+rect3.height/2), cv::Point2i(rect3.x+rect3.width/2, rect3.y+rect3.height/2), cv::Scalar(255,0,255),5);
            
                cv::Mat rotation_vector;
                cv::Mat translation_vector;

                Point2d center = cv::Point2d(vierge.cols/2,vierge.rows/2);
                cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << vierge.cols, 0, center.x, 0, vierge.rows, center.y, 0, 0, 1);

                std::vector<cv::Point3d> model_points;
                model_points.push_back(cv::Point3d(-1,-1,0));
                model_points.push_back(cv::Point3d(-1,1,0));
                model_points.push_back(cv::Point3d(1,1,0));
                model_points.push_back(cv::Point3d(1,-1,0));

                std::vector<cv::Point2d> image_points;
                image_points.push_back(cv::Point2d(rect0.x+rect0.width/2, rect0.y+rect0.height/2));
                image_points.push_back(cv::Point2d(rect1.x+rect1.width/2, rect1.y+rect1.height/2));
                image_points.push_back(cv::Point2d(rect2.x+rect2.width/2, rect2.y+rect2.height/2));
                image_points.push_back(cv::Point2d(rect3.x+rect3.width/2, rect3.y+rect3.height/2));

                // Find matrix of homographie 
                cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type);
                cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);
            
                std::cout << rotation_vector << std::endl;
                std::cout << translation_vector << std::endl;
                cv::Mat rotation_matrix;
                cv::Rodrigues(rotation_vector, rotation_matrix);

                cv::Mat viewMatrix = cv::Mat::eye(4, 4, CV_64FC1);
                rotation_matrix.copyTo(viewMatrix(cv::Rect(0, 0, 3, 3)));
                translation_vector.copyTo(viewMatrix(cv::Rect(3,0,1,3)));

                cv::Mat cvToGl = cv::Mat::eye(4, 4, CV_64F);
                cvToGl.at<double>(1, 1) = -1.0f; // Invert the y axis
                viewMatrix = cvToGl * viewMatrix;
                // Transposée

                cv::transpose(viewMatrix , *glViewMatrix);
            }
            else{
                //std::cout << "Lost tracking" << std::endl;
            }
        }
        catch (exception& e)
        {
            // std::cout << "Lost tracking" << std::endl;
        }

        return frame;
    }
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

    cv::Mat noirblanc;
        cvtColor(vierge, noirblanc, cv::COLOR_BGR2GRAY);
        int x_max = max(carre[1].x,carre[2].x);
        int y_max = max(carre[2].y,carre[3].y);
        int x_min = min(carre[0].x,carre[3].x);
        int y_min = min(carre[0].y,carre[1].y);
        cv::Mat mur(y_max - y_min, x_max-x_min, CV_8U);


    try {
        int compteur = 0;

        for (int y=0; y < y_max-y_min; y++){
            for (int x=0; x < x_max - x_min; x++){
                if (static_cast<int>(noirblanc.at<uchar>(cv::Point(x+x_min,y + y_min))) > 127){
                    mur.at<uchar>(cv::Point(x,y)) = static_cast<uchar>(255);
                }
                else{
                    mur.at<uchar>(cv::Point(x,y)) = static_cast<uchar>(0);
                    if (x < x_max - x_min - 1 && y < y_max-y_min - 1){
                        if (static_cast<int>(noirblanc.at<uchar>(cv::Point(x+x_min+1,y + y_min+1))) < 127){
                            if (static_cast<int>(noirblanc.at<uchar>(cv::Point(x+x_min+1,y + y_min))) < 127){
                                app->set_tabl_value(compteur,(double)(x+x_min)/(SCR_WIDTH/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,(double)(y + y_min)/(SCR_HEIGHT/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,0.85);
                                compteur++;
                                app->set_tabl_value(compteur,0);
                                compteur++;
                                app->set_tabl_value(compteur,0);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(x+x_min+1))/(SCR_WIDTH/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(y + y_min))/(SCR_HEIGHT/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,0.85);
                                compteur++;
                                app->set_tabl_value(compteur,1);
                                compteur++;
                                app->set_tabl_value(compteur,0);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(x+x_min+1))/(SCR_WIDTH/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(y + y_min+1))/(SCR_HEIGHT/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,0.85);
                                compteur++;
                                app->set_tabl_value(compteur,1);
                                compteur++;
                                app->set_tabl_value(compteur,1);
                                compteur++;
                            }
                            if (static_cast<int>(noirblanc.at<uchar>(cv::Point(x+x_min,y + y_min+1))) < 127){
                                app->set_tabl_value(compteur,((double)(x+x_min))/(SCR_WIDTH/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(y + y_min))/(SCR_HEIGHT/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,0.85);
                                compteur++;
                                app->set_tabl_value(compteur,0);
                                compteur++;
                                app->set_tabl_value(compteur, 0);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(x+x_min))/(SCR_WIDTH/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(y + y_min+1))/(SCR_HEIGHT/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,0.85);
                                compteur++;
                                app->set_tabl_value(compteur,0);
                                compteur++;
                                app->set_tabl_value(compteur,1);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(x+x_min+1))/(SCR_WIDTH/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,((double)(y + y_min+1))/(SCR_HEIGHT/2)-1);
                                compteur++;
                                app->set_tabl_value(compteur,0.85);
                                compteur++;
                                app->set_tabl_value(compteur,1);
                                compteur++;
                                app->set_tabl_value(compteur,1);
                                compteur++;
                            }
                        }
                    }
                }
            }
            std::cout << compteur << std::endl;
        }
        app->build_laby();
        app->tabl_size = compteur;

        for(int i=0; i < compteur/5; i+=5) {
            std::cout << app->get_tabl_value(i) << "," 
            << app->get_tabl_value(i+1) << ","
            << app->get_tabl_value(i+2) << ","
            << app->get_tabl_value(i+3) << ","
            << app->get_tabl_value(i+4) << ";"<< std::endl;
        }

        std::cout << compteur << std::endl;

    } catch (exception& e) {
        std::cout << "Can't build labyrinthe" << std::endl;
        return false;
    }

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
    }
    catch (exception& e) {
        std::cout << "Can't build ROI" << std::endl;
        return false;
    }

    return true;
}

void OpenCV_App::trackingSymbols(cv::Mat image, cv::Ptr<cv::Tracker>& tracker) {
    cv::Rect rect(ROI0Coord, ROI0Size);
    tracker->update(image, rect);
    std::cout << rect.x << " - "<<  rect.y << std::endl;
}

//N'est utilisé que par la fonction update()
cv::Mat& OpenCV_App::getCap(void) {
    capture.read(current_frame);
    return current_frame;
}

bool OpenCV_App::is_Capture_Open() {
    return capture.isOpened();
}

