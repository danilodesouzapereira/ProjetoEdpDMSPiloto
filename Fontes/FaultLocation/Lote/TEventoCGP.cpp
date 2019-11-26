//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEventoCGP.h"
#include "..\Auxiliares\FuncoesFL.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
__fastcall TEventoCGP::TEventoCGP()
{

}
//---------------------------------------------------------------------------
__fastcall TEventoCGP::~TEventoCGP()
{

}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::InicializaEvento()
{

}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetRelayID(String RelayID)
{
	this->RelayID = RelayID;

	// Obtém o código do alimentador a partir do ID do relé
   CodAlimentador = RelayID.SubString(1, 4);
   CodAlimentador += "0";
   CodAlimentador += RelayID.SubString(6, CodAlimentador.Length());
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetEventTime(String EventTime)
{
    this->EventTime = EventTime;
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetEventDate(String EventDate)
{
	this->EventDate = EventDate;
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetEventType(String EventType)
{
   this->EventType = EventType;
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetEventLocation(String strEventLocation)
{
   String valor = StrReplace(strEventLocation, ".", ",");
   this->EventLocation = valor.ToDouble();
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetIAFault(String strIAFault)
{
   String valor = StrReplace(strIAFault, ".", ",");
   this->IAFault = valor.ToDouble();
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetIBFault(String strIBFault)
{
   String valor = StrReplace(strIBFault, ".", ",");
   this->IBFault = valor.ToDouble();
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetICFault(String strICFault)
{
   String valor = StrReplace(strICFault, ".", ",");
   this->ICFault = valor.ToDouble();
}
//---------------------------------------------------------------------------
void __fastcall TEventoCGP::SetINFault(String strINFault)
{
   String valor = StrReplace(strINFault, ".", ",");
   this->INFault = valor.ToDouble();
}
//---------------------------------------------------------------------------
