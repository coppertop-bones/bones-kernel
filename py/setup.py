import os, setuptools, sys

parent_folder = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
python_include_dir = os.path.abspath(os.path.join(sys.executable, 'include'))

setuptools.setup(ext_modules=[
    setuptools.Extension(
        "bones.jones",
        [os.path.join(parent_folder, "src/jones/jones.c")],
        include_dirs=[python_include_dir]
    ),
    setuptools.Extension(
        "bones.qu",
        [os.path.join(parent_folder, "src/jones/jones_qu.c")],
        include_dirs=[python_include_dir],
    ),
])
