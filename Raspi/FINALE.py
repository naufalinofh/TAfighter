import picamera
import time
import pika
from ftplib import FTP
import datetime

import sys
import serial
import json
import codecs
from urllib.request import urlopen
import smbus

import geocoder
import requests
import pygame
import os
import subprocess

now = datetime.datetime.now().time()
#usbport= '/dev/ttyACM0'
usbport= '/dev/ttyUSB0'
ser = serial.Serial(usbport,115200,timeout=1)
    
bus = smbus.SMBus(1)
address = 0x60

##create connection pika
credentials = pika.PlainCredentials('turret.tank030', 'TA171801030')
parameters = pika.ConnectionParameters('167.205.7.226',5672,'/turret',credentials)

#credentials = pika.PlainCredentials('rangga', '1234')
#parameters = pika.ConnectionParameters('192.168.43.49',5672,'/',credentials)

connection = pika.BlockingConnection(parameters)
channel = connection.channel()

args={'x-max-length':2}
channel.queue_declare(queue='raspitoholo', durable=False, arguments=args )
                                                        
channel.exchange_declare(exchange='ex_raspitoholo', exchange_type='topic', durable=False, arguments=args)

channel.queue_bind(queue='raspitoholo',
                   exchange='ex_raspitoholo',
                   routing_key='rk_raspitoholo'
                   )

channel.queue_declare(queue='holotoraspi', durable=False,
                      arguments=args)

channel.queue_declare(queue='inTopic', durable=False, arguments=args)
                                                        
channel.exchange_declare(exchange='amq.topic', exchange_type='topic', durable=True, arguments=args)

channel.queue_bind(queue='inTopic',
                   exchange='amq.topic',
                   routing_key='inTopic'
                   )


##########################################
##program utama FTP

##########################################

#freegeoip = "http://freegeoip.net/json"
#geo_r = requests.get(freegeoip)
#geo_json = geo_r.json()

#user_postition = [geo_json["latitude"], geo_json["longitude"]]
#lat_ip=user_postition[0]
#lon_ip=user_postition[1]

lat_ip=1
lon_ip=2

##############################################

#bgupload = subprocess.Popen(["sudo","python3", "/home/pi/TA/uploadfile.py"])
print('x1')
bgupload = subprocess.Popen(["python3", "uploadfile.py"])

##############################################

status = 'Manual'

keyboardcam = True

yaw = 0.0
pitch = 0.0
yawcam = 0.0
pitchcam = 0.0
prev_keyboardcam = True
##############################################
                                
def bearing3599():
   bear1 = bus.read_byte_data(address, 2)
   bear2 = bus.read_byte_data(address, 3)
   bear = (bear1 << 8) + bear2
   bear = bear/10.0
   return bear
   #return 132

def inputmode(keyboardcam, yawcam, pitchcam, yaw, pitch):
    import pygame
    
    print('fungsi terpanggil')
    pygame.init()
    print('init')
    screen = pygame.display.set_mode((640, 480))
    pygame.display.set_caption('The amazing key presser!')
    pygame.key.set_repeat(1, 200)
    
    for e in pygame.event.get():
        if e.type == pygame.KEYDOWN:
            if (keyboardcam == True) :
                if (e.key == pygame.K_k):
                    pitchcam -= 1
                    if pitchcam <= 0 :
                        pitchcam = 0
#                   print ("Pitch = %d"%pitch)
                elif (e.key == pygame.K_i):
                    pitchcam += 1
                    if pitchcam >= 20 :
                        pitchcam = 20
#                   print ("Pitch = %d"%pitch)
                elif (e.key == pygame.K_l):
                    yawcam += 1
                    yawcam = yawcam % 360
#                    print ("Yaw = %d"%yaw)
                elif (e.key == pygame.K_j):
                    yawcam -= 1
                    yawcam = yawcam % 360
#                    print ("Yaw = %d"%yaw)
                elif (e.key == pygame.K_s):
                    pitch -= 1.0
                    if pitch <= 0.0 :
                        pitch = 0.0
#                   print ("Pitch = %d"%pitch)
                elif (e.key == pygame.K_w):
                    pitch += 1.0
                    if pitch >= 20.0 :
                        pitch = 20.0
#                   print ("Pitch = %d"%pitch)
                elif (e.key == pygame.K_d):
                    yaw += 1.0
                    yaw = yaw % 360
#                    print ("Yaw = %d"%yaw)
                elif (e.key == pygame.K_a):
                    yaw -= 1.0
                    yaw = yaw % 360
                elif (e.key == pygame.K_q):
                    keyboardcam = True
                elif (e.key == pygame.K_e):
                    keyboardcam = False
                else :
                    keyboardcam = keyboardcam
                    yaw = yaw
                    pitch = pitch
                    yawcam = yawcam
                    pitchcam = pitchcam
            else :
                if (e.key == pygame.K_s):
                    pitch -= 1.0
                    if pitch <= 0.0 :
                        pitch = 0.0
