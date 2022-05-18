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

listoffiles = glob.glob('data/*.txt')

timestamps = []
transpower = []
for mydatafile in listoffiles :
    with open(mydatafile) as ff:
        for row in ff:
           if row.find('Date_Time')>= 0:
               continue
           timestamps.append( datetime.datetime.fromisoformat(row.split(',')[0]) )
           transpower.append( float(row.split(',')[3])*1.0e6 )

myFmt = dt.DateFormatter('20%y-%m-%d %H:%M')
plt.figure()
#plt.plot( timestamps , transpower , 'ro' , markersize = 1.5 , zorder=-32 )
plt.plot( timestamps , transpower , 'ro' , markersize = 1.5 )
plt.ylabel('Transmitted power [$\mu$W]')
plt.gca().xaxis.set_major_formatter(myFmt)
plt.gcf().autofmt_xdate()
       
