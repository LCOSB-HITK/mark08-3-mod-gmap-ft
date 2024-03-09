# logs_handler.py

class LogsHandler:
    def __init__(self):
        self.logs = []

    def add_log(self, log_entry):
        self.logs.append(log_entry)

    def get_logs(self, num_logs):
        return self.logs[-num_logs:]
