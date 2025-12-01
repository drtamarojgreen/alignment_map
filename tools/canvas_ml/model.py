import random
import math

class KMeans:
    def __init__(self, k=3, max_iters=100, tol=0.001):
        self.k = k
        self.max_iters = max_iters
        self.tol = tol
        self.centroids = []
        self.min_vals = []
        self.max_vals = []

    def fit(self, data):
        """
        Fits the K-Means model to the data (list of vectors).
        Includes normalization step.
        """
        if not data:
            return

        n_samples = len(data)
        n_features = len(data[0])

        # 1. Normalize Data (Min-Max Scaling)
        self.min_vals = [float('inf')] * n_features
        self.max_vals = [-float('inf')] * n_features

        for row in data:
            for i, val in enumerate(row):
                if val < self.min_vals[i]: self.min_vals[i] = val
                if val > self.max_vals[i]: self.max_vals[i] = val

        # Create normalized dataset for training
        normalized_data = [self._normalize(row) for row in data]

        # Initialize centroids randomly from normalized data
        self.centroids = [normalized_data[i] for i in random.sample(range(n_samples), min(self.k, n_samples))]

        for _ in range(self.max_iters):
            clusters = [[] for _ in range(self.k)]

            # Assign samples to closest centroid
            for point in normalized_data:
                distances = [self._euclidean_distance(point, c) for c in self.centroids]
                closest_idx = distances.index(min(distances))
                clusters[closest_idx].append(point)

            # Update centroids
            new_centroids = []
            max_shift = 0.0

            for i in range(self.k):
                cluster = clusters[i]
                if not cluster:
                    # Keep old centroid if cluster is empty
                    new_centroids.append(self.centroids[i])
                    continue

                # Calculate mean
                mean_vector = [0.0] * n_features
                for point in cluster:
                    for j in range(n_features):
                        mean_vector[j] += point[j]

                mean_vector = [x / len(cluster) for x in mean_vector]
                new_centroids.append(mean_vector)

                shift = self._euclidean_distance(self.centroids[i], mean_vector)
                if shift > max_shift:
                    max_shift = shift

            self.centroids = new_centroids

            if max_shift < self.tol:
                break

    def predict(self, point):
        """
        Predicts the cluster index for a single point.
        """
        if not self.centroids:
            return -1

        # Normalize the point using the fit parameters
        norm_point = self._normalize(point)

        distances = [self._euclidean_distance(norm_point, c) for c in self.centroids]
        return distances.index(min(distances))

    def _normalize(self, point):
        """Helper to normalize a vector using stored min/max."""
        normalized = []
        for i, val in enumerate(point):
            range_val = self.max_vals[i] - self.min_vals[i]
            if range_val == 0:
                normalized.append(0.0)
            else:
                normalized.append((val - self.min_vals[i]) / range_val)
        return normalized

    def _euclidean_distance(self, p1, p2):
        return math.sqrt(sum((a - b) ** 2 for a, b in zip(p1, p2)))

# Simple logic to interpret cluster as "High/Low Value"
def predict_value(features, cluster_idx):
    # This is a dummy logic. Real logic would depend on labeled data or known good clusters.
    # For now, we assume cluster 0 is "Standard", 1 is "High Value", 2 is "Regressive" just for the prototype structure.
    # In a real unsupervised scenario, we'd analyze the centroid properties (e.g. high contrast = good).

    labels = ["Standard Improvement", "High Impact Innovation", "Potential Regression"]
    return labels[cluster_idx % len(labels)]
