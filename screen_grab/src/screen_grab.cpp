/*
 * Copyright (c) 2013 Lucas Walter
 * November 2013
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cv_bridge/cv_bridge.h>
#include <screen_grab/screen_grab.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>

// X Server includes
#include <X11/Xutil.h>

void XImage2RosImage(XImage& ximage, Display& _xDisplay, Screen& _xScreen, sensor_msgs::Image& im)
{
  im.header.stamp = ros::Time::now();

  if (_xScreen.depths->depth == 24)
  {
    const int wd = ximage.width;
    const int ht = ximage.height;
    const int frame_size = wd * ht * 4;
    im.width = wd;
    im.height = ht;
    im.step = im.width * 4;
    im.data.resize(frame_size);
    memcpy(&im.data[0], ximage.data, frame_size);
  }
  else
  {
    Colormap colmap = DefaultColormap(&_xDisplay, DefaultScreen(&_xDisplay));
    XColor color;
    for (unsigned int x = 0; x < ximage.width; x++)
    {
      for (unsigned int y = 0; y < ximage.height; y++)
      {
        color.pixel = XGetPixel(&ximage, x, y);
        XQueryColor(&_xDisplay, colmap, &color);
        // fallback omitted
      }
    }
  }
}

namespace screen_grab
{

ScreenGrab::ScreenGrab()
  : x_offset_(0), y_offset_(0), width_(640), height_(480), first_error_(false)
{
}

void ScreenGrab::roiCallback(const sensor_msgs::RegionOfInterest::ConstPtr& msg)
{
  x_offset_ = msg->x_offset;
  y_offset_ = msg->y_offset;
  width_ = msg->width;
  height_ = msg->height;
  updateConfig();
}

void ScreenGrab::checkRoi(int& x_offset, int& y_offset, int& width, int& height)
{
  if (width == 0) width = screen_w_;
  if (height == 0) height = screen_h_;
  if ((x_offset + width) > screen_w_)
    x_offset = std::max(0, screen_w_ - width);
  if ((y_offset + height) > screen_h_)
    y_offset = std::max(0, screen_h_ - height);
}

bool ScreenGrab::screenshotCallback(screen_grab::GetScreenshot::Request& req, screen_grab::GetScreenshot::Response& res)
{
  return grabRosImage(res.image);
}

void ScreenGrab::callback(screen_grab::ScreenGrabConfig& config, uint32_t level)
{
  if (level & 1)
  {
    checkRoi(config.x_offset, config.y_offset, config.width, config.height);
    x_offset_ = config.x_offset;
    y_offset_ = config.y_offset;
    width_ = config.width;
    height_ = config.height;
  }

  if (level & 2)
  {
    update_rate_ = config.update_rate;
  }

  if (level & 3)
  {
    publishing_enabled_ = config.publishing_enabled;
  }
}

void ScreenGrab::updateConfig()
{
  checkRoi(x_offset_, y_offset_, width_, height_);

  screen_grab::ScreenGrabConfig config;
  config.update_rate = update_rate_;
  config.publishing_enabled = publishing_enabled_;
  config.x_offset = x_offset_;
  config.y_offset = y_offset_;
  config.width = width_;
  config.height = height_;

  server_->updateConfig(config);
}

void ScreenGrab::onInit()
{
  ROS_INFO_STREAM("=== ScreenGrab::onInit() started ===");

  ROS_INFO_STREAM("Trying to connect to X display: " << getenv("DISPLAY"));
  display = XOpenDisplay(NULL);
  if (!display)
  {
    ROS_ERROR_STREAM("XOpenDisplay failed! DISPLAY=" << getenv("DISPLAY"));
    return;
  }
  ROS_INFO_STREAM("Connected to X display");

  screen = DefaultScreenOfDisplay(display);
  if (!screen)
  {
    ROS_ERROR_STREAM("DefaultScreenOfDisplay failed");
    return;
  }
  ROS_INFO_STREAM("Got default screen");

  Window wid = DefaultRootWindow(display);
  if (wid <= 0)
  {
    ROS_ERROR_STREAM("Failed to get root window");
    return;
  }

  XWindowAttributes xwAttr;
  Status ret = XGetWindowAttributes(display, wid, &xwAttr);
  screen_w_ = xwAttr.width;
  screen_h_ = xwAttr.height;

  ROS_INFO_STREAM("Screen dimensions: " << screen_w_ << "x" << screen_h_);

  double update_rate = 15;
  bool publishing_enabled = false;
  int x_offset = 0, y_offset = 0, width = 0, height = 0;

  ros::NodeHandle nh = getPrivateNodeHandle();
  nh.getParam("update_rate", update_rate);
  nh.getParam("publishing_enabled", publishing_enabled);
  nh.getParam("x_offset", x_offset);
  nh.getParam("y_offset", y_offset);
  nh.getParam("width", width);
  nh.getParam("height", height);

  update_rate_ = update_rate;
  publishing_enabled_ = publishing_enabled;
  x_offset_ = x_offset;
  y_offset_ = y_offset;
  width_ = width;
  height_ = height;

  nh.getParam("encoding", encoding_);
  ROS_INFO_STREAM("encoding: " << encoding_);

  server_.reset(new ReconfigureServer(dr_mutex_, nh));
  ReconfigureServer::CallbackType cbt = boost::bind(&ScreenGrab::callback, this, _1, _2);
  server_->setCallback(cbt);

  updateConfig();

  roi_sub_ = nh.subscribe("roi", 0, &ScreenGrab::roiCallback, this);

  ROS_INFO_STREAM("About to advertise get_screenshot service...");
  screenshot_service_ = nh.advertiseService("get_screenshot", &ScreenGrab::screenshotCallback, this);
  ROS_INFO_STREAM("Successfully advertised get_screenshot service");

  screen_pub_ = nh.advertise<sensor_msgs::Image>("image", 5);

  const float period = 1.0 / update_rate_;
  timer_ = nh.createTimer(ros::Duration(period), &ScreenGrab::spinOnce, this);
}

void ScreenGrab::spinOnce(const ros::TimerEvent&)
{
  if (!publishing_enabled_)
    return;

  sensor_msgs::ImagePtr im(new sensor_msgs::Image);
  if (grabRosImage(*im))
    screen_pub_.publish(im);
}

bool ScreenGrab::grabRosImage(sensor_msgs::Image& im)
{
  xImageSample = XGetImage(display, DefaultRootWindow(display),
                           x_offset_, y_offset_, width_, height_,
                           AllPlanes, ZPixmap);

  if (!xImageSample)
  {
    if (first_error_)
      ROS_ERROR_STREAM("Error taking screenshot! " << x_offset_ << " " << y_offset_
                       << " " << width_ << " " << height_ << " "
                       << screen_w_ << " " << screen_h_);
    first_error_ = false;
    return false;
  }

  if (!first_error_)
    ROS_INFO_STREAM("Captured image: " << width_ << "x" << height_);
  first_error_ = true;

  XImage2RosImage(*xImageSample, *display, *screen, im);
  XDestroyImage(xImageSample);
  im.encoding = encoding_;
  return true;
}

}  // namespace screen_grab

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(screen_grab::ScreenGrab, nodelet::Nodelet)
