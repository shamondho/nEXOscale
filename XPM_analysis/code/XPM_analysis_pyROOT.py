import sys, os
import numpy as np
from matplotlib import pyplot as plt
from matplotlib import dates as dt
import datetime
import ROOT

# Check version of python, works with two or three
version = sys.version_info.major

#hardcoded stuff
path = '/mnt/c/Users/exouser/Desktop/xpm_fitter_data/'    




def rfc868(in_time):
    out_time = []
    offset_time = in_time-126144000-2208988800
    for i,e in enumerate(offset_time):
        out_time.append(datetime.datetime.fromtimestamp(e))
    return out_time

def tm2hrs(in_time): return (in_time-in_time[0])/3600.

def smooth(in_arr, window):
    buf_arr = in_arr
    for i in range(window-1):
        in_arr = np.row_stack((in_arr,np.roll(buf_arr,i-window/2)))
    avg_arr = np.mean(in_arr,axis=0)
    err_arr = np.std(in_arr,axis=0)
    avg_arr[0] = np.mean(buf_arr[:100])
    err_arr[0] = np.std(buf_arr[:100])
    avrg_arr = avg_arr[::window]
    errs_arr = err_arr[::window]
    return avrg_arr,errs_arr

def cut(in_arr,time_arr,t_min,t_max):
    return in_arr[np.argmin(np.absolute(time_arr-t_min)):np.argmin(np.absolute(time_arr-t_max))]

#Function to get the file
def file_sel(version):

    if version == 3:
        file_name = input('Enter filename (Without extension) ')
    else: 
        file_name = raw_input('Enter filename (Without extension) ')
    
    return file_name 
    
#function to set the date time objects
def time_step(version):
    print ('For the time axis we will be using: hours, if that is okay press enter. If you want to use days instead of hours press the letter "d" and then enter.')

    if version == 3:
        selection = input('')
    else: 
        selection = raw_input('')
    if selection == 'd':
        return 'day'
    else:
        return 'hour'

# check if the file exists, if not give user one more chance
file_name = file_sel(version)
file_path = path + file_name + '.txt'
if not os.path.isfile(file_path):
    print('file is not found, choose one of the following files in the XPM fitter data folder')
    print(os.listdir(path))
    file_name = file_sel(version)
    
#load data from file
input_file = np.genfromtxt(path+file_name+'.txt',delimiter=',')

input_file_clean = input_file[input_file[:,5]>0]

tree = ROOT.TTree('xpmdata','')
tree.ReadFile(path+file_name+'.txt','Tc:Ta:TcRise:TaRise:cat:an:offst:datime:IR:UV:chi2:nAvg:nTrig',',')
tree.Draw('Entry$:datime','','goff')
#Set time step for X axis
time_choice = time_step(version)
    
#input_file_clean = input_file_clean[input_file_clean[:,4]/input_file_clean[:,5]>1]
if time_choice == 'day':
    norm_time_clean = rfc868(input_file_clean[:,7])
    norm_time = rfc868(input_file[:,7])
elif time_choice == 'hour':
    norm_time_clean = tm2hrs(input_file_clean[:,7])
    norm_time = tm2hrs(input_file[:,7])
else: sys.exit('Uncorrect time selection')


print('\nPlot Displayer:\n1 - Purity Trend (e- lifetime)\n2 - Cathode/Anode Trend\n3 - Power Monitors trend\n4 - Purity vs Cathode\n5 - Purity vs Power Monitors \n6 - Exit\n')

