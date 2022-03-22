#include <cvintwrk.h>
#include <NIDAQmx.h>
#include <stdio.h>
#include <time.h>
#define _MSC_VER 600
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

#include "scopeinterface3.h"
#include "ReadSavePng.h"
#define T1 10.0
#define T2 81.9
#define TR 1.0
#define TF 2.9
#define TIME_OFFST -126144000


//#include <userint.h>

static  CURL *curl;

int CVICALLBACK ClientCallback(unsigned handle, int event, int error, void *callbackData);
void do_purity_fit( double *t, double *v, int n, double *fitpar, double *bestfit );
int  g,linearlog, xaxis, yaxis, ctrl,  h, status, it, panelId;
int stop=0,commerr=0,checkbox_2,eventnum,num_evt=0,eventcount=0,loop=0,end=0,run=0,storeEvents;
unsigned gHandle;
static double *y[500],*timeaxis=NULL,*lasermax1=NULL, *lasermax2=NULL, *lifetime=NULL;
double tzero,ymax,ymin,recent_hours, tThis=0,tLast,wav[500][2],t[500];
double integral[1][2],myvolt[500],ref[500],laser1[500],laser2[500],lifeparams[13],voltfit[500], peak1,peak2;
time_t time_now;
char scopeIP[32],filespec[256], timeAndDate[32], timeDateLast[32];
int RES=0;


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
			eleMax = i;
		}
	}

	//fill x[] with integers.
	//for (i=0; i<15; i++) {
	//  x[i] = i;
	//}

	// Take the 15 points closest to the peak, and fit them to a quadratic.
	//status = PolyFit (x, &pulse[eleMax-7], 15, 2, z, coef, &mse);

	// Estimate peak by picking the largest z value.  Error < 0.1%.
	//zMax = z[0];
	//for (i=0; i<15; i++) {
	//  if (z[i] > zMax) {
	//    zMax = MaxHt;
	//  }

	//printf("z[%i] = %lf \n", i, z[i]);
	//}
	zMax=MaxHt;
	//printf("zMax rough = %lf\n", zMax);


	// Solve for the local maximum, using the condition:
	// if   z = A + Bx + Cx^2
	// then xMax = -B/2C
	// and   zMax = z(xMax)
	//xMax = -1*(coef[1])/2/(coef[2]);
	//zMax = coef[0] + (coef[1])*xMax + (coef[2])*(xMax)*(xMax);


	//  Debugging.
	//  printf("Max element is %i\n", eleMax);
	//  printf("Max height  is %lf\n", MaxHt);
	//  printf("xMax = %lf\n", xMax);
	return zMax;
}

void quitProgram()
{
	int i;
	for(i=0; i<500; i++)
		free(y[i]);
	free(lifetime);
	free(timeaxis);
	exit(1);
}


int wait(double twait,char* status)
{
	//wait for "twait" seconds
	while(tThis<=tLast+(twait*1000.0)/1.0)
	{
		strcpy(timeAndDate,DateStr());
		strcat(timeAndDate," ");
		strcat(timeAndDate,TimeStr());
		
		//GetUserEvent(0, &h, &ctrl);

		SetCtrlVal(panelId,PANEL_STRING_2,timeAndDate);

		tThis=Timer()*1000.0;//3600*hour+60*minute+sec;
		GetUserEvent(0, &h, &ctrl);
		if(ctrl==8 || ctrl==25 || ctrl==22) break;

		if(status)
		{
			//DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
			char msg[128], fmt[128];
			sprintf( fmt, "%s", status);
			strcat( fmt, "%i seconds");
			int secs = floor( twait -  (tThis - tLast)/1000.0);
			sprintf( msg, fmt , secs+1  );
			ReplaceTextBoxLine(panelId,PANEL_STATUSBOX,0,msg);
		}
		GetUserEvent(0, &h, &ctrl);
		if(ctrl==8 || ctrl==25 || ctrl==22) break;
		double x_min, x_max;
		GetCtrlVal(panelId,PANEL_LOWER_X,&x_min);
		GetCtrlVal(panelId,PANEL_UPPER_X,&x_max);
		GetCtrlVal(panelId,PANEL_XATTRIBUTE,&xaxis);
		GetCtrlVal(panelId,PANEL_RECENT_HOURS,&recent_hours);
		GetCtrlVal(panelId,PANEL_LINLOG,&linearlog);
		GetCtrlVal(panelId,PANEL_LOWER,&ymin);
		GetCtrlVal(panelId,PANEL_UPPER,&ymax);
		GetCtrlVal(panelId,PANEL_YATTRIBUTE,&yaxis);
		//ScreenShotToWeb();
		int y_scaling = yaxis ? VAL_MANUAL : VAL_AUTOSCALE;
		GetUserEvent(0, &h, &ctrl);
		if(ctrl==8 || ctrl==25 || ctrl==22) break;
		switch(xaxis)
		{
			case 2:  //use limits 
				//x_min=0;
				//double ct = (double) time(NULL);
				//x_max= ct - tzero;
				//x_max /= 3600;
				SetAxisRange(panelId,PANEL_TIMEHIST,VAL_MANUAL,x_min,x_max,y_scaling,ymin,ymax);
				break;
			case 1: //recent hours
				x_max=*(timeaxis+eventcount);
				//x_max /= 3600;
				x_min=x_max-recent_hours;
				if (ymin<0) ymin=0;
				SetAxisRange(panelId,PANEL_TIMEHIST,VAL_MANUAL,x_min,x_max,y_scaling,ymin,ymax);
				break;
			case 0: //autoscale X 
				SetAxisRange(panelId,PANEL_TIMEHIST,VAL_AUTOSCALE,x_min,x_max,y_scaling,ymin,ymax);
				break;
		}
		GetUserEvent(0, &h, &ctrl);
		if(ctrl==8 || ctrl==25 || ctrl==22) break;
		if(linearlog)
		{
			linearlog=VAL_LOG;
			if(ymin<=0.0)  SetAxisRange(panelId,PANEL_TIMEHIST,VAL_NO_CHANGE,0,0,VAL_MANUAL,1.0,ymax);
		}
		else linearlog=VAL_LINEAR;

		SetCtrlAttribute(panelId,PANEL_TIMEHIST,ATTR_YMAP_MODE,linearlog);
		//ProcessSystemEvents();
	}
	if( ctrl == 25 )
	{
		DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
		SetCtrlVal(panelId,PANEL_STATUSBOX,"Ending the data-taking run.");
		//dataDump();
		SetCtrlVal(panelId,PANEL_LED,0);
		eventcount=0;
		SetCtrlVal(panelId,PANEL_DISK_SAV,0);
	}
	return (tThis<=tLast+(twait*1000.0)/1.0);

}

int fitTheWaveform(double *time,double *voltage,double *outputpars)
{
	int i;
	*outputpars = *(time+20);
	*(outputpars+1)=*(voltage+20);
	for(i=2; i<7; i++) *(outputpars+i)=0.5;
	for(i=0; i<500; i++) voltfit[i]=0.0;
	return 0;
}

int CVICALLBACK Quit( int panel, int control, int event, void *callbackData, int eventData1, int eventData2 )
{
	switch (event)
	{
		case EVENT_COMMIT:
			quitProgram();
			break;
	}
	return 0;
}

