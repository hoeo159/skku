import numpy as np 
import cv2
import random
import time


from scipy.optimize import least_squares


def compute_avg_reproj_error(_M, _F):
    N = _M.shape[0]

    X = np.c_[ _M[:,0:2] , np.ones( (N,1) ) ].transpose()
    L = np.matmul( _F , X ).transpose()
    norms = np.sqrt( L[:,0]**2 + L[:,1]**2 )
    L = np.divide( L , np.kron( np.ones( (3,1) ) , norms ).transpose() )
    L = ( np.multiply( L , np.c_[ _M[:,2:4] , np.ones( (N,1) ) ] ) ).sum(axis=1)
    error = (np.fabs(L)).sum()

    X = np.c_[_M[:, 2:4], np.ones((N, 1))].transpose()
    L = np.matmul(_F.transpose(), X).transpose()
    norms = np.sqrt(L[:, 0] ** 2 + L[:, 1] ** 2)
    L = np.divide(L, np.kron(np.ones((3, 1)), norms).transpose())
    L = ( np.multiply( L , np.c_[ _M[:,0:2] , np.ones( (N,1) ) ] ) ).sum(axis=1)
    error += (np.fabs(L)).sum()

    return error/(N*2)

def compute_F_raw(M):
    N = M.shape[0]
    A = np.zeros((N, 9)).astype(np.float64)
    for i in range(N):
        x1, y1, x2, y2 = M[i]
        A[i] = [x1*x2, x2*y1, x2, x1*y2, y1*y2, y2, x1, y1, 1]

    [u, s, vT] = np.linalg.svd(A)
    F = np.reshape(vT[-1], (3, 3))
    
    return F

def get_normalize(points):
    N = points.shape[0]
    
    mean = np.mean(points, axis=0)
    sub_mean = points - mean
    
    distances = np.linalg.norm(sub_mean, axis=1)
    max_distance = np.max(distances)
    
    factor = np.sqrt(2) / max_distance
    ret = sub_mean * factor
    
    T = np.array([[factor, 0, -factor * mean[0]],
                  [0, factor, -factor * mean[1]],
                  [0, 0, 1]])
    
    return T, ret

def compute_F_norm(M):
    T1, norm_pts1 = get_normalize(M[:, :2])
    T2, norm_pts2 = get_normalize(M[:, 2:])
    
    norm_M = np.hstack((norm_pts1, norm_pts2))
    F_norm = compute_F_raw(norm_M)
    
    F = T2.T @ F_norm @ T1
    
    return F

def compute_F_mine(M):
    final_F = None
    min_err = None
    iterations = 20000
    N = M.shape[0]
    
    start = time.time()

    for tmp in range(iterations):
        ran_idx = random.sample(range(N), 50)
        
        sample_M = M[ran_idx]
        
        F = compute_F_norm(sample_M)
        err = compute_avg_reproj_error(M, F)

        if min_err is None or min_err > err:
            min_err = err
            final_F = F
            
        end = time.time()
        if(end - start > 4.8):
            break
    
    return final_F

def draw_epipolar_line(img, line, color):
    tmp1, width, tmp2 = img.shape
    x0, y0 = map(int, [0, -line[2] / line[1]])
    x1, y1 = map(int, [width, -(line[2] + line[0] * width) / line[1]])
    cv2.line(img, (x0, y0), (x1, y1), color, 1)

idx_image = 0
img1_arr = np.array(["temple1.png", "house1.jpg", "library1.jpg"])
img2_arr = np.array(["temple2.png", "house2.jpg", "library2.jpg"])
match_arr = np.array(["temple_matches.txt", "house_matches.txt", "library_matches.txt"])
mine_F = np.zeros((3, 3, 3))

dirloc = "CV_Assignment_3_Data/"

while idx_image < 3:
    img1_name = dirloc + img1_arr[idx_image]
    img2_name = dirloc + img2_arr[idx_image]
    match_name = dirloc + match_arr[idx_image]

    M = np.loadtxt(match_name)
    N = M.shape[0]

    result_raw = compute_F_raw(M)
    result_norm = compute_F_norm(M)

    start = time.time()
    mine_F[idx_image] = compute_F_mine(M)
    end = time.time()

    print("time compute_mine : ", end - start, "s")

    error_raw = compute_avg_reproj_error(M, result_raw)
    error_norm = compute_avg_reproj_error(M, result_norm)
    error_mine = compute_avg_reproj_error(M, mine_F[idx_image])

    print("Average Reprojection Errors (", img1_arr[idx_image], " and ", img2_arr[idx_image], ")")
    print("\tRaw  = ", error_raw)
    print("\tNorm = ", error_norm)
    print("\tMine = ", error_mine)
    
    idx_image += 1

idx_image = 0

while idx_image < 3:
    img1_name = dirloc + img1_arr[idx_image]
    img2_name = dirloc + img2_arr[idx_image]
    match_name = dirloc + match_arr[idx_image]

    img1 = cv2.imread(img1_name, cv2.IMREAD_ANYCOLOR)
    img2 = cv2.imread(img2_name, cv2.IMREAD_ANYCOLOR)

    M = np.loadtxt(match_name)
    N = M.shape[0]
    
    while True:
        ran_idx = random.sample(range(N), 3)
        pts1 = M[ran_idx, :2].astype(int)
        pts2 = M[ran_idx, 2:].astype(int)
        
        lines1 = [mine_F[idx_image] @ np.array([pt[0], pt[1], 1]) for pt in pts1]
        lines2 = [mine_F[idx_image].T @ np.array([pt[0], pt[1], 1]) for pt in pts2]
        
        img1_copy = img1.copy()
        img2_copy = img2.copy()
        
        colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
        
        for pt1, pt2, line1, line2, color in zip(pts1, pts2, lines1, lines2, colors):
            cv2.circle(img1_copy, pt1.astype(int), 5, color, -1)
            cv2.circle(img2_copy, pt2.astype(int), 5, color, -1)
            
            draw_epipolar_line(img1_copy, line2, color)
            draw_epipolar_line(img2_copy, line1, color)
        
        img_result = np.hstack((img1_copy, img2_copy))
        cv2.imshow('Epipolar Lines', img_result)
        
        key = cv2.waitKey(0) & 0xFF
        if key == ord('q'):
            idx_image += 1
            break

cv2.destroyAllWindows()