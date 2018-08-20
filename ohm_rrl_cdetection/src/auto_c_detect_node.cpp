﻿/**
 * @file   auto_thermal_c_detect_node.cpp
 * @author Johanna Gleichauf
 * @date   Stand 01.06.2018
 *
 *
 */
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>

using namespace cv;
using namespace std;

// Struct for C-Template
struct C_template{
  string name;
  Mat src;
  std::string filename;
  Mat gray;
  unsigned int kernel_size = 1;
  Mat canny;
  vector<vector<Point>> contours;
  vector<Moments> mu;

};



// Create c_templates vector
std::vector<C_template> cs;


Mat src_unten;
Mat src_unten_gray;
Mat src_links;
Mat src_links_gray;
Mat src_rechts;
Mat src_rechts_gray;
Mat src_oben;
Mat src_oben_gray;
Mat src_los;
Mat src_los_gray;
Mat src_lus;
Mat src_lus_gray;
Mat src_ros;
Mat src_ros_gray;
Mat src_rus;
Mat src_rus_gray;
Mat img;
Mat img_gray;
int thresh = 100;
int max_thresh = 255;

bool u_min = false, o_min = false, r_min = false, l_min = false, ro_min = false, ru_min = false, lo_min = false,
    lu_min = false;
bool u_mid = false, o_mid = false, r_mid = false, l_mid = false, ro_mid = false, ru_mid = false, lo_mid = false,
    lu_mid = false;
double max_area = 0;
double min_area = 10000;
double mid_area;

namespace enc = sensor_msgs::image_encodings;

std::string g_path_to_template;

vector<vector<Point> > g_detected_contours;

image_transport::Publisher pubContours;
//image_transport::Publisher pubGray;
image_transport::Publisher pubDetected;

RNG rng(12345);

/// Function header
void thresh_callback(int, void*);



cv::Mat _Input;
cv::Mat _InputM;
cv::Mat thermo_bin;
cv_bridge::CvImagePtr frame;




cv::Mat loadSourceImage(cv::Mat& src, std::string filename) //TODO: src is not written into c_template
{
  /// Load source image
  std::string img_path = g_path_to_template + filename;
//  std::cout << "filename " << filename << std::endl;
  std::cout << "img_path: " << img_path << std::endl;
  src = imread(img_path, 1);
  if(!src.data)                              // Check for invalid input
  {
    cout << "Could not open or find the image " << img_path << std::endl;
  }
  return src;
}


C_template loadSourceImage(C_template c_template)
{
  cv::Mat image;
  C_template c_temp_filled;
  c_temp_filled = c_template;
  image = loadSourceImage(c_template.src, c_template.filename);
  std::cout << "size c_template " << image.size() << std::endl;
  c_temp_filled.src = image;
  std::cout << "name c_template_filled " << c_temp_filled.name << std::endl;
  std::cout << "size c_template filled " << c_temp_filled.src.cols << std::endl;
  return c_temp_filled;
}


//std::vector<C_template> loadSourceImages(std::vector<C_template> cs)
//{
//  std::vector<C_template> c;
//  std::cout << "before whole template is filled " << std::endl;
//  std::cout << "size cs " << cs.size() << std::endl;
//  for(unsigned int i=0; i< cs.size(); i++)
//  {
//    std::cout << "within for " << i << std::endl;
//    c[i] = loadSourceImage(cs[i]);
//    std::cout << "size c 0 " << c[0].src.size() << std::endl;
//  }
//  return c;
//}



bool templateIsSimilar(C_template c, cv::Moments mu)
{

//    if((    abs(mu.nu20 - c.mu[0].nu20) < 0.007) && (abs(mu[i].nu11 - cs[k].mu[j].nu11) < 0.007)
//        && (abs(mu.nu02 - c.mu.nu02) < 0.007) && (abs(mu[i].nu30 - cs[k].mu[j].nu30) < 0.007)
//        && (abs(mu.nu21 - c.mu.nu21) < 0.007) && (abs(mu[i].nu12 - cs[k].mu[j].nu12) < 0.007)
//        && (abs(mu.nu03 - c.mu.nu03) < 0.007))
          return true;


          ///@todo maybe replaces the momentIsSimilar function
  return false;
}


void calculateMoments(std::vector<C_template> cs)
{
  std::cout << "within calculateMoments v1 " << std::endl;
  std::cout << "content cs[0] " << cs[0].src.size() << std::endl;

  // Get moments for all C-Templates
  for(unsigned int i=0; i<cs.size(); i++)
  {
    std::cout << "within first for loop " << std::endl;
    for(unsigned int j = 0; j < cs[i].contours.size(); j++)
        cs[i].mu.push_back(cv::moments(cs[i].contours[j], false));

  }
  std::cout << "end of calculateMoments " << std::endl;
}

