INCLUDE(TribitsAddExecutableAndTest)
INCLUDE(TribitsCopyFilesToBinaryDir)

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  Segregated
  SOURCES main.cpp ud_functions.cpp
  ARGS -c
  NUM_MPI_PROCS 2
  COMM serial mpi
  )

TRIBITS_COPY_FILES_TO_BINARY_DIR(data_test_fsi
  SOURCE_FILES data
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(solverParameter
  SOURCE_FILES xmlParameters.xml
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(vessel_coarse.mesh_test_fsi
  SOURCE_FILES vessel_coarse.mesh
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/lifev/fsi/data/mesh/inria/
)

TRIBITS_COPY_FILES_TO_BINARY_DIR(tube_coarse.mesh_test_fsi
  SOURCE_FILES tube_coarse.mesh
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/lifev/fsi/data/mesh/inria/
)

