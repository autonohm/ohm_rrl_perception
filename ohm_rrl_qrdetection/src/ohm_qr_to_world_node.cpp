/*
 * ohm_qr_to_world_node.cpp
 *
 *  Created on: Mar 25, 2015
 *      Author: jon
 */

#include <ros/ros.h>
#include "QrCodeToWorldRealSense.h"

int main(int argc, char** argv)
{
  ros::init(argc, argv, "oh_qr_to_world_node");

  QrCodeToWorldRealSense qrCodeToWorldRealSense;

//  qrCodeToWorld.run();
  ros::spin();
}
