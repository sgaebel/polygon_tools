# Polygon_Tools

Meta-package of multiple CPython extensions for manipulating and analysing polygons. It specifically adds a wrapper for the `polygon_neighbours` submodule to make it much more user friendly.

## Submodules

We expose these functions:

`polygon_contains_point` => `point_in_polygon`

`polygons_share_edge` => `share_edge`

`polygon_neighbours` => `bordering_polygons` (Wrapper for `find_neighbours`)
