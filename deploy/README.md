# Deploy Tool

This is a tool that allows a user to deploy a cosim to a given directory in order to run the cosimulation.

# Test Runner

The `test_runner.py` file will discover and run unit tests within the project. Currently it is geared towards running the plugins, and I expect changes may need to be made in the future to promote discoverability of other unit tests for commands and other tools.

## Running the tests

* Launch the apptainer shell and run everything through that image.
* Ensure that you have the federates built and installed. If you are unsure of this, simply cd into the `src` directory and run `./scripts/build.sh` and it will build and create an install directory for you at `src/install/debug`.
* Once you know where your install directory is, cd into `deploy`
* From there, run `python -m test_runner --install-dir=<path_to_install_dir> --write-dir=temp`.
    * The `write-dir` can be any path that you want to write to, but temp is simple and should be ignored by `.gitignore`.
    * For full options on running, call `python -m test_runner --help`
    * If running tests back to back, be sure to `rm -rf temp/*` to ensure that you have a clean working directory for the tests.