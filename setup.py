from distutils.core import setup

long_description = """Metapackage of various CPython extensions
for manipulating and analysing polygons.
"""

setup (name = 'polygon_tools',
       version = '1.0.1',
       description = 'Collection of polygon extensions.',
       url = 'https://github.com/sgaebel/polygon_tools',
       author = 'Dr. Sebastian M. Gaebel',
       author_email = 'gaebel.sebastian@gmail.com',
       license = 'MIT',
       long_description = long_description,
       install_requires = ['numpy', 'polygon_neighbours', 'polygons_share_edge',
                           'polygon_contains_point'],
       classifiers = [
              'Development Status :: 5 - Production/Stable',
              'Intended Audience :: Developers',
              'Topic :: Scientific/Engineering :: Mathematics',
              'License :: OSI Approved :: MIT License',
              'Programming Language :: Python :: 3.10',
              'Operating System :: POSIX :: Linux'],
       keywords = 'geometry polygon',
       python_requires = '>=3.10')
