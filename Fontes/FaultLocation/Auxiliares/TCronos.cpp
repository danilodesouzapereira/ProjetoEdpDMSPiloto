/***
 *  Classe para medição de tempo dos processos do algoritmo
 **/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TCronos.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TCronos::TCronos()
{
	this->horarioIni = Now();
}
//---------------------------------------------------------------------------
__fastcall TCronos::~TCronos()
{

}
//---------------------------------------------------------------------------
double __fastcall TCronos::GetSegundos()
{
	unsigned short hour, min, sec, msec;
   double resp = 0.;

 	TDateTime agora = Now();
   TDateTime dif = agora - horarioIni;
   dif.DecodeTime(&hour, &min, &sec, &msec);

	resp = hour * 3600.;
   resp += min * 60.;
   resp += sec;

   return resp;
}
//---------------------------------------------------------------------------
double __fastcall TCronos::GetMinutos()
{
	unsigned short hour, min, sec, msec;
   double resp = 0.;

 	TDateTime agora = TDateTime();
   TDateTime dif = agora - horarioIni;
   dif.DecodeTime(&hour, &min, &sec, &msec);

	resp = hour * 60.;
   resp += min;
   resp += sec / 60.;

   return resp;
}
//---------------------------------------------------------------------------
void __fastcall TCronos::Reinicia()
{
	this->horarioIni = TDateTime();
}
//---------------------------------------------------------------------------
