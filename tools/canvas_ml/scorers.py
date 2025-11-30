"""
Scoring metrics for visual analysis.
"""

def calculate_contrast_score(image_bytes):
    """
    Estimates a contrast score based on pixel value variance.
    """
    data = list(image_bytes)
    if not data:
        return 0.0

    # Calculate mean
    mean = sum(data) / len(data)

    # Calculate variance
    variance = sum((x - mean) ** 2 for x in data) / len(data)

    # Normalize roughly to 0-1 range (assuming 8-bit variance max is ~16000 for uniform dist, but real images vary)
    # We'll just return scaled variance
    return min(variance / 5000.0, 1.0)

def calculate_whitespace_ratio(image_bytes):
    """
    Estimates whitespace ratio.
    Assumes 'white' or 'background' is high value (near 255) in our byte stream.
    """
    data = list(image_bytes)
    if not data:
        return 0.0

    # Arbitrary threshold for "light" pixel
    threshold = 240
    white_pixels = sum(1 for x in data if x > threshold)

    return white_pixels / len(data)

def get_visual_scores(image_bytes):
    """
    Returns a dictionary of scores.
    """
    return {
        "contrast": calculate_contrast_score(image_bytes),
        "whitespace": calculate_whitespace_ratio(image_bytes),
        # Dummy values for difficult-to-calculate metrics without OpenCV
        "legibility": 0.8,
        "color_harmony": 0.7
    }
