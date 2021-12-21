
import subprocess
import re
def mute():
    print("mac mute")
    stream = subprocess.run('.\modules\SoundVolumeView.exe /Mute "DefaultCaptureDevice"')
    

def get_volume():
    print("win get_volume")
    stream = subprocess.run('.\modules\SoundVolumeView.exe /GetPercent "DefaultCaptureDevice"')
    print(stream.returncode)

def get_mute():
    print("win get_volume")
    stream = subprocess.run('.\modules\SoundVolumeView.exe /GetMute "DefaultCaptureDevice"')
    return str(stream.returncode)

def unmute():
    print("win unmute")
    stream = subprocess.run('.\modules\SoundVolumeView.exe /UnMute "DefaultCaptureDevice"')

unmute()
print(get_mute() == "0")