# !/bin/sh

echo "============Current dir:================="
pwd
echo "============Delete build folder================="
rm -rf build
echo "============Create build folder================="
mkdir build
echo "============Go to build folder ================="
cd build
echo "============Run cmake================="
cmake ..
echo "============Build================="
make 
echo "================Run tests(with sudo)================="

OUTPUT=$(sudo ./test_exec 2>&1)
EXIT_CODE=$?

echo "$OUTPUT"
echo "======================================================"

# Check for known error string or non-zero exit code
if echo "$OUTPUT" | grep -q "bind() failed: Permission denied" || [ $EXIT_CODE -ne 0 ]; then
    echo "Test failed"
else
    echo "Test successful"
fi