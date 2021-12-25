import platform
import modules.mac as mac
import modules.win as win

class MicControl:
    def __init__(self):
        ""
        self.system = platform.system()

    def is_muted(self):
        if(self.system == "Darwin"):
            return mac.get_volume() == '0'
        
        if(self.system == "Windows"):
            return win.get_mute() == '1'

    def mute(self):
        if(self.system == "Darwin"):
            mac.mute()

        elif(self.system == "Windows"):
            win.mute()
        else:
            print("System not supported")

    def unmute(self):
        if(self.system == "Darwin"):
            mac.unmute()
     
        elif(self.system == "Windows"):
            win.unmute()

        else:
            print("System not supported")
    
    def toggle(self):
        if(self.is_muted()):
            self.unmute()
            return False;
        else:
            self.mute() 
            return True; 



if __name__ == "__main__":
    c = MicControl()
    c.is_muted()