while(1):
    sel = raw_input('---- ') 
    if sel == str(1):
        avg_samples = int(raw_input('Number of Samples to Average: '))
        nbinsX = int( tree.GetEntries()/avg_samples )
        t0 = tree.GetV2()[0]
        t1 = tree.GetV2()[tree.GetEntries()-1]
        myhist = ROOT.TH2F('myhist','',nbinsX,0.0,(t1-t0)/3600.0,50,0.0,50000.0)
        com = '(Tc-Ta)/log(an/cat):(datime-'+str(t0)+')/3600.0>>myhist'
        maxdiff = float(raw_input('Maximum cathode-anode difference [mV]: '))
        tcut = 'cat>0 && an>0 && an<(cat-'+str(maxdiff)+')'
        print(tree.Draw(com,tcut,'goff'))
        myprof = myhist.ProfileX()
        myprof.SetMarkerStyle(20)
        #myprof.Draw('e')
        x = []
        y = []
        e = []
        for bin in range(1,myprof.GetNbinsX()+1) :
            if( myprof.GetBinEntries(bin)<=1 ): continue
            x.append(myprof.GetBinCenter(bin))
            y.append(myprof.GetBinContent(bin))
            e.append(myprof.GetBinError(bin))
        fexp = ROOT.TF1('fexp','expo + [2]',x[0],x[-1])
        myprof.Fit(fexp,'NO')
        fpts = 500
        yfit = []
        xfit = []
        for pt in range(0,fpts):
            xfit.append( x[0] + pt*(x[-1] - x[0])/fpts )
            yfit.append( fexp.Eval( xfit[pt] ) )
        col_labels=['Fit']
        row_labels=['$\chi^2$/ndf','Constant','Slope','Baseline']
        table_vals=[['{0:.3g}'.format(fexp.GetChisquare())+'/'+'{0:.3g}'.format(fexp.GetNDF())],
                    ['{0:.3g}'.format(fexp.GetParameter(0))+'$\pm$'+'{0:.3g}'.format(fexp.GetParError(0))],
                    ['{0:.3g}'.format(fexp.GetParameter(1))+'$\pm$'+'{0:.3g}'.format(fexp.GetParError(1))],
                    ['{0:.3g}'.format(fexp.GetParameter(2))+'$\pm$'+'{0:.3g}'.format(fexp.GetParError(2))]]
	# the rectangle is where I want to place the table
        the_table = plt.table(cellText=table_vals,
                                rowLabels=row_labels,
                  		colLabels=col_labels,
                                fontsize=8,
                                colWidths=[0.3]*3,
                                loc='upper right')
        plt.title(tcut)
        plt.errorbar(x,y,e,fmt='ro')
        plt.plot(xfit,yfit,'b-')
        plt.xlabel('Hours')
        plt.ylabel('e$^{-}$ lifetime [$\mu$s]')
        #plt.show()
        ########
        subsample_norm_time = norm_time_clean[::avg_samples]
        lf_time = (input_file_clean[:,1]-input_file_clean[:,0])/np.log(input_file_clean[:,4]/input_file_clean[:,5])
        avg,err = smooth(lf_time,avg_samples)
        llsel = raw_input('1 - Average only\n2 - Scatter points only\n3 - Scatter+Average\n')
        if llsel == str(1):
            #plt.errorbar(subsample_norm_time,avg,yerr=err/np.sqrt(avg_samples-1),fmt='ro')
            #plt.title('Lifetime Averaged Over %i Samples'%avg_samples)
						pass
        elif llsel == str(2):
            plt.plot(norm_time_clean,lf_time,'o',markersize=1.5,zorder=-32)
        elif llsel == str(3):
            plt.plot(norm_time_clean,lf_time,'o',markersize=1.5,zorder=-32)           
            #plt.errorbar(subsample_norm_time,avg,yerr=err/np.sqrt(avg_samples-1),fmt='ro')
            #plt.title('Raw Lifetime and Lifetime Averaged Over %i Samples'%avg_samples)
        if time_choice == 'day': plt.xticks(rotation=25)
        if time_choice == 'hour': plt.xlabel('Hours') 
        plt.ylabel('e$^{-}$ lifetime [$\mu$s]')
        
    elif sel == str(2):
        f, axarr = plt.subplots(2, sharex=True)
        axarr[0].plot(norm_time, input_file[:,4],'co')
        axarr[0].set_ylabel('Cathode Amplitude [mV]')
        axarr[1].plot(norm_time, input_file[:,5],'go')
        axarr[1].set_ylabel('Anode Amplitude [mV]')
        if time_choice == 'day': plt.xticks(rotation=25)
        if time_choice == 'hour': plt.xlabel('Hours')

    elif sel == str(3):
        plt.plot(norm_time,input_file[:,8],'b+',markersize=2,label='blue: IR')
        plt.plot(norm_time,input_file[:,9],'m+',markersize=2,label='red: UV')
        plt.legend()
        plt.ylim(0,1000)
        if time_choice == 'day': plt.xticks(rotation=25)
        if time_choice == 'hour': plt.xlabel('Hours')
        plt.ylabel('Power Monitors [au]')

    elif sel == str(4):
        plt.ion()
        lf_time = (input_file_clean[:,1]-input_file_clean[:,0])/np.log(input_file_clean[:,4]/input_file_clean[:,5])
        plt.scatter(input_file_clean[:,5],lf_time)
        plt.xlabel('Cathode Amplitude [mV]')
        plt.ylabel('e$^{-}$ lifetime [$\mu$s]')
        plt.show(block=False)
        ct = raw_input('1 - Input Time Cuts\nAny Keys to Go Back\n')
        if ct == str(1):
            mi,ma = raw_input('Enter min&Max: ').split()
        mI = float(mi)
        Ma = float(ma)
        plt.clf()
        plt.scatter(cut(input_file_clean[:,5],norm_time_clean,mI,Ma),cut(lf_time,norm_time_clean,mI,Ma),color='green')
        plt.xlabel('Cathode Amplitude [mV]')
        plt.ylabel('e$^{-}$ lifetime [$\mu$s]')
        plt.show(block=False)

    elif sel == str(5):
        print('hello')
        plt.ion()
        lf_time = (input_file_clean[:,1]-input_file_clean[:,0])/np.log(input_file_clean[:,4]/input_file_clean[:,5])
        plt.scatter(input_file_clean[:, 8],lf_time, color = 'blue', label='blue: IR')
        plt.scatter(input_file_clean[:, 9],lf_time, color = 'red', label='red: UV')
        plt.xlabel('Power Monitors [au]')
        plt.ylabel('e$^{-}$ lifetime [$\mu$s]')
        plt.legend()
        plt.show(block=False)

    elif sel == str(6): break

    else: print('Press 1-5 or 6 to exit')
    plt.show()
'''    elif sel == str(6):
        lf_time = (input_file[:,1]-input_file[:,0])/np.log(input_file[:,4]/input_file[:,5])
        avg,err = smooth(lf_time,2)
        buff = input_file[:,5]
        buff = buff[::2]
        plt.scatter(buff,err)'''
        

