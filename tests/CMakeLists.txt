include_directories(../source/src/engine)
include_directories(../source/include/modules)
include_directories(../source/include)

# To add a new test:
#   - Write the test file newTest.cpp in source/tests/
#       - If headers other than those in source/src/engine are used, add an include_directiores(path/to/headers)
#   - Add in this file add_executable(newTest newTest.cpp)
#   - Add in this file add_test(uniqueTestName nodeTest)
#       - uniqueTestName is the name that will be shown in the terminal when running test.sh, nodeTest is the name of the executable found in build
#   - To see the output of the test, go to build/tests/Testing/Temporary/LastTest.log

add_executable(nodeTest nodeTest.cpp)
add_test(NodeTest nodeTest)


add_executable(transformTest transformTest.cpp)
add_test(TransformTest transformTest)
