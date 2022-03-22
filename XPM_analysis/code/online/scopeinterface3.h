/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2005. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                           1
#define  PANEL_NUMPULSE                  2
#define  PANEL_NUMEVT                    3
#define  PANEL_SCOPEIP                   4       /* callback function: enterIP */
#define  PANEL_FILEPATH                  5
#define  PANEL_BEGINBUTTON               6       /* callback function: Begin */
#define  PANEL_PAUSEBUTTON               7       /* callback function: Pause */
#define  PANEL_ABORTBUTTON               8       /* callback function: Abort */
#define  PANEL_TIMEHIST                  9
#define  PANEL_GRAPH                     10
#define  PANEL_SHOT_FREQ                 11
#define  PANEL_OFFST                     12
#define  PANEL_TARISE                    13
#define  PANEL_TA                        14
#define  PANEL_TCRISE                    15
#define  PANEL_TC                        16
#define  PANEL_CATHODE                   17
#define  PANEL_ANODE                     18
#define  PANEL_LIFE                      19
#define  PANEL_EVENTNUM                  20
#define  PANEL_LED                       21
#define  PANEL_QUITBUTTON                22      /* callback function: Quit */
#define  PANEL_DISK_SAV                  23
#define  PANEL_CHECKBOX                  24
#define  PANEL_ENDBUTTON                 25      /* callback function: End */
#define  PANEL_LINLOG                    26
#define  PANEL_UPPER_X                   27
#define  PANEL_LOWER_X                   28
#define  PANEL_UPPER                     29
#define  PANEL_LOWER                     30
#define  PANEL_STRING_3                  31
#define  PANEL_STRING_2                  32
#define  PANEL_XATTRIBUTE                33
#define  PANEL_YATTRIBUTE                34
#define  PANEL_RECENT_HOURS              35
#define  PANEL_WEB                       36
#define  PANEL_CONNECT                   37
#define  PANEL_CHECKBOX_2                38
#define  PANEL_LASER1                    39
#define  PANEL_LASER2                    40
#define  PANEL_TEXTMSG_2                 41
#define  PANEL_TEXTMSG                   42
#define  PANEL_DECORATION                43
#define  PANEL_DECORATION_3              44
#define  PANEL_DECORATION_2              45
#define  PANEL_DECORATION_4              46
#define  PANEL_STATUSBOX	             47
#define  PANEL_WEEKEND					 48
#define  PANEL_SHUTTERTIME				 49


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK Abort(int panel, int control, int event, void *callbackData, 
int eventData1, int eventData2);
int  CVICALLBACK Begin(int panel, int control, int event, void *callbackData, 
int eventData1, int eventData2);
int  CVICALLBACK End(int panel, int control, int event, void *callbackData, int 
eventData1, int eventData2);
int  CVICALLBACK enterIP(int panel, int control, int event, void *callbackData, 
int eventData1, int eventData2);
int  CVICALLBACK Pause(int panel, int control, int event, void *callbackData, 
int eventData1, int eventData2);
int  CVICALLBACK Quit(int panel, int control, int event, void *callbackData, int 
eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
