INCLUDE(TribitsCopyFilesToBinaryDir)
INCLUDE(TribitsAddExecutable)

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  test_block_solver
  SOURCES main.cpp
  ARGS -c
  NUM_MPI_PROCS 4
  COMM serial mpi
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(data_test_block_solver
  CREATE_SYMLINK
  SOURCE_FILES data
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(solverOptions
  CREATE_SYMLINK
  SOURCE_FILES solversOptionsFast.xml
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(mesh_test_block_solver
  SOURCE_FILES tube20.mesh
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/lifev/fsi/data/mesh/inria
)

