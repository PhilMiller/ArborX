find_package(Boost 1.56.0 REQUIRED COMPONENTS program_options)

add_subdirectory(brute_force_vs_bvh)
add_subdirectory(bvh_driver)
add_subdirectory(dbscan)
add_subdirectory(execution_space_instances)

if (ARBORX_ENABLE_MPI)
  add_subdirectory(distributed_tree_driver)
endif()
