//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEvento.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
__fastcall TEvento::TEvento()
{
	// Inicializa parâmetros do evento
   InicializaEvento();
}
//---------------------------------------------------------------------------
__fastcall TEvento::~TEvento()
{
	// Nada a fazer
}
//---------------------------------------------------------------------------
void __fastcall TEvento::InicializaEvento()
{
	// Inicializa parâmetros do evento
   DtInicio = "";
   DtAcionamento = "";
   DtChegada = "";
   DtConclusao = "";
   PDF = "";
   CodAlimentador = "";
   RefDef = "";
   CausaID = -1;
   TipoCC = -1;
   LocSEL = 0.;
   LocOMS = 0.;
   LocFLIISR = 0.;
   Icc = 0.;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetDtInicio(String DtInicio)
{
	this->DtInicio = DtInicio;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetDtAcionamento(String DtAcionamento)
{
	this->DtAcionamento = DtAcionamento;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetDtChegada(String DtChegada)
{
	this->DtChegada = DtChegada;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetDtConclusao(String DtConclusao)
{
	this->DtConclusao = DtConclusao;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetPDF(String PDF)
{
	this->PDF = PDF;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetCodAlimentador(String CodAlimentador)
{
	this->CodAlimentador = CodAlimentador;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetCausaID(int CausaID)
{
	this->CausaID = CausaID;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetRefDef(String RefDef)
{
	this->RefDef = RefDef;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetTipoCC(int TipoCC)
{
	this->TipoCC = TipoCC;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetLocSel(double LocSEL)
{
	this->LocSEL = LocSEL;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetLocOMS(double LocOMS)
{
	this->LocOMS = LocOMS;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetLocFLIISR(double LocFLIISR)
{
	this->LocFLIISR = LocFLIISR;
}
//---------------------------------------------------------------------------
void __fastcall TEvento::SetIcc(double Icc)
{
	this->Icc = Icc;
}
//---------------------------------------------------------------------------

