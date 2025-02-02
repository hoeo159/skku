import cv2
import numpy as np

def get_transformed_image(img, M):
    ret = np.zeros((801, 801))
    ret = np.full_like(ret, 255, dtype=np.uint8)
    cv2.arrowedLine(ret, (0, 400), (800, 400), (0.0, 0.0, 0.0), thickness= 2, tipLength=0.02)
    cv2.arrowedLine(ret, (400, 800), (400, 0), (0.0, 0.0, 0.0), thickness= 2, tipLength=0.02)
    
    h, w = img.shape
    
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            transformed = np.array([0, 0, 0]).astype(float)
            for k in range(3):
                transformed[k] = M[k][0] * (i - h / 2) + M[k][1] * (j - w / 2) + M[k][2] * 1
            if img[i, j] != 255:
                X1 = 400 + int(transformed[0])
                Y1 = 400 + int(transformed[1])
                
                X3 = 400 + np.round(transformed[0]).astype(int)
                Y3 = 400 + np.round(transformed[1]).astype(int)
                
                ret[X1, Y1] = img[i, j]

                if(ret[X3, Y3] == 255): # rotate artifacts
                    ret[X3, Y3] = img[i, j]

                # 굳이 1이 아니다 > 이게 필요한가?
                if(M[0][0] != 1): # height artifacts
                    scale = (int)(M[0][0])
                    for m in range(1, scale + 1):
                        if ret[X1 + m, Y1] == 255:
                            ret[X1 + m, Y1] = img[i, j]
                        if ret[X1 - m, Y1] == 255:
                            ret[X1 - m, Y1] = img[i, j]
                if(M[1][1] != 1): # width articfacts
                    scale = (int)(M[1][1])
                    for m in range(1, scale + 1):
                        if ret[X1, Y1 + m] == 255:
                            ret[X1, Y1 + m] = img[i, j]
                        if ret[X1, Y1 - m] == 255:
                            ret[X1, Y1 - m] = img[i, j]

    return ret

img_name = "smile.png"
img = cv2.imread(img_name, cv2.IMREAD_GRAYSCALE)

M = np.array([[1.0, 0, 0],
              [0, 1.0, 0],
              [0, 0, 1.0]])

plane = get_transformed_image(img, M)
while True:
    cv2.imshow("image", plane)
    key = cv2.waitKey(0) & 0xFF
    if key == ord('a'):
        R = np.array([[1, 0, 0],
                      [0, 1, -5],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('d'):
        R = np.array([[1, 0, 0],
                      [0, 1, 5],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('w'):
        R = np.array([[1, 0, -5],
                      [0, 1, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('s'):
        R = np.array([[1, 0, 5],
                      [0, 1, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('r'):
        R = np.array([[np.cos(np.pi / 36), -np.sin(np.pi / 36), 0],
                      [np.sin(np.pi / 36), np.cos(np.pi / 36), 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('t'):
        R = np.array([[np.cos(-np.pi / 36), -np.sin(-np.pi / 36), 0],
                      [np.sin(-np.pi / 36), np.cos(-np.pi / 36), 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('f'):
        R = np.array([[1, 0, 0],
                      [0, -1, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('g'):
        R = np.array([[-1, 0, 0],
                      [0, 1, 0],
                      [0, 0, 1]])
        
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('y'):
        R = np.array([[0.95, 0, 0],
                      [0, 1, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('u'):
        R = np.array([[1.05, 0, 0],
                      [0, 1, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('x'):
        R = np.array([[1, 0, 0],
                      [0, 0.95, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('c'):
        R = np.array([[1, 0, 0],
                      [0, 1.05, 0],
                      [0, 0, 1]])
        M = np.dot(R, M)
        plane = get_transformed_image(img, M)
    elif key == ord('h'):
        M = np.array([[1, 0, 0],
                     [0, 1, 0],
                     [0, 0, 1]])
        plane = get_transformed_image(img, M)
    elif key == ord('q'):
        break

cv2.destroyAllWindows()
