#include "gridpack.hpp"

int main(int argc, char *argv[]) {
  gridpack::Environment env(argc, argv);
  gridpack::parallel::Communicator world;
  
  if (world.rank() == 0) {
    printf("=========================================\n");
    printf("GridPACK Installation Verified Successfully!\n");
    printf("Running on %d processors.\n", world.size());
    printf("=========================================\n");
  }
  
  // Create a parallel vector to verify PETSc/Trilinos math linkage
  gridpack::math::Vector vec(world, 100);
  vec.zero();
  
  return 0;
}