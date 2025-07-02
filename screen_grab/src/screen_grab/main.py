#!/usr/bin/env python3

import rospy
import subprocess
import os
import cv2
from sensor_msgs.msg import Image
from screen_grab.srv import GetScreenshot, GetScreenshotResponse
from cv_bridge import CvBridge

TEMP_SCREENSHOT_PATH = "/tmp/screenshot.jpg"

def take_screenshot():
    """
    Takes a fresh screenshot using scrot -o and returns it as OpenCV BGR image.
    """
    os.environ["DISPLAY"] = ":0"

    # Remove old screenshot if it exists
    if os.path.exists(TEMP_SCREENSHOT_PATH):
        os.remove(TEMP_SCREENSHOT_PATH)

    # Run scrot with -o to overwrite screenshot.jpg
    result = subprocess.run(
        ["scrot", "-o", TEMP_SCREENSHOT_PATH],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    if result.returncode != 0:
        raise RuntimeError(f"scrot failed: {result.stderr.decode().strip()}")

    if not os.path.exists(TEMP_SCREENSHOT_PATH):
        raise RuntimeError("screenshot file was not created.")

    image = cv2.imread(TEMP_SCREENSHOT_PATH)
    if image is None:
        raise RuntimeError("Image file couldn't be read by OpenCV.")

    return image

def handle_get_screenshot(req):
    rospy.loginfo("Handling screenshot request...")
    bridge = CvBridge()
    response = GetScreenshotResponse()

    try:
        bgr_image = take_screenshot()
        ros_image = bridge.cv2_to_imgmsg(bgr_image, encoding="bgr8")
        response.image = ros_image
        rospy.loginfo("Screenshot captured and returned successfully.")
    except Exception as e:
        rospy.logerr(f"Failed to capture screenshot: {e}")

    return response

def main():
    rospy.init_node("screen_grab_service_node", anonymous=False)
    rospy.Service("/screen_grab/get_screenshot", GetScreenshot, handle_get_screenshot)
    rospy.loginfo("ROS service [/screen_grab/get_screenshot] is ready.")
    rospy.spin()

if __name__ == '__main__':
    main()
