
import subprocess
import re
import ctypes
import time

import win32gui
import win32api
import win32con
import win32process

def mute():
    print("mac mute")
    stream = subprocess.run('./modules/SoundVolumeView.exe /Mute "DefaultCaptureDevice"')
    

def get_volume():
    print("win get_volume")
    stream = subprocess.run('./modules/SoundVolumeView.exe /GetPercent "DefaultCaptureDevice"')
    print(stream.returncode)

def get_mute():
    print("win get_volume")
    stream = subprocess.run('./modules/SoundVolumeView.exe /GetMute "DefaultCaptureDevice"')
    return str(stream.returncode)

def unmute():
    print("win unmute")
    stream = subprocess.run('./modules/SoundVolumeView.exe /UnMute "DefaultCaptureDevice"')

def lock():
    ctypes.windll.user32.LockWorkStation()


def is_screen_locked():
    import subprocess
    import time
    time.sleep(5)
    process_name='LogonUI.exe'
    callall='TASKLIST'
    outputall=subprocess.check_output(callall)
    outputstringall=str(outputall)
    if process_name in outputstringall:
        return True
    else: 
        return False



if __name__ == "__main__":
    #lock()
    #time.sleep(5)
    print(is_screen_locked())