#                   print ("Pitch = %d"%pitch)
                elif (e.key == pygame.K_w):
                    pitch += 1.0
                    if pitch >= 20.0 :
                        pitch = 20.0
#                   print ("Pitch = %d"%pitch)
                elif (e.key == pygame.K_d):
                    yaw += 1.0
                    yaw = yaw % 360
                elif (e.key == pygame.K_a):
                    yaw -= 1.0
                    yaw = yaw % 360
                elif (e.key == pygame.K_q):
                    keyboardcam = True
                elif (e.key == pygame.K_e):
                    keyboardcam = False
                else :
                    keyboardcam = keyboardcam
                    yaw = yaw
                    pitch = pitch
                    yawcam = yawcam
                    pitchcam = pitchcam
            
                
    return (keyboardcam, yawcam, pitchcam, yaw, pitch)

def callback(ch, method, properties, body):
                
                global status
                global yaw
                global pitch
                global yawcam
                global pitchcam
                global keyboardcam
                global prev_keyboardcam
                global i
                numAttempt = 0
                
                with open('timeraspi.txt','a') as f:
                    f.write(str(datetime.datetime.now().time()))
                    f.write('\t')
                print('x')
                    
                try:
                
                    x = body.decode('utf8')
                    jsonObject = json.loads(x)
                    c=jsonObject['yaw']
                    t=jsonObject['pitch']
                    
                    #print(c, t)
                    
                    print("dari %r ke %r"%(prev_keyboardcam, keyboardcam))
                    
                    if (jsonObject['indBoxTrackedPlusOne'] == 0):
                        
                        print('json masuk')   
                            
                        (keyboardcam,yawcam,pitchcam,yaw,pitch)= inputmode(keyboardcam,yawcam,pitchcam,yaw,pitch)
                        
                        print('key presser aktif')
                        if ((keyboardcam == True)and(prev_keyboardcam == False)):
                            yawcam = c
                            pitchcam = t
                            
                        if (keyboardcam == True):
			    #mode keyboard camera on

                            status = 'Manual keyboard camera on'
                            
                            #i=1
                            
                            print('c{0:.2f},t{1:.2f},g{2:.2f},y{3:.2f}'.format(yawcam,pitchcam,pitch,yaw))
       
                            
                            ser.write(bytes('c{0:.2f},t{1:.2f},g{2:.2f},y{3:.2f}\n'.format(yawcam,pitchcam,pitch,yaw),'UTF-8'))
                            
                            
                            prev_keyboardcam = True
                        else :
			    #mode keyboard camera off
                            status = 'Manual keyboard camera off'
                            
                            #print (status)
                            
                            print('c{0:.2f},t{1:.2f},g{2:.2f},y{3:.2f}'.format(c,t,pitch,yaw))
           
                            
                            #i = 0
                            ser.write(bytes('c{0:.2f},t{1:.2f},g{2:.2f},y{3:.2f}\n'.format(c,t,pitch,yaw),'UTF-8'))
                            
                            prev_keyboardcam = False

                             
                    else :
                        dataSudut1={
                        "cam":"%f"%c,
                        "tilt":"%f"%t,
                        "gun":"%f"%c,
                        "yaw_act":"%f"%c
                         }
                        jsonSudut1 = json.dumps(dataSudut1)
#print(1)
                        print('c{0:.2f},t{1:.2f},g{2:.2f},y{3:.2f}'.format(c,t,t,c)) 
#  EDIT
#                            ser.write(bytes((jsonSudut1),'UTF-8'))
                        yaw = c
                        pitch = t
                        ser.write(bytes('c{0:.2f},t{1:.2f},g{1:.2f},y{0:.2f}\n'.format(jsonObject['yaw'],jsonObject['pitch']),'UTF-8'))
                           
                        status = 'Tracking'
                        i = 0
                    
                    yawdata={
                        "yaw":"%d"%yaw
                        }
                    
                    jsonYaw = json.dumps(yawdata)
                    #print("yaw : {0} ; pitch : {1}".format(yaw,pitch))
                    
                    channel.basic_publish(exchange='amq.topic',
                                    routing_key='inTopic',
                                    body= jsonYaw)
                    data={
                        "sensor1":"compass",
                        "head":"%d"%bearing3599(),
                        "sensor2":"GPS",
                        "latitude":"%s"%lat_ip,
                        "longitude":"%s"%lon_ip
                      }
                    
#                   print("%d"%bearing3599())
                    jsonData = json.dumps(data)
                    channel.basic_publish(exchange='ex_raspitoholo',
                                            routing_key='rk_raspitoholo',
                                            body= jsonData)
                    
                    #return (status)
                    
                    print(status)
                except :
                    numAttempt +=1
                    if(numAttempt >= 10):
                        bgupload.terminate()
                    print("error")
                    #sys.exit(1)
                with open('timeraspi.txt','a') as f:
                    f.write(str(datetime.datetime.now().time()))
                    f.write('\n')

        
channel.basic_consume(callback,queue='holotoraspi',no_ack=True)

channel.start_consuming()
###############################################






