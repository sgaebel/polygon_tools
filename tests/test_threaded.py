import os
import numpy as np
import time
import multiprocessing


def regular_polygon(n_vertices, x_offset, y_offset):
    assert n_vertices > 2
    angles = np.linspace(0., 2*np.pi, n_vertices+1)
    xs, ys = np.sin(angles) + x_offset, np.cos(angles) + y_offset
    coord_pairs = list(zip(xs, ys))
    coord_pairs[-1] = coord_pairs[0]
    return np.array(coord_pairs)


def test_neighours():
    N_polygons = 25
    N_CPU = multiprocessing.cpu_count()
    data = {'n_polygons': N_polygons,
            'arr_0': np.array([(0., -1.), (1., 0.), (0., 1.), (-1., 0.), (0., -1.)]),
            'len_0' : 5,
            'arr_1': np.array([(1., 0.), (3., 2.), (2., 3.), (0., 1.), (1., 0.)]),
            'len_1' : 5,
            'arr_2': np.array([(3., 2.), (2., 3.), (3., 4.), (4., 3.), (3., 2.)]),
            'len_2' : 5,
            'arr_3': np.array([(1., 0.), (3., 2.), (3., 0.), (1., 0.)]),
            'len_3' : 4,
            'arr_4': np.array([(3., 2.), (5., 1.5), (3., 0.), (3., 2.)]),
            'len_4' : 4,
            'arr_5': np.array([(1.5, 4.), (0., 4.), (0., 2.), (1.5, 2.), (1.5, 4.)]),
            'len_5' : 5,
            'test_indices': (1, 4, 5, 9, 16, 8, 21, 14)}
    data['n_test_polygons'] = len(data['test_indices'])
    # add some random polygons which are not expected to share any edges.
    for i in range(6, N_polygons):
        N = np.random.randint(3, 15)
        x0, y0 = np.random.standard_normal(size=2)
        data[f'len_{i}'] = N + 1  # closed
        data[f'arr_{i}'] = regular_polygon(N, x0, y0)
    for i in range(N_polygons):
        try:
            assert data[f'len_{i}'] == data[f'arr_{i}'].shape[0]
        except KeyError:
            print(*sorted(list(data.keys())), sep='\n')
            raise
        assert data[f'arr_{i}'].shape[1] == 2
    assert data['n_test_polygons'] == N_CPU, repr(N_CPU)
    np.savez('data/polygons_todo.npz', **data)
    print('Save done.', flush=True)
    time.sleep(1.)
    from polygon_tools import find_neighbours
    find_neighbours()
    time.sleep(0.5)
    # expected:
    #   1 -> (0, 2, 3)
    #   4 -> (3,)
    #   5 -> (,)
    expected = [np.array([-1])] * N_CPU
    expected[0] = np.array([0, 2, 3])
    expected[1] = np.array([3,])
    for i in range(multiprocessing.cpu_count()):
        neighbours = np.load(f'data/neighbours_{i}.npy')
        assert np.all(expected[i] == neighbours)

    os.remove('data/polygons_todo.npz')
    for i in range(multiprocessing.cpu_count()):
        os.remove(f'data/neighbours_{i}.npy')
    return
