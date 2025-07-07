import os, setuptools, sys, numpy as np

this_folder = os.path.abspath(os.path.dirname(__file__))
# parent_folder = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
target_folder = os.path.abspath(os.path.join(this_folder, 'bones'))
python_include_folder = os.path.abspath(os.path.join(sys.executable, 'include'))
numpy_include_folder = np.get_include()

print('----------------------------------------------------------------------------------------------------------------')
print(f'Python include: {python_include_folder}')
print(f'Numpy include: {numpy_include_folder}\n\n')

if not os.path.exists(target_folder) or not os.path.isdir(target_folder): os.makedirs(target_folder, exist_ok=True)
# https://stackoverflow.com/questions/61692952/how-to-pass-debug-to-build-ext-when-invoking-setup-py-install
COMPILE_ARGS = ['-O2'] #, '-std=c99']


# path/to/pythonX.Y setup.py build_ext --debug install

setuptools.setup(
    ext_modules=[
        setuptools.Extension(
            'bones.jones',
            sources=[
                os.path.join('src/jones/mod_jones.c')
            ],
            include_dirs=[
                python_include_folder,
                numpy_include_folder,
            ],
            extra_compile_args=COMPILE_ARGS
        ),
        # setuptools.Extension(
        #     'bones.jones_wip',
        #     [os.path.join(parent_folder, 'src/jones/mod_jones_wip.c')],
        #     include_dirs=[python_include_folder, np.get_include()],
        #     extra_compile_args=COMPILE_ARGS,
        # ),
        setuptools.Extension(
            'bones.qu',
            sources=[
                os.path.join('src/jones/mod_qu.c')
            ],
            include_dirs=[
                python_include_folder,
                numpy_include_folder,
            ],
            extra_compile_args=COMPILE_ARGS,
        ),
        # setuptools.Extension(
        #     'bones.qu',
        #     sources=[
        #
        #     ],
        #     include_dirs=[
        #         python_include_folder,
        #         numpy_include_folder,
        #     ],
        # ),
    ]
)

