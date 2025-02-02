import cv2
import numpy as np
import random

def Hamming_Distance(des1, des2):
    distance = 0
    
    for byte1, byte2 in zip(des1, des2):
        xor_result = byte1 ^ byte2
        distance += bin(xor_result).count('1')
        # while xor_result:
        #     distance += xor_result & 1
        #     xor_result >>= 1
    
    return distance

def get_normalize(points):
    N = points.shape[0]
    
    mean = np.mean(points, axis = 0)
    sub_mean = points - mean
    
    max_distance = -1.0
    for i in range(N):
        dist = np.sqrt(sub_mean[i, 0] ** 2 + sub_mean[i, 1] ** 2)
        if max_distance < 0.0 or max_distance < dist:
            max_distance = dist

    factor = np.sqrt(2) / max_distance
    
    ret = sub_mean * factor
            
    # new X = (X - Xmean) * factor
    T = np.array([[factor, 0, -factor * mean[0]],
                  [0, factor, -factor * mean[1]],
                  [0, 0, 1]])
    
    return ret, T

def compute_homography(srcP, destP):
    
    srcNorm, Ts = get_normalize(srcP)
    destNorm, Td = get_normalize(destP)
    
    N = srcP.shape[0]
    A = np.zeros((N*2, 9), dtype=np.float32)
    
    for i in range(N):
        x1, y1 = srcNorm[i, 0], srcNorm[i, 1]
        x2, y2 = destNorm[i, 0], destNorm[i, 1]
        
        A[2*i] = [-x1, -y1, -1, 0, 0, 0, x2*x1, x2*y1, x2]
        A[2*i + 1] = [0, 0, 0, -x1, -y1, -1, y2*x1, y2*y1, y2]
    
    [u, s, vT] = np.linalg.svd(A)
    
    h = np.reshape(vT[-1], (3, 3))
    
    H = np.linalg.inv(Td) @ h @ Ts
    ret = H / H[2,2]
    
    return ret

def compute_homography_ransac(srcP, destP, threshold):
    max_inliers = []
    final_H = None
    iterations = 1000
    N = srcP.shape[0]

    for tmp in range(iterations):
        ran_idx = random.sample(range(N), 4)
        src_sample = srcP[ran_idx]
        dest_sample = destP[ran_idx]

        H = compute_homography(src_sample, dest_sample)
        
        srcP_homogeneous = np.zeros((N, 3), dtype=np.float32)
        for i in range(N):
            srcP_homogeneous[i, 0] = srcP[i, 0]
            srcP_homogeneous[i, 1] = srcP[i, 1]
            srcP_homogeneous[i, 2] = 1

        projected_destP = (H @ srcP_homogeneous.T).T
        
        for i in range(N):
            projected_destP[i, 0] /= projected_destP[i, 2]
            projected_destP[i, 1] /= projected_destP[i, 2]
            projected_destP[i, 2] /= projected_destP[i, 2]
        
        # throughput 이랑 비교
        errors = np.linalg.norm(destP - projected_destP[:, :2], axis=1)
        inliers = np.where(errors < threshold)[0]

        if len(inliers) > len(max_inliers):
            max_inliers = inliers
            final_H = H

    if final_H is not None:
        # print(max_inliers)
        src_inliers = srcP[max_inliers]
        dest_inliers = destP[max_inliers]
        final_H = compute_homography(src_inliers, dest_inliers)

    return final_H



img_name = "cv_cover.jpg"
img1 = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

img_name = "cv_desk.png"
img2 = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

orb = cv2.ORB_create()
kp1 = orb.detect(img1, None)
kp1, des1 = orb.compute(img1, kp1) # key point and descriptor

kp2 = orb.detect(img2, None)
kp2, des2 = orb.compute(img2, kp2) # key point and descriptor

matches = []
for i, descriptor1 in enumerate(des1):
    min_distance = float(-1.0)
    min_index = -1
    for j, descriptor2 in enumerate(des2):
        distance = Hamming_Distance(descriptor1, descriptor2)
        if distance < min_distance or min_distance < 0:
            min_distance = distance
            min_index = j
    # DMatch 타입으로 array 만들기
    match = cv2.DMatch(_queryIdx = i, _trainIdx = min_index, _distance = (float)(min_distance))
    matches.append(match)
    
