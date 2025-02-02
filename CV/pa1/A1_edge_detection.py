import cv2
import numpy as np
import math
import time
import cv_image_filtering as cif

def compute_image_gradient(img):
    
    SobelX = np.array([[1, 0, -1],
                       [2, 0, -2],
                       [1, 0, -1]])
    
    SobelY = np.array([[1, 2, 1],
                       [0, 0, 0],
                       [-1, -2, -1]])
    
    sobelXimg = cif.cross_correlation_2d(img, SobelX)
    sobelYimg = cif.cross_correlation_2d(img, SobelY)

    mag = np.sqrt(sobelXimg ** 2 + sobelYimg ** 2)
    mag_normalized = cif.normalize(mag, 0, 255).astype(np.uint8)

    dir = np.arctan2(sobelYimg, sobelXimg)
    return mag_normalized, dir

def non_maximum_suppression_dir(mag, dir):
    
    unit_dir = (((((-dir + 0.125 * math.pi) / math.pi)) * 4) + 4).astype(np.uint8) % 8
    suppressed_mag = np.zeros((mag.shape)).astype(np.uint8)
    
    for i in range(1, mag.shape[0] - 1):
        for j in range(1, mag.shape[1] - 1):
            if unit_dir[i, j] % 8 == 0 or unit_dir[i, j] % 8 == 4:
                x = [i, i]
                y = [j - 1, j + 1]
                
            elif unit_dir[i, j] % 8 == 1 or unit_dir[i, j] % 8 == 5:
                x = [i - 1, i + 1]
                y = [j + 1, j - 1]
                
            elif unit_dir[i, j] % 8 == 2 or unit_dir[i, j] % 8 == 6:
                x = [i - 1, i + 1]
                y = [j, j]
                
            elif unit_dir[i, j] % 8 == 3 or unit_dir[i, j] % 8 == 7:
                x = [i - 1, i + 1]
                y = [j - 1, j + 1]
            else:
                print("error!")
                
            neighbors = [mag[x[0], y[0]], mag[x[1], y[1]]] 

            if mag[i, j] >= max(neighbors):
                suppressed_mag[i, j] = mag[i, j]
                # get only one local max
                mag[x[0], y[0]] = max(0, mag[x[0], y[0]] - 1)
                mag[x[1], y[1]] = max(0, mag[x[1], y[1]] - 1)
            else:
                suppressed_mag[i, j] = 0
                
    return suppressed_mag

img_name = input("insert png name : ")
img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

gaussian_filter = cif.get_gaussian_filter_2d(7, 1.5)
filtered_img = cif.cross_correlation_2d(img, gaussian_filter)

start = time.time()
mag, dir = compute_image_gradient(filtered_img)
end = time.time()
timeslice = end - start
print(img_name, ":: compute_image_gradient time ", timeslice, "s")

cv2.imwrite("./result/part_2_edge_raw_" + img_name, mag)

start = time.time()
suppressed_mag = non_maximum_suppression_dir(mag , dir)
end = time.time()
timeslice = end - start
print(img_name, ":: non_maximum_suppression_dir time ", timeslice, "s")

cv2.imwrite("./result/part_2_edge_sup_" + img_name, suppressed_mag)