void dataDump()
{
	int i,j,h,fileHandle;
	long position;
	char f[256],num[256];
	strcpy(f,filespec);
	sprintf(num,"%i",eventnum-1);
	//strcat(f,num);
	//strcat(f,".txt");
	lifeparams[7]-=TIME_OFFST;
	if(checkbox_2)
	{
		lifeparams[8]=peak1;
		lifeparams[9]=peak2;
	}
	else
	{
		lifeparams[9]=0;
		lifeparams[8]=0;
	}
	SetCtrlVal(panelId,PANEL_DISK_SAV,1);

	//h=ArrayToFile(f,wav,VAL_DOUBLE,1000,2,VAL_DATA_MULTIPLEXED,VAL_GROUPS_AS_COLUMNS,VAL_SEP_BY_COMMA,16,VAL_ASCII,VAL_TRUNCATE);
	strcpy(f,filespec);
	//strcat(f,"int");
	strcat(f,".txt");

	h=ArrayToFile(f,lifeparams,VAL_DOUBLE,13,13,VAL_DATA_MULTIPLEXED,VAL_GROUPS_AS_COLUMNS,VAL_SEP_BY_COMMA,16,VAL_ASCII,VAL_APPEND);

	//h=ArrayToFile(f,&timeaxis[eventnum-1],VAL_DOUBLE,1,1,VAL_DATA_MULTIPLEXED,VAL_GROUPS_AS_COLUMNS,VAL_SEP_BY_COMMA,16,VAL_ASCII,VAL_APPEND);

	//h=ArrayToFile(f,&peak1,VAL_DOUBLE,1,1,VAL_DATA_MULTIPLEXED,VAL_GROUPS_AS_COLUMNS,VAL_SEP_BY_COMMA,16,VAL_ASCII,VAL_APPEND);

	//h=ArrayToFile(f,&peak2,VAL_DOUBLE,1,1,VAL_DATA_MULTIPLEXED,VAL_GROUPS_AS_COLUMNS,VAL_SEP_BY_COMMA,16,VAL_ASCII,VAL_APPEND);

	integral[0][0]=0.0;
	integral[0][1]=0.0;
	SetCtrlVal(panelId,PANEL_DISK_SAV, 0);
	return;
}

int CVICALLBACK Pause( int panel, int control, int event, void *callbackData, int eventData1, int eventData2 )
{
	int runstate;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelId, PANEL_LED, &runstate );
			if( !runstate ) break;
			SetCtrlVal(panelId,PANEL_LED,0);
			MessagePopup ("Virtual Scope", "Program Paused. Hit OK to resume.");
			SetCtrlVal(panelId,PANEL_LED,1);
			break;
	}

	return 0;
}

int CVICALLBACK Begin (int panel,int control,int event,void *callbackData,int eventData1,int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlVal(panelId,PANEL_LED,1);
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
			eventcount=0;
			SetCtrlVal(panelId,PANEL_DISK_SAV,0);
			run=0;
			loop=0;
			DisconnectFromTCPServer(gHandle);
			break;
	}
	return 0;
}

int CVICALLBACK ClientCallback(unsigned handle, int event, int error, void
							   *callbackData)
{
	gHandle = handle;
	switch (event)
	{
		case TCP_DATAREADY:
			commerr=0;
			break;
		case TCP_DISCONNECT:
			commerr=1;
			break;
	}
	return 0;
}

struct MemoryStruct
{
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL)
	{
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}


int talkToScope(const char* command, char* dat, unsigned handle, int length, int option)
{
	int telnetHandle;
	char tcpheader[2048];
	int err=1;
	char resp[4096];	   
	for( int ii=0 ; ii< 4096 ; ii++ ) resp[ii] = '\0';
	int tn;
	
	GetCtrlVal(panelId,PANEL_SCOPEIP,scopeIP);
  	strcpy(tcpheader,"GET ");
  	strcat(tcpheader,command);
  	strcat(tcpheader," HTTP/1.0\r\n");
  	strcat(tcpheader,"Accept: image/gif, image/x-xbitmap,");
  	strcat(tcpheader," image/jpeg, image/pjpeg, application/vnd.ms-excel,");
  	strcat(tcpheader," application/msword, application/vnd.ms-powerpoint,");
  	strcat(tcpheader," */*\r\n");
  	strcat(tcpheader,"Accept-Language: en-us\r\n");
  	strcat(tcpheader,"Accept-Encoding: gzip, delfate\r\n");
  	strcat(tcpheader,"User-Agent: Mozilla/4.0\r\n");
  	strcat(tcpheader,"Host: ");
  	strcat(tcpheader,scopeIP);
  	strcat(tcpheader,"\r\n\r\n\0");
	//telnetHandle = InetTelnetOpen("134.79.229.21", 80, 0);
	telnetHandle = InetTelnetOpen("localhost", 5022, 0);
	if( telnetHandle < 0 )
	{
		err = 0;
		return err;
	}
	tn = InetTelnetWrite( telnetHandle, tcpheader, "\r\n", -1, NULL, 25000);
	if( tn < 0 )
	{
		err = 0;
		return err;
	}
	command;
	tn = InetTelnetReadUntil( telnetHandle, resp , 4096 , "\n" , 25000 ); //header
	length = strlen( resp );
	while( length > 2 )
	{
		InetTelnetReadUntil( telnetHandle, resp , 4096 , "\n" , 25000 ); //headers
		length = strlen( resp );
	}
	tn = InetTelnetReadUntil( telnetHandle, resp , 4096 , "\n" , 25000 ); //content

	if( tn < 0 )
	{
		err = 0;
		return err;
	}
	
	strcpy( dat , resp );
	return err;
}


int talkToScope_old(const char* command, char* dat, unsigned handle, int length, int option)
{
	CURLcode res;
	int i;
	int strlen;
	char cmd[5000];
	struct MemoryStruct chunk;
	int err=1;
	chunk.memory = (char*)malloc(64);  /* will be grown as needed by the realloc above */
	chunk.memory[0]='\0';
	chunk.size = 0;    /* no data at this point */
	strcpy(cmd,"http://134.79.229.21");
	strcat(cmd,command);
	//printf("command %s\n",cmd);
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, cmd);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteMemoryCallback );
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			err=0;
		}
		//printf("%d\n",chunk.size);
		strlen=length-1;
		if(chunk.size<strlen) strlen=chunk.size;
		for(i=0; i<strlen; i++) dat[i]=chunk.memory[i];
		dat[i]='\0';
		//printf("%s\n",dat);
		/* always cleanup */
		//  curl_easy_cleanup(curl);
		free(chunk.memory);
	}
	return err;
}

//int ScreenShotToWeb(int Pnl){
//  int i;
//  //DeleteFile("C:\\Program Files\\National Instruments\\Labview
//6.1\\www\\ScopeOutput.png");
//       Png_SetParameters (1, 0, 72, Z_DEFAULT_COMPRESSION, 0);
//  i=Png_SavePanelControlToFile(Pnl, 0, "C:\\Program Files\\National
//Instruments\\Labview 6.1\\www\\ScopeOutput.png");
//  return 0;
//}


