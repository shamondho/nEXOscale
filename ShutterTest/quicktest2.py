from tkinter import *
import socket
from threading import Thread
import multiprocessing
import time
import datetime
import sys
import os
import random
import struct
import matplotlib.pyplot as plt
import numpy as np
import time
import sched
import csv

from WF_SDK import device, scope, wavegen   # import instruments
# from matplotlib.figure import Figure

from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg)

# connect to the device
device_data = device.open()
class test:
    def open(self, wt):
        # print(wt)
        dc = 0.0
        print('Duty Cycle: ', dc)
        wavegen.generate(device_data, channel=1, function=wavegen.function.square, offset=2.5, frequency=1e02,
                         amplitude=2.5, symmetry=dc, wait=0.0, run_time=wt, repeat=1, data=[])
        time.sleep(wt)

    def close(self, wt):
        # print(wt)
        dc = 10
        print(dc)
        wavegen.generate(device_data, channel=1, function=wavegen.function.square, offset=2.5, frequency=1e02,
                         amplitude=2.5, symmetry=dc, wait=0.0, run_time=wt, repeat=1, data=[])
        time.sleep(wt)

class grafit(Frame):
    def on_closing(self):
        self.run = False
        os._exit(0)

    def set_dc(self, dat=0.5):  # set the duty cycle (PULSED mode only)
        if self.btn_var.get() == 'CW':  # if CW, turn ON/OFF the solenoid
            msg = self.btn_var.get() + self.btn_var2.get() + '\n'
            self.ser.send(msg.encode('iso-8859-1'))
            return
        if self.btn_var2.get() == 'ON':  # turn on the pulsing
            msg = str(dat)
        else:
            msg = self.btn_var2.get()  # turn off the pulsing
        msg = msg + '\n'
        self.ser.send(msg.encode('iso-8859-1'))
    def events(self):
        self.schedule.run()

    def plotit(self):
        df = open(r'C:\Users\skyphysics\Downloads\ShutterTestData\ShutterTest_Data', 'w')  # open csv for writing - add file path
        writer = csv.writer(df)  # create writer object

        self.is_running = (self.btn_var3.get() == 'STOP')
        if self.is_running == True :
            volt_1 = scope.measure(device_data, 1)  # voltage measured from pos side sol (CH1)
            volt_2 = scope.measure(device_data, 2)  # voltage measured from neg side sol (CH2)
            print(volt_1)
            print(volt_2)
            v = volt_2 - volt_1
            print('voltage: ', v)
            volt = v / 6.0  # current through sol, divide by resistance of L (6 ohms)
            print('current: ', volt)

            writer.writerow([v])
            writer.writerow([volt])

            if len(self.xar) == 0:
                self.start_time = time.time()
                self.evth.start()
            self.xar.append((time.time() - self.start_time) / 3600.0)
            self.yar.append(volt)
            if len(self.xar) > 86400:
                self.xar.pop(0)
                self.yar.pop(0)
            #print(self.xar,self.yar)
            self.axes.cla()
            self.axes.plot(self.xar, self.yar, 'ro-')
           #self.axes.title("Current vs Time")
            self.axes.set_ylabel('Current (A)')
            self.axes.set_xlabel('Time (hours)')
            #self.plot_widget.grid(row=0, column=2)
            #self.graph.set_ydata(self.yar)
            #self.graph.set_xdata(self.xar)
            #self.axes.draw_artist(self.graph)

            if len(self.schedule.queue) == 1:
                self.cycles += 1
                #for i in range (0,10):
                self.schedule.enter(334*self.cycles, 1, self.mytest.open, [66.0])  # start time, repeat, function, run time
                self.schedule.enter(334*self.cycles + 66, 1, self.mytest.close, [128.0])
                self.schedule.enter(334*self.cycles + 66 + 128, 1, self.mytest.close, [140.0])
            self.fig.canvas.draw()
        self.window.after(1000, self.plotit)

        #df.close()  # close csv file

        #time.sleep(1.0)


    def __init__(self):
        self.window = Tk()
        self.window.geometry('750x625')
        self.window.title('XPM Shutter Control')
        self.ser = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #self.ser.connect(('10.0.0.4', 5047))  # Arduino server connection
        #self.ser.connect(('127.0.0.1', 5047))  # Arduino server connection

        Label(self.window, text='DC [%]').grid(column=3, row=1) # slider label
        self.xar = []
        self.yar = []
        self.q = multiprocessing.Queue()
        self.is_running = False

        # duty cycle slider
        self.dc_var = DoubleVar(self.window, 50.0)
        self.dc_slider = Scale(self.window, command=self.set_dc, orient=HORIZONTAL, resolution=-1, from_=10.0, to=100.0,
                               variable=self.dc_var)
        self.dc_slider.grid(column=3, row=2)

        # DC/pulse button
        self.btn_var = StringVar(self.window, 'CW')
        LABEL0, LABEL1 = 'CW', 'PULSED'
        self.xr = Checkbutton(self.window, textvariable=self.btn_var, width=12, variable=self.btn_var,
                              offvalue=LABEL0, onvalue=LABEL1, indicator=False, command=self.set_dc)
        self.xr.grid(column=3, row=0)

        # solenoid ON/OFF
        self.btn_var2 = StringVar(self.window, 'OFF')
        LABEL2, LABEL3 = 'ON', 'OFF'
        self.sol = Checkbutton(self.window, textvariable=self.btn_var2, width=12, variable=self.btn_var2,
                               offvalue=LABEL3, onvalue=LABEL2, indicator=False, command=self.set_dc)
        self.sol.grid(column=3, row=3)

        # start/stop graphing
        self.btn_var3 = StringVar(self.window, 'START')
        LABEL4, LABEL5 = 'START', 'STOP'
        self.st = Checkbutton(self.window, textvariable=self.btn_var3, width=12, variable=self.btn_var3,
                               offvalue=LABEL4, onvalue=LABEL5, indicator=False)
        self.st.grid(column=3, row=4)
        self.start_time = time.time()

        Label(self.window, text=' ').grid(row=2)
        self.window.protocol('WM_DELETE_WINDOW', self.on_closing)
        self.fig = plt.figure(1)
        self.axes = self.fig.add_subplot(111)
        self.graph, = self.axes.plot(self.xar, self.yar, 'ro-')
        #self.fig.text(0.5,0.04,'Time (Hours)',ha ='center',va = 'center')
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.window)
        self.plot_widget = self.canvas.get_tk_widget()
        self.plot_widget.grid(row=4, column=2)

        self.canvas.draw()
        self.evth = Thread(target=self.events)
        #self.plotter.start()
        self.schedule = sched.scheduler(time.time, time.sleep)
        self.mytest = test()
        self.schedule.enter(0, 1, self.mytest.open, [5.0])
        self.schedule.enter(30, 1, self.mytest.close, [20.0])
        self.cycles = 0
        self.plotit()
        self.window.mainloop()

appl = grafit()
