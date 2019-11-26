//---------------------------------------------------------------------------
#pragma hdrstop
#include "TGeraDefeito.h"
#include "..\DSS\TTryFault.h"
#include "Auxiliares\FuncoesFL.h"
#include <PlataformaSinap\DLL_Inc\Ordena.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Grafico\VTGrafico.h>
#include <PlataformaSinap\Fontes\Ordena\VTOrdena.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <System.StrUtils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TGeraDefeito::TGeraDefeito(VTApl* apl)
{
	this->apl = apl;
   redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
   path = (VTPath*)apl->GetObject(__classid(VTPath));

	strGeraDefeito = NULL;
   rede = NULL;
   barra = NULL;

   lisMedicoesBarras = new TStringList();
   lisMedicoesTrechos = new TStringList();
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::SetConfiguracoes(StrGeraDefeito* strGeraDefeito)
{
	this->strGeraDefeito = strGeraDefeito;
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::Executa()
{
	bool Inicial;
   double Rf;
	int IDbarra1;
   int patamar;
   String fasesFalta;

   InicializaParametros();

   tryFault = new TTryFault();  //< objeto para testes de defeitos utilizando o OpenDSS
   IniciaTryFault(strGeraDefeito->caminhoDSS);

   // Usa obj da classe TTryFault para testar a falta representada pelo indivíduo
   patamar = 1;
   IDbarra1 = barra->Id;
   fasesFalta = strGeraDefeito->TipoDefeito;
   Inicial = true;
   Rf = strGeraDefeito->Rfalta;
   tryFault->TestaCurtoRf_GeraDefeitos(patamar, IDbarra1, fasesFalta, Rf, Inicial);

   // Pega as tensões calculadas
	TStringList* lisAux_calcV = new TStringList();
   TStringList* lisAux_calcI = new TStringList();

   tryFault->GetLisCalcV(lisAux_calcV);
	tryFault->GetLisCalcI(lisAux_calcI);

   MostraResultadosMemo(lisAux_calcV, lisAux_calcI);
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::ExportarResultados(String pathTxtResultados)
{
	if(pathTxtResultados == "") return;

  	memoResultadosGeraDefeito->Lines->SaveToFile(pathTxtResultados + "\\Resultados.txt");
}
//---------------------------------------------------------------------------
int __fastcall TGeraDefeito::GetIDbarra(String CodigoChaveRef)
{
	VTLigacao* liga;

   liga = NULL;
	for(int i=0; i<rede->LisLigacao()->Count; i++)
   {
    	liga = (VTLigacao*) rede->LisLigacao()->Items[i];
      if(liga->Codigo.AnsiCompare(CodigoChaveRef) == 0)
      	break;
      else
      	liga = NULL;
   }
   if(liga == NULL) return -1;
   int IDbarra = liga->Barra(0)->Id;
   return (IDbarra);
}
//---------------------------------------------------------------------------
VTLigacao* __fastcall TGeraDefeito::GetLigacaoInicial(VTRede* rede)
{
	VTBarra* barraIni;
   VTLigacao* liga;
   VTOrdena* ordena = (VTOrdena*) apl->GetObject(__classid(VTOrdena));
   if(ordena == NULL)
   	apl->Add(ordena = DLL_NewObjOrdena(apl));
	ordena->Executa(redes);

	if(rede == NULL) return NULL;

   barraIni = rede->BarraInicial();
   liga = NULL;
   for(int i=0; i<rede->LisLigacao()->Count; i++)
   {
   	liga = (VTLigacao*) rede->LisLigacao()->Items[i];
		if(liga->Barra(0) == barraIni || liga->Barra(1) == barraIni)
      	break;
      else
      	liga = NULL;
   }

   if(liga->Tipo() != eqptoTRECHO)
   {
      for(int i=0; i<rede->LisLigacao()->Count; i++)
      {
         VTLigacao* ligaAux = (VTLigacao*) rede->LisLigacao()->Items[i];
         if(ligaAux->ligaPai == liga && ligaAux->Tipo() == eqptoTRECHO)
         {
            liga = ligaAux;
            break;
         }
      }
   }

   return (liga);
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::InicializaParametros()
{
	for(int i=0; i<redes->LisRede()->Count; i++)
   {
		rede = (VTRede*) redes->LisRede()->Items[i];
		if(ReplaceStr(rede->Codigo, "-", "") == ReplaceStr(strGeraDefeito->CodigoAlimentador, "-", ""))
      	break;
      else
      	rede = NULL;
   }
   if(rede == NULL) return;

   for(int i=0; i<rede->LisBarra()->Count; i++)
   {
		barra = (VTBarra*) rede->LisBarra()->Items[i];
		if(barra->Codigo.AnsiCompare(strGeraDefeito->CodigoBarra) == 0)
      	break;
      else
      	barra = NULL;
   }
   if(barra == NULL) return;
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::IniciaTryFault(String caminhoDSS)
{
   // Prepara o TryFault
	tryFault->SetCaminhoDSS(caminhoDSS);
	tryFault->IniciaBarrasTrechos();

   lisMedicoesBarras->Clear();
   lisMedicoesTrechos->Clear();

   // Insere código do trecho inicial do alimentador
   VTLigacao* ligaInicial = GetLigacaoInicial(rede);
   lisMedicoesTrechos->Add(ligaInicial->Codigo + ";0;0;0");

   // Pega os IDs das barras com medição de tensão (com qualímetros)
   TStringList* lisCadQualimetros = new TStringList();
   lisCadQualimetros->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv");
   for(int i=0; i<lisCadQualimetros->Count; i++)
   {
      String linha = lisCadQualimetros->Strings[i];
      String CodigoRede = ReplaceStr(GetCampoCSV(linha, 0, ";"), "-", "");
      if(ReplaceStr(rede->Codigo, "-", "") != CodigoRede) continue;

      String CodigoChaveRef = GetCampoCSV(linha, 2, ";");
      String IDbarraChave = String(GetIDbarra(CodigoChaveRef));
      lisMedicoesBarras->Add(IDbarraChave + ";0;0;0");
   }

   tryFault->SetMedicoesBarras(lisMedicoesBarras);
   tryFault->SetMedicoesTrechos(lisMedicoesTrechos);

   delete lisCadQualimetros; lisCadQualimetros = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::MostraResultadosMemo(TStringList* lisAux_calcV, TStringList* lisAux_calcI)
{
	String linhaFinal, mod, fase;

	if(lisAux_calcV == NULL || lisAux_calcI == NULL) return;

   memoResultadosGeraDefeito->Lines->Clear();
   for(int i=0; i<lisAux_calcV->Count; i++)
   {
      String linha = lisAux_calcV->Strings[i];
      String IDbarra = GetCampoCSV(lisMedicoesBarras->Strings[i],0,";");

      memoResultadosGeraDefeito->Lines->Add("Barra: " + IDbarra);

      mod = String(Round(GetCampoCSV(linha,0,";").ToDouble(), 2));
      fase = String(Round(GetCampoCSV(linha,1,";").ToDouble(), 2));
      memoResultadosGeraDefeito->Lines->Add(mod);
      memoResultadosGeraDefeito->Lines->Add(fase);

      mod = String(Round(GetCampoCSV(linha,2,";").ToDouble(), 2));
      fase = String(Round(GetCampoCSV(linha,3,";").ToDouble(), 2));
      memoResultadosGeraDefeito->Lines->Add(mod);
      memoResultadosGeraDefeito->Lines->Add(fase);

      mod = String(Round(GetCampoCSV(linha,4,";").ToDouble(), 2));
      fase = String(Round(GetCampoCSV(linha,5,";").ToDouble(), 2));
      memoResultadosGeraDefeito->Lines->Add(mod);
      memoResultadosGeraDefeito->Lines->Add(fase);
   }

   for(int i=0; i<lisAux_calcI->Count; i++)
   {
      String linha = lisAux_calcI->Strings[i];
      String CodigoTrecho = GetCampoCSV(lisMedicoesTrechos->Strings[i],0,";");

      memoResultadosGeraDefeito->Lines->Add("Trecho: " + CodigoTrecho);

      mod = String(Round(GetCampoCSV(linha,0,";").ToDouble(), 2));
      fase = String(Round(GetCampoCSV(linha,1,";").ToDouble(), 2));
      memoResultadosGeraDefeito->Lines->Add(mod);
      memoResultadosGeraDefeito->Lines->Add(fase);

      mod = String(Round(GetCampoCSV(linha,2,";").ToDouble(), 2));
      fase = String(Round(GetCampoCSV(linha,3,";").ToDouble(), 2));
      memoResultadosGeraDefeito->Lines->Add(mod);
      memoResultadosGeraDefeito->Lines->Add(fase);

      mod = String(Round(GetCampoCSV(linha,4,";").ToDouble(), 2));
      fase = String(Round(GetCampoCSV(linha,5,";").ToDouble(), 2));
      memoResultadosGeraDefeito->Lines->Add(mod);
      memoResultadosGeraDefeito->Lines->Add(fase);
   }
}
//---------------------------------------------------------------------------
void __fastcall TGeraDefeito::SetMemo(TMemo* memoResultadosGeraDefeito)
{
   this->memoResultadosGeraDefeito = memoResultadosGeraDefeito;
}
//---------------------------------------------------------------------------
