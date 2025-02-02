import numpy as np

def get_normalize(points):
    N = len(points)
    mean = np.mean(points, axis=0)
    ret = points - mean
    
    max_distance = -1.0
    for i in range(N):
        dist = np.sqrt(ret[i][0] ** 2 + ret[i][1] ** 2)
        if max_distance < 0.0 or max_distance < dist:
            max_distance = dist
            
    factor = np.sqrt(2) / max_distance
    
    ret = ret * factor
            
    # new X = (X - Xmean) * factor
    T = np.array([[factor, 0, -factor * mean[0]],
                  [0, factor, -factor * mean[1]],
                  [0, 0, 1]])
    
    return ret, T
    
def compute_homography(srcP, destP):
    srcNorm, Ts = get_normalize(srcP)
    destNorm, Td = get_normalize(destP)
    
    print("srcNorm:\n", srcNorm)
    print("Ts:\n", Ts)
    print("destNorm:\n", destNorm)
    print("Td:\n", Td)
    
    N = srcNorm.shape[0]
    A = []
    for i in range(N):
        x1, y1 = srcNorm[i][0], srcNorm[i][1]
        x2, y2 = destNorm[i][0], destNorm[i][1]
        
        A.append([-x1, -y1, -1, 0, 0, 0, x2*x1, x2*y1, x2])
        A.append([0, 0, 0, -x1, -y1, -1, y2*x1, y2*y1, y2])
    
    A = np.array(A)
    print("A:\n", A)
    
    u, s, vT = np.linalg.svd(A)
    print("u:\n", u)
    print("s:\n", s)
    print("vT:\n", vT)
    
    h = vT[-1].reshape(3, 3)
    print("h:\n", h)
    
    H = np.dot(np.linalg.inv(Td), np.dot(h, Ts))
    print("H (before normalization):\n", H)
    return H / H[2,2]

# 테스트 예제
srcP = np.array([[10, 20], [30, 40], [50, 60], [70, 80]])
destP = np.array([[15, 25], [35, 45], [55, 65], [75, 85]])

H = compute_homography(srcP, destP)

print("Computed Homography Matrix H:\n", H)

A = np.array([[1, 2, 4], [2, 6, 1], [3, 2, 4]])
U, Sigma, V_T = np.linalg.svd(A)
print(V_T)
x = V_T.T[:, -1]
h = V_T[-1]

print(x)
print(A@x)
print(h)

