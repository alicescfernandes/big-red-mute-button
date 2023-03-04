import platform
from os_layer import win, mac
from ctypes import CDLL
import sys
import Quartz
loginPF = CDLL('/System/Library/PrivateFrameworks/login.framework/Versions/Current/login')
import time
import threading
from utils import current_milli_time, threaded

class LockControl:
    def __init__(self):
        ""
        self.system = platform.system()
        self.time_start = current_milli_time()
        self.time_current = self.time_start

    # The button only has a single state, and cannot be toggled because the unlock  is by fingerprint
    def toggle(self):
        if(self.is_screen_locked() is  False):
            self.lock()
        return True;
  
    def is_screen_locked(self):
        if(self.system == "Darwin"):
            mac.is_screen_locked()
        elif(self.system == "Windows"):
            win.is_screen_locked()
        else:
            print("System not supported")

    def lock(self):
        if(self.system == "Darwin"):
            mac.lock()

        elif(self.system == "Windows"):
            win.lock()
        else:
            print("System not supported")

    @threaded
    def check_status(self, callbackLock, callbackUnlock,interval_ms = 500):
        previousLockStatus = True
        exitLoop = False
        while exitLoop == False:
            self.time_current = current_milli_time()
            delta = self.time_current - self.time_start
            if(delta >= interval_ms):
                is_screen_locked = self.is_screen_locked();
                
                if(is_screen_locked is True and previousLockStatus is False):
                    callbackLock()

                if(is_screen_locked is False and previousLockStatus is True):
                    callbackUnlock()
                
                if(previousLockStatus != is_screen_locked):
                    previousLockStatus = is_screen_locked

                self.time_start = current_milli_time()

if __name__ == "__main__":    
    def lock():
        print("lock")

    def unlock():
        print("unlock")


    if len(sys.argv) < 2:
        print('missing command')
        sys.exit()

    c = LockControl()
    input_path = sys.argv[1]
    c.check_lock_status(lock, unlock)
    if(input_path == "lock"):
        c.lock_screen()
    
    if(input_path == "check"):
        c.is_screen_locked()
