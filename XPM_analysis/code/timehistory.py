import matplotlib as mpl
mpl.rc('figure',facecolor='white')
#mpl.rc('font', **{'family':'sans-serif','sans-serif':['Arial']})
import matplotlib.pyplot as plt
import time
import matplotlib.dates as dt
import numpy as np
import datetime
import ROOT

start_times = []
stop_times = []

tree = ROOT.TTree('xpmdata','')
#tree.ReadFile('ultemA.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control6.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control6aint.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control6bint.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.Draw('datime-126144000-2208988800','','goff')
entries = tree.GetEntries()
start_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[0])) 
stop_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries-1])) 
tree.ReadFile('control7.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control7d.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control7e.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control7r.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('control7r2.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.Draw('datime-126144000-2208988800','','goff')
start_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries])) 
entries = tree.GetEntries()
stop_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries-1])) 
tree.ReadFile('torlonE.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('torlonE2.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('torlonE3.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('torlonE4.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.Draw('datime-126144000-2208988800','','goff')
start_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries])) 
entries = tree.GetEntries()
stop_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries-1])) 
tree.ReadFile('ultemA.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.ReadFile('ultemAd.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV',',')
tree.Draw('datime-126144000-2208988800','','goff')
start_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries])) 
entries = tree.GetEntries()
stop_times.append(datetime.datetime.fromtimestamp(tree.GetV1()[entries-1])) 
tree.Draw('(Tc-Ta)/log(an/cat):datime-126144000-2208988800','cat>0 && an>0 && an<(cat-0.2)','goff')

timestamps = tree.GetV1()
timebins = []
timecenters =[]
bucket = []
t = 0.0
num = 0

dotsx = []
dotsy = []

for pt in range(0,tree.GetSelectedRows()) :
    dotsx.append( datetime.datetime.fromtimestamp(tree.GetV2()[pt]) )
    dotsy.append( tree.GetV1()[pt] )



#plt.show()

myFmt = dt.DateFormatter('20%y-%m-%d %H:%M')
#ax.xaxis_date()
#ax.xaxis.set_major_formatter(myFmt)
plt.figure()
plt.plot(dotsx,dotsy,'ro',markersize=1.5,zorder=-32)           
plt.gca().xaxis.set_major_formatter(myFmt)
plt.gcf().autofmt_xdate()
plt.ylabel('e$^{-}$ lifetime [$\mu$s]')
print(start_times[0],stop_times[0])
plt.axvspan(start_times[0], stop_times[0], color='g', alpha=0.25)
plt.gca().annotate('Control #6',xy = (start_times[0],27500.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#bfdfbf", alpha=0.99) )
plt.axvspan(start_times[1], stop_times[1], color='c', alpha=0.25)
plt.gca().annotate('Control #7',xy = (start_times[1],27500.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#bfeeee", alpha=0.99) )
plt.axvspan(start_times[2], stop_times[2], color='m', alpha=0.25)
plt.gca().annotate('Torlon E',xy = (start_times[2],27500.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#eebfee", alpha=0.99) )
plt.axvspan(start_times[3], stop_times[3], color='darkorange', alpha=0.25)
plt.gca().annotate('Ultem A',xy = (start_times[3],27500.0),xycoords='data',va='center',ha='center',color='k',bbox=dict(boxstyle="round", fc="#ffe2bf", alpha=0.99) )
#curlyBrace(plt.gcf(), plt.gca(), [start_times[0],20000.0], [stop_times[0],20000.0], 0.1, bool_auto=True, str_text='Control #6', color='darkorange')
dte = datetime.datetime.strptime('2020-11-13 10:50','20%y-%m-%d %H:%M')
plt.gca().annotate('torlon E in\n 2020-11-13',xy=(dte, 0.0), xycoords='data',xytext=(dte, 15000.0), va='center',ha='center',bbox=dict(boxstyle="round", fc="w"),textcoords='data',arrowprops=dict(arrowstyle="->"))

dte = datetime.datetime.strptime('2021-1-19 03:00','20%y-%m-%d %H:%M')
plt.gca().annotate('power outage \n 2021-1-19',xy=(dte, 0.0), xycoords='data',xytext=(dte, 20000.0), va='center',ha='center',bbox=dict(boxstyle="round", fc="w"),textcoords='data',arrowprops=dict(arrowstyle="->"))


dte = datetime.datetime.strptime('2021-1-21 10:23','20%y-%m-%d %H:%M')
plt.gca().annotate('FSM implemented \n 2021-1-21',xy=(dte, 0.0), xycoords='data',xytext=(dte, 10000.0), va='center',ha='center',bbox=dict(boxstyle="round", fc="w"),textcoords='data',arrowprops=dict(arrowstyle="->"))


dte = datetime.datetime.strptime('2021-3-17 11:28','20%y-%m-%d %H:%M')
plt.gca().annotate('XPM short-circuit \n 2021-3-17',xy=(dte, 0.0), xycoords='data',xytext=(dte, 25000.0), va='center',ha='center',bbox=dict(boxstyle="round", fc="w"),textcoords='data',arrowprops=dict(arrowstyle="->"))


dte = datetime.datetime.strptime('2021-4-29 14:00','20%y-%m-%d %H:%M')
plt.gca().annotate('Beam steering \n 2021-4-29',xy=(dte, 0.0), xycoords='data',xytext=(dte, 10000.0), va='center',ha='center',bbox=dict(boxstyle="round", fc="w"),textcoords='data',arrowprops=dict(arrowstyle="->"))


dte = datetime.datetime.strptime('2021-3-11 11:00','20%y-%m-%d %H:%M')
plt.gca().annotate('ultem A in \n 2021-3-11',xy=(dte, 0.0), xycoords='data',xytext=(dte, 15000.0), va='center',ha='center',bbox=dict(boxstyle="round", fc="w"),textcoords='data',arrowprops=dict(arrowstyle="->"))




plt.show()
