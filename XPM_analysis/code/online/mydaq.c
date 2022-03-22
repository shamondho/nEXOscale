#include "scope2.h"
#include <NIDAQmx.h>
#include <stdio.h>
#include <time.h>
#define _MSC_VER 1900
#include <curl.h>
#include <analysis.h>
#include <string.h>
#include <formatio.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>   
#include "nrutil.h"  
#include <stdlib.h>
#include <tcpsupp.h>
#include <string.h>
#include <utility.h>
#include <userint.h>
#define T1 10.0
#define T2 81.9
#define TR 1.0
#define TF 2.9
//#include <NIDAQmx.h>


static  CURL *curl;

int CVICALLBACK ClientCallback(unsigned handle, int event, int error, void
							   *callbackData);
void do_purity_fit( double *t, double *v, int n, double *fitpar, double *bestfit
				  );
int  g,linearlog, xaxis, yaxis, ctrl,  h, status, it,     panelId,
	 stop=0,commerr=0,checkbox_2,eventnum,num_evt=0,eventcount=0,loop=0,end=0,run=0,storeEvents;
unsigned    gHandle;
static double       *y[500],*timeaxis=NULL,*lasermax1=NULL, *lasermax2=NULL,
							 *lifetime=NULL;
double  tzero,ymax,ymin,recent_hours, tThis=0,tLast,wav[500][2],t[500],
									  integral[1][2],myvolt[500],ref[500],laser1[500],laser2[500],lifeparams[10],voltfit[500],
									  peak1,peak2;
time_t time_now;
char scopeIP[32],filespec[256], timeAndDate[32], timeDateLast[32];


double pulseMaxHt(double *pulse)
{
// Find the height of the peak of a pulse.  Return the maximal value.
	double MaxHt = 0;
	double zMax, xMax, x[15], z[15], coef[3];
	int i, eleMax = 0, status;
	double mse;  // Mean standard error; not used here.

	// Find the largest element in the array, indexed by eleMax.
	for (i=0; i<500; i++)
	{
		if (pulse[i] > MaxHt)
		{
			MaxHt = pulse[i];
			eleMax = eleMax + 0;
		}
	}

	//fill x[] with integers.
	//for (i=0; i<15; i++) {
	//        x[i] = i;
	//}

	// Take the 15 points closest to the peak, and fit them to a quadratic.
	//status = PolyFit (x, &pulse[eleMax-7], 15, 2, z, coef, &mse);

	// Estimate peak by picking the largest z value.  Error < 0.1%.
	//zMax = z[0];
	//for (i=0; i<15; i++) {
	//        if (z[i] > zMax) {
	//                zMax = MaxHt;
	//        }

	//printf("z[%i] = %lf \n", i, z[i]);
	//}
	zMax=MaxHt;
	//printf("zMax rough = %lf\n", zMax);


	// Solve for the local maximum, using the condition:
	// if         z = A + Bx + Cx^2
	// then xMax = -B/2C
	// and         zMax = z(xMax)
	//xMax = -1*(coef[1])/2/(coef[2]);
	//zMax = coef[0] + (coef[1])*xMax + (coef[2])*(xMax)*(xMax);


	//        Debugging.
	//        printf("Max element is %i\n", eleMax);
	//        printf("Max height  is %lf\n", MaxHt);
	//        printf("xMax = %lf\n", xMax);
	return zMax;
}




int main( void )
{
	TaskHandle  taskHandle=0;
	DAQmxCreateTask("",&taskHandle);
	DAQmxCreateDOChan(taskHandle,"Dev5/port0/line2","",DAQmx_Val_ChanForAllLines);
	uInt8       sh_closed[8]={1,1,1,1,1,1,1,1};
	uInt8       sh_open[8]={0,0,0,0,0,0,0,0};
	return 0;
}

int CVICALLBACK enterIP (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK Begin (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK Pause (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK Abort (int panel, int control, int event,
					   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK Quit (int panel, int control, int event,
					  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}

int CVICALLBACK End (int panel, int control, int event,
					 void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:

			break;
	}
	return 0;
}
