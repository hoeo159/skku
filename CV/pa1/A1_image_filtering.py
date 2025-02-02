import cv2
import numpy as np
import math
import time

# the same intensity value as the nearest pixel inside the image
def pad_image(img, heightPad, widthPad):
    height, width = img.shape
    
    ret = np.zeros((height + 2 * heightPad, width + 2 * widthPad)).astype(np.uint8)
    
    ret[heightPad : -heightPad, widthPad : -widthPad] = img
    
    ret[:heightPad, widthPad:-widthPad] = img[0, :]
    ret[-heightPad:, widthPad:-widthPad] = img[-1, :]
    ret[heightPad:-heightPad, :widthPad] = img[:, 0].reshape(-1, 1)
    ret[heightPad:-heightPad, -widthPad:] = img[:, -1].reshape(-1, 1)
    
    ret[:heightPad, :widthPad] = img[0, 0]
    ret[:heightPad, -widthPad:] = img[0, -1]
    ret[-heightPad:, :widthPad] = img[-1, 0]
    ret[-heightPad:, -widthPad:] = img[-1, -1]
    
    return ret 

def cross_correlation_1d(img, kernel):
    
    axisFlag = True # if axis Flag is True > vertical kernel, False > horizontal kernel
    
    if kernel.shape[0] == 1:
        axisFlag = False
        sizeKernel = kernel.shape[1]
    elif kernel.shape[1] == 1:
        axisFlag = True
        sizeKernel = kernel.shape[0]
    else:
        print("Not 1D Kernel")
        return
    
    imgHeight = img.shape[0]
    imgWidth = img.shape[1]
    
    sizePad = (int)((sizeKernel - 1) / 2)
    
    padimg = pad_image(img, sizePad, sizePad)
    ret = np.zeros((imgHeight, imgWidth))
    
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            if axisFlag: # veritcal
                ret[i, j] = np.sum(padimg[i : i + sizeKernel, j + sizePad : j + sizePad + 1] * kernel)
            else:
                ret[i, j] = np.sum(padimg[i + sizePad : i + sizePad + 1, j : j + sizeKernel] * kernel)
    
    return ret
    
def cross_correlation_2d(img, kernel):
    heightKernel = kernel.shape[0]
    widthKernel = kernel.shape[1]
    
    imgHeight = img.shape[0]
    imgWidth = img.shape[1]
    
    heightPad = (int)((heightKernel - 1) / 2)
    widthPad = (int)((widthKernel - 1) / 2)
    
    padimg = pad_image(img, heightPad, widthPad)
    ret = np.zeros((imgHeight, imgWidth))
    
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            ret[i, j] = np.sum(padimg[i : i + heightKernel, j : j + widthKernel] * kernel)
    
    return ret

def get_gaussian_filter_1d(size, sigma):
    x = np.zeros((1, size))
    
    half = (int)((size - 1) / 2)

    for i in range(size):
        x[0, i] = i - half

    kernel = (np.exp(-x**2 / (2 * sigma**2))) / (math.sqrt(2 * math.pi) * sigma)
    kernel /= np.sum(kernel)
    
    return kernel
    
def get_gaussian_filter_2d(size, sigma):
    x = np.zeros((size, size))
    y = np.zeros((size, size))
    
    half = (int)((size - 1) / 2)
    
    for i in range(size):
        for j in range(size):
            x[i, j] = i - half
            y[i, j] = j - half
    
    kernel = (np.exp(-(x**2 + y**2) / (2 * sigma**2))) / ((2 * math.pi) * sigma**2)
    kernel /= np.sum(kernel)
    
    return kernel

img_name = input("insert png name : ")
img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
imgHeight = img.shape[0]

verticalKernel = np.ones((11,1)) / 11
horizonKernel = np.ones((1,11)) / 11
Kernel_2d = np.ones((5,11)) / 55

print()
print("### get_gaussian_filter_1d(5, 1) ###")
print(get_gaussian_filter_1d(5, 11))
print()
print("### get_gaussian_filter_2d(5, 1) ###")
print(get_gaussian_filter_2d(5, 1))
print()

gaussian_size = [5, 11, 17]
gaussian_sigma = [1, 6, 11]

result_images = []
timeslice2D = 0.0
timeslice1D = 0.0
target_size = 17
target_sigma = 6

for size in gaussian_size:
    for sigma in gaussian_sigma:
        kernel = get_gaussian_filter_2d(size, sigma)
        filtered_img = cross_correlation_2d(img, kernel).astype(np.uint8)
        cv2.putText(filtered_img, f"{size}x{size}, s={sigma}", (15, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 0), 2, cv2.LINE_AA)
        result_images.append(filtered_img)

result_images = np.array(result_images).reshape(3, 3, img.shape[0], img.shape[1])
result_ = np.block([[result_images[i, j] for j in range(len(gaussian_size))] for i in range(len(gaussian_size))])

start_time = time.time()
kernel_ = get_gaussian_filter_1d(target_size, target_sigma)
filtered_img_1D = cross_correlation_1d(img, kernel_).astype(np.uint8)
kernel_ = kernel_.reshape(target_size, 1)
filtered_img_1D = cross_correlation_1d(filtered_img_1D, kernel_).astype(np.uint8)
end_time = time.time()
timeslice1D = end_time - start_time

start_time = time.time()
kernel_ = get_gaussian_filter_2d(target_size, target_sigma)
filtered_img_2D = cross_correlation_2d(img, kernel_).astype(np.uint8)
end_time = time.time()
timeslice2D = end_time - start_time


diff_img = np.abs(filtered_img_2D - filtered_img_1D)
diff_sum = np.sum(diff_img)

print("sum of intensity differences : ", diff_sum)
print(img_name, ":: 1D filtering 2 time(17x17 s=6) ", timeslice1D, "s")
print(img_name, ":: 2D filtering 1 time(17x17 s=6) ", timeslice2D, "s")
print("time diff :  ", timeslice1D - timeslice2D)

cv2.imwrite("./result/part_1_gaussian_filtered_" + img_name, result_)

cv2.imshow("part_1_gaussian_filtered_" + img_name, diff_img)
cv2.waitKey(0)