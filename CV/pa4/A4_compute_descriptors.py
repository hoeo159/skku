import numpy as np
from sklearn.cluster import KMeans
from sklearn.preprocessing import normalize
import bin_rw as rw

dir_name = "./features/"
dir_cnn = dir_name + "cnn/"
dir_sift = dir_name + "sift/"

def compute_vlad(cnn_features, kmeans):
    h, w, d = cnn_features.shape

    flat_features = cnn_features.reshape(-1, d)
    
    assignments = kmeans.predict(flat_features)
    k = kmeans.n_clusters
    vlad = np.zeros((k, d), dtype=np.float32)
    
    for i in range(k):
        if np.sum(assignments == i) > 0:
            vlad[i] = np.sum(flat_features[assignments == i] - kmeans.cluster_centers_[i], axis=0)
    
    vlad = vlad.flatten()
    # vlad = vlad / np.linalg.norm(vlad)
    vlad = vlad / np.sum(np.abs(vlad))

    return vlad

def compute_cnn_descriptor(cnn_features):
    pooled_features = np.mean(cnn_features, axis=(0, 1))
    return pooled_features

def mean_compute_cnn_descriptor(cnn_features):
    pooled_features = np.mean(cnn_features, axis=(0, 1))  # (14, 14, 512) -> (512)
    return pooled_features
def max_compute_cnn_descriptor(cnn_features):
    pooled_features = np.mean(cnn_features, axis=(0, 1))  # (14, 14, 512) -> (512)
    return pooled_features
            
cnn_features = rw.read_cnn_features()

flat_features = cnn_features.reshape(-1, cnn_features.shape[-1])

dimension = 8192
num_clusters = dimension // cnn_features.shape[-1]
kmeans = KMeans(n_clusters=num_clusters).fit(flat_features)

cluster_labels = kmeans.predict(flat_features)
cluster_centers = kmeans.cluster_centers_

vlad_descriptors = np.zeros((2000, dimension))

num_clusters_bovw = 256  # BoVW를 위한 클러스터 수, 예: 256
kmeans_bovw = KMeans(n_clusters=num_clusters_bovw).fit(flat_features)

bovw_histograms = np.zeros((2000, num_clusters_bovw))
for i in range(2000):
    image_features = cnn_features[i].reshape(-1, cnn_features.shape[-1])
    cluster_labels = kmeans_bovw.predict(image_features)
    for label in cluster_labels:
        bovw_histograms[i, label] += 1
# descriptors = np.array([compute_vlad(cnn, kmeans) for cnn in cnn_features])
bovw_histograms = normalize(bovw_histograms, norm='l1')

# VLAD
descriptors = normalize(vlad_descriptors, norm='l1')
# Mean Pooling
# mean_descriptors = np.array([mean_compute_cnn_descriptor(cnn) for cnn in cnn_features])
#(2000, 512)
# mean_pooled_descriptors = np.mean(descriptors.reshape(2000, dimension // 512, 512), axis=1)
max_pooled_descriptors = np.max(descriptors.reshape(2000, 16, 512), axis=1)
mean_pooled_descriptors = np.mean(bovw_histograms.reshape(2000, 16, 512), axis=1)


# combined_descriptors = np.concatenate((mean_pooled_descriptors, max_pooled_descriptors), axis=1)

descriptors = max_pooled_descriptors

rw.save_descriptors(mean_pooled_descriptors, './A4_2020312914.des')
num_images, descriptor_dim, reduced_descriptors = rw.read_test_descriptors('./A4_2020312914.des')

print(f"cnn_features shape: {cnn_features.shape}")
print(f"cnn_feature reshape : {flat_features.shape}")
print(f"vlad_descriptors shape: {vlad_descriptors.shape}")
print(f"Descriptors shape: {descriptors.shape}")  # Expecting shape (2000, k * 512)
print(f"Number of images: {num_images}")
print(f"Descriptor dimension: {descriptor_dim}")