#include "simuc.h"
#include "io_treiber.h"

typedef struct BHandle_Data {
	BYTE Board_allocated;	// 1=allocated, 0=free
	BYTE Port_A_Direction;	// 1=Output, 0=Input
	BYTE Port_B_Direction;
	BYTE Port_C_Direction;
	BYTE Port_D_Direction;
} BHandle;
#define PA      0x90
#define PB      0x82
#define PC      0x88
#define PD      0x81
#define PAOUT   0x8B
#define PBOUT   0x99
#define PCOUT   0x93
#define PDOUT   0x9A

BHandle BoardHandle_Data;
DSCB GlobalBoardHandle = &BoardHandle_Data;

BYTE Init(DSCB BoardHandle, unsigned long int Steuerwort) {
BYTE a=0,b=0,c=0,d=0;//a=Port_A_Direction...

 if(BoardHandle->Board_allocated==0)//Hardware ist frei
 {
     switch(Steuerwort)
     {
     case 0x80: a=1;b=1;c=1;d=1; break;
     case 0x81: a=1;b=1;c=1;d=0; break;
     case 0x88: a=1;b=1;c=0;d=1; break;
     case 0x89: a=1;b=1;c=0;d=0; break;
     case 0x82: a=1;b=0;c=1;d=1; break;
     case 0x83: a=1;b=0;c=1;d=0; break;
     case 0x8A: a=1;b=0;c=0;d=1; break;
     case 0x8B: a=1;b=0;c=0;d=0; break;
     case 0x90: a=0;b=1;c=1;d=1; break;
     case 0x91: a=0;b=1;c=1;d=0; break;
     case 0x98: a=0;b=1;c=0;d=1; break;
     case 0x99: a=0;b=1;c=0;d=0; break;
     case 0x92: a=0;b=0;c=1;d=1; break;
     case 0x93: a=0;b=0;c=1;d=0; break;
     case 0x9A: a=0;b=0;c=0;d=1; break;
     case 0x9B: a=0;b=0;c=0;d=0; break;
     }
     if(a==1)//PORT A SCHREIB/LESEZUGRIFF
     {     io_out16(DIR0, (io_in16(DIR0)|0xff));
         BoardHandle->Port_A_Direction=1;
     }else
          io_out16(DIR0,(io_in16(DIR0)&0xff00));
     if(b==1)//PORT B SCHREIB/LESEZUGRIFF
     {     io_out16(DIR0,(io_in16(DIR0)|0xff00));
         BoardHandle->Port_B_Direction=1;
     }else
          io_out16(DIR0,(io_in16(DIR0)&0xff));
     if(c==1)//PORT C SCHREIB/LESEZUGRIFF
     {     io_out16(DIR1, (io_in16(DIR1)|0xff));
         BoardHandle->Port_C_Direction=1;
     }else
          io_out16(DIR1,(io_in16(DIR1)&0xff00));
     if(d==1)//PORT D SCHREIB/LESEZUGRIFF
     {     io_out16(DIR1,(io_in16(DIR1)|0xff00));
         BoardHandle->Port_D_Direction=1;
     }else
          io_out16(DIR1,(io_in16(DIR1)&0xff));


      BoardHandle->Board_allocated=1;
      return 0;


      }
      else //Hardware konnte nicht reserviert werden.
      {
          return 1;
      }
     }


BYTE InputByte(DSCB BoardHandle, BYTE Port, BYTE *DigitalValue) {
    short int fin=0;
    if(!BoardHandle->Board_allocated==1)
        return 1;
    switch(Port)
    {
        case PA :
            if(!BoardHandle->Port_A_Direction)
                fin=io_in16(IN0)&0xFF;
            break;
        case PB:
            if(!BoardHandle->Port_B_Direction)
                fin=(io_in16(IN0)>>8)&0xFF;
            break;
        case PC:
            if(!BoardHandle->Port_C_Direction)
                fin=io_in16(IN1)&0xFF;
            break;
        case PD:
            if(!BoardHandle->Port_A_Direction)
                fin=(io_in16(IN1)>>8)&0xFF;
            break;
        default:
            return 2;
            break;
    }
    *DigitalValue=fin;
    return 1;
}

BYTE OutputByte(DSCB BoardHandle, BYTE Port, BYTE DigitalValue) {
    if(!BoardHandle->Board_allocated==1)
        return 1;
    switch(Port)
    {
        case PAOUT :
            if(BoardHandle->Port_A_Direction)
            {
                io_out16(OUT0, (DigitalValue&0xFF)|(io_in16(OUT0)&0xFF00));
            }
            return DigitalValue==(io_in16(OUT0)&0xff00)? 0:3;
        case PBOUT:
            if(BoardHandle->Port_B_Direction)
                io_out16(OUT0, (DigitalValue<<8)|(io_in16(OUT0)&0xFF));
            return DigitalValue==(io_in16(OUT0)>>8)?0:3;
        case PCOUT:
            if(BoardHandle->Port_C_Direction)
                io_out16(OUT1, DigitalValue|(io_in16(OUT1)&0xFF00));
            return DigitalValue==(io_in16(OUT1)&0xff00)?0:3;
        case PDOUT:
            if(BoardHandle->Port_D_Direction)
                io_out16(OUT1, (DigitalValue<<8)|(io_in16(OUT1)&0xFF));
            return DigitalValue==(io_in16(OUT1)>>8)?0:3;
        default:
            return 2;
    }
    return 0;
}

BYTE Free(DSCB BoardHandle) {

    Init(BoardHandle, 0x80);
    io_out16(OUT0, 0x0000);
    io_out16(OUT1, 0x0000);

    BoardHandle->Board_allocated=0;
    return BoardHandle->Board_allocated;
}