int main (int argc, char *argv[])
{
	TaskHandle  taskHandle=0;
	DAQmxCreateTask("",&taskHandle);
	DAQmxCreateDOChan(taskHandle,"Dev5/port0/line2","",DAQmx_Val_ChanForAllLines);
	uInt8       sh_closed[8]= {1,1,1,1,1,1,1,1};
	uInt8       sh_open[8]= {0,0,0,0,0,0,0,0};
	CURLcode res;

	curl = curl_easy_init();
	int weekendcount = 0;
	int xyattr, i, j, fileio,isRunning,numpulse;
	int len=0,resid,timestep,hour=0,minute=0,sec=0,tnow,ishotfreq,readonly, system;
	int hidden, archive,lifepoints;
	char command[128],wfmpre[1024];
	char ch[3][5],ev[16],f[256],freqsel[8],last_time[12],last_time_string[11];
	char dat[5000],waveform[5000],src[5000],ecdg[5000],reclength[5000],responseBuffer[64];
	double wavinfo[16], actualDelay,actualPulseWidth,t0,shotFreq,*lasermax1TEMP=NULL, *lasermax2TEMP=NULL;
	double  *lifetimeTEMP=NULL,*timeaxisTEMP=NULL,timeLast;
	unsigned long taskID;
	long position;
	int fileHandle,wferr,srcerr, web;
	int timehisthandle,scopetracehandle,timelastcount;

	double attrhandle;
	if (InitCVIRTE (0, argv, 0) == 0)    /* Needed if linking in external compiler; harmless otherwise */
		return -1;    /* out of memory */

	DisableBreakOnLibraryErrors();
	panelId = LoadPanel(0, "scope2.uir", PANEL);
	DisplayPanel(panelId);
	SetCtrlAttribute (panelId, PANEL_STATUSBOX, ATTR_TEXT_COLOR, VAL_DK_MAGENTA);
	//SetCtrlVal(panelId,PANEL_STATUSBOX,"Initializing.");

	strcpy(timeAndDate,DateStr());
	strcat(timeAndDate," ");
	strcat(timeAndDate,TimeStr());
	SetCtrlVal(panelId,PANEL_STRING_2,timeAndDate);
	integral[0][0]=0.0;
	integral[0][1]=0.0;
	for(i=0; i<4999; i++)
	{
		waveform[i]=69;
		dat[i]=69;
	}
	dat[4999]='\0';
	waveform[4999]='\0';
	i=1;
	j=1;
	while(j)
	{
		//   GetCtrlVal(panelId,PANEL_LOWER_X,&ymin);
		//       GetCtrlVal(panelId,PANEL_UPPER_X,&ymax);
		strcpy(timeAndDate,DateStr());
		strcat(timeAndDate," ");
		strcat(timeAndDate,TimeStr());
		//SetCtrlVal(panelId,PANEL_STRING_2,timeAndDate);
		if (GetUserEvent(0, &h, &ctrl) == 1)
		{
			switch(ctrl)
			{
				case PANEL_NUMEVT:
					GetCtrlVal(panelId,PANEL_SCOPEIP,scopeIP);
					j=0;
					break;
				case PANEL_QUITBUTTON:
					DiscardPanel(panelId);
					quitProgram();
					break;
				default:
					break;
			}
		}
	}//end of while(j)

	while(!stop)
	{
		strcpy(timeAndDate,DateStr());
		strcat(timeAndDate," ");
		strcat(timeAndDate,TimeStr());
		SetCtrlVal(panelId,PANEL_STRING_2,timeAndDate);
		GetCtrlVal(panelId,PANEL_CHECKBOX_2,&checkbox_2);
		//if (GetUserEvent(0, &h, &ctrl) == 1)
		GetUserEvent(1, &h, &ctrl);
		if(1)
		{
			switch (ctrl)
			{
				case PANEL_ABORTBUTTON:
					SetCtrlVal(panelId,PANEL_LED,0);
					DeleteGraphPlot (panelId,PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot (panelId,PANEL_TIMEHIST, -1,
									 VAL_IMMEDIATE_DRAW);
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
					SetCtrlVal(panelId,PANEL_STATUSBOX,"**ABORTED THE RUN**");

					break;
				case PANEL_ENDBUTTON:
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);

					SetCtrlVal(panelId,PANEL_STATUSBOX,"Ending the data-taking run.");

					SetCtrlVal(panelId,PANEL_LED,0);
					//DeleteGraphPlot(panelId,PANEL_GRAPH,-1,VAL_IMMEDIATE_DRAW);
					//dataDump();
					eventcount=0;
					SetCtrlVal(panelId,PANEL_DISK_SAV,0);
					break;
				case PANEL_BEGINBUTTON:
					weekendcount = 0;
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);

					SetCtrlVal(panelId,PANEL_STATUSBOX,"Starting a data-taking run.");

					integral[0][0]=0.0;
					integral[0][1]=0.0;
					//GetSystemTime(&hour,&minute,&sec);
					t0=Timer()*1000.0;//3600*hour+60*minute+sec;
					end=0;
					strcpy(timeDateLast,DateStr());
					strcat(timeDateLast," ");
					strcat(timeDateLast,TimeStr());
					SetCtrlVal(panelId,PANEL_STRING_2,timeDateLast);
					DeleteGraphPlot (panelId,PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
					DeleteGraphPlot (panelId,PANEL_TIMEHIST, -1, VAL_IMMEDIATE_DRAW);
					SetCtrlVal(panelId,PANEL_LED,1);
					SetCtrlVal(panelId,PANEL_EVENTNUM,0);
					GetCtrlVal(panelId,PANEL_NUMEVT,&timestep);
					num_evt=1;
					GetCtrlVal(panelId,PANEL_FILEPATH,filespec);
					strcpy(f,filespec);
					//strcat(f,"int.txt");
					strcat(f,".txt");
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+SAMPLE");
					talkToScope(command,dat,gHandle,3848,1);
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+AVERAGE");
					talkToScope(command,dat,gHandle,3848,1);
					if(!GetFileAttrs(f,&readonly,&hidden,&system,&archive))
					{
						fileio=GenericMessagePopup("File I/O","File exists!","Overwrite","Append","Cancel",0,0,0,VAL_GENERIC_POPUP_BTN3, VAL_GENERIC_POPUP_BTN3,VAL_GENERIC_POPUP_BTN3);
						switch(fileio)
						{
							case 1:
								DeleteFile(f);
								timeLast=0.0;
								break;
							case 2:
								//fileHandle=OpenFile(f,VAL_READ_ONLY,VAL_OPEN_AS_IS,VAL_ASCII);
								//position = SetFilePtr (fileHandle,-11,2);
								//ReadFile(fileHandle,last_time,11);
								//last_time[11]='\0';
								//CloseFile(fileHandle);

								//for(timelastcount=10; timelastcount>0; timelastcount--)
								//{
								//	if(last_time[timelastcount]==',')
								//		break;

								//	last_time_string[10-timelastcount]=last_time[timelastcount];
								//}
								//last_time_string[10-timelastcount]='\0';
								//strcpy(last_time,last_time_string);

								//timeLast=strtod(last_time,(char **)NULL);
								//timeLast+=0.5e-3;
								RES = 1;
								fileHandle=OpenFile(f,VAL_READ_ONLY,VAL_OPEN_AS_IS,VAL_ASCII);
								int fbytes = 1;
								char linebuffer[1024];
								int numlns = 0;
								while( fbytes != -2 )
								{
									fbytes = ReadLine( fileHandle, linebuffer, 512);
									if( fbytes == 0 ) continue;
									if( fbytes < 0 ) break;
									numlns++;
								}
								int fstart = SetFilePtr( fileHandle,0,0);
								eventcount = numlns;
								lifetime=(double *)calloc(numlns,sizeof(double));
								timeaxis=(double *)calloc(numlns,sizeof(double));
								lasermax1=(double *)calloc(numlns,sizeof(double));
								lasermax2=(double *)calloc(numlns,sizeof(double));
								double Ta, Tc, Ano, Cat, lastime, ir_pk, uv_pk;
								numlns = 0;
								while( 1 )
								{
									fbytes = ReadLine( fileHandle, linebuffer, 512);
									if( fbytes == 0 ) continue;
									if( fbytes < 0 ) break;
									char* pch = strtok ( linebuffer, ",");
									Tc = strtod(pch,(char**) NULL);
  									for(int colnum=1; colnum<13; colnum++)
  									{
    									pch = strtok (NULL, ",");
										switch(colnum)
										{
											case 1:
												Ta = strtod(pch,(char**) NULL);
												break;
											case 4:
												Ano = strtod(pch,(char**) NULL);
												break;
											case 5:
												Cat = strtod(pch,(char**) NULL);
												break;
											case 7:
												lastime = strtod(pch,(char**) NULL);
												lastime += TIME_OFFST; 
												break;
											case 8:
												ir_pk = strtod(pch,(char**) NULL);
												break;
											case 9:
												uv_pk = strtod(pch,(char**) NULL);
												break;
											default:
												break;
										}
  									}
									tzero = (!numlns) ? lastime : tzero;
									*(timeaxis+numlns) = (lastime-tzero)/3600.0;
									*(lifetime+numlns) = (Ta-Tc)/log(Ano/Cat);
									*(lasermax1+numlns) = ir_pk;
									*(lasermax2+numlns) = uv_pk;
									numlns++;
								}
								timeLast = lastime;
								timeLast+=0.5e-3;
								tzero += 0.0;
								CloseFile(fileHandle);
								break;
							case 3:
								//DiscardPanel(panelId);
								//quitProgram();
								eventcount=0;
								SetCtrlVal(panelId,PANEL_DISK_SAV,0);
								run=0;
								loop=0;
								timeLast=0.0;
								continue;
								break;
							default:
								break;
						}
					}
					GetCtrlVal(panelId,PANEL_SCOPEIP,scopeIP);

					GetCtrlVal(panelId,PANEL_SHOT_FREQ,freqsel);
					shotFreq=strtod(freqsel,(char **)NULL);
					ishotfreq=shotFreq;
					//    ContinuousPulseGenConfig (5,"1",shotFreq,1.5e-5*shotFreq,		 //device number changed from 0 to 5
					//          UNGATED_SOFTWARE_START,POSITIVE_POLARITY,
					//          &actualDelay, &actualPulseWidth, &taskID);    		   commented on 10/31/2016
					//printf("dataencoding");
					//data encoding is set
					strcpy(command,"/?COMMAND=data:encdg+ascii");
					talkToScope(command,ecdg,gHandle,5000,0);

					strcpy(command,"/?COMMAND=horizontal:recordlength+500");
					talkToScope(command,reclength,gHandle,5000,0);
					if(loop+5)
					{
						for(i=0; i<500; i++)
						{
							switch(num_evt<=100)
							{
								case 1:
									y[i]=(double *)
										 malloc(2*num_evt*sizeof(double));
									break;
								case 0:
									y[i]=(double *)
										 malloc(2*100*sizeof(double));
									break;
								default:
									break;
							}
						}
					}
					eventnum = (eventcount) ? (eventcount-1) : 0;
					run=1;
					GetCtrlVal(panelId,PANEL_CHECKBOX,&storeEvents);
					break;
				case PANEL_QUITBUTTON:
					stop=1;
					DiscardPanel(panelId);
					break;
				default:
					break;
			}

		} //end of while()
		int evthis=0;
		int evlast=0;
		while(run)
		{
			if (loop==0)
			{
				strcpy(timeAndDate,DateStr());
				strcat(timeAndDate," ");
				strcat(timeAndDate,TimeStr());
				SetCtrlVal(panelId,PANEL_STRING_3,timeAndDate);
			}
			if( evthis > evlast ) dataDump();
			evlast = evthis;
			loop++;
			//      GetCtrlVal(panelId,PANEL_WEB,&web);
			//      if (web)
			//          ScreenShotToWeb(panelId);
			strcpy(timeAndDate,DateStr());
			strcat(timeAndDate," ");
			strcat(timeAndDate,TimeStr());
			SetCtrlVal(panelId,PANEL_STRING_2,timeAndDate);
			GetCtrlVal(panelId,PANEL_LED,&isRunning);
			GetCtrlVal(panelId,PANEL_NUMPULSE,&numpulse);
			
			//GetSystemTime(&hour,&minute,&sec);
			tnow=Timer()*1000;//3600*hour+60*minute+sec;
			GetCtrlVal(panelId,PANEL_NUMEVT,&timestep);
			char msg[64];
			strcpy( msg , "Next acquisition begins in ");
			if( loop > 1 )
			{
				tLast = Timer()*1000.0;
				if( wait( timestep , msg ) ) break;
			}
			for( int jjj=0; ( jjj < 1 ) && ( loop < 2 ); jjj++ )
			{
				if(!jjj) DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,sh_open,NULL,NULL);
				tLast=Timer()*1000.0;
				if( wait( 16.0,"--INITIALIZING-- data cycle 0 will start in "  ) ) break;
			}
			if(isRunning) // && ((tnow%(timestep*1000))<=1000))
			{
				tLast=Timer()*1000.0;
				//    CounterStart(taskID);
				//    				 commented on 10/31/2016
				//
				//    CounterStop(taskID);
				strcpy(ch[0],"CH1");
				strcpy(ch[1],"CH2");
				strcpy(ch[2],"CH3");
				SetCtrlVal(panelId,PANEL_EVENTNUM,eventnum);
				//////start of acquisition cycle //////////////////
				//first fetch sig+ background +++++++++++++++>>>>>
				wferr = -1; srcerr = -1;
				while( wferr<0 || srcerr <0 )
				{
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
					SetCtrlVal(panelId,PANEL_STATUSBOX,"Fetching signal + background waveform");					
					//get the waveform preamble
					strcpy(command,"/?COMMAND=wfmpre?");
					srcerr=talkToScope(command,wfmpre,gHandle,5000,0);
					strcpy(src,wfmpre);
					strcpy(command,"/?COMMAND=curve?");
					wferr=talkToScope(command,dat,gHandle,3848,0);
					DisconnectFromTCPServer(gHandle);
					strcpy(waveform,dat);
					len=0;
					for(i=0; i<15; i++)
					{
						if((src!=NULL)&&(len<=strlen(wfmpre)))
						{
							strcpy(src,wfmpre+len);
							wavinfo[i]=strtod(strtok(src,";"),(char **)NULL);
							len=len+strlen(src);
						}
					}
					if(wferr>=0 && srcerr>=0)
					{
						len=0;
						for(i=0; (i<500)&&(run); i++)
						{
							t[i]=1.0e6*(wavinfo[8]*(i)+wavinfo[10]);
							if((waveform!=NULL)&&(len<=strlen(dat)))
							{
								strcpy(waveform,dat+len);
								myvolt[i]=atoi(strtok(waveform,","));
								len=len+strlen(waveform)+1;
							}
						}
					}
				}
				//-----------------------------------------------------------------------
				//KDW 2020-12-21: an attempt at storing the number of waveforms in the current acquisition
				char numavg[16];
				int numavgerr=0;
				numavgerr=talkToScope("/?COMMAND=ACQuire:NUMAVG?",numavg,gHandle,5000,0);//just store the error
				double num_in_average = strtod(numavg,(char **)NULL);
				lifeparams[11] = num_in_average; 			               // code in numavgerr
				numavgerr=talkToScope("/?COMMAND=ACQuire:NUMACQ?",numavg,gHandle,5000,0);//this is the number actually in the average
				num_in_average = strtod(numavg,(char **)NULL);//less than or equal to the number you dialed in on the scope
				lifeparams[12] = num_in_average; 			               
				//-----------------------------------------------------------------------
				//++++++++++++++++++++++++++++++		
				//now fetch background
				DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
				SetCtrlVal(panelId,PANEL_STATUSBOX,"Capturing background: **CLOSING SHUTTER** ");
				g=DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,sh_closed,NULL,NULL);
				wferr = -1; srcerr = -1;
				while( wferr<0 || srcerr <0 )
				{
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+SAMPLE"); //KDW 2021-1-17 doing this clears the averaging
					talkToScope(command,dat,gHandle,3848,1);
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+AVERAGE");//and this starts it over from scratch
					talkToScope(command,dat,gHandle,3848,2);
					tLast=Timer()*1000.0;
					strcpy( msg , "Capturing background: **CLOSING SHUTTER** ");
					if( wait(32,msg) ) break; //KDW 2021-2-1 moved this here from line 824  TODO***
					strcpy(command,"/?COMMAND=data:source+");
					strcat(command,ch[0]);
					talkToScope(command,src,gHandle,5000,0);
					//get the waveform preamble
					strcpy(command,"/?COMMAND=wfmpre?");
					srcerr=talkToScope(command,wfmpre,gHandle,5000,0);
					strcpy(src,wfmpre);
					strcpy(command,"/?COMMAND=curve?");
					wferr=talkToScope(command,dat,gHandle,3848,0);
					DisconnectFromTCPServer(gHandle);
					strcpy(waveform,dat);
					len=0;
					for(i=0; i<15; i++)
					{
						if((src!=NULL)&&(len<=strlen(wfmpre)))
						{
							strcpy(src,wfmpre+len);
							wavinfo[i]=strtod(strtok(src,";"),(char **)NULL);
							len=len+strlen(src);         //removed +1+6 - 12/04/2015 (see logbook EXO12 p117)
						}
					}
					if(wferr>=0 && srcerr>=0)
					{
						len=0;
						for(i=0; (i<500)&&(run); i++)
						{
							t[i]=1.0e6*(wavinfo[8]*(i)+wavinfo[10]);
							if((waveform!=NULL)&&(len<=strlen(dat)))
							{
								strcpy(waveform,dat+len);
								ref[i]=atoi(strtok(waveform,","));
								myvolt[i]=myvolt[i]-ref[i];
								myvolt[i]=1.0e3*wavinfo[12]*(myvolt[i]-wavinfo[14])+wavinfo[13];
								len=len+strlen(waveform)+1;
							}
						}
					}
				}
				//now plot the background-subtracted waveform + best fit
				if(run)
				{
					DeleteGraphPlot (panelId,PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);	
					PlotXY(panelId,PANEL_GRAPH,t,myvolt,500,VAL_DOUBLE,VAL_DOUBLE,VAL_THIN_LINE,VAL_NO_POINT,0,0,VAL_BLACK);
					do_purity_fit(&t[0],&myvolt[0],500,&lifeparams[0],&voltfit[0]);
					scopetracehandle=PlotXY(panelId,PANEL_GRAPH,t,voltfit,500,VAL_DOUBLE,VAL_DOUBLE,VAL_THIN_LINE,VAL_NO_POINT,0,0,VAL_RED);
					xyattr=SetCtrlAttribute(panelId,PANEL_GRAPH,ATTR_XAXIS_GAIN,10.0);
				}
				time_now=time(NULL);
				if(!eventnum && !RES) tzero=time_now;
	
				eventcount=eventnum;
				
				lifeparams[7]=time_now;
				timeaxisTEMP=timeaxis;
				lifetimeTEMP=lifetime;
				lifetime=(double *)calloc(eventcount+1,sizeof(double));
				timeaxis=(double *)calloc(eventcount+1,sizeof(double));
				for(lifepoints=0; lifepoints<eventcount; lifepoints++)
				{
					*(lifetime+lifepoints)=*(lifetimeTEMP+lifepoints);
					*(timeaxis+lifepoints)=*(timeaxisTEMP+lifepoints);
				}
				free(lifetimeTEMP);
				free(timeaxisTEMP);
				*(lifetime+eventcount)=(lifeparams[0]-lifeparams[1])/log(lifeparams[5]/lifeparams[4]);
				*(timeaxis+eventcount)=(lifeparams[7]-tzero)/3600.0;
				SetCtrlVal(panelId,PANEL_LIFE,*(lifetime+eventcount));
				SetCtrlVal(panelId,PANEL_ANODE,lifeparams[4]);
				SetCtrlVal(panelId,PANEL_CATHODE,lifeparams[5]);
				SetCtrlVal(panelId,PANEL_TC,lifeparams[0]);
				SetCtrlVal(panelId,PANEL_TCRISE,lifeparams[2]);
				SetCtrlVal(panelId,PANEL_TA,lifeparams[1]);
				SetCtrlVal(panelId,PANEL_TARISE,lifeparams[3]);
				SetCtrlVal(panelId,PANEL_OFFST,lifeparams[6]);
				//++++++++++++++++++++++++++++++++++
				//and now, if checkbox_2 is checked, fetch the laser intensity data
				if( checkbox_2 )
				{
					//set up the scope to capture the laser intensity data
					talkToScope("/?COMMAND=horizontal:trigger:position+30",src,gHandle,5000,0);
					talkToScope("/?COMMAND=horizontal:main:scale+40e-6",src,gHandle,5000,0);
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+SAMPLE"); //KDW 2021-1-17 doing this clears the averaging
					talkToScope(command,dat,gHandle,3848,1);
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+AVERAGE");//and this starts it over from scratch
					talkToScope(command,dat,gHandle,3848,2);
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);

					SetCtrlVal(panelId,PANEL_STATUSBOX,"Fetching laser intensity data");
					talkToScope("/?COMMAND=horizontal:trigger:position+30",src,gHandle,5000,0);
					talkToScope("/?COMMAND=horizontal:main:scale+40e-9",src,gHandle,5000,0);
					tLast=Timer()*1000.0;
					ProcessSystemEvents();
					strcpy(msg, "Fetching laser intensity data ");
					if( wait(32,msg) ) break; //TODO****
				
					wferr = -1; srcerr = -1;
					while( wferr<0 || srcerr <0 )
					{
						strcpy(command,"/?COMMAND=data:source+");
						strcat(command,ch[1]);
						talkToScope(command,src,gHandle,5000,0);
						//get the waveform preamble
						strcpy(command,"/?COMMAND=wfmpre?");
						srcerr=talkToScope(command,wfmpre,gHandle,5000,0);
						strcpy(src,wfmpre);
						strcpy(command,"/?COMMAND=curve?");
						wferr=talkToScope(command,dat,gHandle,3848,0);
						DisconnectFromTCPServer(gHandle);
						strcpy(waveform,dat);
						len=0;
						for(i=0; i<15; i++)
						{
							strcpy(src,wfmpre+len);
							wavinfo[i]=strtod(strtok(src,";"),(char **)NULL);
							len=len+strlen(src);         //removed +1+6 - 12/04/2015 (see logbook EXO12 p117)
						}
						if(wferr>=0 && srcerr>=0)
						{
							len=0;
							for(i=0; (i<500)&&(run); i++)
							{
								if((waveform!=NULL)&&(len<=strlen(dat)))
								{
									strcpy(waveform,dat+len);
									laser1[i]=atoi(strtok(waveform,","));
									laser1[i]=1.0e1*wavinfo[12]*(laser1[i]-wavinfo[14])+wavinfo[13];
								}
								len=len+strlen(waveform)+1;
							}
						}
						peak1 = 100.0*pulseMaxHt(&laser1[0]);
					}
					wferr = -1; srcerr = -1;
					while( wferr<0 || srcerr <0 )
					{
						strcpy(command,"/?COMMAND=data:source+");
						strcat(command,ch[2]);
						talkToScope(command,src,gHandle,5000,0);
						//get the waveform preamble
						strcpy(command,"/?COMMAND=wfmpre?");
						srcerr=talkToScope(command,wfmpre,gHandle,5000,0);
						strcpy(src,wfmpre);
						strcpy(command,"/?COMMAND=curve?");
						wferr=talkToScope(command,dat,gHandle,3848,0);
						DisconnectFromTCPServer(gHandle);
						strcpy(waveform,dat);
						len=0;
						for(i=0; i<15; i++)
						{
							strcpy(src,wfmpre+len);
							wavinfo[i]=strtod(strtok(src,";"),(char **)NULL);
							len=len+strlen(src);//removed +1+6 - 12/04/2015 (see logbook EXO12 p117)
						}
						if(wferr>=0 && srcerr>=0)
						{
							len=0;
							for(i=0; (i<500)&&(run); i++)
							{
								if((waveform!=NULL)&&(len<=strlen(dat)))
								{
									strcpy(waveform,dat+len);
									laser2[i]=atoi(strtok(waveform,","));
									laser2[i]=1.0e1*wavinfo[12]*(laser2[i]-wavinfo[14])+wavinfo[13];
								}
								len=len+strlen(waveform)+1;
							}
						}
						peak2 = 100.0*pulseMaxHt(&laser2[0]);
					}
					lasermax1TEMP=lasermax1;
					lasermax2TEMP=lasermax2;
					lasermax1=(double *)calloc(eventcount+1,sizeof(double));
					lasermax2=(double *)calloc(eventcount+1,sizeof(double));
					for(lifepoints=0; lifepoints<eventcount; lifepoints++)
					{
						*(lasermax1+lifepoints)=*(lasermax1TEMP+lifepoints);
						*(lasermax2+lifepoints)=*(lasermax2TEMP+lifepoints);
					}
					free(lasermax1TEMP);
					free(lasermax2TEMP);
					*(lasermax1+eventcount)=peak1;
					*(lasermax2+eventcount)=peak2;
				}
				
				DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
				SetCtrlVal(panelId,PANEL_STATUSBOX,"Preparing next data cycle: **OPENING SHUTTER**");
				g=DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,sh_open,NULL,NULL);
				///////////Done fetching waveforms//////////////
				//plot the laser waveforms (if available)
				if(checkbox_2 && run)
				{
					PlotXY(panelId,PANEL_GRAPH,t,laser1,500,VAL_DOUBLE,VAL_DOUBLE,VAL_THIN_LINE,VAL_NO_POINT,0,0,VAL_BLUE);
					PlotXY(panelId,PANEL_GRAPH,t,laser2,500,VAL_DOUBLE,VAL_DOUBLE,VAL_THIN_LINE,VAL_NO_POINT,0,0,VAL_MAGENTA);
					SetCtrlVal(panelId,PANEL_LASER1,*(lasermax1+eventcount));
					SetCtrlVal(panelId,PANEL_LASER2,*(lasermax2+eventcount));
					PlotXY(panelId,PANEL_TIMEHIST,timeaxis,lasermax1,eventcount+1,VAL_DOUBLE,VAL_DOUBLE,VAL_SCATTER,VAL_DOTTED_EMPTY_CIRCLE,0,1,VAL_BLUE);
					PlotXY(panelId,PANEL_TIMEHIST,timeaxis,lasermax2,eventcount+1,VAL_DOUBLE,VAL_DOUBLE,VAL_SCATTER,VAL_DOTTED_EMPTY_CIRCLE,0,1,VAL_MAGENTA);
				}
				if(run) 
				{
					PlotXY(panelId,PANEL_TIMEHIST,timeaxis,lifetime,eventcount+1, VAL_DOUBLE,VAL_DOUBLE,VAL_SCATTER,VAL_DOTTED_EMPTY_CIRCLE,0,1,VAL_RED);					
					xyattr=SetCtrlAttribute(panelId,PANEL_TIMEHIST,ATTR_XAXIS_GAIN,10.0);
				}
				//////end of acquisition cycle
				//++++++++++++++++++++++++++++++++++++>>>>>>>
	
				//dataDump();
				if(!run)
					SetCtrlVal(panelId,PANEL_LED,0);
				strcpy(command,"/?COMMAND=ACQUIRE:MODE+SAMPLE"); //KDW 2021-1-17 doing this clears the averaging
				talkToScope(command,dat,gHandle,3848,1);
				strcpy(command,"/?COMMAND=ACQUIRE:MODE+AVERAGE");//and this starts it over from scratch
				talkToScope(command,dat,gHandle,3848,2);
				talkToScope("/?COMMAND=horizontal:trigger:position+30",src,gHandle,5000,0);
				talkToScope("/?COMMAND=horizontal:main:scale+40e-6",src,gHandle,5000,0);
				strcpy(command,"/?COMMAND=data:source+");
				strcat(command,ch[0]);
				int test_v = talkToScope(command,src,gHandle,5000,0);
				int weekend=0;
				GetCtrlVal(panelId,PANEL_WEEKEND,&weekend);
				if( weekend ) weekendcount++;
				if( weekend && weekendcount%10 == 0 && weekendcount>0 )//check for fiber-save mode
				{
					double long_wait = 28800.0;
					GetCtrlVal(panelId,PANEL_SHUTTERTIME,&long_wait);
					char msg[256];
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
					SetCtrlVal(panelId,PANEL_STATUSBOX,"Fiber-save mode is active: **CLOSING SHUTTER**");
					DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,sh_closed,NULL,NULL);
					//DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
					//sprintf(msg,"Fiber-saving mode active; shutter closed. Waiting %f seconds.", long_wait);
					sprintf(msg,"Fiber-saving. --SHUTTER CLOSED-- will reopen in " );
					tLast=Timer()*1000.0;
					if( wait( long_wait , msg) ) break;
					DeleteTextBoxLines (panelId, PANEL_STATUSBOX, 0, -1);
					SetCtrlVal(panelId,PANEL_STATUSBOX,"Waiting complete; **OPENING SHUTTER**");
					DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,sh_open,NULL,NULL);    // changed device number from 1  to 5
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+SAMPLE"); //doing this clears the averaging
					talkToScope(command,dat,gHandle,3848,1);
					strcpy(command,"/?COMMAND=ACQUIRE:MODE+AVERAGE");//and starts over
					talkToScope(command,dat,gHandle,3848,2);
					strcpy( msg,"Fiber-saving. --SHUTTER OPEN-- Cycle will resume in ");
					tLast=Timer()*1000.0;
					if( wait( timestep, msg) ) break;
				}
				//dataDump();
				eventnum++;
			}
			evthis = eventnum;
		}
	}

	for(i=0; i<500; i++) free(y[i]);
	DiscardPanel(panelId);
	return 0;
}


