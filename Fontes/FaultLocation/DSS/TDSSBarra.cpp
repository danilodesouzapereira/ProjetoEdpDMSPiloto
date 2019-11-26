//---------------------------------------------------------------------------
#pragma hdrstop
#include "TDSSBarra.h"
#include "DSSEnums.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TDSSBarra::TDSSBarra()
{
	// Inicializa parâmetros
   this->Fases = faseINDEF;
   this->NumFases = 0;
   this->Codigo = "";
   this->ID = -1;
}
//---------------------------------------------------------------------------
String __fastcall TDSSBarra::GetCodigo()
{
	return Codigo;
}
//---------------------------------------------------------------------------
int __fastcall TDSSBarra::GetID()
{
	return ID;
}
//---------------------------------------------------------------------------
int __fastcall TDSSBarra::GetIndice_VetorBarras()
{
	return Indice_VetorBarras;
}
//---------------------------------------------------------------------------
int __fastcall TDSSBarra::GetFases()
{
	return Fases;
}
//---------------------------------------------------------------------------
int __fastcall TDSSBarra::GetNumFases()
{
	return NumFases;
}
//---------------------------------------------------------------------------
void __fastcall TDSSBarra::SetCodigo(String Codigo)
{
	this->Codigo = Codigo;
}
//---------------------------------------------------------------------------
void __fastcall TDSSBarra::SetID(int ID)
{
	this->ID = ID;
}
//---------------------------------------------------------------------------
void __fastcall TDSSBarra::SetIndice_VetorBarras(int Indice_VetorBarras)
{
	this->Indice_VetorBarras = Indice_VetorBarras;
}
//---------------------------------------------------------------------------
void __fastcall TDSSBarra::SetFases(int Fases)
{
	switch(this->Fases)
   {
   case faseINDEF:
   	if(Fases == 1)
      {
      	this->Fases = faseA;
         this->NumFases = 1;
      }
      else if(Fases == 2)
      {
         this->Fases = faseB;
         this->NumFases = 1;
      }
      else if(Fases == 3)
      {
         this->Fases = faseC;
         this->NumFases = 1;
      }
      break;

   case faseA:
   	if(Fases == 2)
      {
         this->Fases = faseAB;
         this->NumFases = 2;
      }
      else if(Fases == 3)
      {
         this->Fases = faseCA;
         this->NumFases = 2;
      }
   	break;

   case faseB:
   	if(Fases == 1)
      {
         this->Fases = faseAB;
         this->NumFases = 2;
      }
      else if(Fases == 3)
      {
         this->Fases = faseBC;
         this->NumFases = 2;
      }
   	break;

   case faseC:
   	if(Fases == 1)
      {
         this->Fases = faseCA;
         this->NumFases = 2;
      }
      else if(Fases == 2)
      {
         this->Fases = faseBC;
         this->NumFases = 2;
      }
   	break;

   case faseAB:
   	if(Fases == 3)
      {
         this->Fases = faseABC;
         this->NumFases = 3;
      }
   	break;

   case faseBC:
   	if(Fases == 1)
      {
         this->Fases = faseABC;
         this->NumFases = 3;
      }
   	break;

   case faseCA:
   	if(Fases == 2)
      {
         this->Fases = faseABC;
         this->NumFases = 3;
      }
   	break;

   case faseABC:
   	break;

   default:
   	break;
   }
}
//---------------------------------------------------------------------------
