conda activate b311
. ./mac_set_paths.sh

python setup.py build_ext --force

python -m bones.jones.tests.test_tm


crash reports in ~/Library/Logs/DiagnosticReports/Retired/python3.11-2024-05-08-055006.ips

