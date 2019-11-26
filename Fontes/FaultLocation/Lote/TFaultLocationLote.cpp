//---------------------------------------------------------------------------
#pragma hdrstop
#include "..\Auxiliares\FuncoesFL.h"
#include "TFaultLocationLote.h"
#include "TEvento.h"
#include "TEventoCGP.h"
#include "TEventoOMS.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
//---------------------------------------------------------------------------
__fastcall TFaultLocationLote::TFaultLocationLote(VTApl* apl)
{
    // Salva parâmetros
    this->apl = apl;
    path = (VTPath*) apl->GetObject(__classid(VTPath));

    // Inicializa listas
    lisEventos = new TList();
    lisEventosCGP = new TList();
    lisEventosOMS = new TList();
    lisLinhasEventosCGP = new TStringList();
    lisLinhasEventosOMS = new TStringList();
}
//---------------------------------------------------------------------------
__fastcall TFaultLocationLote::~TFaultLocationLote()
{
	// Destroi objetos
   if(lisEventos) {delete lisEventos; lisEventos = NULL;}
   if(lisEventosOMS) {delete lisEventosOMS; lisEventosOMS = NULL;}
   if(lisEventosCGP) {delete lisEventosCGP; lisEventosCGP = NULL;}
   if(lisLinhasEventosCGP) {delete lisLinhasEventosCGP; lisLinhasEventosCGP = NULL;}
   if(lisLinhasEventosOMS) {delete lisLinhasEventosOMS; lisLinhasEventosOMS = NULL;}
}
//---------------------------------------------------------------------------
/***
 * 02 - Método para ler os arquivos CSV com os eventos
 */
void __fastcall TFaultLocationLote::CarregarArquivos()
{
	String pathDirLFLote;

   // Caminho do diretório
   pathDirLFLote = path->DirImporta() + "\\LFLote";

   // Carrega linhas dos arquivos CSV em TStringLists
   lisLinhasEventosOMS->LoadFromFile(pathDirLFLote + "\\EventosOMS.csv");
   lisLinhasEventosCGP->LoadFromFile(pathDirLFLote + "\\EventosCGP.csv");
}
//---------------------------------------------------------------------------
/***
 * 01 - Método principal, chamado a partir do form
 */
void __fastcall TFaultLocationLote::CarregarDados()
{
	CarregarArquivos();

   ParseEventos();



//   //debug
//   for(int i=0; i<lisEventosCGP->Count; i++)
//   {
//   	TEventoCGP* ev = (TEventoCGP*) lisEventosCGP->Items[i];
//      int a =0;
//   }
}
//---------------------------------------------------------------------------
/***
 * 03 - Gera objetos de eventos do CGP e do OMS (GSE Operação)
 */
void __fastcall TFaultLocationLote::ParseEventos()
{
	int nCampos;
	String linha, valorCampo;
   TEventoCGP* evCGP;

	// Leitura dos eventos do CGP
   for(int i=0; i<lisLinhasEventosCGP->Count; i++)
   {
		linha =  lisLinhasEventosCGP->Strings[i];

      // Cria obj de evento do CGP
      evCGP = new TEventoCGP();

		valorCampo = GetCampoCSV(linha, 2, ";");
      evCGP->SetRelayID(valorCampo);
		valorCampo = GetCampoCSV(linha, 7, ";");
      evCGP->SetEventTime(valorCampo);
		valorCampo = GetCampoCSV(linha, 8, ";");
      evCGP->SetEventDate(valorCampo);
		valorCampo = GetCampoCSV(linha, 10, ";");
      evCGP->SetEventType(valorCampo);
		valorCampo = GetCampoCSV(linha, 12, ";");
      evCGP->SetEventLocation(valorCampo);

      // Valores das correntes
		valorCampo = GetCampoCSV(linha, 16, ";");
      evCGP->SetIAFault(valorCampo);
		valorCampo = GetCampoCSV(linha, 17, ";");
      evCGP->SetIBFault(valorCampo);
		valorCampo = GetCampoCSV(linha, 18, ";");
      evCGP->SetICFault(valorCampo);
		valorCampo = GetCampoCSV(linha, 21, ";");
      evCGP->SetINFault(valorCampo);

      // Salva em lista
      lisEventosCGP->Add(evCGP);
   }
}
//---------------------------------------------------------------------------
