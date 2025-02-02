import cv2
import numpy as np
import time
import cv_image_filtering as cif

def uniform_window_function(x, y, winSize, center):
    cx, cy = center
    wx, wy = winSize

    left, right = cx - wx // 2, cx + wx // 2
    top, bottom = cy - wy // 2, cy + wy // 2

    if left <= x <= right and top <= y <= bottom:
        return 1
    else:
        return 0

def compute_corner_response(img):
    winSize = 5
    winHalf = winSize // 2
    
    SobelX = np.array([[1, 0, -1],
                       [2, 0, -2],
                       [1, 0, -1]])
    
    SobelY = np.array([[1, 2, 1],
                       [0, 0, 0],
                       [-1, -2, -1]])
    
    Ix = cif.cross_correlation_2d(img, SobelX)
    Iy = cif.cross_correlation_2d(img, SobelY)
    
    Ixx = Ix ** 2
    Iyy = Iy ** 2
    Ixy = Ix * Iy
    
    ret = np.zeros((img.shape))
    
    for i in range(winHalf, img.shape[0] - winHalf):
        for j in range(winHalf, img.shape[1] - winHalf):
            wIxx = Ixx[i - winHalf : i + winSize  - winHalf,
                       j - winHalf : j + winSize - winHalf]
            wIyy = Iyy[i - winHalf : i + winSize  - winHalf,
                       j - winHalf : j + winSize - winHalf]
            wIxy = Ixy[i - winHalf : i + winSize  - winHalf,
                       j - winHalf : j + winSize - winHalf]
            
            Mxx = wIxx.sum()
            Myy = wIyy.sum()
            Mxy = wIxy.sum()
            
            detM = Mxx * Myy - Mxy ** 2
            traceM = Mxx + Myy
            k = 0.04
            R = detM - k * (traceM ** 2)
            ret[i, j] = R

    return ret

def non_maximum_suppression_win(R, winSize):
    ret = np.zeros((R.shape))
    winHalf = winSize // 2
    threshold = 0.1
    
    for i in range(winHalf, R.shape[0] - winHalf):
        for j in range(winHalf, R.shape[1] - winHalf):
            window = R[i - winHalf : i + winSize  - winHalf,
                       j - winHalf : j + winSize - winHalf]
            if R[i, j] <= threshold or R[i, j] < window.max():
                ret[i, j] = 0
            else:
                ret[i, j] = R[i, j]
    return ret

img_name = input("insert png name : ")
img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

gaussian_filter = cif.get_gaussian_filter_2d(7, 1.5)
filtered_img = cif.cross_correlation_2d(img, gaussian_filter)

start = time.time()
R = compute_corner_response(filtered_img)
end = time.time()
timeslice = end - start
print(img_name, ":: compute_corner_response time ", timeslice, "s")

R[R < 0] = 0
result = cif.normalize(R, 0, 255)
cv2.imwrite("./result/part_3_corner_raw_" + img_name, result)

R = cif.normalize(R, 0, 1)

imgCpy = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
threshold = 0.1
imgCpy[R > threshold] = [0, 255, 0]
cv2.imwrite("./result/part_3_corner_bin_" + img_name, imgCpy)

winSize = 11

start = time.time()
suppressed_R = non_maximum_suppression_win(R, winSize)
end = time.time()
timeslice = end - start
print(img_name, ":: non_maximum_suppression_win time ", timeslice, "s")

imgCircle = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)

for i in range(img.shape[0]):
    for j in range(img.shape[1]):
        if suppressed_R[i, j] > 0:
            imgCircle = cv2.circle(imgCircle, (j, i), 5, (0, 255, 0), 2)
            
cv2.imwrite("./result/part_3_corner_sup_" + img_name, imgCircle)