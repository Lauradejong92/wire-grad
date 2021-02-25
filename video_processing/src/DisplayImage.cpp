#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(int argc, char** argv )
{
	printf("check \n");
    if ( argc != 2 )
    {
        Mat im2;
        im2 = imread( "/home/laura/Pictures/vangoghmuseum-s0176V1962-800.jpg", 1 );
        namedWindow("Disp Image", WINDOW_AUTOSIZE );
        imshow("Disp Image", im2);
        waitKey(0);
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }
    Mat image;
    image = imread( argv[1], 1 );
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);
    waitKey(0);
    return 0;
}