std::vector<cv::Moments> calculateMoments(vector<vector<Point>> contours)
{
  std::cout << "within caculateMoments v2 " << std::endl;
  std::vector<cv::Moments> mu;

  for(unsigned int i = 0; i < contours.size(); i++)
  {
    std::cout << "within for loop v2 " << std::endl;
    mu.push_back(cv::moments(contours[i], false));
  }
  std::cout << "after for loop v2 " << std::endl;
  return mu;
}


bool momentIsSimilar(C_template c, std::vector<cv::Moments> mu, float th = 0.007)
{

  std::cout << "Within momentIsSimilar " << std::endl;
  for(const auto m : mu)
  {
    for(unsigned int i=0 ; i<c.contours.size() ; ++i)
    {
      if(abs(m.nu20 - c.mu[i].nu20) < th) return true;
    }
  }

  /// @todo find solution for similarity and make a function
  return false;
}


//callback Intel Realsense camera data
void callCam(const sensor_msgs::ImageConstPtr& camImage)
{
  std::cout << "within camera callback " << std::endl;
  try
  {
    frame = cv_bridge::toCvCopy(camImage, camImage->encoding);
  }
  catch(cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
  }

  if((frame->image.rows == 0))
    std::cout << "No data" << std::endl;
//  else
//    std::cout << "Data available" << std::endl;

  //Convert _Input to gray
  //RGB Image
  std::cout << "before cvtColor " << std::endl;
  cvtColor(frame->image, img_gray, CV_RGB2GRAY);
  std::cout << "after cvtColor " << std::endl;
  blur(img_gray, img_gray, Size(3, 3));

  constexpr unsigned int kernel_size = 1;

  thresh_callback(0, 0);
}
;

/** @function main */
int main(int argc, char** argv)
{
  ros::init(argc, argv, "camera");
  ros::NodeHandle nh;
  ros::NodeHandle private_nh("~");

  image_transport::ImageTransport it(nh);
  sensor_msgs::ImagePtr msg;



  private_nh.param("path_to_template", g_path_to_template, std::string("/home/johanna/catkin/src/ohm_rrl_perception/ohm_rrl_cdetection/images/"));

  if(g_path_to_template.length() == 0)
  {
    ROS_ERROR("path for templates not set. I will exit");
    exit(1);
  }

  //Subscriber Realsense D415
  image_transport::Subscriber imgSub = it.subscribe("/usb_cam/image_raw", 1, callCam); //"/gripper/color/image_raw"
//  pubGray = it.advertise("/gripper/image_gray", 1);
  pubContours = it.advertise("/gripper/image_contours", 1);
//  pubDetected = it.advertise("/gripper/detected_contours", 1);


  //Fill c-template vector
  for(unsigned int i=0; i<=7; i++)
    cs.push_back(C_template());

  // Assignment C Templates Names
  cs[0].name = "bottom";
  cs[1].name = "top";
  cs[2].name = "left";
  cs[3].name = "right";
  cs[4].name = "bottom_right";
  cs[5].name = "bottom_left";
  cs[6].name = "top_right";
  cs[7].name = "top_left";

  // Assignment C Templates filenames
  cs[0].filename = "C_gross_unten.jpg" ;
  cs[1].filename = "C_gross_oben.jpg" ;
  cs[2].filename = "C_gross_links.jpg" ;
  cs[3].filename = "C_mittel_rechts.jpg" ;
  cs[4].filename = "C_gross_rechts_unten.jpg" ;
  cs[5].filename = "C_gross_links_unten.jpg" ;
  cs[6].filename = "C_gross_rechts_oben.jpg" ;
  cs[7].filename = "C_gross_links_oben.jpg" ;

  std::cout << "before templates are loaded " << std::endl;

  //Convert C-Template image and blur it
  for(unsigned int j=0; j<cs.size(); j++)
  {
    //Load templates
    cs[j] = loadSourceImage(cs[j]);
    std::cout << "size cs.src " << cs[j].src.size() << std::endl;
    std::cout << "before cvtColor " << j << std::endl;
    cvtColor(cs[j].src, cs[j].gray, CV_BGR2GRAY);
    std::cout << "after cvtColor " << j << std::endl;
    blur(cs[j].gray, cs[j].gray, Size(cs[j].kernel_size, cs[j].kernel_size));
    std::cout << "after blur " << j << std::endl;
  }

 std::cout << "templates loaded and blurred " << std::endl;


  ros::spin();
}

