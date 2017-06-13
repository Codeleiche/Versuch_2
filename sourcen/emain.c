#include "simuc.h"
#include "io_treiber.h"

// Die Verwendung der folgenden Zustandsnamen ist verbindlich

// Die folgenden Defines muessen sinnvoll genutzt werden
#define BIT_T2		5 //Port C
#define BIT_T1		4
#define BIT_ESR		3
#define BIT_ESL		2

#define BIT_M_Re	2//Port D
#define BIT_M_Li	1
#define BIT_M_An	0

#define PA          0x90
#define PB          0x82
#define PC          0x88
#define PD          0x81
#define PAOUT       0x8B
#define PBOUT       0x99
#define PCOUT       0x93
#define PDOUT       0x9A
typedef enum{Steht,StarteRunter,StarteRauf,Notaus,FahreRauf_MIT_OF,FahreRauf_OHNE_OF,FahreRunter_MIT_OF,FahreRunter_OHNE_OF} state_t;

typedef unsigned char chr;

void steuerung(chr *M_Re, chr *M_Li, chr *M_An, unsigned int *endpos, unsigned int *startpos, unsigned int timer, chr Betrieb, chr NotAus, chr ESR, chr ESL, chr HPR, chr HPL,  state_t *stater)
{

    switch(*stater)
    {
    case Steht:
        *stater=ESR ? StarteRauf : (ESL ? StarteRunter: NotAus ? Notaus:Steht);
        *M_Re=*M_Li=*M_An=0;
        break;
    case StarteRauf:
        *stater=NotAus ? Notaus:(*startpos=timer) <= (*endpos=(*startpos+60000)%65535)? FahreRauf_OHNE_OF:FahreRauf_MIT_OF;
        *M_Re=1;*M_Li=0;*M_An=1;
        break;
    case StarteRunter:
        *stater=NotAus? Notaus:((*startpos=timer) >= (*endpos=(*startpos-60000)%65535)? FahreRunter_OHNE_OF:FahreRunter_MIT_OF);
        *M_Re=0;*M_Li=1;*M_An=1;
        break;
    case FahreRauf_OHNE_OF:
        *stater=NotAus ? Notaus:((timer>*endpos)||(timer<*startpos)?Steht:(ESR? StarteRauf:FahreRauf_OHNE_OF));
        *M_Re=1;*M_Li=0;*M_An=1;
        break;
    case FahreRauf_MIT_OF:
        *stater=NotAus? Notaus :((timer>*endpos)&&(timer<*startpos)?Steht:(ESR? StarteRauf:FahreRauf_MIT_OF));
        *M_Re=1;*M_Li=0;*M_An=1;
        break;
    case FahreRunter_OHNE_OF:
        *stater=NotAus?Notaus:((timer<*endpos)||(timer>*startpos)?Steht:(ESL? StarteRunter:FahreRunter_OHNE_OF));
        *M_Re=0;*M_Li=1;*M_An=1;
        break;
    case FahreRunter_MIT_OF:
        *stater=NotAus?Notaus:((timer<*endpos)&&(timer>*startpos)?Steht:(ESL? StarteRunter:FahreRunter_MIT_OF));
        *M_Re=0;*M_Li=1;*M_An=1;
        break;
    case Notaus:
        *stater=(Betrieb&&!NotAus) ?Steht:Notaus;
        *M_Re=*M_Li=*M_An=0;
        break;
    default:
        *stater=Steht;
        *M_Re=*M_Li=*M_An=0;
        break;
    }
}

// Hier die Steuerungsfunktion definieren
// ...

void emain(void* arg) 
{
	// Hier alle benoetigten Variablen deklarieren
	// ...
    unsigned char M_Re=0,M_Li=0,M_An=0,Betrieb=0,NotAus=0,ESR=0,ESL=0,HPR=0,HPL=0;
    unsigned int timer=0,endpos=0,startpos=0,temp=0,ausgabe=0;
    state_t stater=Steht;
	INIT_BM_WITH_REGISTER_UI; // Hier unbedingt einen Break-Point setzen !!!

	
    // Hier die Treiberfunktionen aufrufen und testen (Aufgabe 1)
	// ...
/*    //TEST 1 - InputByte()
        unsigned short int wert;

        BYTE testWertD=42;//101010
        BYTE testWertC=255;//11111111
        BYTE testWertB=128;//10000000
    BYTE testWertA=1;

        Init(GlobalBoardHandle,0x9B);
        InputByte(GlobalBoardHandle,PD,&wert); //PORT D  auslesen
        printf("Wert in DigitalValue: ");
        printf("%i",wert);
        printf("\n");
        InputByte(GlobalBoardHandle,PC,&wert); //PORT D  auslesen
        printf("Wert in DigitalValue: ");
        printf("%i",wert);
        printf("\n");
        InputByte(GlobalBoardHandle,PB,&wert); //PORT D  auslesen
        printf("Wert in DigitalValue: ");
        printf("%i",wert);
        printf("\n");
        InputByte(GlobalBoardHandle,PA,&wert); //PORT D  auslesen
        printf("Wert in DigitalValue: ");
        printf("%i",wert);
        printf("\n");
        Free(GlobalBoardHandle);

        //TEST 2 - OutputByte()
             Init(GlobalBoardHandle,0x80);
             OutputByte(GlobalBoardHandle,PDOUT,testWertD);
             OutputByte(GlobalBoardHandle,PCOUT,testWertC);
             OutputByte(GlobalBoardHandle,PBOUT,testWertB);
             OutputByte(GlobalBoardHandle,PAOUT,testWertA);
             Free(GlobalBoardHandle);

  /*

    Init( GlobalBoardHandle, PAOUT);
    OutputByte(GlobalBoardHandle, PAOUT, 0b10001011);

    Init( GlobalBoardHandle, PA);
    InputByte(GlobalBoardHandle, PA, DigitalValue);
    Init( GlobalBoardHandle, PAOUT);
    OutputByte(GlobalBoardHandle, PAOUT, 0b10001011);
    Free(GlobalBoardHandle);
*/


    Init(GlobalBoardHandle, PDOUT);
	// Ab hier beginnt die Endlosschleife fuer den Automaten (Aufgabe 2)
	while(1) {

		SYNC_SIM; 

		// Hier die Eingabesignale einlesen
		// ...
        InputByte(GlobalBoardHandle,PC,&HPL);
        HPR=(HPL>>1)&1;
        ESL=(HPL>>2)&1;
        ESR=(HPL>>3)&1;
        NotAus=(HPL>>4)&1;
        Betrieb=(HPL>>5)&1;
        HPL&=1;


        InputByte(GlobalBoardHandle,PB,&timer);
        InputByte(GlobalBoardHandle,PA,&temp);
        timer=((timer<<8)|(temp&0xFF))%65535;

		// Hier die Steuerungsfunktion aufrufen
		// ...
        steuerung(&M_Re, &M_Li, &M_An, &endpos, &startpos, timer, Betrieb, NotAus, ESR, ESL, HPR, HPL, &stater);



		// Hier die Ausgabesignale ausgeben
		// ...
        ausgabe=((M_Re<<BIT_M_Re)|(M_Li<<BIT_M_Li)|(M_An<<BIT_M_An));
        OutputByte(GlobalBoardHandle,PDOUT,ausgabe);

	} // while(1)..

	
}

  
