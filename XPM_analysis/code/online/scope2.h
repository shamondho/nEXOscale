/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1
#define  PANEL_NUMPULSE                   2       /* control type: numeric, callback function: (none) */
#define  PANEL_NUMEVT                     3       /* control type: numeric, callback function: (none) */
#define  PANEL_SCOPEIP                    4       /* control type: string, callback function: enterIP */
#define  PANEL_FILEPATH                   5       /* control type: string, callback function: (none) */
#define  PANEL_BEGINBUTTON                6       /* control type: command, callback function: Begin */
#define  PANEL_PAUSEBUTTON                7       /* control type: command, callback function: Pause */
#define  PANEL_ABORTBUTTON                8       /* control type: command, callback function: Abort */
#define  PANEL_TIMEHIST                   9       /* control type: graph, callback function: (none) */
#define  PANEL_GRAPH                      10      /* control type: graph, callback function: (none) */
#define  PANEL_SHOT_FREQ                  11      /* control type: ring, callback function: (none) */
#define  PANEL_OFFST                      12      /* control type: numeric, callback function: (none) */
#define  PANEL_TARISE                     13      /* control type: numeric, callback function: (none) */
#define  PANEL_TA                         14      /* control type: numeric, callback function: (none) */
#define  PANEL_TCRISE                     15      /* control type: numeric, callback function: (none) */
#define  PANEL_TC                         16      /* control type: numeric, callback function: (none) */
#define  PANEL_CATHODE                    17      /* control type: numeric, callback function: (none) */
#define  PANEL_ANODE                      18      /* control type: numeric, callback function: (none) */
#define  PANEL_LIFE                       19      /* control type: numeric, callback function: (none) */
#define  PANEL_EVENTNUM                   20      /* control type: numeric, callback function: (none) */
#define  PANEL_LED                        21      /* control type: LED, callback function: (none) */
#define  PANEL_QUITBUTTON                 22      /* control type: command, callback function: Quit */
#define  PANEL_DISK_SAV                   23      /* control type: LED, callback function: (none) */
#define  PANEL_CHECKBOX                   24      /* control type: radioButton, callback function: (none) */
#define  PANEL_ENDBUTTON                  25      /* control type: command, callback function: End */
#define  PANEL_LINLOG                     26      /* control type: binary, callback function: (none) */
#define  PANEL_UPPER_X                    27      /* control type: numeric, callback function: (none) */
#define  PANEL_LOWER_X                    28      /* control type: numeric, callback function: (none) */
#define  PANEL_UPPER                      29      /* control type: numeric, callback function: (none) */
#define  PANEL_LOWER                      30      /* control type: numeric, callback function: (none) */
#define  PANEL_STRING_3                   31      /* control type: string, callback function: (none) */
#define  PANEL_STRING_2                   32      /* control type: string, callback function: (none) */
#define  PANEL_XATTRIBUTE                 33      /* control type: ring, callback function: (none) */
#define  PANEL_YATTRIBUTE                 34      /* control type: ring, callback function: (none) */
#define  PANEL_RECENT_HOURS               35      /* control type: numeric, callback function: (none) */
#define  PANEL_WEB                        36      /* control type: radioButton, callback function: (none) */
#define  PANEL_CONNECT                    37      /* control type: LED, callback function: (none) */
#define  PANEL_CHECKBOX_2                 38      /* control type: radioButton, callback function: (none) */
#define  PANEL_LASER1                     39      /* control type: numeric, callback function: (none) */
#define  PANEL_LASER2                     40      /* control type: numeric, callback function: (none) */
#define  PANEL_TEXTMSG_2                  41      /* control type: textMsg, callback function: (none) */
#define  PANEL_TEXTMSG                    42      /* control type: textMsg, callback function: (none) */
#define  PANEL_DECORATION                 43      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_3               44      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_2               45      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_4               46      /* control type: deco, callback function: (none) */
#define  PANEL_STATUSBOX                  47      /* control type: textBox, callback function: (none) */
#define  PANEL_WEEKEND                    48      /* control type: radioButton, callback function: (none) */
#define  PANEL_SHUTTERTIME                49      /* control type: numeric, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK Abort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Begin(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK End(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK enterIP(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Pause(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Quit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
