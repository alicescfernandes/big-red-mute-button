# TODO:
# 1. Get all available serial ports
# 2. Select one and confirm
# 3. Connect to the serial
# 4. Write code to turn on/off led 
# 

import tkinter as tk
r = tk.Tk()
r.title('Counting Seconds')
button = tk.Button(r, text='Quit', width=25, command=r.destroy)
button.pack()


w = tk.Label(r, text='GeeksForGeeks.org!')
w.pack()

scrollbar = tk.Scrollbar(r)
scrollbar.pack( side = tk.RIGHT, fill = tk.Y )
mylist = tk.Listbox(r, yscrollcommand = scrollbar.set )
for line in range(100):
    mylist.insert(tk.END, 'This is line number' + str(line))
mylist.pack( side = tk.LEFT, fill = tk.BOTH )
scrollbar.config( command = mylist.yview )

r.mainloop()
