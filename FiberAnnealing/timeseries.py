import matplotlib as mpl
mpl.rc('figure',facecolor='white')
mpl.rc('lines', markersize = 1.6 )
mpl.rc('lines', markeredgewidth = 0.0 )
#mpl.rc('font', **{'family':'sans-serif','sans-serif':['Arial']})
import matplotlib.pyplot as plt
import time
import matplotlib.dates as dt
import numpy as np
import datetime
import ROOT
import glob
import os


listoffiles = glob.glob('data/*.txt')
stoptimes = []
starttimes = []
timestamps = []
transpower = []
rowcount = 0
for mydatafile in listoffiles :
    with open(mydatafile) as ff:
        for row in ff:
           if row.find('Date_Time')>= 0:
               continue
           timestamps.append( datetime.datetime.fromisoformat(row.split(',')[0]) )
    starttimes.append(timestamps[rowcount])
    stoptimes.append(timestamps[-1])
    rowcount = len(timestamps)
    #print(rowcount)

dat = np.array([np.array(stoptimes),np.array(listoffiles),np.array(starttimes)])
listoffiles = dat[1][dat[0,:].argsort()]
starttimes = dat[2][dat[0,:].argsort()]
stoptimes = dat[0][dat[0,:].argsort()]

timestamps = []

for mydatafile in listoffiles :
    with open(mydatafile) as ff:
        for row in ff:
           if row.find('Date_Time')>= 0:
               continue
           timestamps.append( datetime.datetime.fromisoformat(row.split(',')[0]) )
           transpower.append( float(row.split(',')[3])*1.0e6 )

timestamps = np.array(timestamps)
transpower = np.array(transpower)
myFmt = dt.DateFormatter('20%y-%m-%d %H:%M')
plt.figure()
plt.plot( timestamps , transpower , 'o-' , markersize = 1.5, color='blue', markerfacecolor='red', linewidth=0.25 )
plt.ylabel('Transmitted power [$\mu$W]')
plt.gca().xaxis.set_major_formatter(myFmt)
plt.gcf().autofmt_xdate()
plt.axvspan(starttimes[17], stoptimes[17], color='m', alpha=0.25)
plt.gca().annotate(listoffiles[17],xy = (starttimes[17],35),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#eebfee", alpha=0.99) )
plt.axvspan(starttimes[18], stoptimes[18], color='c', alpha=0.25)
plt.gca().annotate(listoffiles[18],xy = (starttimes[18],37.5),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#bfeeee", alpha=0.99) )
plt.axvspan(starttimes[19], stoptimes[19], color='darkorange', alpha=0.25)
plt.gca().annotate(listoffiles[19],xy = (starttimes[19],26.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#ffe2bf", alpha=0.99) )
plt.axvspan(starttimes[27], stoptimes[27], color='c', alpha=0.25)
plt.gca().annotate(listoffiles[27],xy = (starttimes[27],37.5),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#bfeeee", alpha=0.99) )
plt.axvspan(starttimes[28], stoptimes[28], color='m', alpha=0.25)
plt.gca().annotate(listoffiles[28],xy = (starttimes[28],40.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#eebfee", alpha=0.99) )
plt.axvspan(starttimes[29], stoptimes[29], color='g', alpha=0.25)
plt.gca().annotate(listoffiles[29],xy = (starttimes[29],32.5),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#bfdfbf", alpha=0.99) )
plt.axvspan(starttimes[30], stoptimes[30], color='darkorange', alpha=0.25)
plt.gca().annotate(listoffiles[30],xy = (starttimes[30],26.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#ffe2bf", alpha=0.99) )
plt.axvspan(starttimes[32], stoptimes[32], color='m', alpha=0.25)
plt.gca().annotate(listoffiles[32],xy = (starttimes[32],24.5),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#eebfee", alpha=0.99) )
plt.axvspan(starttimes[33], stoptimes[33], color='g', alpha=0.25)
plt.gca().annotate(listoffiles[33],xy = (starttimes[33],35.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#bfdfbf", alpha=0.99) )
plt.axvspan(starttimes[34], stoptimes[34], color='darkorange', alpha=0.25)
plt.gca().annotate(listoffiles[34],xy = (starttimes[34],38.5),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#ffe2bf", alpha=0.99) )
       
