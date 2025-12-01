import sys
import os

# Ensure we can import from local tools
sys.path.append(os.path.join(os.path.dirname(__file__), '../../'))

from tools.canvas_ml import renderer, cnn_layer, scorers, model

def run_pipeline(url=None):
    print(f"--- Starting Vision ML Pipeline for {url or 'Test Pattern'} ---")

    # 1. Render & Capture
    print("Step 1: Rendering...")
    render_stats = renderer.get_canvas_pixels(url, width=64, height=64)
    pixels = render_stats['pixels']
    w = render_stats['width']
    h = render_stats['height']
    print(f"  Captured {w}x{h} image. Time: {render_stats['duration']:.4f}s")

    if not pixels:
        print("Error: No pixel data obtained.")
        return

    # 2. CNN Feature Extraction
    print("Step 2: Extracting Visual Features (CNN)...")
    cnn_features = cnn_layer.extract_features(pixels, w, h)
    print(f"  Extracted feature vector of length {len(cnn_features)}.")

    # 3. Scoring Metrics
    print("Step 3: Calculating Visual Scores...")
    scores = scorers.calculate_scores(pixels, w, h)
    print(f"  Contrast: {scores['contrast']:.2f}")
    print(f"  Whitespace: {scores['whitespace']:.2%}")
    print(f"  Palette: {scores['palette_complexity']} colors")

    # 4. Construct Full Feature Vector
    # Combine normalized scores and a subset of CNN features (to keep dimensionality low for this demo)
    # We take the average of CNN features to represent "Texture Complexity"
    avg_texture = sum(cnn_features) / len(cnn_features) if cnn_features else 0

    feature_vector = [
        scores['contrast'],
        scores['whitespace'],
        scores['palette_complexity'],
        avg_texture,
        render_stats['duration']
    ]
    print(f"  Final Input Vector: {feature_vector}")

    # 5. Model Prediction (Unsupervised)
    print("Step 4: Running Unsupervised Model...")
    kmeans = model.KMeans(k=3)

    # Train on some dummy historical data mixed with our current point to simulate a "dataset"
    # In a real app, this would load a saved dataset.
    dummy_data = [
        [50, 0.5, 10, 20, 0.5], # Good
        [10, 0.1, 5, 5, 0.2],   # Dark/Simple
        [80, 0.9, 100, 50, 1.0] # Complex/Bright
    ]
    dummy_data.append(feature_vector)

    kmeans.fit(dummy_data)
    cluster_idx = kmeans.predict(feature_vector)
    prediction = model.predict_value(feature_vector, cluster_idx)

    print(f"--- RESULT ---")
    print(f"Cluster: {cluster_idx}")
    print(f"Prediction: {prediction}")

    return prediction

if __name__ == "__main__":
    target_url = sys.argv[1] if len(sys.argv) > 1 else None
    run_pipeline(target_url)
