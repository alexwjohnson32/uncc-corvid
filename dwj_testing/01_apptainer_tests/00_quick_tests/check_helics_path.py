import helics
import os

# 1. Check where the Python wrapper is installed
print(f"Python module: {helics.__file__}")

# 2. Check which shared library (the C++ core) is actually loaded
# The internal _lib object holds the path to the .so file
try:
    # This reaches into the CFFI/library loader for the underlying .so
    lib_path = helics.helics._lib.__file__ if hasattr(helics.helics, '_lib') else "Not Found"
    print(f"Loaded C library: {lib_path}")
except Exception as e:
    print(f"Could not find library path: {e}")

# 3. Double check MPI availability
print(f"MPI Core Available: {helics.helicsIsCoreTypeAvailable('mpi')}")