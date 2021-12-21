
import os
import re
def mute():
    print("mac mute")
    stream = os.popen('osascript -e "set volume input volume 0"')
    output = stream.read()

def get_volume():
    print("mac get_volume")
    stream = os.popen('osascript -e "get volume settings"')
    output = stream.read()
    volume = re.search("input volume:(\d+),", output).groups()[0]
    return volume


def unmute():
    print("mac unmute")
    stream = os.popen('osascript -e "set volume input volume 100"')
    output = stream.read()
