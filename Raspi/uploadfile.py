import picamera
import time
import pika
from ftplib import FTP
import datetime

#now = datetime.datetime.now().time()
#print (datetime.datetime.now().time())
#beginday = datetime.datetime.combine(now.date(), datetime.time(0))
#now = datetime.datetime.now().time()

# create connection Rabbitmq server LSKK
credentials = pika.PlainCredentials('turret.tank030', 'TA171801030')
parameters = pika.ConnectionParameters('167.205.7.226',5672,'/turret',credentials)

# create connection pika
#credentials = pika.PlainCredentials('rangga', '1234')
#parameters = pika.ConnectionParameters('192.168.43.49',5672,'/',credentials)

connection = pika.BlockingConnection(parameters)
channel = connection.channel()

channel.queue_declare(queue='notif', durable=True, arguments={'x-max-length':2})
channel.exchange_declare(exchange='ex_notif',exchange_type='topic',durable=True)

channel.queue_bind(queue='notif',
                   exchange='ex_notif',
                   routing_key='rk_notif'
                   )

#program utama
i = 1
camera = picamera.PiCamera()
#print ('camera open')

#ftp = FTP('167.205.7.226')
#ftp.login('ftpimageproc|imageproc', 'Img0305@')
#ftp = FTP('192.168.43.49')
#ftp.login('FTP-User', 'Yolo123!')
ftp = FTP('ftptugasakhir.pptik.id')
ftp.login('ftptugasakhir.pptik.id|ftptugasakhiruser', 'ZxCzXc123!')


#print('ftp open')
channel.basic_publish(exchange='ex_notif', routing_key='rk_notif', body='0')

print('publish to notif')
while True:
	#now1 = datetime.datetime.now().time()
	print('x')
	
	#camera.capture("/home/pi/TA/upload/tes.jpg")
	camera.capture("upload/tes.jpg")
	print('capture done')
	
	#fin = open("/home/pi/TA/upload/tes.jpg", 'rb')
	fin = open("upload/tes.jpg", 'rb')
	ftp.storbinary ("STOR TA_Turret_Tank/tes.jpg", fin)
	time.sleep(0.5)
	channel.basic_publish(exchange='ex_notif', routing_key='rk_notif', body='1')
	
	
	i += 1
	#time.sleep(0.2)
	time.sleep(0.2)
	channel.basic_publish(exchange='ex_notif', routing_key='rk_notif', body='0')
	time.sleep(0.2)
	
	#print (datetime.datetime.now().time())
	#print ((now - beginday).seconds)
	
	with open('timeftp.txt','a') as f:
            f.write(str(datetime.datetime.now().time()))
            f.write('\n')