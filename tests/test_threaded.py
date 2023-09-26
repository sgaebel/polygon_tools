import os
import numpy as np
import time


def test_neighours():  
    data = {'n_poly': 6,
            'arr_0': [(0., -1.), (1., 0.), (0., 1.), (-1., 0.), (0., -1.)],
            'len_0' : 5,
            'arr_1': [(1., 0.), (3., 2.), (2., 3.), (0., 1.), (1., 0.)],
            'len_1' : 5,
            'arr_2': [(3., 2.), (2., 3.), (3., 4.), (4., 3.), (3., 2.)],
            'len_2' : 5,
            'arr_3': [(1., 0.), (3., 2.), (3., 0.), (1., 0.)],
            'len_3' : 4,
            'arr_4': [(3., 2.), (5., 1.5), (3., 0.), (3., 2.)],
            'len_4' : 4,
            'arr_5': [(1.5, 4.), (0., 4.), (0., 2.), (1.5, 2.), (1.5, 4.)],
            'len_5' : 5,
            'n_test': 3,
            'test_indices': (1, 4, 5)}
    np.savez('data/polygons_todo.npz', **data)
    print('Save done.', flush=True)
    time.sleep(1.)
    from polygon_tools import find_neighbours

    results = find_neighbours()
    # results = np.load('data/neighbours.npy')
    # print(results)
    # expected:
    #   1 -> (0, 2, 3)
    #   4 -> (3,)
    #   5 -> (,)
    os.remove('data/polygons_todo.npz')
    # os.remove('data/neighbours.npy')

test_neighours()
