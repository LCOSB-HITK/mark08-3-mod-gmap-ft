import socketserver

import http.server

class LogHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        log_data = self.rfile.read(content_length).decode('utf-8')
        print(log_data)

        self.send_response(200)
        self.end_headers()

if __name__ == '__main__':
    PORT = 8000

    print(f"Starting server on port {PORT} ...")
    with socketserver.TCPServer(("", PORT), LogHandler) as httpd:
        print(f"Server started on port {PORT}")
        # print the local ip address of the server
        print(httpd.gethostbyname(socket.gethostname()))
        httpd.serve_forever()