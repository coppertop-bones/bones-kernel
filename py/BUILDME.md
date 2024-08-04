conda activate b311
. ./set_paths.sh

python setup.py build_ext --force

python -m jones.tests.test_tm


crash reports in ~/Library/Logs/DiagnosticReports/Retired/python3.11-2024-05-08-055006.ips

