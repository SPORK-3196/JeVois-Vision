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
        img = inframe.getCvBGR()
        outframe.sendCv(img)
