add_executable(ArborX_RayTracing.exe example_raytracing.cpp)
target_link_libraries(ArborX_RayTracing.exe ArborX::ArborX Boost::program_options)
add_test(NAME ArborX_RayTracing_Example COMMAND ./ArborX_RayTracing.exe --nx=10 --ny=10 --nz=10 --lx=1 --ly=100000 --lz=100000 --rays=1000)
