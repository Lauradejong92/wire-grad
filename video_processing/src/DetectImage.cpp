#include <iostream>
#include <math.h>
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>

#include "wire_msgs/WorldEvidence.h"
#include "wire_msgs/ObjectEvidence.h"
#include "problib/conversions.h"

using namespace std;
using namespace cv;

ros::Publisher world_evidence_publisher_;

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
    waitKey(10);
}

void storingData(vector<Vec3f> y, vector<Vec3f> r,vector<Vec3f> b,vector<Vec3f> z){
    static int countycount=0;
    std::ofstream myfile;
    myfile.open("/home/laura/Documents/Data_collection/soccer.m", std::ios::app);
    myfile << "frame{"<< countycount<<"}=[";

    for(size_t i=0; i<y.size(); i++) {
        myfile<<"["<< y[i][0]<<", "<<y[i][1]<<", 1],\n";
    }
    for(size_t i=0; i<r.size(); i++) {
        myfile<<"["<< r[i][0]<<", "<<r[i][1]<<", 2],\n";
    }
    for(size_t i=0; i<b.size(); i++) {
        myfile<<"["<< b[i][0]<<", "<<b[i][1]<<", 3],\n";
    }
    for(size_t i=0; i<z.size(); i++) {
        myfile<<"["<< z[i][0]<<", "<<z[i][1]<<", 4],\n";
    }
    myfile<<"];\n";
    myfile.close();
    countycount++;
}

void addEvidence(wire_msgs::WorldEvidence& world_evidence, double x, double y, double z, const std::string& class_label, const std::string& color) {
    wire_msgs::ObjectEvidence obj_evidence;

    // Set the continuous position property
    wire_msgs::Property posProp;
    posProp.attribute = "position";

    // Set position (x,y,z), set the covariance matrix as 0.005*identity_matrix
    pbl::PDFtoMsg(pbl::Gaussian(pbl::Vector3(x, y, z), pbl::Matrix3(0.0005, 0.0005, 0.0005)), posProp.pdf);
    obj_evidence.properties.push_back(posProp);

    // Set the discrete class label property
    wire_msgs::Property classProp;
    classProp.attribute = "class_label";
    pbl::PMF classPMF;

    // Probability of the class label is 0.7
    classPMF.setProbability(class_label, 0.7);
    pbl::PDFtoMsg(classPMF, classProp.pdf);
    obj_evidence.properties.push_back(classProp);

    // Set the discrete color property with a probability of 0.9
    wire_msgs::Property colorProp;
    colorProp.attribute = "color";
    pbl::PMF colorPMF;

    // The probability of the detected color is 0.9
    colorPMF.setProbability(color, 0.1);
    pbl::PDFtoMsg(colorPMF, colorProp.pdf);
    obj_evidence.properties.push_back(colorProp);

    // Add all properties to the array
    world_evidence.object_evidence.push_back(obj_evidence);
}

void generateEvidence(vector<Vec3f> y, vector<Vec3f> r,vector<Vec3f> b,vector<Vec3f> z){
    // Create world evidence message
    wire_msgs::WorldEvidence world_evidence;

    // Set header
    world_evidence.header.stamp = ros::Time::now();
    world_evidence.header.frame_id = "/map";

    // Add evidence
    for(size_t i=0; i<y.size(); i++) {
        addEvidence(world_evidence, y[i][0], y[i][1], 0, "disc", "yellow");
    }
    for(size_t i=0; i<r.size(); i++) {
//        addEvidence(world_evidence, r[i][0], r[i][1], 0, "disc", "red");
    }
    for(size_t i=0; i<b.size(); i++) {
//        addEvidence(world_evidence, b[i][0], b[i][1], 0, "disc", "blue");
    }
    for(size_t i=0; i<z.size(); i++) {
//        addEvidence(world_evidence, z[i][0], z[i][1], 0, "disc", "black");
    }

    world_evidence_publisher_.publish(world_evidence);
    ROS_INFO("Published world evidence with size %d", world_evidence.object_evidence.size());
}

int main(int argc, char** argv )
{
//    ros::init(argc, argv, "talker");
//    ros::NodeHandle nh;
//    ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);

// Initialize ros and create node handle
    ros::init(argc,argv,"dummy_evidence_publisher");
    ros::NodeHandle nh;

    // Subscriber/publisher
    world_evidence_publisher_ = nh.advertise<wire_msgs::WorldEvidence>("/world_evidence", 100);

    //clear old storage file
    std::ofstream myfile;
    myfile.open("/home/laura/Documents/Data_collection/soccer.m");
    myfile << "";
    myfile.close();

    Scalar_<double> yellow= Scalar(110,225,250);
    Scalar_<double> red= Scalar(65,90,250);//70,80,250
    Scalar_<double> blue= Scalar(250,130,10);
    Scalar_<double> black= Scalar(20,20,20);

    VideoCapture cap("/home/laura/Pictures/testytest.mp4");


    //get the frames rate of the video
    double fps = cap.get(CAP_PROP_FPS);
    cout << "Frames per seconds : " << fps << endl;
    ros::Rate ros_r(fps);

    String window_name = "video";

    //namedWindow(window_name, WINDOW_NORMAL); //create a window

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
        //(image,y,r,b,z);
        //imshow(window_name, blueOnly);

        //storingData(y,r,b,z);
        generateEvidence(y,r,b,z);
        ros_r.sleep();


        //If any key is not pressed withing 10 ms, continue the loop
        if (waitKey(10) == 27)
        {
            cout << "Esc key is pressed by user. Stoppig the video" << endl;
            break;
        }
    }
    return 0;
}