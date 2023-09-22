/* CPython extension for finding neighouring polygons in parallel */

#if 1
  // local builds
  #include <python3.10/Python.h>
#else
  // docker build for wheel
  #include <Python.h>
#endif

#define DEBUG
// #define PARALLEL

#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include "cnpy.h"

#define INPUT_FILE "data/polygons_todo.npz"
#define OUTPUT_FILE "data/neighbours.npy"


class Vertex {
    public:
        double x, y;
        Vertex(const double x, const double y)
        {
            this->x = x;
            this->y = y;
        }
        operator std::string()
        {
            return "[" + std::to_string(this->x) + ", " + std::to_string(this->y) + "]";
        }
};


class Polygon {
    public:
        std::vector<Vertex> vertices;
        Polygon(const std::vector<Vertex> vertices)
        {
            this->vertices = vertices;
        }
        const size_t size() { return this->vertices.size(); }
        const Vertex operator[](const size_t idx) {
            return this->vertices.at(idx);
        }
};


class PolygonEdge
{
    public:
        double x0, y0, x1, y1;

        PolygonEdge(double x0, double y0, double x1, double y1) {
            this->x0 = x0;
            this->y0 = y0;
            this->x1 = x1;
            this->y1 = y1;
        }
        PolygonEdge(const Vertex vertex_0, const Vertex vertex_1) {
            this->x0 = vertex_0.x;
            this->y0 = vertex_0.y;
            this->x1 = vertex_1.x;
            this->y1 = vertex_1.y;
        }
        bool operator==(const PolygonEdge &other);
        std::string print();
};
bool PolygonEdge::operator==(const PolygonEdge &other)
{
    if ((this->x0 == other.x0) &&
        (this->y0 == other.y0) &&
        (this->x1 == other.x1) &&
        (this->y1 == other.y1))
        return true;
    if ((this->x1 == other.x0) &&
        (this->y1 == other.y0) &&
        (this->x0 == other.x1) &&
        (this->y0 == other.y1))
        return true;
    return false;
}
std::string PolygonEdge::print()
{
    std::string representation = "Edge: (";
    representation += std::to_string(this->x0) + ",";
    representation += std::to_string(this->y0) + "),(";
    representation += std::to_string(this->x1) + ",";
    representation += std::to_string(this->y1) + ")";
    return representation;
}


class InputPair
{
    public:
        std::vector<size_t> test_indices;
        std::vector<Polygon> polygons;
        InputPair(const std::vector<size_t> test_indices,
                  const std::vector<Polygon> polygons) {
            this->test_indices = test_indices;
            this->polygons = polygons;
        }
};


const InputPair load_npz(const std::string path)
{
    cnpy::NpyArray temp;
    temp = cnpy::npz_load(path, "n_test");
    size_t* temp2 = temp.data<size_t>();
    const size_t n_test_idx = *temp2;

    temp = cnpy::npz_load(path, "test_indices");
    size_t* idx_data = temp.data<size_t>();
    std::vector<size_t> test_indices;
    for (size_t idx = 0; idx < n_test_idx; ++idx)
        test_indices.push_back(idx_data[idx]);

    temp = cnpy::npz_load(path, "n_poly");
    const int *n_poly = temp.data<int>();
    const int n_polygons = *n_poly;

    #ifdef DEBUG
    std::cout << "N_poly: " << n_polygons << std::endl;
    std::string key = "len" + std::to_string(n_polygons);
    #endif

    std::vector<Polygon> polygons;
    for (size_t idx = 0; idx < n_polygons; ++idx) {
        const std::string len_key = "len_" + std::to_string(idx);
        const std::string arr_key = "arr_" + std::to_string(idx);
        temp = cnpy::npz_load(path, len_key);
        const int* len = temp.data<int>();
        const size_t length = (size_t) *len;
        cnpy::NpyArray array = cnpy::npz_load(path, arr_key);
        double* data = array.data<double>();
        std::vector<Vertex> polygon_data;
        for (size_t i = 0; i < length-1; ++i) {
            std::cout << "TEST 1 " << idx << " - " << i << std::endl << std::flush;
            Vertex tmp = Vertex(data[2*i], data[2*i+1]);
            std::cout << "TEST 2 " << idx << " - " << i << std::endl << std::flush;
            polygon_data.push_back(tmp);
            #ifdef DEBUG
            std::cout << std::string(polygon_data.back()) << std::endl;
            #endif
        }
        polygons.push_back(polygon_data);
        #ifdef DEBUG
        std::cout << std::endl;
        #endif
    }
    return InputPair(test_indices, polygons);
}


