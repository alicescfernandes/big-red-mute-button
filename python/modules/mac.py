
import os

def mute():
    ""
    print("mac mute")
    stream = os.popen('osascript -e "set volume input volume 0"')
    output = stream.read()


def unmute():
    ""
    print("mac unmute")
    stream = os.popen('osascript -e "set volume input volume 100"')
    output = stream.read()