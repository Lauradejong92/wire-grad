#include <iostream>
#include <math.h>
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

// Setup SimpleBlobDetector parameters.

Mat blackFilter(const Mat& src)
{
    Scalar_<double> black = Scalar (100,100,30);
    Scalar_<double> margin = Scalar (155,155,30);
    assert(src.type() == CV_8UC3);

    Mat3b hsv;
    cvtColor(src, hsv, COLOR_BGR2HSV);

    Mat blackOnly;
    //inRange(hsv, Scalar(35, 20, 20), Scalar(85, 255, 255), blueOnly);
    inRange(hsv, black-margin, black+margin,blackOnly);
    //r247g76b61
    return blackOnly;
}

Mat colorFilter(const Mat& src,Scalar_<double> color)
{
    Scalar_<double> margin = Scalar (40,40,40);

    assert(src.type() == CV_8UC3);
    Mat colorOnly;
    inRange(src, color-margin, color+margin, colorOnly);

    return colorOnly;
}

vector<Vec3f> detectCircles(Mat image)
{
    Mat img_blur;
    medianBlur(image, img_blur, 5);

    vector<Vec3f>  circles;
//    HoughCircles(img_blur, circles, HOUGH_GRADIENT, 1, image.rows/64, 200, 10, 25, 35);
    HoughCircles(img_blur, circles, HOUGH_GRADIENT, 1, image.rows/34, 50, 10, 17, 30);//mindist /64//22,35
    return circles;
}

