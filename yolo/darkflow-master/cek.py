import sys
import glob
import os.path
from pathlib import Path

my_file = Path('D:\RANGGA\Kerjaan\TA\Image processing\darkflow-master (python-thtrieu)\darkflow-master\dataset\tes2.jpg')

if os.path.exists(my_file):
	print('Ada')
	
else:
	print('Ga ada')