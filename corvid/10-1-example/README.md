EPIC,UNCC
One GridPACK with multiple GridLAB-D models
A Co-Simulation setup that simulates one IEEE 118 GridPACK transmission model with ten (10) different IEEE-8500 node GridLAB-D distribution models. The setup uses GridPACK for transmission, GridLAB-D for distribution and HELICS which serves as the cosimulation manager.

The set up is briefly described to have

- Ten (10) IEEE-8500 node models in GridLAB-D module which serves as distribution grid models which publishes the aggregated PQ values to the GridPACK and subscribes to the voltage from GridPACK.
- One (1) bus of IEEE-118 bus model in GRIDPACK module which serves as the transmission grid subscribes to the aggregated PQ values of ten (10) GridLAB-D models and publishes its voltage to GridLAB-D.
- HELICS which bridges the power and voltage exchange between the distribution grid and the transmission grid.


-> The cosimulation set up is in the folder with the name "glds_10_118_1_bus". With all the tools installed and to run the cosimulation, run the command:

helics run --path=ieee8500_10_gpk_1_cosim_runner.json

NOTE: To reduce the simulation time, change the time in all the GridLAB-D models as well as the powerflow_ex.cpp file and then recompile before running the cosimulation


The results of the cosimulation is in the file "gpk_118_10_bus_conn.csv"
-> The simulation setup is for 24 hours which runs in less than 10 minutes in our docker setup


---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
To validate the results obtained in the cosimulation set up, navigate to the folder '118_gridpack_alone"

*       If building from the scratch, run the command: cmake -B build .

*       Navigate to to the build folder and run the command:  make

*       Run the following command to run the powerflow:         ./powerflow_ex.x



The results of the powerflow in GridPACK will be in the file "gpk_118_10_conn.csv" which will be in the build folder

NOTE: To reduce the simulation time, change the time in all the GridLAB-D models as well as the powerflow_ex.cpp file and then recompile before running the cosimulation.
