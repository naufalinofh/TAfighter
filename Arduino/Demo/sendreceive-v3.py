import pika
import sys
import serial
import json
import smbus
import time

#usbport= '/dev/ttyUSB1'
usbport= '/dev/ttyACM0'
ser = serial.Serial(usbport,9600,timeout=1)
bus = smbus.SMBus(0)
address = 0x60

credentials = pika.PlainCredentials('anharaf', 'anhar1234')
parameters = pika.ConnectionParameters('192.168.43.132',
                                       5672,
                                       '/',
                                       credentials)

#credentials = pika.PlainCredentials('turret.tank030', 'TA171801030')
#parameters = pika.ConnectionParameters('167.205.7.226',
#                                       5672,
#                                       '/turret',
#                                       credentials)

connection = pika.BlockingConnection()

channel = connection.channel()

channel.queue_declare(queue='raspitoholo',
                        durable=True)

channel.exchange_declare(exchange='ex_raspitoholo',
                        exchange_type='topic',
			durable=True)

channel.queue_bind(queue='raspitoholo',
		   exchange='ex_raspitoholo',
		   routing_key='rk_raspitoholo'
		)

channel.queue_declare(queue='holotoraspi',
			durable=True)


def bearing3599():
        bear1 = bus.read_byte_data(address, 2)
        bear2 = bus.read_byte_data(address, 3)
        bear = (bear1 << 8) + bear2
        bear = bear/10.0
        return bear

def callback(ch, method, properties, body):
	
	jsonObject = json.loads(body)

	if(jsonObject['fire'] == 1) :
		ser.write('y{0},g{1}\n'.format(jsonObject['yaw'],jsonObject['pitch']))
		print('y{0},g{1},f{2}'.format(jsonObject['yaw'],jsonObject['pitch'],jsonObject['fire']))
	if (jsonObject['fire'] == 0):
		ser.write('c{0},t{1}\n'.format(jsonObject['yaw'],jsonObject['pitch']))
		print('c{0},t{1},f{2}'.format(jsonObject['yaw'],jsonObject['pitch'],jsonObject['fire']))

	data={
		"sensor":"compass",
		"head":"%d"%bearing3599()
		}
	jsonData = json.dumps(data)

	channel.basic_publish(exchange='ex_raspitoholo',
			routing_key='rk_raspitoholo',
			body= jsonData)
    #time.sleep(1)

channel.basic_consume(callback,
                      queue='holotoraspi',no_ack=True)

#print(' [*] Waiting for input. To exit press CTRL+C')
channel.start_consuming()

connection.close()

