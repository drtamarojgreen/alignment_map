def convolve_2d(pixels, width, height, kernel):
    """
    Applies a 2D convolution with the given kernel to the grayscale version of the pixels.

    Args:
        pixels (list): Flattened RGBA list [r, g, b, a, ...].
        width (int): Image width.
        height (int): Image height.
        kernel (list of list): 2D square kernel (e.g., 3x3).

    Returns:
        list: 2D map of convolution results.
    """
    k_size = len(kernel)
    offset = k_size // 2

    # Convert to grayscale first
    # Luminosity method: 0.299R + 0.587G + 0.114B
    gray = []
    for i in range(0, len(pixels), 4):
        lum = 0.299 * pixels[i] + 0.587 * pixels[i+1] + 0.114 * pixels[i+2]
        gray.append(lum)

    output = []
    output_w = width - k_size + 1
    output_h = height - k_size + 1

    if output_w <= 0 or output_h <= 0:
        return []

    for y in range(output_h):
        row = []
        for x in range(output_w):
            val = 0.0
            for ky in range(k_size):
                for kx in range(k_size):
                    # Pixel coordinate
                    px = x + kx
                    py = y + ky
                    p_val = gray[py * width + px]
                    val += p_val * kernel[ky][kx]
            row.append(max(0.0, val)) # ReLU activation
        output.append(row)

    return output

def max_pooling_2d(feature_map, pool_size=2):
    """
    Applies Max Pooling to the feature map.

    Args:
        feature_map (list of list): 2D feature map.
        pool_size (int): Size of the pooling window.

    Returns:
        list of list: Downsampled map.
    """
    if not feature_map:
        return []

    h = len(feature_map)
    w = len(feature_map[0])

    out_h = h // pool_size
    out_w = w // pool_size

    output = []

    for y in range(out_h):
        row = []
        for x in range(out_w):
            max_val = -float('inf')
            for py in range(pool_size):
                for px in range(pool_size):
                    val = feature_map[y * pool_size + py][x * pool_size + px]
                    if val > max_val:
                        max_val = val
            row.append(max_val)
        output.append(row)

    return output

def flatten(feature_map):
    """Flattens a 2D feature map into a 1D vector."""
    flat = []
    for row in feature_map:
        flat.extend(row)
    return flat

# Standard Edge Detection Kernel (Sobel-ish / Laplacian)
EDGE_KERNEL = [
    [-1, -1, -1],
    [-1,  8, -1],
    [-1, -1, -1]
]

def extract_features(pixels, width, height):
    """
    Runs the CNN pipeline on the pixels.
    """
    # 1. Convolution
    conv_map = convolve_2d(pixels, width, height, EDGE_KERNEL)

    # 2. Pooling
    pooled_map = max_pooling_2d(conv_map, pool_size=2)

    # 3. Flatten
    vector = flatten(pooled_map)

    return vector
