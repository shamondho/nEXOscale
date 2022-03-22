// File:        ReadSavePng.h
// Purpose:        Header file for ReadSavePng.fp/lib
//                        Saves bitmap, panel or control in a PNG file
//                        Reads a PNG file to a bitmap or a picture control
// For help, see the help of the FP

#ifndef _READ_SAVE_PNG
#define _READ_SAVE_PNG

#ifndef TRUE
        typedef int BOOL;
        #define TRUE 1
        #define FALSE 0
#endif

#ifndef Z_NO_COMPRESSION
        #define Z_NO_COMPRESSION         0
        #define Z_BEST_SPEED             1
        #define Z_BEST_COMPRESSION       9
        #define Z_DEFAULT_COMPRESSION  (-1)
#endif

extern void Png_SetParameters(const double Gamma, const BOOL Interlaced, 
                                        const double Dpi, const int 
CompressionLevel, 
                                        const int Alpha32);
                                        
extern void Png_SetTextTitle(char* Str, BOOL Comp);
extern void Png_SetTextAuthor(char* Str, BOOL Comp);
extern void Png_SetTextDescription(char* Str, BOOL Comp);
extern void Png_SetTextCopyright(char* Str, BOOL Comp);
extern void Png_SetTextCreationTime(char* Str, BOOL Comp);
extern void Png_SetTextSoftware(char* Str, BOOL Comp);
extern void Png_SetTextDisclaimer(char* Str, BOOL Comp);
extern void Png_SetTextWarning(char* Str, BOOL Comp);
extern void Png_SetTextSource(char* Str, BOOL Comp);
extern void Png_SetTextComment(char* Str, BOOL Comp);

extern BOOL Png_SaveBitmapToFile(unsigned char *Bits,
                                        int RowBytes, int PixDepth, 
                                        const int Width, const int Height,
                                        const char* FileName);
                                        
#ifdef _CVI_
extern BOOL Png_SavePanelControlToFile(int Panel, int Control, const char* 
FileName);
#endif

///////////////////////////////////////////////////////////////////////////////
extern void Png_GetParameters(double *Gamma, BOOL *Interlaced, 
                                        double *Dpi, int *CompressionLevel, 
                                        int *Alpha32);

extern BOOL Png_ReadBitmapFromFile(int *Bitmap, unsigned char **Bits,
                                        int *RowBytes, int *PixDepth, 
                                        int *Width, int *Height,
                                        const char* FileName);

extern BOOL Png_GetProperties(int *RowBytes, int *PixDepth, int *NbChan,
                                        int *Width, int *Height,
                                        const char* FileName);

#ifdef _CVI_
extern BOOL Png_ReadControlFromFile(int Panel, int Control, int ImageID, const 
char* FileName);
#endif

const char* Png_GetTextTitle(void);
const char* Png_GetTextAuthor(void);
const char* Png_GetTextDescription(void);
const char* Png_GetTextCopyright(void);
const char* Png_GetTextCreationTime(void);
const char* Png_GetTextSoftware(void);
const char* Png_GetTextDisclaimer(void);
const char* Png_GetTextWarning(void);
const char* Png_GetTextSource(void);
const char* Png_GetTextComment(void);

///////////////////////////////////////////////////////////////////////////////
extern void Png_CopyRGBtoGray(unsigned char *Dest, unsigned char *Source, 
                                                  const int NbPixels, const int 
Ratio);
extern void Png_CopyRGBtoBW(unsigned char *Dest, unsigned char *Source, 
                                                const int Ratio, const int 
Width, const int Height,
                                                const int ColorThreshold);

#ifndef _CVI_
        // for big endian machines, must be called before saving the file
        extern void Png_SwapEndian(unsigned long *Colors, const int Nb);
#endif

#endif // _READ_SAVE_PNG
