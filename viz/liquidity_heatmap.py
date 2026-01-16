#!/usr/bin/env python3
"""
3D Liquidity Heatmap Visualization

Renders an animated 3D heatmap showing liquidity distribution over time.
Uses GPU acceleration when available.

Usage:
    python liquidity_heatmap.py [options]

Options:
    --input FILE    Input data file (optional, uses demo data if not provided)
    --export FILE   Export to MP4/PNG file
    --frames N      Number of animation frames (default: 30)
    --no-display    Don't show interactive window
"""

import argparse
import numpy as np

try:
    import pyvista as pv
except ImportError:
    print("Error: PyVista not installed. Run: pip install pyvista")
    exit(1)

from data_loader import generate_liquidity_heatmap_data


def create_liquidity_heatmap(
    data: np.ndarray,
    frame: int,
    plotter: pv.Plotter
) -> None:
    """Create 3D liquidity heatmap for a single frame."""
    
    frame_data = data[frame]
    n_y, n_x = frame_data.shape
    
    # Create coordinate grids
    x = np.linspace(0, 100, n_x)
    y = np.linspace(0, 100, n_y)
    X, Y = np.meshgrid(x, y)
    
    # Scale Z by liquidity value
    Z = frame_data * 30  # Scale for visibility
    
    # Create surface
    points = np.column_stack([X.ravel(), Y.ravel(), Z.ravel()])
    grid = pv.StructuredGrid()
    grid.points = points
    grid.dimensions = [n_x, n_y, 1]
    
    # Add scalar values for coloring
    grid.point_data['liquidity'] = frame_data.ravel()
    
    return grid


def main():
    parser = argparse.ArgumentParser(description='3D Liquidity Heatmap Visualization')
    parser.add_argument('--input', type=str, help='Input data file')
    parser.add_argument('--export', type=str, help='Export to file (MP4 or PNG)')
    parser.add_argument('--frames', type=int, default=30, help='Number of frames')
    parser.add_argument('--no-display', action='store_true', help='No interactive display')
    args = parser.parse_args()
    
    # Generate demo data
    print("Generating liquidity heatmap data...")
    data = generate_liquidity_heatmap_data(n_frames=args.frames)
    print(f"  Grid size: {data.shape[1]}x{data.shape[2]}")
    print(f"  Frames: {data.shape[0]}")
    
    # Create plotter
    pv.set_plot_theme('dark')
    plotter = pv.Plotter(off_screen=args.no_display)
    plotter.set_background('#0d1117')
    
    # Add title
    plotter.add_text(
        "Liquidity Heatmap",
        position='upper_left',
        font_size=14,
        color='white'
    )
    
    # Create initial mesh
    grid = create_liquidity_heatmap(data, 0, plotter)
    actor = plotter.add_mesh(
        grid,
        scalars='liquidity',
        cmap='plasma',
        smooth_shading=True,
        show_scalar_bar=True
    )
    
    # Configure camera
    plotter.camera_position = [(150, 150, 100), (50, 50, 15), (0, 0, 1)]
    plotter.add_axes(xlabel='X', ylabel='Y', zlabel='Liquidity')
    
    # Animation or static
    if args.export and args.export.endswith('.mp4'):
        print(f"Rendering animation to {args.export}...")
        plotter.open_movie(args.export, framerate=15)
        
        for i in range(data.shape[0]):
            # Update mesh
            new_grid = create_liquidity_heatmap(data, i, plotter)
            actor.mapper.SetInputData(new_grid)
            plotter.camera.azimuth += 360 / data.shape[0]
            plotter.write_frame()
        
        plotter.close()
        print(f"Animation saved to: {args.export}")
    elif args.export:
        plotter.screenshot(args.export)
        print(f"Screenshot saved to: {args.export}")
        if not args.no_display:
            plotter.show()
    elif not args.no_display:
        plotter.show()
    else:
        plotter.close()


if __name__ == '__main__':
    main()
