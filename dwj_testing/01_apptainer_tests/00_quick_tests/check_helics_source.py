import helics as h
import os

# 1. Check the location of the Python module
python_mod_path = h.__file__
print(f"Python module location: {python_mod_path}")

# 2. Check the path of the underlying C shared library
# This is usually where the 'mpi' support lives
try:
    # This reaches into the internal loader to see which .so/.dll was grabbed
    c_lib_path = h.helics._lib.__file__ if hasattr(h.helics, '_lib') else "Unknown"
    print(f"C library location: {c_lib_path}")
except Exception:
    print("Could not determine C library location.")

# 3. Final Verdict
if "site-packages" in python_mod_path:
    print("\nVerdict: 🛑 You are using the PIP-installed version (custom build isn't triggering).")
else:
    print("\nVerdict: ✅ You are using the LOCALLY-installed version.")