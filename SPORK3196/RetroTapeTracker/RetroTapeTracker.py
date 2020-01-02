import libjevois as jevois
import cv2
import numpy as np

## Tracks the center of a retro-reflective tape target
#
# Add some description of your module here.
#
# @author 
# 
# @videomapping YUYV 640 480 30 YUYV 640 480 30 SPORK3196 RetroTapeTracker
# @email robotics@pinelakeprep.org
# @mainurl teamspork.com
# @license MIT
# @ingroup modules
class RetroTapeTracker:
    
    def process(self, inframe, outframe):
        # Get the raw input image
        raw = inframe.getCvBGR()
        
        # Convert the image to HSV
        hsv = cv2.cvtColor(raw, cv2.COLOR_BGR2HSV)
        
        # Threshold the image with min and max HSV values
        hsv_cooked = cv2.inRange(hsv, (85,0,240), (90,100,255))
        
        
        # Output the "cooked" image
        outframe.sendCv(hsv_cooked)
