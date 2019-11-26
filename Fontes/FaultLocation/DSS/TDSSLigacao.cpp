//---------------------------------------------------------------------------
#pragma hdrstop
#include "TDSSLigacao.h"
#include "DSSEnums.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TDSSLigacao::TDSSLigacao()
{
	// Inicializa parâmetros
   this->Codigo = "";
	Fases = faseINDEF;
   NumFases = 0;
}
//---------------------------------------------------------------------------
void __fastcall TDSSLigacao::AddFase(int iFase)
{
	if(Fases == faseINDEF)
   {
      switch(iFase)
      {
      case 1:
         Fases = faseA;
         NumFases = 1;
         break;

      case 2:
      	Fases = faseB;
         NumFases = 1;
         break;

      case 3:
      	Fases = faseC;
         NumFases = 1;
         break;

      default:
      	break;
      }
   }
   else if(Fases == faseA)
   {
   	switch(iFase)
      {
      case 1:
         break;

      case 2:
      	Fases = faseAB;
         NumFases = 2;
         break;

      case 3:
      	Fases = faseCA;
         NumFases = 2;
         break;

      default:
      	break;
      }
   }
   else if(Fases == faseB)
   {
   	switch(iFase)
      {
      case 1:
      	Fases = faseAB;
         NumFases = 2;
         break;

      case 2:
         break;

      case 3:
      	Fases = faseBC;
         NumFases = 2;
         break;

      default:
      	break;
      }
   }
   else if(Fases == faseC)
   {
   	switch(iFase)
      {
      case 1:
      	Fases = faseCA;
         NumFases = 2;
         break;

      case 2:
      	Fases = faseBC;
         NumFases = 2;
         break;

      case 3:
         break;

      default:
      	break;
      }
   }
   else if(Fases == faseAB)
   {
   	switch(iFase)
      {
      case 1:
      case 2:
         break;

      case 3:
      	Fases = faseABC;
         NumFases = 3;
         break;

      default:
      	break;
      }
   }
   else if(Fases == faseBC)
   {
   	switch(iFase)
      {
      case 2:
      case 3:
         break;

      case 1:
      	Fases = faseABC;
         NumFases = 3;
         break;

      default:
      	break;
      }
   }
   else if(Fases == faseCA)
   {
   	switch(iFase)
      {
      case 1:
      case 3:
         break;

      case 2:
      	Fases = faseABC;
         NumFases = 3;
         break;

      default:
      	break;
      }
   }
}
//---------------------------------------------------------------------------
String __fastcall TDSSLigacao::GetCodigo()
{
	return Codigo;
}
//---------------------------------------------------------------------------
int __fastcall TDSSLigacao::GetFases()
{
   return Fases;
}
//---------------------------------------------------------------------------
int __fastcall TDSSLigacao::GetIndice_VetorLigacoes()
{
	return Indice_VetorBarras;
}
//---------------------------------------------------------------------------
void __fastcall TDSSLigacao::SetCodigo(String Codigo)
{
	this->Codigo = Codigo;
}
//---------------------------------------------------------------------------
void __fastcall TDSSLigacao::SetFases(String strFasesBus1, String strFasesBus2)
{
	int comp;
	String strFases;
   String iFase, substr, codigo = "";

   // Verifica o conjunto menor de fases
   if(strFasesBus1.Length() < strFasesBus2.Length())
   {
      strFases = strFasesBus1;
   }
   else if(strFasesBus2.Length() < strFasesBus1.Length())
   {
      strFases = strFasesBus2;
   }
   else
   {
      strFases = strFasesBus1;
   }

   // Insere um "." ao final da string
   strFases += ".";

   // Obtém o comprimento da string
	comp = strFases.Length();
   for(int i=1; i<=comp; i++)
   {
   	// Procura o primeiro "." e pega o código da ligação
   	substr = strFases.SubString(i,1);
		if(substr == "." && codigo == "")
      {
			codigo = strFases.SubString(1,i-1);
      }
      // Pega os índices das fases
      else if(substr == "." && codigo != "")
      {
         iFase = strFases.SubString(i-1,1);
         AddFase(iFase.ToInt());
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TDSSLigacao::SetIndice_VetorLigacoes(int Indice_VetorLigacoes)
{
	this->Indice_VetorBarras = Indice_VetorBarras;
}
//---------------------------------------------------------------------------