import os, setuptools, distutils.core
parent_folder = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
setuptools.setup(
  ext_modules=[distutils.core.Extension("bones.jones", [os.path.join(parent_folder, "src/jones/__jones.c")])],
)
