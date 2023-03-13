keys = {
    "Meets": {
        "appName":"Chrome",
        "letter":"d",
        "modifiers":"command down",
    },
    "Teams": {
        "appName":"Microsoft Teams",
        "letter":"m",
        "modifiers":"command down, shift down",
    },
    "Slack": {
        "appName":"Slack",
        "letter":"m",
        "modifiers":"",
    },
    "Discord": {
        "appName":"Discord",
        "letter":"m",
        "modifiers":"command down, shift down",
    }
}



import os
import re
from ctypes import CDLL
import sys
import Quartz
loginPF = CDLL('/System/Library/PrivateFrameworks/login.framework/Versions/Current/login')


def mute():
    print("mac mute")
    stream = os.popen('osascript -e "set volume input volume 0"')
    output = stream.read()

def mute_app(keyName):
    config = keys[keyName]
    osascript = open("osascript.scpt", "r")
    osascript = osascript.read().replace("appName", config['appName'])
    osascript = osascript.replace("letter", config['letter'])
    osascript = osascript.replace("modifiers", config['modifiers'])
    stream = os.popen("osascript -e '{0}'".format(osascript))


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

def is_screen_locked():
    d=Quartz.CGSessionCopyCurrentDictionary()
    return 'CGSSessionScreenIsLocked' in d.keys()

def lock():
    return loginPF.SACLockScreenImmediate()

