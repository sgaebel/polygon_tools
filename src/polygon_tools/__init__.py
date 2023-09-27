from typing import Iterable
import numpy as _np
import os


def neighbours_by_idx(polygons: list, test_indices: Iterable) -> dict:
    """

    Parameters
    ----------
    polygons: list
        List of polygons under investigation.
    test_indices: list | Iterable
        Iterable with indices of the polygons for which neighbours
        are searched.
    
    Returns
    -------
    neighbours: dict
        Dictionary with entries (idx: neighbour_indices).

    """
    # write the file used as input for the c++ extension
    data = {}
    for idx, polygon in enumerate(polygons):
        data[f'len_{idx}'] = len(polygon)
        data[f'arr_{idx}'] = _np.array(polygon)
    data['n_polygons'] = len(polygons)
    data['test_indices'] = _np.array(test_indices)
    data['n_test_polygons'] = len(test_indices)
    _np.savez('data/_temp_polygons_todo.npz', **data)
    from . import polygon_tools_ext
    polygon_tools_ext.find_neighbours()
    # load results
    neighbours = {}
    for idx in range(len(test_indices)):
        neighbours[test_indices[idx]] = _np.load(f'data/_temp_neighbours_{idx}.npy')
    # remove temporary input and output files
    os.remove('data/_temp_polygons_todo.npz')
    for idx in range(len(test_indices)):
        os.remove(f'data/_temp_neighbours_{idx}.npy')
    return neighbours
