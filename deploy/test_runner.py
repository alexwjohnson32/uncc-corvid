import unittest
import argparse
import sys
import os
import pathlib
import tests.tools.base_plugin_test_case as base

def main():
    parser = argparse.ArgumentParser(description="Plugin Test Central Runner")

    # Configuration Arguments
    parser.add_argument('--install-dir', type=str, required=True,
                        help='Path to installation, this will remain unchanged by testing')
    parser.add_argument('--write-dir', type=str, required=True,
                        help='Path to a writable directory, any contents within may be deleted or modified')

    # Discovery Arguments
    parser.add_argument('--discover-dir', type=str, default='tests',
                        help='Directory to start discovery (default: tests)')
    parser.add_argument('--pattern', type=str, default='*.py',
                        help='Pattern to match test files (default: test_*.py)')

    # Unittest Verbosity
    parser.add_argument('-v', '--verbose', action='store_true', help='Verbose output')

    args = parser.parse_args()

    try:
        # 1. Inject configuration into the Base Class
        # Because all tests inherit from this, they will all "see" these values.
        # The install dir must exist before call time
        base.BasePluginTestCase.install_dir = pathlib.Path(args.install_dir).resolve(strict=True)
        # Get the deploy dir, and if it does not exist, create it.
        base.BasePluginTestCase.deploy_dir = pathlib.Path(args.write_dir, "deploy")
        if not base.BasePluginTestCase.deploy_dir.exists():
            base.BasePluginTestCase.deploy_dir.mkdir()
        # At this point it should exist, so get the absolute path with a strict resolve
        base.BasePluginTestCase.deploy_dir = base.BasePluginTestCase.deploy_dir.resolve(True)

        # 2. Discover Tests
        # This looks through 'start_dir' and finds all files matching 'pattern'
        loader = unittest.TestLoader()
        suite = loader.discover(start_dir=args.discover_dir, pattern=args.pattern)

        # 3. Run the Suite
        print(f"Discovering tests in: {pathlib.Path(args.discover_dir).resolve()}")
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