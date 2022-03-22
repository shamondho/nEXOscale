from tkinter import *
import socket, threading
import time
import datetime
import pyvisa as visa
from ThorlabsPM100 import ThorlabsPM100
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
#from PIL import ImageTK, Image

LARGEFONT = ("Times", 35)

class tkinterApp(Frame):
    # noinspection PyMissingConstructor
    def __init__(self, *args, **kwargs):
        self.window = Tk()
        self.window.title("Optical Fiber Experiment")
        self.window.geometry("1300x650")

        container = Frame()
        container.pack(side="top", fill="both", expand=True)

        container.grid_rowconfigure(0, weight=1)
        container.grid_columnconfigure(0, weight=1)

        self.frames = {}

        self.frames[StartPage] = StartPage(container, self)
        self.frames[Page1] = Page1(container, self)

        self.frames[StartPage].grid(row=0, column=0, sticky="nsew")
        self.frames[Page1].grid(row=0, column=0, sticky="nsew")

        self.show_frame(StartPage)

    def show_frame(self, cont):
        frame = self.frames[cont]
        frame.tkraise()

class StartPage(Frame):
    def __init__(self, parent, controller):
        Frame.__init__(self, parent)
        label = Label(self, text="Main Menu", font=LARGEFONT)
        label.place(x=520,y=0)

        button1 = Button(self, text="Begin",
                             command=lambda: controller.show_frame(Page1))

        button1.place(x=625,y=475)

        button2 = Button(self,text="Exit", command= exit)
        button2.place(x=630,y=500)



    def exit(self):
        self.quit()


class Page1(Frame):

    def __init__(self, parent, controller):
        Frame.__init__(self,parent)
        self.cli_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        HOST = '10.0.0.4'
        PORT = 5023
        self.cli_sock.connect((HOST, PORT))

        print('Connected to remote host (thermocouple server) ' + HOST)
        global data
        data = ''
        global temp1
        temp1 = ''
        global temp2
        temp2 = ''
        global power
        power = ''
        global start_time
        start_time = time.time()
        global save_button_click
        save_button_click = False

        def printValue():
            global wlength
            wlength = wavelength.get()

        def gettxt():
            global txt
            txt = txt_doc.get()

        def serialnumber():
            global serial
            serial = SN.get()

        def save():
            global save_button_click
            save_button_click = True
            exit_button.place(x =37, y = 550)
            save_button.destroy()

        save_button = Button(self,text = "Stop",command = save)
        save_button.place(x=37,y=550)

        button1 = Button(self, text="Main Menu",command=lambda: controller.show_frame(StartPage))
        button1.place(x=0,y=575)

        exit_button = Button(self,text = "Exit",command = exit)

        E1 = Entry(self)
        E1.place(x=550,y=450)
        wavelength = E1
        button2 = Button(self,text="Start",command = self.execute)
        button2.place(x=0,y=550)

        E2 = Entry(self)
        E2.place(x=550, y =400)
        txt_doc = E2
        txt_button= Button(self,text = "Enter",command = gettxt)
        txt_button.place(x=674,y=400)
        label2 = Label(self,text = " Enter Trial Name")
        label2.place(x=550,y=375)

        E3 = Entry(self)
        E3.place(x=550, y = 350)
        SN = E3
        SN_button = Button(self, text = "Enter",command =serialnumber)
        SN_button.place(x=674,y=350)
        label3 = Label(self,text = "Enter Fiber Serial Number")
        label3.place(x=550,y=325)

        label = Label(self,text="Enter Wavelength")
        label.place(x=550,y=425)
        button3 = Button(self,text="Enter",command =printValue)
        button3.place(x=675,y=447)


    def record(self,temp, power, ti,temp_2):
        f = open(txt + ".txt", "a")
        f.write(ti + "," + serial + "," + wlength + "," + str(power) + "," + temp + "," + temp_2 + '\n')
        f.close()

    def receive(self):
        global save_button_click

        # print 'Ready to receive\n'
        f = open(txt +".txt", "a")
        f.write("Date_Time" + "," + "Serial#" + ","+"Wavelength[nm]" + "," +"Power[W]" + "," + "Temperature_1[C]" + "," + "Temperature_2[C]" + '\n')
        f.close()
        global data
        global power
        rm = visa.ResourceManager()
        rm.list_resources()
        inst = rm.open_resource(rm.list_resources()[0])
        power_meter = ThorlabsPM100(inst=inst)
        inst.timeout = None
        while True:
            try:
                c = b'~'
                data = ''
                while c.decode() != '\r':
                    c = self.cli_sock.recv(1)
                    data = data + c.decode()

                data = data[0:-1]
                x = data.split(",")
                temp1 = x[0]
                temp2 = x[1]
                ThorlabsPM100.sense.correction.wavelength = int(wlength)
                power = power_meter.read
                timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

                self.record(temp1, power, timestamp, temp2)
                self.xar.append((time.time() - start_time) / 3600.0)
                self.yar.append(power)
                self.yar2.append(float(temp1))
                self.yar3.append(float(temp2))

                if len(self.xar) > 5000:
                    self.xar.pop(0)
                    self.yar.pop(0)
                    self.yar2.pop(0)
                    self.yar3.pop(0)
                self.fig.clf()
                self.plot1 = self.fig.add_subplot(211)
                self.plot1.plot(self.xar, self.yar, 'ro')
                self.plot1.set_title("Power vs Time")
                self.plot1.set_ylabel('Power(W)')
                self.plot2 = self.fig.add_subplot(212)
                self.plot2.set_title("Temperature vs Time")
                self.plot2.plot(self.xar, self.yar2, 'bo-')
                self.plot2.plot(self.xar, self.yar3, 'go-')
                self.plot2.set_ylabel('Temperature (Celsius)')
                self.fig.text(0.5,0.04,'Time (Hours)',ha ='center',va = 'center')
                #f, axarr = plt.subplots(2, sharex='col')
                #axarr[0].plot(self.xar,self.yar,'ro')
                #axarr[0].plot(self.xar,self.yar2,'bo')
                self.canvas.draw()

                try:
                    glob_temp = float(data)
                except ValueError:
                    glob_temp = -9999.0
                time.sleep(3)

            except KeyboardInterrupt:
                print('Closing connection')
                exit()
            if save_button_click == True:
                pngfile = txt + ".png"
                self.fig.savefig(pngfile,dpi=300,bbox_inches='tight')
                break

    def execute(self):
        print("wavelength = "+ wlength)
        self.xar = []
        self.yar = []
        self.yar2 = []
        self.yar3 = []
        self.fig = Figure(figsize=(10, 6), dpi=100)
        self.plot1 = self.fig.add_subplot(211)
        self.plot2 = self.fig.add_subplot(212)
        self.canvas = FigureCanvasTkAgg(self.fig, master=self)

        self.canvas.draw()
        self.canvas.get_tk_widget().place(x=150, y=0)
        utemp = threading.Thread(target=self.receive)
        utemp.start()
        time.sleep(60)

        if __name__ == "__main__":
            print("Thermocouple Client")

app = tkinterApp()
app.window.mainloop()

