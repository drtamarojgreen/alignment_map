import math

def calculate_contrast(pixels):
    """
    Calculates the RMS contrast (standard deviation of pixel intensities).
    """
    if not pixels:
        return 0.0

    luminances = []
    for i in range(0, len(pixels), 4):
        # 0.299R + 0.587G + 0.114B
        lum = 0.299 * pixels[i] + 0.587 * pixels[i+1] + 0.114 * pixels[i+2]
        luminances.append(lum)

    n = len(luminances)
    if n == 0:
        return 0.0

    mean_lum = sum(luminances) / n
    variance = sum((l - mean_lum) ** 2 for l in luminances) / n
    return math.sqrt(variance)

def calculate_whitespace_ratio(pixels, bg_color=(255, 255, 255), tolerance=10):
    """
    Calculates the ratio of pixels that are close to the background color.
    """
    if not pixels:
        return 0.0

    white_count = 0
    total_pixels = len(pixels) // 4

    for i in range(0, len(pixels), 4):
        r = pixels[i]
        g = pixels[i+1]
        b = pixels[i+2]

        # Check distance to bg_color
        dist = math.sqrt((r - bg_color[0])**2 + (g - bg_color[1])**2 + (b - bg_color[2])**2)
        if dist <= tolerance:
            white_count += 1

    return white_count / total_pixels if total_pixels > 0 else 0.0

def analyze_color_palette(pixels):
    """
    Simple histogram analysis to find dominant color count (proxy for palette complexity).
    Returns the number of unique colors (quantized).
    """
    colors = set()
    for i in range(0, len(pixels), 4):
        # Quantize to reduce noise (step of 16)
        r = (pixels[i] // 16) * 16
        g = (pixels[i+1] // 16) * 16
        b = (pixels[i+2] // 16) * 16
        colors.add((r, g, b))

    return len(colors)

def calculate_scores(pixels, width, height):
    """
    Computes all scalar metrics for the image.
    """
    return {
        "contrast": calculate_contrast(pixels),
        "whitespace": calculate_whitespace_ratio(pixels),
        "palette_complexity": analyze_color_palette(pixels)
    }
