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
        kernel = np.ones((3,3),np.uint8)
        
        # Get the raw input image
        raw = inframe.getCvBGR()
        outImg = raw
        
        # Convert the image to HSV
        hsv = cv2.cvtColor(raw, cv2.COLOR_BGR2HSV)
        
        # Threshold the image with min and max HSV values
        hsv_cooked = cv2.inRange(hsv, (70,50,200), (100,220,255))
                                    # (70,0,170), (130,255,255) what we're doing
                                    #  50,0,180,   100,255,255
                                    #  70,0,120,   100,255,255
                                    #  70,30,220,  100,140,255
        
        # Remove noise from the image (small patches of detection)
        hsv_cooked = cv2.morphologyEx(hsv_cooked, cv2.MORPH_OPEN, kernel)
        
        # Find edges using Canny edge detection
        edgesImg = cv2.Canny(hsv_cooked, 1, 1) # Note: I have no idea what these number mean or do
        #edgesImg = cv2.line(edgesImg, (100,0), (100,500), (127,127,127), 5)
        
        try:
            edges = cv2.HoughLinesP(edgesImg, 1, np.pi/180, 10, 10, 10)
            avgX = 0.0
            avgY = 0.0
            text = "edges:" + str(len(edges))
            jevois.LINFO(text)
            for edge in edges:
                for x1,y1,x2,y2 in edge:
                    avgX += (x1+x2)/2.0
                    avgY += (y1+y2)/2.0
                    edgesImg = cv2.line(edgesImg, (x1, y1), (x2, y2), (127,127,127), 3)
                    #text += "\t(" + str(x1) + "," + str(y1) + ") (" + str(x2) + "," + str(y2) + ")"
            avgX = avgX / len(edges)
            avgY = avgY / len(edges)
            outImg = cv2.line(raw, (int(avgX), 0), (int(avgX), 500), (0,0,0), 2)
            outImg = cv2.line(outImg, (0, int(avgY)), (700, int(avgY)), (0,0,0), 2)
            text += "\t" + str(int(avgX))
            jevois.LINFO(text)
        except:
            # Do nothing!
            jevois.LINFO("Nada")
        
        # Output the "cooked" image
        outframe.sendCv(outImg)
