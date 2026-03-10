import argparse

def main():
    top_parser = argparse.ArgumentParser("UNCC Deploy Tool for initializing runnable cosimulation models")
    subparsers = top_parser.add_subparsers(dest="command", help="The Deploy Tool Commands")

if __name__ == '__main__':
    main()