import unittest
import argparse
import sys
import os
import pathlib
import base_plugin_test_case

def main():
    parser = argparse.ArgumentParser(description="Plugin Test Central Runner")

    # Configuration Arguments
    parser.add_argument('--install-dir', type=str, required=True,
                        help='Path to installation, this will remain unchanged by testing')
    parser.add_argument('--deploy-dir', type=str, required=True,
                        help='Path to deployment, any contents within may be deleted or modified')

    # Discovery Arguments
    parser.add_argument('--start-dir', type=str, default='..',
                        help='Directory to start discovery (default: parent)')
    parser.add_argument('--pattern', type=str, default='*.py',
                        help='Pattern to match test files (default: *.py)')

    # Unittest Verbosity
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')

    args = parser.parse_args()

    try:
        # 1. Inject configuration into the Base Class
        # Because all tests inherit from this, they will all "see" these values.

        # The install dir must exist before call time
        base_plugin_test_case.BasePluginTestCase.install_dir = pathlib.Path(args.install_dir).resolve(strict=True)
        # Get the deploy dir, and if it does not exist, create it.
        base_plugin_test_case.BasePluginTestCase.deploy_dir = pathlib.Path(args.deploy_dir)
        if not base_plugin_test_case.BasePluginTestCase.deploy_dir.exists():
            base_plugin_test_case.BasePluginTestCase.deploy_dir.mkdir()
        # At this point it should exist, so get the absolute path with a strict resolve
        base_plugin_test_case.BasePluginTestCase.deploy_dir = base_plugin_test_case.BasePluginTestCase.deploy_dir.resolve(True)

        print(f"BaseTestInstallDir: {base_plugin_test_case.BasePluginTestCase.install_dir}")
        print(f"BaseTestDeployDir: {base_plugin_test_case.BasePluginTestCase.deploy_dir}")

        # 2. Discover Tests
        # This looks through 'start_dir' and finds all files matching 'pattern'
        loader = unittest.TestLoader()
        suite = loader.discover(start_dir=args.start_dir, pattern=args.pattern)

        # 3. Run the Suite
        print(f"Discovering tests in: {pathlib.Path(args.start_dir).resolve()}")
        print("-" * 40)

        runner = unittest.TextTestRunner(verbosity=2 if args.verbose else 1)
        result = runner.run(suite)

        # Exit with appropriate code
        sys.exit(not result.wasSuccessful())
    except FileNotFoundError as e:
        print(f"The following file was not found: {e.filename}")
    except PermissionError as e:
        print(f"Permission Denied for: {e.filename}")
        if e.errno:
            print(f"OS Error Descriptor: {os.strerror(e.errno)}")
    except OSError as e:
        print(f"Permission Denied for: {e.filename}")
        if e.errno:
            print(f"OS Error Descriptor: {os.strerror(e.errno)}")
    except BaseException as e:
        print(f"An Exception Occured: {str(e)}")

    # We are only here with an exception, so lets fail.
    sys.exit(1)

if __name__ == '__main__':
    main()