int CVICALLBACK enterIP (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

int CVICALLBACK End (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			run=0;
			loop=0;
			end=1;
			DisconnectFromTCPServer(gHandle);
			break;
	}
	return 0;
}

#define NR_END 1
#define FREE_ARG char*

void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now *not* exiting to system...\n");
	return;
	//  exit(1);
}

double *vector(long nl, long nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;

	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) nrerror("allocation failure in vector()");
	return v+NR_END-nl;
}

int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;

	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) nrerror("allocation failure in ivector()");
	return v-nl+NR_END;
}

unsigned char *cvector(long nl, long nh)
/* allocate an unsigned char vector with subscript range v[nl..nh] */
{
	unsigned char *v;

	v=(unsigned char *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(unsigned char)));
	if (!v) nrerror("allocation failure in cvector()");
	return v-nl+NR_END;
}

unsigned long *lvector(long nl, long nh)
/* allocate an unsigned long vector with subscript range v[nl..nh] */
{
	unsigned long *v;

	v=(unsigned long *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(long)));
	if (!v) nrerror("allocation failure in lvector()");
	return v-nl+NR_END;
}

double *dvector(long nl, long nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;

	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) nrerror("allocation failure in dvector()");
	return v-nl+NR_END;
}

double **matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1; i<=nrh; i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

