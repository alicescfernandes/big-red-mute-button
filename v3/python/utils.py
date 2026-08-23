import time
import threading

def current_milli_time():
    return round(time.time() * 1000)

def threaded(fn):
    def wrapper(*args, **kwargs):
        threading.Thread(target=fn, args=args, kwargs=kwargs).start()
    return wrapper
