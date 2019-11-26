//---------------------------------------------------------------------------
#pragma hdrstop
#include "TXMLEqpto.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TXMLEqpto::TXMLEqpto()
{
	// nada a fazer
}
//---------------------------------------------------------------------------
__fastcall TXMLEqpto::TXMLEqpto(String timeStamp)
{
	this->timeStamp = timeStamp;
}
//---------------------------------------------------------------------------
String __fastcall TXMLEqpto::GetTimeStamp()
{
	return timeStamp;
}
//---------------------------------------------------------------------------
String __fastcall TXMLEqpto::GetCodigoAlimentador()
{
   return codigoAlimentador;
}
//---------------------------------------------------------------------------
String __fastcall TXMLEqpto::GetCodigoEqpto()
{
   return codigoEqpto;
}
//---------------------------------------------------------------------------
int __fastcall TXMLEqpto::GetTipoEqpto()
{
   return tipoEqpto;
}
//---------------------------------------------------------------------------
int __fastcall TXMLEqpto::GetTipoAlarme()
{
   return tipoAlarme;
}
//---------------------------------------------------------------------------
double __fastcall TXMLEqpto::GetDfaltaRele()
{
	return DfaltaRele;
}
//---------------------------------------------------------------------------
void __fastcall TXMLEqpto::SetTimeStamp(String timeStamp)
{
	this->timeStamp = timeStamp;
}
//---------------------------------------------------------------------------
void __fastcall TXMLEqpto::SetCodigoAlimentador(String codigoAlimentador)
{
	this->codigoAlimentador = codigoAlimentador;
}
//---------------------------------------------------------------------------
void __fastcall TXMLEqpto::SetCodigoEqpto(String codigoEqpto)
{
	this->codigoEqpto = codigoEqpto;
}
//---------------------------------------------------------------------------
void __fastcall TXMLEqpto::SetTipoEqpto(int tipoEqpto)
{
	this->tipoEqpto = tipoEqpto;
}
//---------------------------------------------------------------------------
void __fastcall TXMLEqpto::SetTipoAlarme(int tipoAlarme)
{
   this->tipoAlarme = tipoAlarme;
}
//---------------------------------------------------------------------------
void __fastcall TXMLEqpto::SetDfaltaRele(double DfaltaRele)
{
	this->DfaltaRele = DfaltaRele;
}
//---------------------------------------------------------------------------
