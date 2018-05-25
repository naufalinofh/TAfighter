from ftplib import FTP
import time

ftp = FTP('167.205.7.226')
ftp.login('ftpimageproc|imageproc', 'Img0305@')

while True:
	fhandle = open('dataset/tes.jpg', 'wb')
	ftp.retrbinary('RETR ' + 'tes.jpg', fhandle.write)
	
	time.sleep(1)