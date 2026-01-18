#!/usr/bin/env python3
"""
3D Order Book Surface Visualization

Renders a 3D surface showing order book depth across price levels and time.
Bids are shown in green, asks in red.

Usage:
    python orderbook_surface.py [options]

Options:
    --input FILE    CSV file with order book data (optional, uses demo data if not provided)
    --export FILE   Export to PNG/HTML file
    --rotate        Enable auto-rotation animation
    --no-display    Don't show interactive window (for headless export)
"""

import argparse
import numpy as np

try:
    import pyvista as pv
except ImportError:
    print("Error: PyVista not installed. Run: pip install pyvista")
    exit(1)

from data_loader import generate_orderbook_surface_data


def create_orderbook_surface(
    prices: np.ndarray,
    times: np.ndarray,
    bid_depth: np.ndarray,
    ask_depth: np.ndarray,
    plotter: pv.Plotter
) -> None:
    """Create 3D order book surface visualization."""
    
    # Create meshgrid for surface
    T, P = np.meshgrid(times, prices)
    
    # Normalize for visualization
    price_norm = (P - P.min()) / (P.max() - P.min()) * 100
    time_norm = T / T.max() * 100
    
    # Create bid surface (green)
    bid_points = np.column_stack([
        time_norm.ravel(),
        price_norm.ravel(),
        (bid_depth * 20).ravel()  # Scale for visibility
    ])
    bid_grid = pv.StructuredGrid()
    bid_grid.points = bid_points
    bid_grid.dimensions = [len(times), len(prices), 1]
    
    # Create ask surface (red)
    ask_points = np.column_stack([
        time_norm.ravel(),
        price_norm.ravel(),
        (ask_depth * 20).ravel()
    ])
    ask_grid = pv.StructuredGrid()
    ask_grid.points = ask_points
    ask_grid.dimensions = [len(times), len(prices), 1]
    
    # Add surfaces to plotter
    plotter.add_mesh(
        bid_grid,
        color='#00FFFF',  # Aqua/Teal for Bids
        opacity=0.8,
        smooth_shading=True,
        label='Bids'
    )
    plotter.add_mesh(
        ask_grid,
        color='#FF69B4',  # HotPink for Asks
    )
    
    # Add mid-price plane
    mid_idx = len(prices) // 2
    mid_price_norm = (prices[mid_idx] - prices.min()) / (prices.max() - prices.min()) * 100
    plane = pv.Plane(
        center=(50, mid_price_norm, 5),
        direction=(0, 1, 0),
        i_size=100,
        j_size=20
    )
    plotter.add_mesh(plane, color='white', opacity=0.2, label='Mid Price')


def main():
    parser = argparse.ArgumentParser(description='3D Order Book Surface Visualization')
    parser.add_argument('--input', type=str, help='Input CSV file (uses demo data if not provided)')
    parser.add_argument('--export', type=str, help='Export to file (PNG or HTML)')
    parser.add_argument('--rotate', action='store_true', help='Enable auto-rotation')
    parser.add_argument('--no-display', action='store_true', help='No interactive display')
    args = parser.parse_args()
    
    # Load or generate data
    print("Loading order book data...")
    prices, times, bid_depth, ask_depth = generate_orderbook_surface_data()
    print(f"  Price levels: {len(prices)}")
    print(f"  Time points: {len(times)}")
    
    # Create plotter
    pv.set_plot_theme('dark')
    plotter = pv.Plotter(off_screen=args.no_display)
    plotter.set_background('#050510') # Deep dark blue/black
    
    # Add title
    plotter.add_text(
        "Order Book Depth Surface",
        position='upper_left',
        font_size=14,
        color='white'
    )
    
    # Create visualization
    create_orderbook_surface(prices, times, bid_depth, ask_depth, plotter)
    
    # Configure camera
    plotter.camera_position = 'iso'
    plotter.add_axes(
        xlabel='Time',
        ylabel='Price',
        zlabel='Depth',
        line_width=2
    )
    
    # Add legend
    plotter.add_legend(bcolor=(0.1, 0.1, 0.2, 0.8))
    
    # Export if requested
    if args.export:
        if args.export.endswith('.html'):
            plotter.export_html(args.export)
            print(f"Exported to: {args.export}")
        else:
            plotter.screenshot(args.export)
            print(f"Screenshot saved to: {args.export}")
    
    # Show or close
    if not args.no_display:
        if args.rotate:
            plotter.open_gif("orderbook_rotation.gif") if args.export and args.export.endswith('.gif') else None
            for angle in range(0, 360, 2):
                plotter.camera.azimuth = angle
                plotter.render()
        plotter.show()
    else:
        plotter.close()


if __name__ == '__main__':
    main()
