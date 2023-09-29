from typing import Iterable as _Iterable
import numpy as _np
from polygon_contains_point import point_in_polygon
from polygons_share_edge import share_edge
from polygon_neighbours import find_neighbours as _find_neighbours


# convenience wrapper for find_neighbours
def bordering_polygons(polygons: list, test_indices: _Iterable) -> dict:
    """
    Wraps `polygon_neighbours.find_neighbours` to provide a
    usable, pythonic interface.

    Finds the indices of polygons bordering each other.

    Parameters
    ----------
    polygons: list
        List of polygons as list of coordinate pairs.
    test_indices: Iterable
        Iterable containing the polygons for which bordering
        polygons are searched.

    Returns
    -------
    neighbours: dict
        Dictionary mapping test indices to iterables of bordering
        neighbours.

    """
    input_path = '/tmp/_temp_polygons_todo.npz'
    output_path_template = '/tmp/_temp_polygon_neighbours_{idx}.npy'
    # package input data into a dict for passing to numpy.savez
    data = {'n_polygons': len(polygons)}
    for idx, polygon in enumerate(polygons):
        data[f'arr_{idx}'] = _np.array(polygon)
        data[f'len_{idx}'] = len(polygon)
    data['test_indices'] = _np.array(test_indices)
    data['n_test_polygons'] = len(test_indices)
    _np.savez(input_path, **data)
    _find_neighbours()
    neighbours = {}
    for idx, test_idx in enumerate(test_indices):
        result = _np.load(output_path_template.format(idx=idx))
        if _np.all(result == _np.array([-1])):
            neighbours[test_idx] = _np.array([])
        else:
            neighbours[test_idx] = result
    return neighbours
