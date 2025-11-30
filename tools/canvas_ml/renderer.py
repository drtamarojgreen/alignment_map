import asyncio
import time
import os
try:
    import resource
except ImportError:
    resource = None
from playwright.async_api import async_playwright

async def render_page(url="http://example.com"):
    """
    Renders a page using Playwright and captures a screenshot.
    Returns:
        screenshot_bytes (bytes): The raw PNG data.
        duration (float): Time taken in seconds.
        peak_memory (int): Peak memory usage of the Python process (KB).
    """
    print(f"Starting render for {url}...")
    start_time = time.time()

    screenshot_data = None

    async with async_playwright() as p:
        # Launch browser.
        # Note: In some environments, specific args might be needed (e.g. --no-sandbox)
        # We will try standard launch first.
        browser = await p.chromium.launch(headless=True)
        page = await browser.new_page()
        await page.goto(url)

        # Capture screenshot
        screenshot_data = await page.screenshot()
        await browser.close()

    end_time = time.time()

    peak_memory = 0
    if resource:
        # getrusage returns memory in kilobytes on Linux
        usage = resource.getrusage(resource.RUSAGE_SELF)
        peak_memory = usage.ru_maxrss

    duration = end_time - start_time

    print(f"Render complete.")
    print(f"Time: {duration:.4f}s")
    print(f"Peak Memory (Python process): {peak_memory} KB")

    return screenshot_data, duration, peak_memory

if __name__ == "__main__":
    asyncio.run(render_page())
