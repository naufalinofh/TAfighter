import time
from ftplib import FTP

ftp = FTP('167.205.7.226')
ftp.login('ftpimageproc|imageproc', 'Img0305@')	
fin = open("videofile_1080_20fps.mp4", 'rb')
ftp.storbinary("STOR videofile_1080_20fps.mp4", fin)
print("Kirim done!")