matches = sorted(matches, key = lambda x: x.distance)
    
matching_result = cv2.drawMatches(img1, kp1, img2, kp2, matches[:10], None, flags = 2)

# N = 30
# N = 15
N = 13

srcP = np.zeros((N, 2), dtype = np.float32)
destP = np.zeros((N, 2), dtype = np.float32)

for idx in range(N):
    tmp = matches[idx]
    srcP[idx] = kp1[tmp.queryIdx].pt
    destP[idx] = kp2[tmp.trainIdx].pt

hn = compute_homography(srcP, destP)
hr = compute_homography_ransac(srcP, destP, 5.0)

height, width = img2.shape

img_name = "hp_cover.jpg"
img3 = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
scr_height, dst_width = img1.shape[:2]
img3_resized = cv2.resize(img3, (dst_width, scr_height))

norm_result = cv2.warpPerspective(img1, hn, (width, height))
ransac_result = cv2.warpPerspective(img1, hr, (width, height))
hp_result = cv2.warpPerspective(img3_resized, hr, (width, height))

result_norm = np.copy(img2)
for i in range(height):
    for j in range(width):
        if norm_result[i, j] != 0:
            result_norm[i, j] = norm_result[i, j]
        else:
            result_norm[i, j] = img2[i, j]
            
result_ransac = np.copy(img2)
for i in range(height):
    for j in range(width):
        if ransac_result[i, j] != 0:
            result_ransac[i, j] = ransac_result[i, j]
        else:
            result_ransac[i, j] = img2[i, j] 
            
result_hp = np.copy(img2)
for i in range(height):
    for j in range(width):
        if hp_result[i, j] != 0:
            result_hp[i, j] = hp_result[i, j]
        else:
            result_hp[i, j] = img2[i, j] 

img_name = "diamondhead-10.png"
img_left = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)
img_name = "diamondhead-11.png"
img_right = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

# image stitch

orb = cv2.ORB_create()
kp1 = orb.detect(img_right, None)
kp1, des1 = orb.compute(img_right, kp1) # key point and descriptor

kp2 = orb.detect(img_left, None)
kp2, des2 = orb.compute(img_left, kp2) # key point and descriptor

matches = []
for i, descriptor1 in enumerate(des1):
    min_distance = float(-1.0)
    min_index = -1
    for j, descriptor2 in enumerate(des2):
        distance = Hamming_Distance(descriptor1, descriptor2)
        if distance < min_distance or min_distance < 0:
            min_distance = distance
            min_index = j
    # DMatch 타입으로 array 만들기
    match = cv2.DMatch(_queryIdx = i, _trainIdx = min_index, _distance = (float)(min_distance))
    matches.append(match)
    
matches = sorted(matches, key = lambda x: x.distance)

N = 30
srcPstitch = np.zeros((N, 2), dtype = np.float32)
destPstitch = np.zeros((N, 2), dtype = np.float32)
    
for idx in range(N):
    tmp = matches[idx]
    srcPstitch[idx] = kp1[tmp.queryIdx].pt
    destPstitch[idx] = kp2[tmp.trainIdx].pt

H = compute_homography_ransac(srcPstitch, destPstitch, 5.0)
    
height, width = img_left.shape

stitch_result = cv2.warpPerspective(img_right, H, (width + img_right.shape[1], height))
stitch_result[0 : img_left.shape[0], 0 : img_left.shape[1]] = img_left

cv2.imshow("ORB", matching_result)

cv2.imshow("norm_result", result_norm)
cv2.imshow("norm", norm_result)

cv2.imshow("ransac", ransac_result)
cv2.imshow("ransac_result", result_ransac)

cv2.imshow("hp", hp_result)
cv2.imshow("hp_result", result_hp)

cv2.imshow("stitch", stitch_result)

cv2.waitKey()
cv2.destroyAllWindows()
