#!/usr/bin/env python3
"""
3D Latency Landscape Visualization

Renders a 3D landscape showing latency distribution and percentiles.
Perfect for visualizing benchmark results from the C++ benchmark_runner.

Usage:
    python latency_landscape.py [options]

Options:
    --input FILE    Input JSON file from benchmark_runner (uses demo data if not provided)
    --export FILE   Export to PNG/HTML file
    --rotate        Enable auto-rotation
    --no-display    Don't show interactive window
"""

import argparse
import numpy as np

try:
    import pyvista as pv
except ImportError:
    print("Error: PyVista not installed. Run: pip install pyvista")
    exit(1)

from data_loader import load_latency_json, generate_demo_data


def create_latency_histogram_3d(data: dict, plotter: pv.Plotter) -> None:
    """Create 3D bar chart of latency histogram."""
    
    histogram = data.get('histogram', {})
    buckets = list(histogram.keys())
    values = list(histogram.values())
    
    if not values:
        return
    
    # Normalize values
    max_val = max(values)
    norm_values = [v / max_val * 50 for v in values]
    
    # Create 3D bars
    colors = ['#00ff88', '#44ff44', '#88ff00', '#ffff00', '#ff8800', '#ff4400', '#ff0000']
    
    for i, (bucket, height, color) in enumerate(zip(buckets, norm_values, colors)):
        # Create a box for each bar
        x_pos = i * 15
        box = pv.Box(bounds=(x_pos, x_pos + 12, 0, 10, 0, height))
        plotter.add_mesh(box, color=color, opacity=0.9)
        
        # Add label
        plotter.add_point_labels(
            [(x_pos + 6, 5, height + 2)],
            [bucket],
            font_size=10,
            text_color='white',
            shape_color=(0.1, 0.1, 0.2),
            shape_opacity=0.7
        )


def create_latency_surface(data: dict, plotter: pv.Plotter) -> None:
    """Create 3D surface from latency samples."""
    
    samples = data.get('samples', [])
    if len(samples) < 100:
        samples = np.random.exponential(1000, 1000).tolist()
    
    # Reshape samples into 2D grid for surface
    n = int(np.sqrt(len(samples)))
    grid_data = np.array(samples[:n*n]).reshape(n, n)
    
    # Normalize for visualization
    grid_data = np.log1p(grid_data)  # Log scale for better visualization
    grid_data = (grid_data / grid_data.max()) * 30
    
    # Create coordinate grid
    x = np.linspace(0, 100, n)
    y = np.linspace(0, 100, n)
    X, Y = np.meshgrid(x, y)
    
    # Create surface
    points = np.column_stack([X.ravel(), Y.ravel(), grid_data.ravel()])
    grid = pv.StructuredGrid()
    grid.points = points
    grid.dimensions = [n, n, 1]
    grid.point_data['latency'] = grid_data.ravel()
    
    plotter.add_mesh(
        grid,
        scalars='latency',
        cmap='viridis',
        smooth_shading=True,
        opacity=0.85
    )


def add_percentile_markers(data: dict, plotter: pv.Plotter) -> None:
    """Add markers for key percentiles."""
    
    percentiles = [
        ('P50', data.get('p50_ns', 0), '#00ff88'),
        ('P95', data.get('p95_ns', 0), '#ffff00'),
        ('P99', data.get('p99_ns', 0), '#ff8800'),
        ('P99.9', data.get('p999_ns', 0), '#ff0000'),
    ]
    
    # Create info panel
    info_text = f"""
    LATENCY STATISTICS
    ══════════════════
    Samples: {data.get('count', 0):,}
    Min:     {data.get('min_ns', 0):,.0f} ns
    Max:     {data.get('max_ns', 0):,.0f} ns
    Mean:    {data.get('mean_ns', 0):,.0f} ns
    
    PERCENTILES
    ══════════════════
    P50:     {data.get('p50_ns', 0):,.0f} ns
    P95:     {data.get('p95_ns', 0):,.0f} ns
    P99:     {data.get('p99_ns', 0):,.0f} ns
    P99.9:   {data.get('p999_ns', 0):,.0f} ns
    """
    
    plotter.add_text(
        info_text,
        position='upper_right',
        font_size=9,
        color='white'
    )


def main():
    parser = argparse.ArgumentParser(description='3D Latency Landscape Visualization')
    parser.add_argument('--input', type=str, help='Input JSON file from benchmark_runner')
    parser.add_argument('--export', type=str, help='Export to file (PNG or HTML)')
    parser.add_argument('--rotate', action='store_true', help='Enable auto-rotation')
    parser.add_argument('--no-display', action='store_true', help='No interactive display')
    parser.add_argument('--mode', choices=['surface', 'histogram', 'both'], 
                        default='both', help='Visualization mode')
    args = parser.parse_args()
    
    # Load or generate data
    if args.input:
        print(f"Loading latency data from: {args.input}")
        data = load_latency_json(args.input)
    else:
        print("Using demo latency data...")
        data = generate_demo_data()
    
    print(f"  Samples: {data.get('count', 0):,}")
    print(f"  Mean: {data.get('mean_ns', 0):,.0f} ns")
    print(f"  P99: {data.get('p99_ns', 0):,.0f} ns")
    
    # Create plotter
    pv.set_plot_theme('dark')
    plotter = pv.Plotter(off_screen=args.no_display)
    plotter.set_background('#0d1117')
    
    # Add title
    plotter.add_text(
        "HFT Latency Landscape",
        position='upper_left',
        font_size=16,
        color='white'
    )
    
    # Create visualization based on mode
    if args.mode in ('surface', 'both'):
        create_latency_surface(data, plotter)
    
    if args.mode in ('histogram', 'both'):
        # Offset histogram for 'both' mode
        if args.mode == 'both':
            plotter.camera_position = [(200, -50, 100), (50, 50, 15), (0, 0, 1)]
    
    # Add percentile info
    add_percentile_markers(data, plotter)
    
    # Configure view
    plotter.add_axes(xlabel='Sample X', ylabel='Sample Y', zlabel='Latency (log)')
    
    # Export
    if args.export:
        if args.export.endswith('.html'):
            plotter.export_html(args.export)
            print(f"Exported interactive HTML to: {args.export}")
        else:
            plotter.screenshot(args.export)
            print(f"Screenshot saved to: {args.export}")
    
    # Show or close
    if not args.no_display:
        if args.rotate:
            print("Rotating view (close window to stop)...")
            path = plotter.generate_orbital_path(n_points=72, shift=50)
            plotter.open_gif("latency_orbit.gif") if args.export and 'gif' in args.export else None
            plotter.orbit_on_path(path, write_frames=bool(args.export))
        else:
            plotter.show()
    else:
        plotter.close()


if __name__ == '__main__':
    main()
