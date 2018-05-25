import cv2
from darkflow.net.build import TFNet
import numpy as np
import time
import json
import pika

option = {
	#'model': 'cfg/yolo.cfg',
	'model': 'cfg/yolo-obj.cfg',
	#'load': 'bin/yolo.weights',
	'load': 'bin/yolo-obj.weights',
	'treshold': 0.15,
	'gpu': 0.5
}

#inisiasi pika dan rmq
credentials = pika.PlainCredentials('turret.tank030', 'TA171801030')
parameters = pika.ConnectionParameters('167.205.7.226',5672,'/turret',credentials)

connection = pika.BlockingConnection(parameters)
channel = connection.channel()

channel.queue_declare(queue='servertoholo', durable=True)

channel.exchange_declare(exchange='ex_servertoholo', exchange_type='topic', durable=True)

channel.queue_bind(queue='servertoholo',
		   exchange='ex_servertoholo',
		   routing_key='rk_servertoholo'
		)
		
#capture video

tfnet = TFNet(option)

capture = cv2.VideoCapture(0)
#capture = cv2.VideoCapture('tes.mp4')
#capture = cv2.VideoCapture('preview.png')
colors = [tuple(255 * np.random.rand(3)) for i in range(5)]

while (capture.isOpened()):
	stime = time.time()
	ret, frame = capture.read()
	results = tfnet.return_predict(frame)
	
	
	if ret:
		sum_box = 0
		for color, result in zip(colors, results):
			sum_box += 1
		
		if (sum_box == 0) :
			data = {
			"sum_box":"%d"%0,
			"tlx":"%d"%0,
			"tly":"%d"%0,
			"brx":"%d"%0,
			"bry":"%d"%0}
	
			jsonData = json.dumps(data)
			if(bool(data)):
				channel.basic_publish(exchange='ex_servertoholo', routing_key='rk_servertoholo', body=jsonData)
		else:	
			for color, result in zip(colors, results):
				tl = (result['topleft']['x'], result['topleft']['y'])
				br = (result['bottomright']['x'], result['bottomright']['y'])
				label = result['label']
				frame = cv2.rectangle(frame, tl, br, color, 7)
				frame = cv2.putText(frame, label, tl, cv2.FONT_HERSHEY_COMPLEX, 1, (0, 0, 0), 2)
				
				data = {
				"sum_box":"%d"%sum_box,
				"tlx":"%d"%tl[0],
				"tly":"%d"%tl[1],
				"brx":"%d"%br[0],
				"bry":"%d"%br[1]}
	
				jsonData = json.dumps(data)
			
				if(bool(data)):
					channel.basic_publish(exchange='ex_servertoholo', routing_key='rk_servertoholo', body=jsonData)
		
			
		cv2.imshow('frame', frame)
		print('FPS {:.1f}'.format(1 / (time.time() - stime)))
		
		
		
		
		if cv2.waitKey(1) & 0xFF == ord('q'):
			break
	else:
		capture.release()
		cv2.destroyAllWindows()
		
		break
		


	




		


