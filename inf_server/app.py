# app.py

from flask import Flask, request, jsonify
from config import SERVER_HOST, SERVER_PORT
from logs_handler import LogsHandler

app = Flask(__name__)
logs_handler = LogsHandler()

@app.route('/log', methods=['POST'])
def log_endpoint():
    content = request.get_json()
    log_entry = content.get('log_entry')

    if log_entry:
        logs_handler.add_log(log_entry)
        return jsonify({'message': 'Log received successfully'}), 200
    else:
        return jsonify({'error': 'Invalid request'}), 400

@app.route('/get_logs', methods=['GET'])
def get_logs_endpoint():
    num_logs = int(request.args.get('num_logs', 10))
    logs = logs_handler.get_logs(num_logs)
    return jsonify({'logs': logs}), 200

if __name__ == '__main__':
    app.run(host=SERVER_HOST, port=SERVER_PORT)
