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
ser = serial.Serial(usbport,9600,timeout=1)
bus = smbus.SMBus(1)
address = 0x60

##create connection pika
#credentials = pika.PlainCredentials('turret.tank030', 'TA171801030')
#parameters = pika.ConnectionParameters('167.205.7.226',5672,'/turret',credentials)
#credentials = pika.PlainCredentials('anharaf', 'anhar1234')
#parameters = pika.ConnectionParameters('192.168.43.44',5672,'/',credentials)
#credentials = pika.PlainCredentials('anharaf', 'anhar1234')
#parameters = pika.ConnectionParameters('192.168.43.37',5672,'/',credentials)
credentials = pika.PlainCredentials('rangga', '1234')
parameters = pika.ConnectionParameters('192.168.43.49',5672,'/',credentials)

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

#subprocess.call('cd /home/pi/TA', shell = True)
#subprocess.call('python3 receiveholo.py', shell = True)
bgupload = subprocess.Popen(["python3", "uploadfile.py"])
#bgreceiveholo = subprocess.Popen(["python3", "receiveholo.py"])

##############################################

#bgkeymanual = subprocess.Popen(["python3", "keyboardmanual.py"])
status = 'Manual'

yaw = 0
pitch = 0


##############################################

#bgreceiveholo = subprocess.Popen(["python3", "receiveholo_editted.py"],
                                 #stdout=subprocess.PIPE,
                                 #stderr=subprocess.STDOUT)

#print('cel')
#output = bgreceiveholo.stdout.readline()
#print(output)
##############################################
                                
def bearing3599():
   bear1 = bus.read_byte_data(address, 2)
   bear2 = bus.read_byte_data(address, 3)
   bear = (bear1 << 8) + bear2
   bear = bear/10.0
   return bear
#   return 132

#pygame.init()
#screen = pygame.display.set_mode((640, 480))
#pygame.display.set_caption('The amazing key presser!')

#endProgram = False

#yaw = 0
#pitch = 0

#pygame.key.set_repeat(1, 200)

#############################################
#print('asshole')
#output = subprocess.check_output(['python3','receiveholo_editted.py'])
#output = bgreceiveholo.stdout.read()

#print('Wow ' + output)

def key(yaw, pitch):
    #############################################
    pygame.init()
    screen = pygame.display.set_mode((640, 480))
    pygame.display.set_caption('The amazing key presser!')

    endProgram = False


    pygame.key.set_repeat(1, 200)

    #############################################

    #while not endProgram:
    for e in pygame.event.get() :
        if e.type == pygame.KEYDOWN:
            if (e.key == pygame.K_w):
                pitch -= 1
                if pitch <= 0 :
                    pitch = 0
#                   print ("Pitch = %d"%pitch)
            elif (e.key == pygame.K_s):
                pitch += 1
                if pitch >= 20 :
                    pitch = 20
#                   print ("Pitch = %d"%pitch)
            elif (e.key == pygame.K_a):
                yaw += 1
                yaw = yaw % 360
#                    print ("Yaw = %d"%yaw)
            elif (e.key == pygame.K_d):
                yaw -= 1
                yaw = yaw % 360
#                    print ("Yaw = %d"%yaw)
            elif (e.key == pygame.K_q):
                endProgram = True
            else:
                yaw=yaw
                pitch=pitch
                
    return (yaw, pitch)
                
        #print("yaw : {0} ; pitch : {1}".format(yaw,pitch))
#       ser.write('t{0}\n'.format(pitch))


def callback(ch, method, properties, body):	
                #channel.basic.consume(callback2,queue='outTopic',no_ack=True)
                
                global status
                global yaw
                global pitch
                
                try:
                
                    x = body.decode('utf8')
                    jsonObject = json.loads(x)
                    #print(x)
                    #print(jsonObject['yaw'])
                    c=jsonObject['yaw']
                    t=jsonObject['pitch']
                
                    if (jsonObject['indBoxTrackedPlusOne'] == 0):
                            #print(0)
                            #if (status == 'Tracking'):
                                #bgkeymanual = subprocess.Popen(["python3", "keyboardmanual.py"])
                            yaw, pitch = key(yaw, pitch)
                            status = 'Manual'
#EDIT
                            print('c{0},t{1},g{2},y{3}'.format(c,t,pitch,yaw))
#EDIT
                            dataSudut0={
                            "cam":"%d"%c,
                            "tilt":"%d"%t,
                            "gun":"%d"%pitch,
                            "yaw_act":"%d"%yaw
                              }
                            
                            #jsonSudut0 = json.dumps(dataSudut0)
                            #ser.write(bytes((jsonSudut0),'UTF-8'))
#                            ser.write(bytes(("1"),'UTF-8'))
                            ser.write(bytes('c{0},t{1},g{2},y{3}\n'.format(jsonObject['yaw'],jsonObject['pitch'],pitch,yaw),'UTF-8'))
#                            print('camera{0},pitch{1},gun{2},turret{3}'.format(jsonObject['yaw'],jsonObject['pitch'],pitch,yaw))
                             
                    else :
                            dataSudut1={
                            "cam":"%f"%c,
                            "tilt":"%f"%t,
                            "gun":"%f"%c,
                            "yaw_act":"%f"%c
                              }
                            jsonSudut1 = json.dumps(dataSudut1)
#print(1)
                            print('c{0},t{1},g{2},y{3}'.format(c,t,t,c)) 
#  EDIT
#                            ser.write(bytes((jsonSudut1),'UTF-8'))
                            yaw = c
                            pitch = t
                            ser.write(bytes('c{0},t{1},g{1},y{0}\n'.format(jsonObject['yaw'],jsonObject['pitch']),'UTF-8'))
                            #print('yaw{0},pitch{1},fire{2},mode{3}'.format(jsonObject['yaw'],jsonObject['pitch'],jsonObject['fire'],jsonObject['indBoxTrackedPlusOne']))
                            
                            #if (status == 'Manual'):
                                #bgkeymanual.terminate()
                           
                            status = 'Tracking'    
                    
                    yawdata={
                        "yaw":"%d"%yaw
                        }
                    
                    jsonYaw = json.dumps(yawdata)
                    print("yaw : {0} ; pitch : {1}".format(yaw,pitch))
                    
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
                    bgupload.terminate()
                    #print("error")
                    #sys.exit(1)
                with open('time.txt','a') as f:
                    f.write(str(datetime.datetime.now().time()))
                    f.write('\n')

        
channel.basic_consume(callback,queue='holotoraspi',no_ack=True)

channel.start_consuming()
###############################################






