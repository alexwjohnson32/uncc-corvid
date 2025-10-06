import http.server

import json
import argparse

class MyServer(http.server.BaseHTTPRequestHandler):
    path = ""

    def do_POST(self):
        if self.path == self.path:
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            decoded_post_data = post_data.decode("utf-8")
            print(decoded_post_data)

            response = { "message": "Received data." }

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode())
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Path not found.')
            print(f"Path '{self.path}' not found.")

def main():
    parser = argparse.ArgumentParser(description="An HTTP test server")
    parser.add_argument("--hostname", dest="hostname", help="The hostname", default="127.0.0.1")
    parser.add_argument("--port", dest="port", help="The port name", default=23555)
    parser.add_argument("--path", dest="path", help="The POST Path", default="/ws/logs")

    args = parser.parse_args()

    MyServer.path = args.path

    try:
        with http.server.HTTPServer((args.hostname, args.port), MyServer) as server:
            print(f"Serving on http://{args.hostname}:{args.port}")
            print(f"POST Path: {MyServer.path}")
            server.serve_forever()
    except KeyboardInterrupt:
        print("\nUser requested close.")
    except Exception as e:
        print(f"\nUnhandled Exception: {e}")
    finally:
        print("Closing.")

if __name__ == "__main__":
    main()