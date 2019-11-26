//---------------------------------------------------------------------------
#pragma hdrstop
#include "TConfigRede.h"
#include "..\Auxiliares\FuncoesFL.h"
#include <complex>
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTSuprimento.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TConfigRede::TConfigRede(VTApl* apl)
{
	this->apl = apl;
   path = (VTPath*) apl->GetObject(__classid(VTPath));
	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));

   // Inicializações
   lisPotenciasCurtoCircuito = new TList();
   lisParamTrafosSE = new TList();
}
//---------------------------------------------------------------------------
__fastcall TConfigRede::~TConfigRede()
{
	// Destroi listas
   for(int i=0; i<lisPotenciasCurtoCircuito->Count; i++)
   {
   	StrPotCurtoCircuito* potCC =  (StrPotCurtoCircuito*) lisPotenciasCurtoCircuito->Items[i];
      delete potCC;
   }
  	if(lisPotenciasCurtoCircuito) {delete lisPotenciasCurtoCircuito; lisPotenciasCurtoCircuito = NULL;}
   for(int i=0; i<lisParamTrafosSE->Count; i++)
   {
   	StrParamTrafosSE* paramTrafo =  (StrParamTrafosSE*) lisParamTrafosSE->Items[i];
      delete paramTrafo;
   }
  	if(lisParamTrafosSE) {delete lisParamTrafosSE; lisParamTrafosSE = NULL;}
}
//---------------------------------------------------------------------------
// Abre arquivo de pot. de curto circuito e gera structs associadas, que são salvas
// em lista: lisPotenciasCurtoCircuito.
bool __fastcall TConfigRede::AbreArquivoPotCurtoCircuito(String pathArquivoSuprimento)
{
	String pathArquivo, valor, tipoInformacao;
   String CodigoSE, NivelTensao;
   TStringList* lisArquivo;
   StrPotCurtoCircuito* potCC;
   std::complex<double> scc_3f_mva, scc_ft_mva, scc_3f_pu, scc_ft_pu, z0_pu, z1_pu;
   VTSuprimento* sup;

   try
   {
//      if(pathArquivoSuprimento == "")
//	      pathArquivo = path->DirDat() + "\\FaultLocation\\ConfigRede\\PotenciasCurtoCircuito.csv";
//      else
//      	pathArquivo = pathArquivoSuprimento;

		pathArquivo = pathArquivoSuprimento;

      lisArquivo = new TStringList();
      lisArquivo->LoadFromFile(pathArquivo);

      for(int i=0; i<lisArquivo->Count; i++)
      {
         String linha = lisArquivo->Strings[i];

         CodigoSE = GetCampoCSV(linha, 0, ";");
         NivelTensao = GetCampoCSV(linha, 1, ";");

         sup = GetSuprimento(NivelTensao, CodigoSE);
         if(sup == NULL) continue;

         potCC = new StrPotCurtoCircuito();
         potCC->sup = sup;
         potCC->CodigoSE = CodigoSE;
         potCC->NivelTensao = CodigoSE;

         tipoInformacao = GetCampoCSV(linha, 2, ";");
         if(tipoInformacao == "PCC")
         {
            potCC->pcc_ft = GetCampoCSV(linha, 3, ";").ToDouble();
            potCC->qcc_ft = GetCampoCSV(linha, 4, ";").ToDouble();
            potCC->pcc_3f = GetCampoCSV(linha, 5, ";").ToDouble();
            potCC->qcc_3f = GetCampoCSV(linha, 6, ";").ToDouble();

            // Converte valores de potências de curto-circuito para impedânicas equivalentes
            scc_3f_pu = std::complex<double> (potCC->pcc_3f, potCC->qcc_3f) / 100.;
            scc_ft_pu = std::complex<double> (potCC->pcc_ft, potCC->qcc_ft) / 100.;
            z0_pu     = (3. / conj(scc_ft_pu)) - (2. / conj(scc_3f_pu));
            z1_pu     = (1. / conj(scc_3f_pu));

            potCC->r0 = z0_pu.real();
            potCC->x0 = z0_pu.imag();
            potCC->r1 = z1_pu.real();
            potCC->x1 = z1_pu.imag();
         }
         else if(tipoInformacao == "ZCC")
         {
            potCC->r0 = GetCampoCSV(linha, 3, ";").ToDouble();
            potCC->x0 = GetCampoCSV(linha, 4, ";").ToDouble();
            potCC->r1 = GetCampoCSV(linha, 5, ";").ToDouble();
            potCC->x1 = GetCampoCSV(linha, 6, ";").ToDouble();

            // Converte valores de impedânicas equivalentes para potências de curto-circuito
            z0_pu = std::complex<double>(potCC->r0, potCC->x0);
            z1_pu = std::complex<double>(potCC->r1, potCC->x1);
            scc_3f_mva = (1. / conj(z1_pu)) * 100.;
            scc_ft_mva = (3. / ((2. * conj(z1_pu)) + conj(z0_pu))) * 100.;

            potCC->pcc_3f = scc_3f_mva.real();
            potCC->qcc_3f = scc_3f_mva.imag();
            potCC->pcc_ft = scc_ft_mva.real();
            potCC->qcc_ft = scc_ft_mva.imag();
         }
         lisPotenciasCurtoCircuito->Add(potCC);
      }
      delete lisArquivo; lisArquivo = NULL;
   }
   catch(Exception &e)
   {
		return (false);
   }

   return (true);
}
//---------------------------------------------------------------------------
// Abre arquivo de parâmetros de trafos SE e gera structs associadas, que são salvas
// em lista: lisParamTrafosSE.
bool __fastcall TConfigRede::AbreArquivoParamTrafosSE(String pathArquivoParamTrafosSE)
{
	String pathArquivo, valor, PotenciaBase;
   String CodigoSE, CodigoTrafoSE;
   TStringList* lisArquivo;
   StrParamTrafosSE* paramTrafo;
   std::complex<double> scc_3f_mva, scc_ft_mva, scc_3f_pu, scc_ft_pu, z0_pu, z1_pu;
   VTTrafo* trafoSE;

   try
   {
//      if(pathArquivoParamTrafosSE == "")
//			pathArquivo = path->DirDat() + "\\FaultLocation\\ConfigRede\\ParametrosTrafosSE.csv";
//      else
//         pathArquivo = pathArquivoParamTrafosSE;

      pathArquivo = pathArquivoParamTrafosSE;

      lisArquivo = new TStringList();
      lisArquivo->LoadFromFile(pathArquivo);

      for(int i=0; i<lisArquivo->Count; i++)
      {
         String linha = lisArquivo->Strings[i];

         CodigoSE = GetCampoCSV(linha, 0, ";");
         CodigoTrafoSE = GetCampoCSV(linha, 1, ";");

         trafoSE = GetTrafoSE(CodigoSE, CodigoTrafoSE);
         if(trafoSE == NULL) continue;

         paramTrafo = new StrParamTrafosSE();
         paramTrafo->trafoSE = trafoSE;
         paramTrafo->CodigoSE = CodigoSE;
         paramTrafo->CodigoTrafoSE = CodigoTrafoSE;
         paramTrafo->Snom = GetCampoCSV(linha, 2, ";").ToDouble();
         if(paramTrafo->Snom == 0.) continue;

         PotenciaBase = GetCampoCSV(linha, 3, ";");
         if(PotenciaBase == "BaseTrafo")
         { // Impedâncias em pu na base do trafo

            paramTrafo->r0_baseTrafo = GetCampoCSV(linha, 4, ";").ToDouble();
            paramTrafo->x0_baseTrafo = GetCampoCSV(linha, 5, ";").ToDouble();
            paramTrafo->r1_baseTrafo = GetCampoCSV(linha, 6, ";").ToDouble();
            paramTrafo->x1_baseTrafo = GetCampoCSV(linha, 7, ";").ToDouble();

            paramTrafo->r0 = paramTrafo->r0_baseTrafo * 100. / paramTrafo->Snom;
            paramTrafo->x0 = paramTrafo->x0_baseTrafo * 100. / paramTrafo->Snom;
            paramTrafo->r1 = paramTrafo->r1_baseTrafo * 100. / paramTrafo->Snom;
            paramTrafo->x1 = paramTrafo->x1_baseTrafo * 100. / paramTrafo->Snom;
         }
         else if(PotenciaBase == "Base100MVA")
         {  // Impedâncias em pu na base 100 MVA
            paramTrafo->r0 = GetCampoCSV(linha, 4, ";").ToDouble();
            paramTrafo->x0 = GetCampoCSV(linha, 5, ";").ToDouble();
            paramTrafo->r1 = GetCampoCSV(linha, 6, ";").ToDouble();
            paramTrafo->x1 = GetCampoCSV(linha, 7, ";").ToDouble();

            paramTrafo->r0_baseTrafo = paramTrafo->r0 * paramTrafo->Snom / 100.;
            paramTrafo->x0_baseTrafo = paramTrafo->x0 * paramTrafo->Snom / 100.;
            paramTrafo->r1_baseTrafo = paramTrafo->r1 * paramTrafo->Snom / 100.;
            paramTrafo->x1_baseTrafo = paramTrafo->x1 * paramTrafo->Snom / 100.;
         }

         lisParamTrafosSE->Add(paramTrafo);
      }
      delete lisArquivo; lisArquivo = NULL;
   }
   catch(Exception &e)
   {
		return (false);
   }

   return (true);
}
//---------------------------------------------------------------------------
void __fastcall TConfigRede::AlteraPotenciasCurtoCircuito()
{
   std::complex<double> scc_3f_pu, scc_ft_pu, z0_pu, z1_pu;
   VTSuprimento* sup;

   for(int i=0; i<lisPotenciasCurtoCircuito->Count; i++)
   {
    	StrPotCurtoCircuito* potCC =  (StrPotCurtoCircuito*) lisPotenciasCurtoCircuito->Items[i];
		sup = potCC->sup;
      if(sup == NULL) continue;

		// Atualiza potências de curto-circuito do suprimento
      sup->pcc_ft.p = potCC->pcc_ft;
      sup->pcc_ft.q = potCC->qcc_ft;
		sup->pcc_3f.p = potCC->pcc_3f;
      sup->pcc_3f.q = potCC->qcc_3f;

      // Atualiza imp.equivalente do suprimento
      sup->zeq0.r = z0_pu.real();
      sup->zeq0.x = z0_pu.imag();
      sup->zeq1.r = z1_pu.real();
      sup->zeq1.x = z1_pu.imag();
   }
}
//---------------------------------------------------------------------------
void __fastcall TConfigRede::AlteraParametrosTrafosSE()
{
   VTTrafo* trafoSE;

   for(int i=0; i<lisParamTrafosSE->Count; i++)
   {
    	StrParamTrafosSE* paramTrafo =  (StrParamTrafosSE*) lisParamTrafosSE->Items[i];

		trafoSE = paramTrafo->trafoSE;
      if(trafoSE == NULL) continue;

		// Atualiza os parâmetros do trafo SE
      trafoSE->z0.r = paramTrafo->r0_baseTrafo;
      trafoSE->z0.x = paramTrafo->x0_baseTrafo;;
      trafoSE->z1.r = paramTrafo->r1_baseTrafo;;
      trafoSE->z1.x = paramTrafo->x1_baseTrafo;;
   }
}
//---------------------------------------------------------------------------
VTSuprimento* __fastcall TConfigRede::GetSuprimento(String NivelTensao, String CodigoRede)
{
	TList* lisSup = NULL;
	VTRede* rede = NULL;
   VTSuprimento* sup = NULL;

	if(NivelTensao == "" || CodigoRede == "") return NULL;

   // Determina a rede em questão
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
      rede = (VTRede*) redes->LisRede()->Items[i];
      if((NivelTensao == "AT" && (rede->TipoRede->Segmento == redePRI || rede->TipoRede->Segmento == redeSEC)) ||
         (NivelTensao == "MT" && rede->TipoRede->Segmento != redePRI))
      {
         rede = NULL;
         continue;
      }

      if(rede->Codigo == CodigoRede)
         break;
      else
         rede = NULL;
   }
   if(!rede) return NULL;

   // Proteção
   if(rede->LisBarra()->Count == 0) return NULL;

   // Busca o suprimento da rede
   lisSup = new TList();

   // Verifica a barra inicial
   VTBarra* barraIni = rede->BarraInicial();
   lisSup->Clear();
	barraIni->LisEqbar(lisSup, eqptoSUPRIMENTO);
   if(lisSup->Count > 0)
   	sup = (VTSuprimento*) lisSup->Items[0];


	// Destroi lista
   if(lisSup) {delete lisSup; lisSup = NULL;}

   return sup;
}
//---------------------------------------------------------------------------
VTTrafo* __fastcall TConfigRede::GetTrafoSE(String CodigoSE, String CodigoTrafoSE)
{
	VTRede* rede = NULL;
	VTTrafo* trafo = NULL;

	if(CodigoTrafoSE == "") return NULL;

   // Determina a rede em questão
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
      rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento == redePRI || rede->TipoRede->Segmento == redeSEC)
      {
         rede = NULL;
         continue;
      }

      if(rede->Codigo == CodigoSE)
         break;
      else
         rede = NULL;
   }
   if(!rede) return NULL;

   // Obtém o trafo de SE desejado
   TList* lisLigacao = rede->LisLigacao();
   for(int i=0; i<lisLigacao->Count; i++)
   {
		VTLigacao* liga = (VTLigacao*) lisLigacao->Items[i];
      if(liga->Tipo() != eqptoTRAFO) continue;

      trafo = (VTTrafo*) liga;
      if(trafo->Codigo.AnsiCompare(CodigoTrafoSE) == 0)
      	break;
      else
      	trafo = NULL;
   }
   if(trafo == NULL) return NULL;

   return (trafo);
}
//---------------------------------------------------------------------------
void __fastcall TConfigRede::PopulaListViewPotCurto(TListView* listview, String tipoExibicao)
{
	String CodigoSE;
   TListItem* item;

   if(!listview) return;

   for(int i=0; i<lisPotenciasCurtoCircuito->Count; i++)
   {
    	StrPotCurtoCircuito* potCC =  (StrPotCurtoCircuito*) lisPotenciasCurtoCircuito->Items[i];
      CodigoSE = potCC->CodigoSE;

      item = listview->Items->Add();
      item->Data = potCC->sup;
      item->Caption = "";
      item->SubItems->Add(potCC->CodigoSE);

      if(tipoExibicao == "PCC")
      {
         item->SubItems->Add(Round(potCC->pcc_3f, 2));
         item->SubItems->Add(Round(potCC->qcc_3f, 2));
         item->SubItems->Add(Round(potCC->pcc_ft, 2));
         item->SubItems->Add(Round(potCC->qcc_ft, 2));
      }
      else if(tipoExibicao == "ZCC")
      {
         item->SubItems->Add(Round(potCC->r0, 4));
         item->SubItems->Add(Round(potCC->x0, 4));
         item->SubItems->Add(Round(potCC->r1, 4));
         item->SubItems->Add(Round(potCC->x1, 4));
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TConfigRede::PopulaListViewParamTrafosSE(TListView* listview, String opcao)
{
   TListItem* item;

   if(!listview) return;

   for(int i=0; i<lisParamTrafosSE->Count; i++)
   {
    	StrParamTrafosSE* paramTrafo =  (StrParamTrafosSE*) lisParamTrafosSE->Items[i];

      item = listview->Items->Add();

      item->Data = paramTrafo->trafoSE;
      item->Caption = "";
      item->SubItems->Add(paramTrafo->CodigoSE);
      item->SubItems->Add(paramTrafo->CodigoTrafoSE);
      item->SubItems->Add(Round(paramTrafo->Snom, 1));

      if(opcao == "BaseTrafo")
      {
         item->SubItems->Add(Round(paramTrafo->r0_baseTrafo, 4));
         item->SubItems->Add(Round(paramTrafo->x0_baseTrafo, 4));
         item->SubItems->Add(Round(paramTrafo->r1_baseTrafo, 4));
         item->SubItems->Add(Round(paramTrafo->x1_baseTrafo, 4));
      }
      else if(opcao == "Base100MVA")
      {
         item->SubItems->Add(Round(paramTrafo->r0, 4));
         item->SubItems->Add(Round(paramTrafo->x0, 4));
         item->SubItems->Add(Round(paramTrafo->r1, 4));
         item->SubItems->Add(Round(paramTrafo->x1, 4));
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TConfigRede::Resseta()
{
	// Destroi objetos
   for(int i=0; i<lisPotenciasCurtoCircuito->Count; i++)
   {
   	StrPotCurtoCircuito* potCC =  (StrPotCurtoCircuito*) lisPotenciasCurtoCircuito->Items[i];
      delete potCC;
   }
   for(int i=0; i<lisParamTrafosSE->Count; i++)
   {
   	StrParamTrafosSE* paramTrafo =  (StrParamTrafosSE*) lisParamTrafosSE->Items[i];
      delete paramTrafo;
   }

   // Limpas listas
   lisPotenciasCurtoCircuito->Clear();
   lisParamTrafosSE->Clear();
}
//---------------------------------------------------------------------------


