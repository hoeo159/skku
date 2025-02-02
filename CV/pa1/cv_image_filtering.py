import numpy as np
import math

def pad_image(img, heightPad, widthPad):
    height, width = img.shape
    
    ret = np.zeros((height + 2 * heightPad, width + 2 * widthPad))
    
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

def normalize(img, low, high):
    min_val = np.min(img)
    max_val = np.max(img)

    normalized_img = ((img - min_val) * (high - low) / (max_val - min_val))

    return normalized_img