/** @function thresh_callback */
void thresh_callback(int, void*)
{
  std::cout << "within callback thresh " << std::endl;
  Mat canny_output;

  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  //Edge detection and find contours for all C-Templates
  for(unsigned int k=0; k<cs.size(); k++)
  {
    std::cout << "within edge detection " << std::endl;
    Canny(cs[k].gray, cs[k].canny, thresh, thresh * 2 , 3);
    findContours(cs[k].canny, cs[k].contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
  }

  //RGB Image
  Canny(img_gray, canny_output, thresh, thresh * 2, 3);
  findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

  std::cout << "before calculation of moments " << std::endl;

  std::cout << "content cs[0] " << cs[0].src.size() << std::endl;

  // Get the moments
  calculateMoments(cs);
  //Moments incoming image camera
  vector<Moments> mu = calculateMoments(contours);

  g_detected_contours.clear();



// detectValidCs(contours, cs, )
  ///@todo make own function of for loop
  for(unsigned int i = 0; i < contours.size(); i++)
  {
    for(unsigned int k = 0 ; k < cs.size() ; ++k)
    {
    for(unsigned int j = 0; j < cs[k].contours.size(); j++)
//      if((    abs(mu[i].nu20 - cs[k].mu[j].nu20) < 0.007) && (abs(mu[i].nu11 - cs[k].mu[j].nu11) < 0.007)
//          && (abs(mu[i].nu02 - cs[k].mu[j].nu02) < 0.007) && (abs(mu[i].nu30 - cs[k].mu[j].nu30) < 0.007)
//          && (abs(mu[i].nu21 - cs[k].mu[j].nu21) < 0.007) && (abs(mu[i].nu12 - cs[k].mu[j].nu12) < 0.007)
//          && (abs(mu[i].nu03 - cs[k].mu[j].nu03) < 0.007))
      if(momentIsSimilar(cs[k], mu)) g_detected_contours.push_back(contours[i]);  //TODO: check Code with actual C's
    }
  }


  ///  Get the mass centers:
  /// @todo make own function
  vector<Point2f> mc(contours.size());
  for(int i = 0; i < contours.size(); i++)
  {
    mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
  }

  /// Draw contours
  ///
  /// @todo make own function for drawing
  /// draw()
  ///   in draw()
  ///      drawCicle()
  ///      drawContours()
  ///      drawTest()
  Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
  const Scalar color = Scalar(0, 0, 255);
  for(int i = 0; i < contours.size(); i++)
  {
//    std::cout << "contours " << contours[i].size() << std::endl;
    if((contours[i].size() < 30)&&(contours[i].size() > 7))
    {
//      std::cout << "contours " << contours[i].size() << std::endl;
      continue;
    }

//     drawContours( drawing, contours,   i, color, 2, 8, hierarchy, 0, Point() );
//     circle( drawing, mc[i], 4, color, -1, 8, 0 );
  }


  //Find cirle around contour
  double cx, cy = 0.0;
  float radius  = 0.0;
  Point2f center;
  Point2f center_old;
//  Mat circle;
  Mat canny_circle;
  Mat canny_diff;
  Mat canny_roi;
  vector<vector<Point> > contour_circle;
  vector<vector<Point> > contour_diff;
  vector<vector<Point> > contour_roi;
  const Scalar color_diff = Scalar(0, 255, 0);
  Mat C_area;
  Mat gap;

  for(unsigned int i = 0; i < g_detected_contours.size(); i++)
  {
    minEnclosingCircle(g_detected_contours.at(i), center, radius);

//    std::cout << "radius " << radius << std::endl;
//    if((radius < 50) &&(radius > 5))
//    {
////      std::cout << "radius " << radius << std::endl;
//      continue;
//    }

//    if(abs(center_old.x - center.x) > 10)
//     break;
//    else

      Rect r(center.x - radius, center.y - radius, radius * 2, radius * 2);
      circle(frame->image, center, radius, (0, 0, 255), 2);

//    std::cout << "Center " << center << std::endl;
//    std::cout << "Radius " << radius << std::endl;

    center_old = center;
  }
//  drawContours(_Input, contour_roi, -1, Scalar(0,0,255), CV_FILLED);
  drawContours(frame->image, g_detected_contours, -1, Scalar(0,255,0), CV_FILLED);
//  imshow("DetectedContours", _Input);
  sensor_msgs::ImagePtr msg_detected = cv_bridge::CvImage(std_msgs::Header(), frame->encoding, frame->image).toImageMsg();
  pubContours.publish(msg_detected);
}
