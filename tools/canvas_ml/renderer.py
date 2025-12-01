import time
import os
import sys

# Try importing playwright, handle if missing for environment safety
try:
    from playwright.sync_api import sync_playwright
except ImportError:
    sync_playwright = None

def get_canvas_pixels(url=None, width=64, height=64):
    """
    Renders the page and attempts to extract pixel data from a canvas element.
    If no canvas is found or extraction fails, returns a generated pattern.

    Args:
        url (str): The URL to visit. Can be None for test mode.
        width (int): Target width for the output pixel grid.
        height (int): Target height for the output pixel grid.

    Returns:
        dict: {
            "pixels": list of int (flattened RGBA),
            "width": int,
            "height": int,
            "duration": float,
            "memory_usage_mb": float
        }
    """
    start_time = time.time()
    pixels = []

    if url and sync_playwright:
        try:
            with sync_playwright() as p:
                browser = p.chromium.launch()
                page = browser.new_page()
                page.goto(url)

                # Attempt to find a canvas and extract data.
                # We resize the canvas content to target width/height for the ML model
                # using a temporary in-memory canvas in JS.
                result = page.evaluate(f"""() => {{
                    const sourceCanvas = document.querySelector('canvas');
                    if (!sourceCanvas) return null;

                    const tempCanvas = document.createElement('canvas');
                    tempCanvas.width = {width};
                    tempCanvas.height = {height};
                    const ctx = tempCanvas.getContext('2d');

                    // Draw source canvas to temp canvas (scaling it)
                    // We assume sourceCanvas has content.
                    ctx.drawImage(sourceCanvas, 0, 0, {width}, {height});

                    const imgData = ctx.getImageData(0, 0, {width}, {height});
                    // Convert Uint8ClampedArray to regular array
                    return Array.from(imgData.data);
                }}""")

                if result:
                    pixels = result

                browser.close()
        except Exception as e:
            print(f"Renderer Error: {e}", file=sys.stderr)
    elif url and not sync_playwright:
        print("Playwright not installed, skipping browser render.", file=sys.stderr)

    # Fallback/Test Data if extraction failed or no URL
    if not pixels:
        if url:
             print("Warning: No canvas data extracted from URL. Generating test pattern.", file=sys.stderr)

        # Generate a simple pattern (gradient)
        pixels = [0] * (width * height * 4)
        for y in range(height):
            for x in range(width):
                idx = (y * width + x) * 4
                # Gradient: Red increases with X, Green increases with Y
                pixels[idx] = int(x / width * 255)     # R
                pixels[idx+1] = int(y / height * 255)  # G
                pixels[idx+2] = 128                    # B
                pixels[idx+3] = 255                    # A

    end_time = time.time()
    duration = end_time - start_time

    # Estimate memory of the pixel list (rough: 28 bytes per int object in Python)
    mem_estimate = (len(pixels) * 28) / (1024 * 1024)

    return {
        "pixels": pixels,
        "width": width,
        "height": height,
        "duration": duration,
        "memory_usage_mb": mem_estimate
    }

if __name__ == "__main__":
    # Test run
    stats = get_canvas_pixels(None, 64, 64)
    print(f"Rendered {stats['width']}x{stats['height']} in {stats['duration']:.4f}s. Memory est: {stats['memory_usage_mb']:.2f}MB")
