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
        kernel = np.ones((30,10),np.uint8)
        
        # Get the raw input image
        raw = inframe.getCvBGR()
        
        # Convert the image to HSV
        hsv = cv2.cvtColor(raw, cv2.COLOR_BGR2HSV)
        
        # Threshold the image with min and max HSV values
        hsv_cooked = cv2.inRange(hsv, (50,0,180), (100,255,255))
                                    #  50,0,180,   100,255,255
        
        # Remove noise from the image (small patches of detection)
        hsv_cooked = cv2.morphologyEx(hsv_cooked, cv2.MORPH_OPEN, kernel)
        
        # Find edges using Canny edge detection
        edgesImg = cv2.Canny(hsv_cooked, 10, 20) # Note: I have no idea what these number mean or do
        
        try:
            edges = cv2.HoughLinesP(edgesImg, 1, np.pi/180, 100, 10, 10)
            for edge in edges:
                for x1,y1,x2,y2 in edge:
                    text = "(" + str(x1) + "," + str(y1) + ") (" + str(x2) + "," + str(y2) + ")"
                    jevois.LINFO(text)
            jevois.LINFO("-----------")
        except:
            # Do nothing!
            a = 0
        
        # Output the "cooked" image
        outframe.sendCv(edgesImg)
