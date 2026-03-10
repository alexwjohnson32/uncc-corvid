import argparse

import commands.list_names

def main():
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

    # list-names command
    list_names_parser = subparsers.add_parser("list-names", help="List the names of the found IDeployable Plugins")

    args = top_parser.parse_args()

    if args.command == "list-names":
        commands.list_names.get_plugin_names()

if __name__ == '__main__':
    main()