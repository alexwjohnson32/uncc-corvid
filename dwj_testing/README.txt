This directory holds some of the troubleshooting work I've been doing in an attempt to get helics, gridlab, and gridpack to work together using mpi communication so they can be run on the hpc.

00_apptainer_builds
    - each sub folder (only one at the moment) contains a different apptainer build.
    - 20260305 
        - folder's contents are similar in structure to the "multi_stage_build" folder. Some of the def files have been modified.
        - The gridpack install works but is a little funky. The gridpack.hpp library is in /usr/local/GridPack/include/gridpack/include

01_apptainer_tests
    - 00_quick_tests
        - These are the most useful for evaluating the apptainer installation
    - 01_helics_test
        - simple helics run
    - 02_gridlab_test
        - simple gridlab run
    - 03 gridpack_test
        - simple gridpack run
        - CURRENTLY BROKEN

02_cosims
    - DESCRIPTION: larger / more sophisticated cosims
    - 00_helics_python_mpi
        - this test confirms that we can use the mpi core type on the cluster to run scalable sims.
        - runs a game of life sim across multiple cores / nodes (when on hpc)
    - 01_helics_gridlab_gridpack_simple
        - very simple cosim that is good for troubleshooting our workflows
        - involves data transfer to / from gridlab and gridpack
    - 02, 01 but trying to run with mpi (mpi coretype, mpirun)