void detectBlobs(Mat im)
{
    Mat image;
    //medianBlur(im, image, 5);

   bitwise_not(im, image);

// Setup SimpleBlobDetector parameters.
    SimpleBlobDetector::Params params;

    // Change thresholds
    params.minThreshold = 10;
    params.maxThreshold = 256;

    // Filter by Area.
    params.filterByArea = true;
    params.minArea = 100;

    // Filter by Circularity
    params.filterByCircularity = true;
    params.minCircularity = 0.05;

    // Filter by Convexity
    params.filterByConvexity = true;
    params.minConvexity = 0.87;

    // Filter by Inertia
    params.filterByInertia = false;
    params.minInertiaRatio = 0.1;


    // Storage for blobs
    vector<KeyPoint> keypoints;


#if CV_MAJOR_VERSION < 3   // If you are using OpenCV 2

    // Set up detector with params
	SimpleBlobDetector detector(params);

	// Detect blobs
	detector.detect( im, keypoints);
#else

    // Set up detector with params
    Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

    // Detect blobs
    detector->detect( image, keypoints);
#endif

    // Draw detected blobs as red circles.
    // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
    // the size of the circle corresponds to the size of blob

    Mat im_with_keypoints;
    drawKeypoints( image, keypoints, im_with_keypoints, Scalar(100,100,155), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    // Show blobs
    imshow("keypoints", im_with_keypoints );
    waitKey(3000);
    printf("Keypoints found: %lu \n",keypoints.size());
}


void plotCircles(Mat image, vector<Vec3f> y, vector<Vec3f> r,vector<Vec3f> b,vector<Vec3f> z )
{
    Scalar_<double> color=Scalar(0, 0, 0);

    for(size_t i=0; i<y.size(); i++) {
        Point center(cvRound(y[i][0]), cvRound(y[i][1]));
        int radius = cvRound(y[i][2]);
        circle(image, center, radius, color, 2, 8, 0);
    }

    for(size_t i=0; i<r.size(); i++) {
        Point center(cvRound(r[i][0]), cvRound(r[i][1]));
        int radius = cvRound(r[i][2]);
        circle(image, center, radius, color, 2, 8, 0);
    }

    for(size_t i=0; i<b.size(); i++) {
        Point center(cvRound(b[i][0]), cvRound(b[i][1]));
        int radius = cvRound(b[i][2]);
        circle(image, center, radius, color, 2, 8, 0);
    }

    for(size_t i=0; i<z.size(); i++) {
        Point center(cvRound(z[i][0]), cvRound(z[i][1]));
        int radius = cvRound(z[i][2]);
        circle(image, center, radius, color, 2, 8, 0);
    }

    imshow("image", image);
    waitKey(1000);
}


int main(int argc, char** argv )
{
    Scalar_<double> yellow= Scalar(110,225,250);
    Scalar_<double> red= Scalar(65,90,250);//70,80,250
    Scalar_<double> blue= Scalar(250,130,10);
    Scalar_<double> black= Scalar(20,20,20);

    VideoCapture cap("/home/laura/Pictures/testytest.mp4");


    //get the frames rate of the video
    double fps = cap.get(CAP_PROP_FPS);
    cout << "Frames per seconds : " << fps << endl;

    String window_name = "video";

    namedWindow(window_name, WINDOW_NORMAL); //create a window

    while (true)
    {
        Mat image;
        bool bSuccess = cap.read(image); // read a new frame from video

        //Breaking the while loop at the end of the video
        if (bSuccess == false)
        {
            cout << "Found the end of the video" << endl;
            break;
        }

        ////Processing:
            // Kleuren:
        Mat yellowOnly = colorFilter(image, yellow);
        Mat redOnly = colorFilter(image, red);
        Mat blueOnly = colorFilter(image, blue);
        Mat blackOnly = colorFilter(image, black);//blackFilter(image);

            // Detect circles:
        vector<Vec3f> y = detectCircles(yellowOnly);
        //printf("yellow circles found: %lu \n",y.size());
        vector<Vec3f> r =detectCircles(redOnly);
        //printf("red circles found: %lu \n",r.size());
        vector<Vec3f> b =detectCircles(blueOnly);
        //printf("blue circles found: %lu \n",b.size());
        vector<Vec3f> z =detectCircles(blackOnly);
        //printf("black circles found: %lu \n",z.size());

        //show the frame in the created window
        plotCircles(image,y,r,b,z);
        //imshow(window_name, blueOnly);

        //wait for for 10 ms until any key is pressed.
        //If the 'Esc' key is pressed, break the while loop.
        //If the any other key is pressed, continue the loop
        //If any key is not pressed withing 10 ms, continue the loop
        if (waitKey(10) == 27)
        {
            cout << "Esc key is pressed by user. Stoppig the video" << endl;
            break;
        }
    }

    return 0;


    String folderpath = "/home/laura/Pictures/Soccer/*.png";
    vector<String> filenames;
    cv::glob(folderpath, filenames);

//    for (size_t i=0; i<filenames.size(); i++)
//    {
//        printf("----------\n Image %lu \n",i+1);
//
//        ////Basis:
//        Mat image = imread(filenames[i]);
//        //imshow("image", image);
//        //waitKey(1000);
//
//        //// Kleuren:
////        Mat yellowOnly = colorFilter(image, yellow);
////        Mat redOnly = colorFilter(image, red);
////        Mat blueOnly = colorFilter(image, blue);
////        Mat blackOnly = colorFilter(image, black);//blackFilter(image);
////    imshow("blackOnly", blackOnly);
////    waitKey(3000);
//
//        //// Detect blobs:
////        detectBlobs(yellowOnly);
////        detectBlobs(redOnly);
////        detectBlobs(blueOnly);
////        detectBlobs(blackOnly);
//
//        //// Detect circles:
//        //detectCirlces(image);
////        vector<Vec3f> y = detectCircles(yellowOnly);
////        printf("yellow circles found: %lu \n",y.size());
////        vector<Vec3f> r =detectCircles(redOnly);
////        printf("red circles found: %lu \n",r.size());
////        vector<Vec3f> b =detectCircles(blueOnly);
////        printf("blue circles found: %lu \n",b.size());
////        vector<Vec3f> z =detectCircles(blackOnly);
////        printf("black circles found: %lu \n",z.size());
////
////        plotCircles(image,y,r,b,z);
//    }

    // detect squares after filtering...
    printf("done\n");

    return 0;
}