const bool share_edge_cpp(Polygon polygon_0,
                    Polygon polygon_1)
{
    std::vector<PolygonEdge> edges_0, edges_1;
    // upper limit requires closed polygon
    for (size_t idx = 0; idx < polygon_0.size()-1; ++idx) {
        edges_0.push_back(PolygonEdge(polygon_0[idx], polygon_0[idx+1]));
    }
    for (size_t idx = 0; idx < polygon_1.size()-1; ++idx) {
        edges_1.push_back(PolygonEdge(polygon_1[idx], polygon_1[idx+1]));
    }

    // checks every edge pair twice, optimise if necessary
    for (size_t idx_0 = 0; idx_0 < edges_0.size(); ++idx_0) {
        for (size_t idx_1 = 0; idx_1 < edges_1.size(); ++idx_1) {
            const bool match = (edges_0.at(idx_0) == edges_1.at(idx_1));
            if (match)
                return true;
        }
    }

    return false;
}


// globals for parallel access
const InputPair input_data = load_npz(INPUT_FILE);


void find_neighbours_task(const size_t thread_idx,
                          const size_t test_polygon_idx,
                          std::vector<size_t> results)
{
    #ifdef DEBUG
    std::cout << "TEST 0 " << test_polygon_idx << std::endl << std::flush;
    #endif
    const std::vector<size_t> test_indices = input_data.test_indices;
    const std::vector<Polygon> polygons = input_data.polygons;

    const Polygon test_polygon = polygons.at(test_polygon_idx);

    #ifdef DEBUG
    std::cout << "TEST 1" << std::endl << std::flush;
    #endif

    // loop through the other polygons searching for a neighouring polygon
    size_t result_idx = 0;
    for (size_t idx = 0; idx < polygons.size(); ++idx) {
        if (idx == test_polygon_idx)
            continue;
        #ifdef DEBUG
        std::cout << "TEST 2" << std::endl << std::flush;
        #endif
        const bool share_edge = share_edge_cpp(test_polygon, polygons.at(idx));
        if (share_edge)
            results.at(result_idx) = idx;
        #ifdef DEBUG
        std::cout << "TEST 3" << std::endl << std::flush;
        #endif
    }
    return;
}


