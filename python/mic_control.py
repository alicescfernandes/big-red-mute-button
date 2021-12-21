import platform
import modules.mac as mac

class MicControl:
    def __init__(self):
        ""
        self.system = platform.system()

    def mute(self):
        ""
        if(self.system == "Darwin"):
            mac.mute()
        else:
            print("System not supported")

    def unmute(self):
        ""
        if(self.system == "Darwin"):
            mac.unmute()
        else:
            print("System not supported")



if __name__ == "__main__":
    c = MicControl()
    c.mute()