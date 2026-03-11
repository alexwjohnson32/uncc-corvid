#!/bin/bash

SIF_FILE=$1

echo "----GRIDLAB: check installed"
apptainer exec $SIF_FILE gridlabd --version

# 1. Activate Environment
apptainer exec $SIF_FILE bash -s <<'EOF' 
echo "--- 1. Activating Environment ---"
if [ -f "/opt/myenv/bin/activate" ]; then
    source /opt/myenv/bin/activate
    echo "Virtual environment activated 🛠️"
else
    echo "Error: /opt/myenv/bin/activate not found! ⚠️"
    exit 1
fi

# 2. Check HELICS Binaries
echo -e "\n--- 2. Checking HELICS Binaries ---"
if command -v helics_broker &> /dev/null; then
    echo "Found helics_broker: $(helics_broker --version) ✅"
else
    echo "Error: helics_broker not in PATH! ⚠️"
fi

# 3. Check Shared Libraries
echo -e "\n--- 3. Checking Library Linkage ---"
# This ensures the OS can find the libhelics.so file
ldd /usr/local/helics/lib64/libhelics.so.3.6.1 | grep "not found"
if [ $? -eq 1 ]; then
    echo "All C++ shared libraries are correctly linked 🔗"
else
    echo "Warning: Some library dependencies are missing! ⚠️"
fi

# 4. Check Python Bindings
echo -e "\n--- 4. Checking Python Bindings ---"
python3 -c "import helics; print(f'Python HELICS Version: {helics.helicsGetVersion()}')" 2>/dev/null
if [ $? -eq 0 ]; then
    echo "Python API successfully loaded the C++ core 🐍"
else
    echo "Error: Python could not load HELICS. Check your PYHELICS_INSTALL path ⚠️"
fi

EOF

apptainer exec --writable-tmpfs $SIF_FILE bash -s <<'EOF' 
#cd /usr/local/gridlabd/
cd /root/develop/gridlabd
/usr/local/gridlabd/bin/gridlabd --validate
EOF

#echo "----GRIDLAB: check helics support"
#apptainer exec $SIF_FILE gridlabd --version && gridlabd --help | grep -i "helics"
#echo "TEST DOES NOT SEEM TO BE FUNCTIONAL"

#echo "----GGRIDLAB: internal checks"
#apptainer exec $SIF_FILE gridlabd --unitstest
#echo "TEST DOES NOT SEEM TO BE FUNCTIONAL"
#test options--------------
#  --dsttest                                       Perform daylight savings rule test
#  --endusetest                                    Perform enduse pseudo-object test
#  --globaldump                                    Perform a dump of the global variables
#  --loadshapetest                                 Perform loadshape pseudo-object test
#  --locktest                                      Perform memory locking test
#  --modtest <module>                              Perform test function provided by module
#  --randtest                                      Perform random number generator test
#  --scheduletest                                  Perform schedule pseudo-object test
#  --test <module>                                 Perform unit test of module (deprecated)
#  --testall=<filename>                            Perform tests of modules listed in file
#  --unitstest                                     Perform unit conversion system test
#  --validate ...                                  Perform model validation check

echo "----ADDITIONAL TESTS: "
echo "run the following from the root directory of the source code: gridlabd --validate"