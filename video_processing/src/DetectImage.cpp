#include <iostream>
#include <math.h>
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

Mat blackFilter(const Mat& src)
{
    Scalar_<double> black = Scalar (100,40,40);
    Scalar_<double> margin = Scalar (40,40,40);
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
    HoughCircles(img_blur, circles, HOUGH_GRADIENT, 1, image.rows/64, 50, 10, 22, 35);

    //printf("circles found: %i \n",circles.size());
//    for(size_t i=0; i<circles.size(); i++) {
//        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
//        int radius = cvRound(circles[i][2]);
//        circle(image, center, radius, Scalar(150, 150, 250), 2, 8, 0);
//        //printf("center at: (%f, %f) \n",circles[i][0],circles[i][1]);
//    }
//


    return circles;
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

    String folderpath = "/home/laura/Pictures/Soccer/*.png";
    vector<String> filenames;
    cv::glob(folderpath, filenames);

    for (size_t i=0; i<filenames.size(); i++)
    {
        printf("----------\n Image %lu \n",i+1);

        ////Basis:
        Mat image = imread(filenames[i]);
        imshow("image", image);
        waitKey(1000);

        //// Kleuren:
        Mat yellowOnly = colorFilter(image, yellow);
        Mat redOnly = colorFilter(image, red);
        Mat blueOnly = colorFilter(image, blue);
        Mat blackOnly = colorFilter(image, black);//blackFilter(image);
//    imshow("blackOnly", blackOnly);
//    waitKey(3000);

        //// Detect circles:
        //detectCirlces(image);
        vector<Vec3f> y = detectCircles(yellowOnly);
        printf("yellow circles found: %lu \n",y.size());

        vector<Vec3f> r =detectCircles(redOnly);
        printf("red circles found: %lu \n",r.size());
        vector<Vec3f> b =detectCircles(blueOnly);
        printf("blue circles found: %lu \n",b.size());
        vector<Vec3f> z =detectCircles(blackOnly);
        printf("black circles found: %lu \n",z.size());

        plotCircles(image,y,r,b,z);
    }

    // detect squares after filtering...
    printf("done\n");

    return 0;
}
