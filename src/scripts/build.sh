#!/bin/bash

# Setting the SCRIPT_DIR and PROJECT_ROOT should let us be able
# to call the script from any location and it still build relative to the
# project root. If you move this script though, its going to have a hard time
# finding the project. Symlinks are allowed.

# 1. Get the absolute path to the directory where this script lives
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# 2. Define the Project Root relative to the script location
PROJECT_ROOT=$(cd -- "$SCRIPT_DIR/.." &> /dev/null && pwd)

# 3. Move to the project root
cd "$PROJECT_ROOT" || { echo "Failed to locate project root"; exit 1; }

# Function to display help information
show_help() {
    echo "Usage: $(basename "$0") [mode] [options] [-- ctest_args]"
    echo ""
    echo "Modes:"
    echo "  debug            Build in debug mode (default)."
    echo "  release          Build in release mode."
    echo "  test             Build for test coverage, wipes old coverage data, and runs tests."
    echo ""
    echo "Options:"
    echo "  --fresh          Force a fresh CMake configuration (removes CMakeCache.txt)."
    echo "  -h, --help       Show this help message and exit."
    echo "  --               Everything after '--' is passed directly to CTest (when in test mode)."
    echo ""
    echo "Additional Arguments:"
    echo "  Any arguments not recognized above (e.g., -DVARIABLE=VALUE) are passed directly to CMake."
    echo ""
    echo "Examples:"
    echo "  ./build.sh debug"
    echo "  ./build.sh release"
    echo "  ./build.sh test --fresh"
    echo "  ./build.sh test -- --output-on-failure -R MyTest"
}

# 4. Set Default Vars
PRESET_CONFIG_NAME="debug"
PRESET_BUILD_NAME="build-debug"
FRESH_FLAG=""
RUN_TESTS=false
CTEST_ARGS=""

# 5. Argument Parsing
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        debug)
            PRESET_CONFIG_NAME="debug"
            PRESET_BUILD_NAME="build-debug"
            shift
            ;;
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
        -*)
            # If it looks like a flag but wasn't caught above, it's likely for CMake
            # We break here to let the remaining args be passed to CMake
            break
            ;;
        *)
            # Positional arguments or unknown flags
            break
            ;;
    esac
done

# 6. Pre-run Cleanup for test coverage
if [ "$RUN_TESTS" = true ]; then
    echo "--- Cleaning old coverage data ---"
    rm -rf "$PROJECT_ROOT/coverage_report"
    rm -f "$PROJECT_ROOT/coverage.info"
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
    ctest --preset "test-coverage" $CTEST_ARGS
fi