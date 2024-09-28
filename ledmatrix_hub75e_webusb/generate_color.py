import sys
import cv2
import numpy as np
import math

imagepath = sys.argv[1]
outputpath = imagepath + ".hex.txt"

image = cv2.imread(imagepath, cv2.IMREAD_COLOR)
height, width, _ = image.shape
image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)


s = f"const uint8_t bitmap_[{math.floor(width * height / 2)}] = {{\n"
for y in range(height):
    for x in range(width):
        pixel = image[y, x]
        col = 0
        red = pixel[0] > 127
        green = pixel[1] > 127
        blue = pixel[2] > 127
        col = 0
        col |= 0x04 if red else 0
        col |= 0x02 if green else 0
        col |= 0x01 if blue else 0
        col = f'{col:x}'
        if x & 0x01 == 0:
            s += f'0x{col}'
        else: s += f'{col},'
    s += "\n"
s += "};\n"

with open(outputpath, "w") as f:
    f.write(s)
