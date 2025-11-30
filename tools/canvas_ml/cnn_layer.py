"""
Manual implementation of Convolutional Neural Network layers in pure Python.
"""
import math

def simple_resize(image_bytes, width=64, height=64):
    """
    A very simple resize function.
    Since we can't use PIL/opencv, we will skip complex resizing for this prototype
    and assume we process a small crop or just process the raw bytes as a 1D stream
    reshaped arbitrarily for the 'concept' of the CNN.

    In a real scenario without libraries, we'd parse the PNG header and data chunks.
    For this prototype, we'll create a dummy grayscale grid from the byte values.
    """
    # Simply take the first w*h bytes to simulate a grayscale image
    # This is obviously not "correct" image processing but serves the architecture pipeline.
    data = list(image_bytes)
    total_pixels = width * height

    # Pad if necessary
    if len(data) < total_pixels:
        data.extend([0] * (total_pixels - len(data)))

    # Crop if necessary
    data = data[:total_pixels]

    # Reshape to 2D
    matrix = []
    for i in range(height):
        row = data[i*width : (i+1)*width]
        matrix.append(row)

    return matrix

def convolve_2d(image_matrix, kernel):
    """
    Applies a 2D convolution with the given kernel.

    Args:
        image_matrix (list of list of int): The input image.
        kernel (list of list of float): The convolution kernel.

    Returns:
        list of list of float: The feature map.
    """
    img_h = len(image_matrix)
    img_w = len(image_matrix[0])
    k_h = len(kernel)
    k_w = len(kernel[0])

    pad_h = k_h // 2
    pad_w = k_w // 2

    # Output dimensions
    out_h = img_h - k_h + 1
    out_w = img_w - k_w + 1

    # We will skip padding logic for simplicity and just reduce output size (valid padding)
    output = [[0.0 for _ in range(out_w)] for _ in range(out_h)]

    for i in range(out_h):
        for j in range(out_w):
            val = 0.0
            for ki in range(k_h):
                for kj in range(k_w):
                    val += image_matrix[i + ki][j + kj] * kernel[ki][kj]
            output[i][j] = val

    return output

def max_pooling(feature_map, pool_size=2, stride=2):
    """
    Applies max pooling.
    """
    h = len(feature_map)
    w = len(feature_map[0])

    out_h = (h - pool_size) // stride + 1
    out_w = (w - pool_size) // stride + 1

    output = [[0.0 for _ in range(out_w)] for _ in range(out_h)]

    for i in range(out_h):
        for j in range(out_w):
            start_i = i * stride
            start_j = j * stride

            max_val = -float('inf')
            for pi in range(pool_size):
                for pj in range(pool_size):
                    if start_i + pi < h and start_j + pj < w:
                        val = feature_map[start_i + pi][start_j + pj]
                        if val > max_val:
                            max_val = val
            output[i][j] = max_val

    return output

def flatten(matrix):
    """Flattens a 2D matrix into a 1D list."""
    flat = []
    for row in matrix:
        flat.extend(row)
    return flat

def extract_features(image_bytes):
    """
    Runs the 'CNN' pipeline on raw image bytes.
    """
    # 1. Preprocess (Mock resize/grayscale)
    input_matrix = simple_resize(image_bytes, 64, 64)

    # 2. Define Kernels
    # Edge detection kernel
    kernel_edge = [
        [-1, -1, -1],
        [-1,  8, -1],
        [-1, -1, -1]
    ]

    # 3. Convolution
    feature_map = convolve_2d(input_matrix, kernel_edge)

    # 4. Pooling
    pooled_map = max_pooling(feature_map, pool_size=2, stride=2)

    # 5. Flatten
    vector = flatten(pooled_map)

    # Normalize vector to have reasonable magnitude
    if not vector:
        return []
    max_val = max(abs(x) for x in vector) if any(vector) else 1
    if max_val == 0: max_val = 1
    normalized = [x / max_val for x in vector]

    # Return a subset to keep vector size manageable for our 'model'
    return normalized[:20]
