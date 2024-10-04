import matplotlib.pyplot as plt
import numpy as np

# Triangle vertices (x, y)
triangle_coords = np.array([
    [1036, 698],
    [1035, 699],
    [1041, 699]
])

# [1] poly_render: (1036, 698), (1035, 699), (1041, 699), crease_len = 393216

def visualize_triangle(coords, pixel_size=20, fill_color='skyblue', edge_color='navy'):
    """
    Visualize a triangle with enlarged pixels and visible edges.

    Parameters:
    - coords: numpy array of shape (3, 2) with triangle vertices (x, y).
    - pixel_size: int, scaling factor for pixel enlargement.
    - fill_color: str, color to fill the triangle.
    - edge_color: str, color of the triangle edges.
    """
    # Extract x and y coordinates
    x = coords[:, 0]
    y = coords[:, 1]

    # Determine the plot limits
    x_min, x_max = x.min() - 1, x.max() + 1
    y_min, y_max = y.min() - 1, y.max() + 1

    # Create a figure and axis with adjusted size
    fig, ax = plt.subplots(figsize=((x_max - x_min) * pixel_size / 10,
                                    (y_max - y_min) * pixel_size / 10))

    # Set the plot limits and aspect ratio
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    ax.set_aspect('equal')

    # Create a grid of points
    x_grid = np.arange(x_min, x_max + 1)
    y_grid = np.arange(y_min, y_max + 1)
    X_grid, Y_grid = np.meshgrid(x_grid, y_grid)
    grid_points = np.vstack((X_grid.ravel(), Y_grid.ravel())).T

    # Function to check if points are inside the triangle
    def points_in_triangle(pts, tri):
        # Barycentric technique
        A = 0.5 * (-tri[1,1]*tri[2,0] + tri[0,1]*(-tri[1,0]+tri[2,0]) + tri[0,0]*(tri[1,1]-tri[2,1]) + tri[1,0]*tri[2,1])
        sign = -1 if A < 0 else 1
        s = (tri[0,1]*tri[2,0] - tri[0,0]*tri[2,1] + (tri[2,1] - tri[0,1])*pts[:,0] + (tri[0,0] - tri[2,0])*pts[:,1]) * sign
        t = (tri[0,0]*tri[1,1] - tri[0,1]*tri[1,0] + (tri[0,1] - tri[1,1])*pts[:,0] + (tri[1,0] - tri[0,0])*pts[:,1]) * sign
        return (s >= 0) & (t >= 0) & ((s + t) <= 2*A*sign)

    # Find the points inside the triangle
    inside = points_in_triangle(grid_points, coords)
    inside_points = grid_points[inside]

    # Plot the pixels
    for point in inside_points:
        rect = plt.Rectangle(point - [0.5, 0.5], 1, 1, color=fill_color, ec=None)
        ax.add_patch(rect)

    # Draw triangle edges
    triangle = plt.Polygon(coords, closed=True, fill=None, edgecolor=edge_color, linewidth=2)
    ax.add_patch(triangle)

    # Remove axes for better visualization
    ax.axis('off')

    # Invert y-axis to match usual graphics coordinate system
    plt.gca().invert_yaxis()

    # Show the plot
    plt.show()

visualize_triangle(triangle_coords, pixel_size=40)
