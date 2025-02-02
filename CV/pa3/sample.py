import cv2
import numpy as np
import matplotlib.pyplot as plt

def load_data():
    # 이미지 로드
    img1 = cv2.imread('temple1.png', cv2.IMREAD_GRAYSCALE)
    img2 = cv2.imread('temple2.png', cv2.IMREAD_GRAYSCALE)

    # 대응점 로드
    M = np.loadtxt('temple_matches.txt')
    
    return img1, img2, M

def compute_F_raw(M):
    N = M.shape[0]
    A = np.zeros((N, 9))
    for i in range(N):
        x1, y1, x2, y2 = M[i]
        A[i] = [x1*x2, y1*x2, x2, x1*y2, y1*y2, y2, x1, y1, 1]
    
    U, S, Vt = np.linalg.svd(A)
    F = Vt[-1].reshape(3, 3)
    
    # 강제적으로 F의 rank를 2로 만듦
    U, S, Vt = np.linalg.svd(F)
    S[2] = 0
    F = np.dot(U, np.dot(np.diag(S), Vt))
    
    return F

def normalize(points):
    mean = np.mean(points, axis=0)
    std = np.std(points)
    scale = np.sqrt(2) / std
    
    T = np.array([
        [scale, 0, -scale * mean[0]],
        [0, scale, -scale * mean[1]],
        [0, 0, 1]
    ])
    
    points_normalized = np.dot(T, np.vstack((points.T, np.ones((1, points.shape[0])))))
    return T, points_normalized[:2].T

def compute_F_norm(M):
    T1, norm_pts1 = normalize(M[:, :2])
    T2, norm_pts2 = normalize(M[:, 2:])
    
    norm_M = np.hstack((norm_pts1, norm_pts2))
    F_norm = compute_F_raw(norm_M)
    
    F = np.dot(T2.T, np.dot(F_norm, T1))
    
    return F

def compute_F_mine(M):
    # Custom Fundamental Matrix 계산 방법 (이 예제에서는 normalized 방법을 그대로 사용)
    return compute_F_norm(M)

def compute_avg_reproj_error(_F, _M):
    N = _M.shape[0]
    
    X1 = np.c_[ _M[:,0:2] , np.ones((N,1)) ].T
    L1 = np.dot(_F, X1).T
    norms1 = np.sqrt(L1[:,0]**2 + L1[:,1]**2)
    L1 = L1 / norms1[:, np.newaxis]
    L1 = np.sum(L1 * np.c_[ _M[:,2:4] , np.ones((N,1)) ], axis=1)
    error1 = np.sum(np.abs(L1))
    
    X2 = np.c_[ _M[:,2:4] , np.ones((N,1)) ].T
    L2 = np.dot(_F.T, X2).T
    norms2 = np.sqrt(L2[:,0]**2 + L2[:,1]**2)
    L2 = L2 / norms2[:, np.newaxis]
    L2 = np.sum(L2 * np.c_[ _M[:,0:2] , np.ones((N,1)) ], axis=1)
    error2 = np.sum(np.abs(L2))
    
    return (error1 + error2) / (N * 2)

def visualize_epipolar_lines(img1, img2, M, F):
    def draw_epipolar_line(img, line, color):
        r, c = img.shape
        x0, y0 = map(int, [0, -line[2] / line[1]])
        x1, y1 = map(int, [c, -(line[2] + line[0] * c) / line[1]])
        cv2.line(img, (x0, y0), (x1, y1), color, 1)
    
    while True:
        idx = np.random.choice(M.shape[0], 3, replace=False)
        pts1 = M[idx, :2]
        pts2 = M[idx, 2:]
        
        img1_copy = img1.copy()
        img2_copy = img2.copy()
        
        colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
        
        for pt1, pt2, color in zip(pts1, pts2, colors):
            cv2.circle(img1_copy, tuple(pt1.astype(int)), 5, color, -1)
            cv2.circle(img2_copy, tuple(pt2.astype(int)), 5, color, -1)
            
            line1 = np.dot(F.T, np.array([pt2[0], pt2[1], 1]))
            line2 = np.dot(F, np.array([pt1[0], pt1[1], 1]))
            
            draw_epipolar_line(img1_copy, line1, color)
            draw_epipolar_line(img2_copy, line2, color)
        
        combined_img = np.hstack((img1_copy, img2_copy))
        cv2.imshow('Epipolar Lines', combined_img)
        
        if cv2.waitKey(0) & 0xFF == ord('q'):
            break
    
    cv2.destroyAllWindows()

if __name__ == "__main__":
    img1, img2, M = load_data()
    
    F_raw = compute_F_raw(M)
    F_norm = compute_F_norm(M)
    F_mine = compute_F_mine(M)
    
    error_raw = compute_avg_reproj_error(F_raw, M)
    error_norm = compute_avg_reproj_error(F_norm, M)
    error_mine = compute_avg_reproj_error(F_mine, M)
    
    print(f"Average Reprojection Errors (temple1.png and temple2.png)")
    print(f"Raw  = {error_raw}")
    print(f"Norm = {error_norm}")
    print(f"Mine = {error_mine}")
    
    visualize_epipolar_lines(img1, img2, M, F_mine)