#ifdef __cplusplus
extern "C" {
#endif
static PyObject* find_neighbours(PyObject* self, PyObject* args)
{
//     int py_n_test_indices;
//     PyObject *py_test_polygon_indices;
//    /* Parse the input tuple */
//     if (!PyArg_ParseTuple(args, "iO", &py_n_test_indices,
//                           &py_test_polygon_indices))
//         return NULL;
//     #ifdef DEBUG
//     std::cout << "py_n_test_indices: " << py_n_test_indices << std::endl;
//     #endif

//     const size_t n_test_indices = (size_t) py_n_test_indices;
//     std::vector<size_t> test_polygon_indices;
//     // index list into vector
//     PyObject *iterator = PyObject_GetIter(py_test_polygon_indices);
//     if (!iterator)
//         return NULL;
//     for (size_t idx = 0; idx < n_test_indices; ++idx) {
//         unsigned long tmp = (unsigned long)PyIter_Next(iterator);
//         test_polygon_indices.push_back((size_t)tmp);
//         #ifdef DEBUG
//         std::cout << "PARSED INDEX: " << idx << " = " << tmp << std::endl;
//         #endif
//     }
    const std::vector<size_t> test_indices = input_data.test_indices;
    const std::vector<Polygon> polygons = input_data.polygons;
    const size_t n_test_indices = test_indices.size();

    // check for the correct number of input values
    #ifdef PARALLEL
    const size_t MAX_THREADS = std::thread::hardware_concurrency()-1;
    // do not attempt to launch more threads than there are results to compute
    const size_t N_THREADS = (n_test_indices < MAX_THREADS) ? n_test_indices : MAX_THREADS;
    #endif

    // initialise results vector
    std::vector<std::vector<size_t>> results;
    for (size_t idx = 0; idx < n_test_indices; ++idx)
        results.push_back(std::vector<size_t>(999));

    // DO STUFF IN PARALLEL
    #ifdef PARALLEL
    std::vector<std::thread> threads(MAX_THREADS-1);
    #endif
    for (size_t thread_idx = 0; thread_idx < n_test_indices; ++thread_idx) {
        #ifdef PARALLEL
        threads.at(thread_idx) = std::thread(find_neighbours_task, thread_idx,
                                             test_polygon_indices.at(thread_idx),
                                             results.at(thread_idx));
        #else

        #ifdef DEBUG
        std::cout << std::endl;
        std::cout << "LAUNCHING:" << std::endl;
        std::cout << "\tthread_idx = " << thread_idx << std::endl;
        std::cout << "\ttest_polygon_idx = " << test_indices.at(thread_idx) << std::endl;
        std::cout << std::endl << std::flush;
        #endif
        find_neighbours_task(thread_idx, test_indices.at(thread_idx),
                             results.at(thread_idx));
        #endif
    }
    #ifdef PARALLEL
    for (size_t thread_idx = 0; thread_idx < n_test_indices - 1; ++thread_idx) {
        threads.at(thread_idx).join();
    }
    #endif

    #ifdef DEBUG
    std::cout << "PROCESSING DONE" << std::endl << std::flush;
    #endif

    // build 2d output
    size_t max_neighbours = 0;
    for (size_t idx = 0; idx < n_test_indices; ++idx)
        max_neighbours = std::max(max_neighbours, results.at(idx).size());
    #define VECTOR_OUTPUT
    #ifdef VECTOR_OUTPUT
    std::vector<std::vector<long>> output;
    for(int i = 0; i < n_test_indices; ++i)
        output.push_back(std::vector<long>());
    for(size_t i = 0; i < n_test_indices; ++i) {
        const size_t length = results.at(i).size();
        for (size_t j = 0; j < max_neighbours; ++j) {
            if (j >= length)
                output[i].push_back(-1);
            else
                output[i].push_back(results.at(i).at(j));
        }
    }
    #else
    int** output = new int*[n_test_indices];
    for(int i = 0; i < n_test_indices; ++i)
        output[i] = new int[max_neighbours];
    for (size_t i = 0; i < n_test_indices; ++i) {
        for (size_t j = 0; j < max_neighbours; ++j) {
            int length = results.at(i).size();
            if (j >= length)
                output[i][j] = -1;
            else
                output[i][j] = results.at(i).at(j);
        }
    }
    #endif

    #ifdef DEBUG
    std::cout << "OUTPUT BUILDING DONE" << std::endl << std::flush;
    #endif

    // write output
    #ifdef VECTOR_OUTPUT
    cnpy::npy_save(OUTPUT_FILE, output.data(), {n_test_indices, max_neighbours}, "w");
    #else
    cnpy::npy_save(OUTPUT_FILE, output, {n_test_indices, max_neighbours}, "w");
    #endif

    #ifdef DEBUG
    std::cout << "OUTPUT SAVING DONE" << std::endl << std::flush;
    #endif

    #ifndef VECTOR_OUTPUT
    for(int i = 0; i < n_test_indices; ++i)
        delete[] output[i];
    delete[] output;
    #endif

    Py_RETURN_NONE;
}


static PyMethodDef pt_methods[] = {
    { "find_neighbours", find_neighbours, METH_NOARGS,
    "Finds the neighbours of polygons within a large set of polygons.\n"
    "Polygons are read from 'data/polygons_todo.npz' and neighours\n"
    "are written to 'data/neighbours.npy.\n"
    "The input file is expected to contain arrays under keys of form\n"
    "'arr_0', 'arr_1', etc. The output file will contain a 2d array with\n"
    "the indices of neighbouring polygons.\n"
    "Parameters\n"
    "----------\n"
    "n_polygons: int\n"
    "    For which to find the neighbours.\n"
    "test_polygon_indices: list\n"
    "    Indices of the polygons for which neighbours are searched.\n"
    "\n"
    "Returns\n"
    "-------\n"
    "None\n"
    "\n"
    "Raises\n"
    "------\n"
    "\n"
    "\n" },
    { NULL, NULL, 0, NULL }
};


static struct PyModuleDef pt_module = {
    PyModuleDef_HEAD_INIT,
    "polygon_tools",
    NULL,
    -1,
    pt_methods
};


PyMODINIT_FUNC PyInit_polygon_tools(void)
{
    return PyModule_Create(&pt_module);
}

#ifdef __cplusplus
}  // closes extern "C"
#endif
