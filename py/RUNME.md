conda activate b311
. ./set_paths.sh

python setup.py build_ext --force

python -m tests.test_tm
