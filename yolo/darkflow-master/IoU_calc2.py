import time
import os
import cv2
from darkflow.net.build import TFNet
import numpy as np
import copy
from matplotlib.widgets import RectangleSelector
import matplotlib.pyplot as plt

def line_select_callback(clk, rls):
    global tl
    global br
    #global object_list
    tl = (int(clk.xdata), int(clk.ydata))
    br = (int(rls.xdata), int(rls.ydata))
    #tl_list.append((int(clk.xdata), int(clk.ydata)))
    #br_list.append((int(rls.xdata), int(rls.ydata)))
    #object_list.append(obj)

def bb_intersection_over_union(tl,br, topleft,botright):
	# determine the (x, y)-coordinates of the intersection rectangle
	xA = max( tl[0], topleft[0])
	yA = max(tl[1], topleft[1])
	xB = min(br[0], botright[0])
	yB = min(br[1], botright[1])
	# compute the area of intersection rectangle
	interArea = (xB - xA + 1) * (yB - yA + 1)

	# compute the area of both the prediction and ground-truth
	# rectangles
	boxAArea = (br[0] - tl[0] + 1) * (br[1] - tl[1] + 1)
	boxBArea = ( botright[0]- topleft[0] + 1) * (botright[1] - topleft[1] + 1)

	# compute the intersection over union by taking the intersection
	# area and dividing it by the sum of prediction + ground-truth
	# areas - the interesection area
	iou = interArea / float(boxAArea + boxBArea - interArea)

	# return the intersection over union value
	return iou

def onkeypress(event):
    #global object_list
    global tl
    global br
    global topleft
    global botright
    global img
    global file
    global i
    if event.key == 'q':
        IoU = bb_intersection_over_union(tl,br,topleft,botright)
        file.write("{}\t{}\t\t{}\t{}\t{}\n".format(tl,br,topleft,botright,IoU))
        image_result = cv2.rectangle(copy.copy(image_res), tl, br, (0,255,0), 1)
        cv2.imwrite('{}.jpg'.format(i),image_result)
        #print(tl,' ',br,' , ',topleft,' ',botright,' ',IoU)
        tl= ()
        br = ()
        topleft = ()
        botright = ()
        #object_list = []
        img = None
        plt.close()
        cv2.destroyAllWindows()

def toggle_selector(event):
    toggle_selector.RS.set_active(True)
    

options = {
    'model': 'cfg/yolo-obj.cfg',
    'load': 'bin/yolo-obj.weights',
    'threshold': 0.6,
    'gpu': 0.5
}



tfnet = TFNet(options)
while True:
    tl = ()
    br = ()
    topleft = ()
    botright = ()
    topleft = ()
    botright = ()
    
    taken_picture_list = []
    fileRead = input("Masukkan Input: ")
    filename, fileExtension = os.path.splitext(fileRead)

    if (fileExtension == '.jpg'):
        image = cv2.imread(fileRead)
        results = tfnet.return_predict(image)
        tl = (results[0]['topleft']['x'],results[0]['topleft']['y'])
        br = (results[0]['bottomright']['x'],results[0]['bottomright']['y'])
        label = results[0]['label']
        image_res = cv2.rectangle(copy.copy(image), tl, br, (0, 255, 0), 5)
        cv2.imshow('image',image)
        cv2.imshow('res',image_res)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    elif (fileExtension == '.0' ):
        fileExtension = fileExtension[-1]
        cam_num = int(fileExtension)
        take_num = int (input("Berapa kali ingin memasukkan input: "))
        print(take_num*2)
        i=0
        cam = cv2.VideoCapture(cam_num)
        file = open("IoU_testfile/testfile8.txt","w")
        '''cam.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cam.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)'''
        while (i<take_num):
            ret, frame = cam.read()
            if not ret:
                break
            frame = cv2.putText(frame, '{}'.format(i+1), (0,0),cv2.FONT_HERSHEY_COMPLEX, 0.8, (0, 0, 0), 1)
            cv2.imshow('test',frame)
            k = cv2.waitKey(1)

            if ((k & 0xFF) == ord(' ')): #SPACE IS PRESSED
                if (i >= take_num):
                    break
                print("taking a picture....")
                taken_picture = copy.copy(frame)
                taken_picture_list.append(taken_picture)
                cv2.destroyAllWindows()
                i = i+1
                #time.sleep(10)
            elif( (k& 0xFF) == ord('q')):
                break
        cam.release()
        cv2.destroyAllWindows()    
        i = 0
        for image in taken_picture_list:
            fig, ax = plt.subplots(1)
            results = tfnet.return_predict(image)
            topleft = (results[0]['topleft']['x'],results[0]['topleft']['y'])
            botright = (results[0]['bottomright']['x'],results[0]['bottomright']['y'])
            label = results[0]['label']
            image_res = cv2.rectangle(copy.copy(image), topleft, botright, (255,255,255), 1)
            ax.imshow(image)

            toggle_selector.RS = RectangleSelector(
                ax, line_select_callback,
                drawtype='box', useblit=True,
                button=[1], minspanx=5, minspany=5,
                spancoords='pixels', interactive=True
            )
            bbox = plt.connect('key_press_event', toggle_selector)
            key = plt.connect('key_press_event', onkeypress)
            plt.show()

            cv2.imshow('res',image_res)
            #cv2.imshow('res',image_res)

            i=i+1
        file.close()
    elif (fileRead == "exit"):
        print ('closing the program.....')
        break
