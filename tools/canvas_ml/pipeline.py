import asyncio
import random
from tools.canvas_ml import renderer, cnn_layer, scorers, model

async def main():
    print("=== Vision ML Integration Pipeline ===")

    # 1. Rendering & Benchmarking
    # We use a dummy URL or specific one.
    target_url = "http://example.com"
    print(f"\n[Stage 1] Rendering {target_url}...")
    try:
        screenshot, duration, memory_kb = await renderer.render_page(target_url)
    except Exception as e:
        print(f"Rendering failed: {e}")
        # Create dummy data if rendering fails (e.g. no internet/browser)
        screenshot = bytes([random.randint(0, 255) for _ in range(4096)])
        duration = 0.5
        memory_kb = 15000

    if screenshot:
        print(f"Screenshot captured. Size: {len(screenshot)} bytes")

    # 2. CNN Feature Extraction
    print("\n[Stage 2] Extracting Visual Features (CNN)...")
    cnn_features = cnn_layer.extract_features(screenshot)
    print(f"Extracted Feature Vector (First 5): {cnn_features[:5]}")

    # 3. Scoring Metrics
    print("\n[Stage 3] Calculating UX Metrics...")
    metrics = scorers.get_visual_scores(screenshot)
    print(f"Metrics: {metrics}")

    # 4. Unsupervised Learning (Clustering)
    print("\n[Stage 4] Running Unsupervised Model...")

    # Construct full feature vector
    # [CNN..., Contrast, Whitespace, Duration, Memory]
    full_vector = cnn_features + [
        metrics['contrast'],
        metrics['whitespace'],
        duration,
        memory_kb / 100000.0 # Normalize memory a bit
    ]

    # Create some dummy historical data to train against
    dummy_data = []
    for _ in range(10):
        # Generate random vectors of same length
        dummy_vec = [random.random() for _ in range(len(full_vector))]
        dummy_data.append(dummy_vec)

    # Add current vector to data
    dummy_data.append(full_vector)

    kmeans = model.KMeans(k=3)
    kmeans.fit(dummy_data)

    cluster_id = kmeans.predict(full_vector)
    print(f"Assigned Cluster ID: {cluster_id}")

    # 5. Value Prediction
    print("\n[Stage 5] Predicting Implementation Value...")
    value_prediction = model.analyze_value(full_vector, cluster_id, metrics)
    print(f"PREDICTION: {value_prediction}")

    print("\n=== Pipeline Complete ===")

if __name__ == "__main__":
    asyncio.run(main())
