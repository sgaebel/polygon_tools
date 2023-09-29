import os
import numpy as np
import time
import multiprocessing

import polygon_tools


N_CPU = multiprocessing.cpu_count()


def regular_polygon(n_vertices, x_offset, y_offset, leave_open=False):
    assert n_vertices > 2
    angles = np.linspace(0., 2*np.pi, n_vertices+1)
    xs, ys = np.sin(angles) + x_offset, np.cos(angles) + y_offset
    coord_pairs = list(zip(xs, ys))
    if not leave_open:
        # usually not closed due to floating point errors in PI and sin/cos
        coord_pairs[-1] = coord_pairs[0]  # close the polygon
    return np.array(coord_pairs)


def polygon_setup(N_polygons, N_test):
    if N_test > N_polygons:
        raise ValueError('Cannot test more polygons than there are.')
    polygons = [[(0., -1.), (1., 0.), (0., 1.), (-1., 0.), (0., -1.)],
                [(1., 0.), (3., 2.), (2., 3.), (0., 1.), (1., 0.)],
                [(3., 2.), (2., 3.), (3., 4.), (4., 3.), (3., 2.)],
                [(1., 0.), (3., 2.), (3., 0.), (1., 0.)],
                [(3., 2.), (5., 1.5), (3., 0.), (3., 2.)],
                [(1.5, 4.), (0., 4.), (0., 2.), (1.5, 2.), (1.5, 4.)]]
    test_indices = [1, 4, 5]
    expected_matches = {1: np.array([0, 2, 3]),
                        4: np.array([3]),
                        5: np.array([])}
    # add extra if N_test > 3, or if N_polygons > 6
    while N_polygons > len(polygons):
        # "true" number of vertices
        # generator returns N+1 vertices for closedness.
        # random number of vertices, at least 2 since 2 would form a line.
        N = np.random.randint(3, 15)
        x0, y0 = np.random.standard_normal(size=2)
        polygons.append(regular_polygon(N, x0, y0))
    while N_test > len(test_indices):
        # use random indices, no matches are expected with the random
        #   extra polygons
        idx = np.random.randint(6, N_polygons)
        if idx in test_indices:
            continue
        test_indices.append(idx)
        expected_matches[idx] = np.array([])
    assert set(test_indices) == set(expected_matches.keys())
    return polygons, test_indices, expected_matches


# we only really need to test the bordering_polygon wwrapper,
#   the other submodule have their own tests.
def test_neighbours_0():
    N_polygons = 6
    N_test = 3
    polygons, test_indices, expectation = polygon_setup(N_polygons=N_polygons,
                                                        N_test=N_test)
    results = polygon_tools.bordering_polygons(polygons=polygons,
                                               test_indices=test_indices)
    for idx in test_indices:
        assert np.all(results[idx] == expectation[idx])
    return

def test_neighbours_1():
    N_polygons = 25
    N_test = N_CPU
    polygons, test_indices, expectation = polygon_setup(N_polygons=N_polygons,
                                                        N_test=N_test)
    results = polygon_tools.bordering_polygons(polygons=polygons,
                                               test_indices=test_indices)
    for idx in test_indices:
        assert np.all(results[idx] == expectation[idx])
    return