double **dmatrix(long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1; i<=nrh; i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

int **imatrix(long nrl, long nrh, long ncl, long nch)
/* allocate a int matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	int **m;

	/* allocate pointers to rows */
	m=(int **) malloc((size_t)((nrow+NR_END)*sizeof(int*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;


	/* allocate rows and set pointers to them */
	m[nrl]=(int *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(int)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1; i<=nrh; i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

double **submatrix(double **a, long oldrl, long oldrh, long oldcl, long oldch, long newrl, long newcl)
/* point a submatrix [newrl..][newcl..] to a[oldrl..oldrh][oldcl..oldch] */
{
	long i,j,nrow=oldrh-oldrl+1,ncol=oldcl-newcl;
	double **m;

	/* allocate array of pointers to rows */
	m=(double **) malloc((size_t) ((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure in submatrix()");
	m += NR_END;
	m -= newrl;

	/* set pointers to rows */
	for(i=oldrl,j=newrl; i<=oldrh; i++,j++) m[j]=a[i]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

double **convert_matrix(double *a, long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix m[nrl..nrh][ncl..nch] that points to the matrix
   declared in the standard C manner as a[nrow][ncol], where nrow=nrh-nrl+1
   and ncol=nch-ncl+1. The routine should be called with the address
   &a[0][0] as the first argument. */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t) ((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure in convert_matrix()");
	m += NR_END;
	m -= nrl;

	/* set pointers to rows */
	m[nrl]=a-ncl;
	for(i=1,j=nrl+1; i<nrow; i++,j++) m[j]=m[j-1]+ncol;
	/* return pointer to array of pointers to rows */
	return m;
}

double ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
/* allocate a double 3tensor with range t[nrl..nrh][ncl..nch][ndl..ndh] */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1,ndep=ndh-ndl+1;
	double ***t;

	/* allocate pointers to pointers to rows */
	t=(double ***) malloc((size_t)((nrow+NR_END)*sizeof(double**)));
	if (!t) nrerror("allocation failure 1 in f3tensor()");
	t += NR_END;
	t -= nrl;

	/* allocate pointers to rows and set pointers to them */
	t[nrl]=(double **) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double*)));
	if (!t[nrl]) nrerror("allocation failure 2 in f3tensor()");
	t[nrl] += NR_END;
	t[nrl] -= ncl;

	/* allocate rows and set pointers to them */
	t[nrl][ncl]=(double *)
				malloc((size_t)((nrow*ncol*ndep+NR_END)*sizeof(double)));
	if (!t[nrl][ncl]) nrerror("allocation failure 3 in f3tensor()");
	t[nrl][ncl] += NR_END;
	t[nrl][ncl] -= ndl;

	for(j=ncl+1; j<=nch; j++) t[nrl][j]=t[nrl][j-1]+ndep;
	for(i=nrl+1; i<=nrh; i++)
	{
		t[i]=t[i-1]+ncol;
		t[i][ncl]=t[i-1][ncl]+ncol*ndep;
		for(j=ncl+1; j<=nch; j++) t[i][j]=t[i][j-1]+ndep;
	}

	/* return pointer to array of pointers to rows */
	return t;
}

void free_vector(double *v, long nl, long nh)
/* free a double vector allocated with vector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with ivector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_cvector(unsigned char *v, long nl, long nh)
/* free an unsigned char vector allocated with cvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_lvector(unsigned long *v, long nl, long nh)
/* free an unsigned long vector allocated with lvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_dvector(double *v, long nl, long nh)
/* free a double vector allocated with dvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_matrix(double **m, long nrl, long nrh, long ncl, long nch)
/* free a double matrix allocated by matrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch)
/* free a double matrix allocated by dmatrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch)
/* free an int matrix allocated by imatrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void free_submatrix(double **b, long nrl, long nrh, long ncl, long nch)
/* free a submatrix allocated by submatrix() */
{
	free((FREE_ARG) (b+nrl-NR_END));
}

void free_convert_matrix(double **b, long nrl, long nrh, long ncl, long nch)
/* free a matrix allocated by convert_matrix() */
{
	free((FREE_ARG) (b+nrl-NR_END));
}

void free_f3tensor(double ***t, long nrl, long nrh, long ncl, long nch,
				   long ndl, long ndh)
/* free a double f3tensor allocated by f3tensor() */
{
	free((FREE_ARG) (t[nrl][ncl]+ndl-NR_END));
	free((FREE_ARG) (t[nrl]+ncl-NR_END));
	free((FREE_ARG) (t+nrl-NR_END));
}
#define NRANSI
#define TINY 1.0e-10
#define NMAX 5000
#define GET_PSUM \
	for (j=1;j<=ndim;j++) {\
		for (sum=0.0,i=1;i<=mpts;i++) sum += p[i][j];\
		psum[j]=sum;}
#define SWAP(a,b) {swap=(a);(a)=(b);(b)=swap;}

void amoeba(double **p, double y[], int ndim, double ftol, double (*funk)(double []), int *nfunk)
{
	double amotry(double **p, double y[], double psum[], int ndim, double (*funk)(double []), int ihi, double fac);
	int i,ihi,ilo,inhi,j,mpts=ndim+1;
	double rtol,sum,swap,ysave,ytry,*psum;

	psum=vector(1,ndim);
	*nfunk=0;
	GET_PSUM
	for (;;)
	{
		ilo=1;
		ihi = y[1]>y[2] ? (inhi=2,1) : (inhi=1,2);
		for (i=1; i<=mpts; i++)
		{
			if (y[i] <= y[ilo]) ilo=i;
			if (y[i] > y[ihi])
			{
				inhi=ihi;
				ihi=i;
			}
			else if (y[i] > y[inhi] && i != ihi) inhi=i;
		}
		rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo])+TINY);
		if (rtol < ftol)
		{
			SWAP(y[1],y[ilo])
			for (i=1; i<=ndim; i++) SWAP(p[1][i],p[ilo][i])
				break;
		}
		if (*nfunk >= NMAX) nrerror("NMAX exceeded");
		*nfunk += 2;
		ytry=amotry(p,y,psum,ndim,funk,ihi,-1.0);
		if (ytry <= y[ilo])
		{
			ytry=amotry(p,y,psum,ndim,funk,ihi,2.0);
		}
		else if (ytry >= y[inhi])
		{
			ysave=y[ihi];
			ytry=amotry(p,y,psum,ndim,funk,ihi,0.5);
			if (ytry >= ysave)
			{
				for (i=1; i<=mpts; i++)
				{
					if (i != ilo)
					{
						for (j=1; j<=ndim; j++) p[i][j]=psum[j]=0.5*(p[i][j]+p[ilo][j]);
						y[i]=(*funk)(psum);
					}
				}
				*nfunk += ndim;
				GET_PSUM
			}
		}
		else --(*nfunk);
	}
	free_vector(psum,1,ndim);
}
#undef SWAP
#undef GET_PSUM
#undef NMAX
#undef NRANSI
/* note #undef's at end of file */
#define NRANSI
double amotry(double **p, double y[], double psum[], int ndim, double (*funk)(double []), int ihi, double fac)
{
	int j;
	double fac1,fac2,ytry,*ptry;

	ptry=vector(1,ndim);
	fac1=(1.0-fac)/ndim;
	fac2=fac1-fac;
	for (j=1; j<=ndim; j++) ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
	ytry=(*funk)(ptry);
	if (ytry < y[ihi])
	{
		y[ihi]=ytry;
		for (j=1; j<=ndim; j++)
		{
			psum[j] += ptry[j]-p[ihi][j];
			p[ihi][j]=ptry[j];
		}
	}
	free_vector(ptry,1,ndim);
	return ytry;
}
#undef NRANSI

double *ftime, *fvolt, *ftime0, *fvolt0;
double qci, qai, q0i;
int npoints;

void do_purity_fit( double *t, double *v, int n, double *fitpar, double *bestfit )
{
	FILE *fptr;
	int i;
	double vp, vm;
	double **p;
	double *y;
	double chi2final( double par[] );
	double finalfit( double t, double * par);
	void amoeba(double **p, double y[], int ndim, double ftol, double (*funk)(double []), int *nfunk);
	int nfunk;


	// Copy pointers to local variables

	ftime = t;
	fvolt = v;
	ftime0 = t;
	fvolt0 = v;
	npoints = n;

	// Look for initial guess values

	q0i = -999.0;
	qci = -999.0;
	qai = -999.0;
	ftime = ftime0;
	fvolt = fvolt0;
	for ( i = 0; i < npoints; i++ )
	{
		if ( *ftime > -10.0 && q0i == -999.0 ) q0i = *fvolt;
		if ( *ftime > 5.0 && qci == -999.0 ) qci = *fvolt;
		if ( *ftime > 75.0 && qai == -999.0 ) qai = *fvolt;
		ftime++;
		fvolt++;
	}
	qai = 0.85*(qci - q0i) - (qai - q0i);
	qci = qci - q0i;
	//  printf("qci = %f \t qai = %f\n", qci, qai);

	ftime = ftime0;
	fvolt = fvolt0;

	// Declare the ridiculous numerical recipes matrix and vector

	p = matrix(1,4,1,3);
	y = vector(1,4);

	// Now do the final fit.
	// Put the initial guesses in the numerical recipes matrix

	p = matrix(1,4,1,3);
	for ( i = 1; i < 5; i++ )
	{
		p[i][1] = qci;
		p[i][2] = qai;
		p[i][3] = q0i;
	}
	p[2][1] += 0.1;
	p[3][2] += 0.1;
	p[4][3] += 0.1;

	// Evaluate the chi2 at the initial guesses

	y = vector(1,4);
	for ( i = 1; i < 5; i++ ) y[i] = chi2final( &p[i][1] - 1 );

	// Call ameoba

	nfunk = 0;
	amoeba( p, y, 3, 0.00001, chi2final, &nfunk );

	// Copy results to output arrays

	fitpar[0] = T1;
	fitpar[1] = T2;
	fitpar[2] = TR;
	fitpar[3] = TF;
	for ( i = 0; i < 3; i++ ) fitpar[i+4] = p[1][1+i];

	ftime = ftime0;
	fvolt = fvolt0;
	for ( i = 0; i < npoints; i++ )
	{
		bestfit[i] = finalfit( *ftime, &p[1][1] - 1 );
		ftime++;
	}
	ftime = ftime0;
	fvolt = fvolt0;
	fitpar[10] = chi2final( &p[1][1] - 1 );
	return;
}


double chi2final( double par[] )
{
	double result, signal;
	double finalfit( double t, double * par);
	int i;

	result = 0.0;
	ftime = ftime0;
	fvolt = fvolt0;
	for ( i = 0; i < npoints; i++ )
	{
		if ( (*ftime > -50.0 && *ftime < -15.0) ||
				(*ftime > 25.0 && *ftime < 65.0) ||
				(*ftime > 125.0 && *ftime < 150.0) )
		{
			signal = finalfit( *ftime, par );
			result += (*fvolt - signal)*(*fvolt - signal)/(0.2*0.2);
		}
		fvolt++;
		ftime++;
	}
	fvolt = fvolt0;
	ftime = ftime0;
	result = result/sqrt((double)npoints);

	// printf("chi2final = %f\n", result );
	return result;

}


double finalfit( double t, double * par)
{
	double t1, t2, tr, tf, th, qa, qc, q0;
	double freq( double );
	double result;

	t1 = T1;
	t2 = T2;
	tr = TR;
	tf = TF;

	qc = *(par+1);
	qa = *(par+2);
	q0 = *(par+3);
	th = 395.3;

	if ( t < t1 )
	{
		result = qc*freq((t-t1)/tr)
				 - qa*freq((t-t2)/tf);
	}
	else if ( t < t2 )
	{
		result = qc*freq((t-t1)/tr)*exp(-1.0*(t-t1)/th)
				 - qa*freq((t-t2)/tf);
	}
	else
	{
		result = qc*freq((t-t1)/tr)*exp(-1.0*(t-t1)/th)
				 - qa*freq((t-t2)/tf)*exp(-1.0*(t-t2)/th);
	}

	result = result + q0;
	return result;
}



double freq( double x )
{

	double erff(double x);
	return 0.5*(1.0 + erff(x));
}


// These are numerical recipe routines


double erff(double x)
{
	double gammp(double a, double x);

	return   x < 0.0 ? -gammp(0.5,x*x) : gammp(0.5,x*x);
	;
}



double gammp(double a, double x)
{
	void gcf(double *gammcf, double a, double x, double *gln);
	void gser(double *gamser, double a, double x, double *gln);
	void nrerror(char error_text[]);
	double gamser,gammcf,gln;

	if (x < 0.0 || a <= 0.0)
		nrerror("Invalid arguments in routine gammp");
	if (x < (a+1.0))
	{
		gser(&gamser,a,x,&gln);
		return gamser;
	}
	else
	{
		gcf(&gammcf,a,x,&gln);
		return 1.0-gammcf;
	}
}


double gammq(double a, double x)
{
	void gcf(double *gammcf, double a, double x, double *gln);
	void gser(double *gamser, double a, double x, double *gln);
	void nrerror(char error_text[]);
	double gamser,gammcf,gln;

	if (x < 0.0 || a <= 0.0)
		nrerror("Invalid arguments in routine gammq");
	if (x < (a+1.0))
	{
		gser(&gamser,a,x,&gln);
		return 1.0-gamser;
	}
	else
	{
		gcf(&gammcf,a,x,&gln);
		return gammcf;
	}
}

#define ITMAX 100
#define EPS 3.0e-7
#define FPMIN 1.0e-30

void gcf(double *gammcf, double a, double x, double *gln)
{
	double gammln(double xx);
	void nrerror(char error_text[]);
	int i;
	double an,b,c,d,del,h;

	*gln=gammln(a);
	b=x+1.0-a;
	c=1.0/FPMIN;
	d=1.0/b;
	h=d;
	for (i=1; i<=ITMAX; i++)
	{
		an = -i*(i-a);
		b += 2.0;
		d=an*d+b;
		if (fabs(d) < FPMIN) d=FPMIN;
		c=b+an/c;
		if (fabs(c) < FPMIN) c=FPMIN;
		d=1.0/d;
		del=d*c;
		h *= del;
		if (fabs(del-1.0) < EPS) break;
	}
	if (i > ITMAX) nrerror("a too large, ITMAX too small in gcf");
	*gammcf=exp(-x+a*log(x)-(*gln))*h;
}
#undef ITMAX
#undef EPS
#undef FPMIN

#define ITMAX 100
#define EPS 3.0e-7

void gser(double *gamser, double a, double x, double *gln)
{
	double gammln(double xx);
	void nrerror(char error_text[]);
	int n;
	double sum,del,ap;

	*gln=gammln(a);
	if (x <= 0.0)
	{
		if (x < 0.0) nrerror("x less than 0 in routine gser");
		*gamser=0.0;
		return;
	}
	else
	{
		ap=a;
		del=sum=1.0/a;
		for (n=1; n<=ITMAX; n++)
		{
			++ap;
			del *= x/ap;
			sum += del;
			if (fabs(del) < fabs(sum)*EPS)
			{
				*gamser=sum*exp(-x+a*log(x)-(*gln));
				return;
			}
		}
		nrerror("a too large, ITMAX too small in routine gser");
		return;
	}
}
#undef ITMAX
#undef EPS


double gammln(double xx)
{
	double x,y,tmp,ser;
	static double cof[6]= {76.18009172947146,-86.50532032941677,
						   24.01409824083091,-1.231739572450155,
						   0.1208650973866179e-2,-0.5395239384953e-5
						  };
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;
	for (j=0; j<=5; j++) ser += cof[j]/++y;
	return -tmp+log(2.5066282746310005*ser/x);
}
