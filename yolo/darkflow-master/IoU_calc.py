import os
import matplotlib.pyplot as plt
import cv2
from matplotlib.widgets import RectangleSelector
from darkflow.net.build import TFNet
import copy

# global constants
img = None
tl = ()
br = ()
topleft = ()
botright = ()
file = open("IoU_testfile/testfile7.txt","w") 
#object_list = []

# constants
image_folder = 'IoU_Image'
#savedir = 'annotations'
#obj = 'fidget_spinner'


#===========================Inisiasi YOLO
options = {
    'model': 'cfg/yolo-voc.cfg',
    'load': 'bin/yolo-voc.weights',
    'threshold': 0.3,
    'gpu': 0.5
}

tfnet = TFNet(options)
#==========================================
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
    global detect_img
    global i
    if event.key == 'q':
        IoU = bb_intersection_over_union(tl,br,topleft,botright)
        file.write("{}\t{}\t\t{}\t{}\t{}\n".format(tl,br,topleft,botright,IoU))
        res_img = cv2.rectangle(copy.copy(detect_img), tl,br, (255,0,0),2)
        cv2.imwrite('{}.jpg'.format(i),res_img)
        #print(tl,' ',br,' , ',topleft,' ',botright,' ',IoU)
        tl= ()
        br = ()
        topleft = ()
        botright = ()
        #object_list = []
        img = None
        plt.close()
        cv2.destroyAllWindows()
        i=i+1

def toggle_selector(event):
    toggle_selector.RS.set_active(True)


if __name__ == '__main__':
    i=0
    for n, image_file in enumerate(os.scandir(image_folder)):
        img = image_file
        fig, ax = plt.subplots(1)
        image = cv2.imread(image_file.path)
        real_image = copy.copy(image)
        result = tfnet.return_predict(image)
        topleft = (result[0]['topleft']['x'],result[0]['topleft']['y'])
        botright = (result[0]['bottomright']['x'],result[0]['bottomright']['y'])
        detect_img = cv2.rectangle(image,topleft,botright,(0,255,0),2)
        #detect_img_RGB = cv2.cvtColor(detect_img, cv2.COLOR_BGR2RGB)
        cv2.imshow('detect',detect_img)
        #cv2.imshow('real',real_image)
        real_image = cv2.cvtColor(real_image, cv2.COLOR_BGR2RGB)
        ax.imshow(real_image)

        toggle_selector.RS = RectangleSelector(
            ax, line_select_callback,
            drawtype='box', useblit=True,
            button=[1], minspanx=5, minspany=5,
            spancoords='pixels', interactive=True
        )
        bbox = plt.connect('key_press_event', toggle_selector)
        key = plt.connect('key_press_event', onkeypress)
        plt.show()
        
    file.close()
