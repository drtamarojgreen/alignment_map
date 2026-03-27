"""
Unsupervised Machine Learning Model (K-Means) Implementation.
"""
import random
import math

class KMeans:
    def __init__(self, k=3, max_iters=100):
        self.k = k
        self.max_iters = max_iters
        self.centroids = []

    def fit(self, data):
        """
        Fits the model to the data.
        data: list of vectors (lists of floats)
        """
        if not data:
            return

        # Initialize centroids randomly from data points
        self.centroids = random.sample(data, min(self.k, len(data)))

        for _ in range(self.max_iters):
            clusters = [[] for _ in range(self.k)]

            # Assign points to clusters
            for point in data:
                distances = [self._euclidean_distance(point, c) for c in self.centroids]
                closest_idx = distances.index(min(distances))
                clusters[closest_idx].append(point)

            # Update centroids
            new_centroids = []
            for cluster in clusters:
                if not cluster:
                    # If empty, keep random existing or re-init (simplified: keep existing)
                    new_centroids.append(self.centroids[len(new_centroids)])
                else:
                    # Mean of cluster
                    dim = len(cluster[0])
                    mean_point = [sum(p[i] for p in cluster) / len(cluster) for i in range(dim)]
                    new_centroids.append(mean_point)

            # Check convergence
            if new_centroids == self.centroids:
                break

            self.centroids = new_centroids

    def predict(self, point):
        """
        Returns the cluster index for a given point.
        """
        distances = [self._euclidean_distance(point, c) for c in self.centroids]
        return distances.index(min(distances))

    def _euclidean_distance(self, p1, p2):
        return math.sqrt(sum((a - b) ** 2 for a, b in zip(p1, p2)))

def analyze_value(features, cluster_id, metrics):
    """
    Predicts value based on features and cluster.
    """
    # Simplified logic as per plan
    score = 0

    # High contrast is good
    score += metrics.get('contrast', 0) * 10

    # Balanced whitespace is good (0.3 - 0.7)
    ws = metrics.get('whitespace', 0)
    if 0.3 <= ws <= 0.7:
        score += 5

    # Cluster based logic (arbitrary for this prototype)
    # Assume cluster 0 is "standard", 1 is "outlier/innovative", 2 is "noisy"
    if cluster_id == 1:
        score += 2 # Bonus for innovation

    if score > 12:
        return "High Value"
    elif score > 8:
        return "Medium Value"
    else:
        return "Low Value"
