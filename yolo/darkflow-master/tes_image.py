import cv2
from darkflow.net.build import TFNet
import matplotlib.pyplot as plt
import numpy as np
import time
import json
import pika
import os.path
from pathlib import Path
from ftplib import FTP

options = {
	'model': 'cfg/yolo-obj.cfg',
	'load': 'bin/yolo-obj.weights',
#    'model': 'cfg/yolo.cfg',
#    'load': 'bin/yolo.weights',
    'threshold': 0.15,
    'gpu': 0.5
}

#inisiasi pika dan rmq
credentials = pika.PlainCredentials('turret.tank030', 'TA171801030')
parameters = pika.ConnectionParameters('167.205.7.226',5672,'/turret',credentials)

connection = pika.BlockingConnection(parameters)
channel = connection.channel()

channel.queue_declare(queue='servertoholo', durable=True)

channel.queue_declare(queue='notif', durable=True)

channel.exchange_declare(exchange='ex_servertoholo', exchange_type='topic', durable=True)

channel.queue_bind(queue='servertoholo',
		   exchange='ex_servertoholo',
		   routing_key='rk_servertoholo'
		)
		
#capture image
tfnet = TFNet(options)

#inisialisasi FTP
#ftp = FTP('167.205.7.226')
#ftp.login('ftpimageproc|imageproc', 'Img0305@')

def callback(ch, method, properties, body):
	print(body[0])
	print(" [x] Received %s" %body)
	if body[0] == 49:
		#fhandle = open('dataset/tes.jpg', 'wb')
		#ftp.retrbinary('RETR ' + 'tes.jpg', fhandle.write)  
		print('Masuk body 1')
		
		img = cv2.imread("dataset/tes.jpg", cv2.IMREAD_COLOR)
		img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

		results = tfnet.return_predict(img)
		
		print('Result :', results)
		print(' ')


		#print('results :',results)	
		
		sum_box = 0
		for result in zip(results):
			#print('result :',result)
			sum_box += 1

		#print('sum box :',sum_box)

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
			for result in zip(results):
				tl = (result[0]['topleft']['x'], result[0]['topleft']['y'])
				br = (result[0]['bottomright']['x'], result[0]['bottomright']['y'])
				label = result[0]['label']
				img = cv2.rectangle(img, tl, br, (0, 255, 0), 7)
				img = cv2.putText(img, label, tl, cv2.FONT_HERSHEY_COMPLEX, 1, (0, 0, 0), 2)
				
				data = {
				"sum_box":"%d"%sum_box,
				"tlx":"%d"%tl[0],
				"tly":"%d"%tl[1],
				"brx":"%d"%br[0],
				"bry":"%d"%br[1]}
		
				jsonData = json.dumps(data)
				
				#print('tl :',tl)
				#print('br :',br)
				
			if(bool(data)):
				channel.basic_publish(exchange='ex_servertoholo', routing_key='rk_servertoholo', body=jsonData)
		
#		time.sleep(1)
	else :
		
		time.sleep(0.1)
		print('Waiting')
	
channel.basic_consume(callback,
                      queue='notif',
                      no_ack=True)
channel.start_consuming()