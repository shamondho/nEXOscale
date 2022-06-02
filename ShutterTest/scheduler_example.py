#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Apr  6 21:15:11 2020

@author: justinslater
"""


import time
import datetime
import threading as thread
import sched
import random
import socket
import sys
#---------------------------------------------------------Communication Parameters----------------------------------------------------------------
ser = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
HOST = '192.168.1.4' #IP address for Spellman HVPS
#HOST = '127.0.0.1'
if len(sys.argv) >= 3 :
    HOST = str( sys.argv[2] )
PORT = 50001 #port number used to commmunicate to spellman
ser.connect((HOST,PORT))
#--------------------------------------------------------- Preset conditions/variables -------------------------------------------------------------
simple_schedule = sched.scheduler(time.time, time.sleep)
ev_queue = []
run1 = True
nTrips = 0
steps_to_run = 21

kV_max = 70000. # Spellman DXM 701200
mA_max = 17.12
fil_A_max = 5.0
pow_max= 1200.
pre_fil_max = 2.5

status_dict = {}
status_dict[('0','0')] = 'OFF'
status_dict[('0','1')] = 'ON'
status_dict[('1','0')] = 'CLOSED'
status_dict[('1','1')] = 'OPEN'
status_dict[('2','0')] = 'NO FAULT'
status_dict[('2','1')] = 'FAULT'
status_dict[('3','0')] = 'LOCAL'
status_dict[('3','1')] = 'REMOTE'


#---------------------------------------------------------- Spellman commnication functions -------------------------------------------------------
def checksum(OutputString):
    X = 0
    for i in OutputString:
        X += ord(i)
        
    CSb1 = 256 - X
    CSb2 = 127 & CSb1
    CSb3 = 64 | CSb2
    CSb4 = chr( int (hex(CSb3),0 ) ) 
    return CSb4

def send(msg):
    try:
        #ser.send( msg )
        ser.send(msg)
    except Exception as exc:
        print('Interupted!')
        run = False

def receive():
    try :
        byte = chr(2)
        data = ''
        while byte != b'\x03' :
            byte = ser.recv(1)
            data = data + byte.decode()
        #print('**** Received ',data)
        return data
    except Exception as exc:
        print('Interupted!')
        run = False
        return '0,0,0,0,0,0,0,0'
    
def STATUS():
    cmd = '22'
    arg = None
    csum = checksum(cmd  + ',')
    msg = chr(2) + cmd + ',' + csum  + chr(3)   
    #ser.write(msg.encode())
    ser.send(msg)
    try:
        #response = str(ser.readlines()[0]).split(',')[1:5]
        response = receive().split(',')[1:5]
    except IndexError:
        #response = str(ser.readlines()[0]).split(',')[1:5]
        response = receive().split(',')[1:5]
    str_resp = ''
    for i in response:
        str_resp += (i + ' ')
    return str_resp

def HVON():
    cmd = '98'
    arg = '1'
    #csum = checksum(cmd  + ',' + arg + ',')
    msg = chr(2) + cmd + ',' + arg + ','  + chr(3)   
    
    # *** PROGRAM DELAY FOR ALARM AND STROBE **** 
    mA = serial_write('15', None).strip(' (mA)')
    mA_setpoint =  float( mA ) 
    serial_write('11','0')   
    
    # TURN ON HV    
    #ser.write(msg.encode())
    ser.send(msg)    
    #response = str(ser.readlines()[0]).split(',')[1]
    response = receive().split(',')[1]
    
    # Programmed safety delay with no beam demand (no X-rays)
    time.sleep(0)
    
    # RESET mA SETPOINT (FROM 0)
    serial_write('11',str(mA_setpoint))     
    
    if response == '$':
        return
    else:
        print ("Error occurred while sending HVON command (unexpected respones)")
        return "Error"

def HVOFF():
    cmd = '98' #command number to turn the HVPS off
    arg = '0'  #Message or argument number to turn the HVPS off
    #csum = checksum(cmd  + ',' + arg + ',')
    msg = chr(2) + cmd + ',' + arg + ',' + chr(3)   
    #ser.send(msg.encode())
    ser.send( msg )
    #response = str(ser.readlines()[0]).split(',')[1]
    response = receive().split(',')[1]
    #response = ser.readlines()[0].split(',')[1]
    if response == '$':
        print(response)
        return
    else:
        print ("Error occurred while sending HVOFF command (unexpected respones)")
        return "Error"
    
def serial_write(cmd, arg):
    arg = scale_cmd_vals(cmd, arg)
    if arg != None:
        csum = checksum(cmd  + ',' + arg + ',')
        msg = chr(2) + cmd + ',' + arg + ','  + csum  + chr(3) 
        #print('Sending',msg)
    elif arg == None:
        csum = checksum(cmd +',' )
        msg = chr(2) + cmd + ',' + csum  + chr(3)    
    #ser.write(msg.encode())
    ser.send(msg.encode())
    if cmd == '68':
        # This response is different than usual, exception here to pull out the whole status register
        #response = ser.readlines()[0].split(',')[1:6]
        response = receive().split(',')[1:6]
    else:
        # The usual case for most commands
        #response = str(ser.readlines()[0]).split(',')[1]
        response = receive().split(',')[1]
    response = scale_resp_vals(cmd,response)
    if response == '$':
        return 
    else:
        return response
    
def scale_resp_vals(cmd, response):
    if response == '$':
        return response
    elif cmd == '14' or cmd =='60':
        return str (  (float(response)/4095.)*kV_max ) + ' (V)'
    elif cmd == '15' or cmd =='61':
        return str (  (float(response)/4095.)*mA_max ) + ' (mA)'
    elif cmd == '16' or cmd =='62':
        return str (  float(response)  ) + ' RANGE (0-4095)'
    elif cmd == '48':
        return str (  (float(response)/pow_max)*pow_max  ) + ' (W)'
    elif cmd == '68':
        vals = ['Arc','Over temperature','Over voltage', 'Under voltage', 'Over current', 'Under current']
        for i in range(0, len(response)):
            
            if response[i] == '1':
                fault = vals[i]
                return fault 

def scale_cmd_vals(cmd, arg):
    if arg == None:
        return arg
    elif cmd == '10':
        arg = float(arg)
        asc_val = int( ( arg/(kV_max) ) * 4095 )
        return str(asc_val)
    elif cmd == '11':
        arg = float(arg)
        asc_val = int( ( arg/(mA_max) ) * 4095 )
        return str(asc_val)
    elif cmd == '12':
        arg = int(arg)
        asc_val = int( ( arg/(fil_A_max) ) * 4095 )
        return str(arg)
    elif cmd == '13':
        arg = int(arg)
        asc_val = int( ( arg/(pre_fil_max) ) * 4095 )
        return str(asc_val)
    elif cmd == '47':
        arg = float(arg)
        asc_val = int( arg )
        return str(asc_val)
            
#def log_it( f , dat) :
#    f = open( f.name , 'a' ) # should be 'a' or 'w+'
#   f.write( ','.join([str(x) for x in dat]))
#    f.write('\n')
#    f.close()
#--------------------------------------------------------------------- Schedular fucntions-------------------------------------------------------

def next_step(kV_set,mA_set):
    global nTrips
    global steps_to_run 
    global run1
    global f
    maxTrips = 25
    steps_to_run  = len( simple_schedule.queue )
    serial_write('10', kV_set)
    serial_write('11', mA_set)
    print('beam current is '+ str(mA_set)+'mA...')
    print('Voltage is increasing to ' +str(kV_set)+'V...')
    print('Time past is ' +str(int(time.time() - start_time))+ ' seconds')
    time_left = ( simple_schedule.queue[0].time - time.time() ) #variable where it takes the time at the start of the queue, minus, the current time. this gives an elasped time on the current step 
    while time_left > 0.5 : # a while loop to prent the status every second once the current step has run 0.5 seconds
        time.sleep(0.9)
        time_left = ( simple_schedule.queue[0].time - time.time() )#note that the simple_schedule.queue[0] is the time for the next step not the step that its on, so therefore time_left is always positive
        print("Time till next seasoning step: "+str(time_left))#time duration every second to give you an idea how long its left till the next step
        beam_kV = serial_write('60',None)
        beam_mA = serial_write('61',None) 
        fil_A = serial_write('62',None)
        HV_state = status_dict[ ('0',STATUS().split(' ')[0])]
        Interlock = status_dict[ ('1',STATUS().split(' ')[1])]
        print( 'Beam voltage: ' + beam_kV )
        print( 'Beam current: ' + beam_mA )
        print( 'Filament current: ' + fil_A )
        print( 'HV demand: ' + HV_state )
        print( 'Interlock: ' + Interlock )
        print( 'Remaining steps' , steps_to_run )
        pretty_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        dat = [pretty_time, beam_kV,beam_mA, fil_A, HV_state, Interlock, nTrips]
        
        if int(time.time())%60 <= 4 :
                f.write( ','.join([str(x) for x in dat]))
                f.write('\n')
                
        if Interlock == 'OPEN':
                print('Interlock is open, exiting..')
                for ev in simple_schedule.queue : 
                    simple_schedule.cancel(ev)
                return
            
        if nTrips > maxTrips:
                print('Maximum arcs exceeded, exiting..')
                for ev in simple_schedule.queue : 
                    simple_schedule.cancel(ev)
                return
            
        if HV_state == 'OFF':
            print('......!!!Arc Occured!!!......')
            nTrips += 1
            for ev in simple_schedule.queue : #cancel the current schedular at ev = a specific queue int(value)
                simple_schedule.cancel(ev)    #  cancel each step of the schedule for 'ev' remaining steps
            time.sleep(5.0) #wait 5 s
            HVON() #turn the voltage back on
            return steps_to_run

def final_step(kV_set,mA_set,final_step_duration_sec):
    global steps_to_run
    global nTrips
    global run1
    global f
    global final_start_time
    maxTrips = 25
    final_start_time = time.time()
    end_time = ev_queue[-1].time + final_step_duration_sec #variable to tell the viewer the time left over on the final step, refer to line 305
    steps_to_run = len(simple_schedule.queue)
    serial_write('10', kV_set)
    serial_write('11', mA_set)
    print('beam current is '+ str(mA_set)+'mA...')
    print('Voltage is increasing to ' +str(kV_set)+'V...')
    print('Time past is ' +str(int(time.time() - start_time))+ ' seconds')# prints time past when current step finishes
    while ( time.time() - final_start_time ) < final_step_duration_sec : 
        time.sleep(0.9)
        beam_kV = serial_write('60',None)
        beam_mA = serial_write('61',None) 
        fil_A = serial_write('62',None)
        HV_state = status_dict[ ('0',STATUS().split(' ')[0])]
        Interlock = status_dict[ ('1',STATUS().split(' ')[1])]
        print( 'Beam voltage: ' + beam_kV )
        print( 'Beam current: ' + beam_mA )
        print( 'Filament current: ' + fil_A )
        print( 'HV demand: ' + HV_state )
        print( 'Interlock: ' + Interlock )
        print( 'Remaining steps' , steps_to_run )
        pretty_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        dat = [pretty_time, beam_kV,beam_mA, fil_A, HV_state, Interlock, nTrips]
        
        print( 'Time left on final seasoning step: ' + str((int(final_start_time ) + int(final_step_duration_sec)) - int(time.time())) +' sec'))
        if int(time.time())%60 <= 4 :
                f.write( ','.join([str(x) for x in dat])) #the file will add to file f and only that file as long as the script is running
                f.write('\n')
                
        if Interlock == 'OPEN':
                print('Interlock is open, exiting..')
                for ev in simple_schedule.queue : #cancel the current schedular at ev = a specific queue int(value)
                    simple_schedule.cancel(ev)    #  cancel each step of the schedule for 'ev' remaining steps
                return
            
        if nTrips > maxTrips:
                print('Maximum arcs exceeded, exiting..')
                for ev in simple_schedule.queue : #cancel the current schedular at ev = a specific queue int(value)
                    simple_schedule.cancel(ev)    #  cancel each step of the schedule for 'ev' remaining steps
                return
        
        if HV_state == 'OFF':
            print('......!!!Arc Occured!!!......')
            nTrips += 1
            for ev in simple_schedule.queue : #cancel the current schedular at ev = a specific queue int(value)
                simple_schedule.cancel(ev)    #  cancel each step of the schedule for 'ev' remaining steps
            time.sleep(5.0) #wait 5 s
            HVON() #turn the voltage back on
            return steps_to_run

             

def Schedule_this():
    beam_current = 5.0
    global start_time #this carries the variable over to multiple functions, must be made a global before you define it
    start_time = time.time()
    for i in range(0,9):
        simple_schedule.enter(i*900,1, next_step,argument = [30000+i*5000,beam_current])
       
    for i in range(0,4):
        simple_schedule.enter(i*900+(9)*900,1,next_step,argument = [50000,beam_current+i*3.0])
        
    for i in range(0,4):
        simple_schedule.enter(i*900+(9+4)*900,1,next_step,argument = [60000,beam_current+i*3.0])
       
    for i in range(0,3):
        simple_schedule.enter(i*900+(9+4+4)*900,1,next_step,argument = [70000,beam_current+i*3.0])
    for i in range(0,1):    
    #final step(below)
        simple_schedule.enter((9+4+4+3)*900,1,final_step,argument = [70000,14,36000.0])
    print('Scheduled steps remaining is...'+str(len(simple_schedule.queue)))
    return simple_schedule.queue  

#----------------------------------------------------------------Initial conditions to spellman --------------------------------------------------
mA_start = 5.0 #initial beam current setting
kV_start = 30000 #intial Voltage setting
serial_write('10', mA_start) #command to tell spellman to set current 
serial_write('11', kV_start)
serial_write('99','1') #set to remote mode
serial_write('12',2900) #set filament current
HVON()#turning tube on to begin Seasoning process
ev_queue = Schedule_this() #establish the schedule
f=open(str(time.time())+'.log','w') #create/open data file to begin logging data

while steps_to_run > 0 : # condition to run the schedule, satisfied by line 20
    simple_schedule.run() #run the schedule
    if steps_to_run != 0 : #an arc must've happened, how does it know when to go here when it only arcs???????
        idx = len( ev_queue ) - steps_to_run - 2 #length of total schedule minus remaining steps minus 2
        if idx < 0 :
            idx = 0
        print('Repopulating the schedule',idx,steps_to_run)#repolulating the schedule at idx'th step with 'step_to_run' steps remaining
        offst = ev_queue[idx].time
        for j,evj in enumerate( ev_queue[idx:] ) :
            simple_schedule.enter( evj.time - offst, 1, evj.action, evj.argument )
    final_step_duration_sec = 3600 #time set for final step in seconds
    if steps_to_run == 0 and ( time.time() - final_start_time ) < (final_step_duration_sec - 0.2) :
        steps_to_run += 1
        idx = len(ev_queue) - steps_to_run - 1
        
        print('Repopulating the schedule',int(idx),int(steps_to_run))
        #print('got here1')
        offst = ev_queue[int(idx)].time
        for j,evj in enumerate( ev_queue[int(idx):] ) :
            simple_schedule.enter( evj.time - offst, 1, evj.action, evj.argument )
f.close() #closing data file
print('Seasoning Ended')
    
