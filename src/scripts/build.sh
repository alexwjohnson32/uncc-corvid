#!/bin/bash

# Setting the SCRIPT_DIR and PROJECT_ROOT should let us be able
# to call the script from any location and it still build relative to the
# project root. If you move this script though, its going to have a hard time
# finding the project. Symlinks are allowed.

# 1. Get the absolute path to the directory where this script lives
# This works even if you call the script via a symlink
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# 2. Define the Project Root relative to the script location
PROJECT_ROOT=$(cd -- "$SCRIPT_DIR/.." &> /dev/null && pwd)

# 3. Move to the project root
cd "$PROJECT_ROOT" || { echo "Failed to locate project root"; exit 1; }

# 4. Set Default Vars
PRESET_CONFIG_NAME="debug"
PRESET_BUILD_NAME="build-debug"
FRESH_FLAG=""
RUN_TESTS=false
CTEST_ARGS=""

# 5. Argument Parsing
while [[ $# -gt 0 ]]; do
    case $1 in
        release)
            PRESET_CONFIG_NAME="release"
            PRESET_BUILD_NAME="build-release"
            shift
            ;;
        test)
            PRESET_CONFIG_NAME="test-coverage-config"
            PRESET_BUILD_NAME="build-test"
            FRESH_FLAG="--fresh"
            RUN_TESTS=true
            shift
            ;;
        --fresh)
            FRESH_FLAG="--fresh"
            shift
            ;;
        --)
            shift # Remove the -- from the arguments
            CTEST_ARGS="$@" # Capture everything left for CTest
            set -- # Clear the remaining positional parameters so they don't go to CMake
            break
            ;;
        *)
            # Pass everything else (like -D flags) to CMake
            break
            ;;
    esac
done

# 6. Pre-run Cleanup for test coverage
if [ "$RUN_TESTS" = true ]; then
    echo "--- Cleaning old coverage data ---"
    rm -rf "$PROJECT_ROOT/coverage_report"
    rm -f "$PROJECT_ROOT/coverage.info"
    # Note: --fresh handles the CMakeCache, but manual rm ensures
    # the HTML folder is wiped before the new one is generated.
fi

# 7. Configure
echo "--- Building with mode: $PRESET_CONFIG_NAME ---"
echo "--- CMake Arguments: $@ ---"
echo "--- Configuring ---"
cmake --preset "$PRESET_CONFIG_NAME" $FRESH_FLAG "$@"

# 8. Build
echo ""
echo "--- Building ---"
cmake --build --preset "$PRESET_BUILD_NAME"

# 9. Test & Coverage Report
if [ "$RUN_TESTS" = true ]; then
    echo "--- Running Tests ---"
    echo "--- CTest Arguments: $CTEST_ARGS ---"
    # Pass the captured CTest arguments here
    ctest --preset "test-coverage" $CTEST_ARGS

    if command -v lcov &> /dev/null; then
        echo "--- Generating HTML Report ---"
        # --no-external ignores /usr/include headers
        lcov --capture --directory . --output-file coverage.info --no-external
        genhtml coverage.info --output-directory coverage_report
        echo "Done! View report: $PROJECT_ROOT/coverage_report/index.html"
    else
        echo "Warning: lcov not found. Tests passed, but no report generated."
    fi
fi