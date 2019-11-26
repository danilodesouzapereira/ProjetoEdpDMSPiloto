//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TFormPtosEquidistantes.h"
#include "Auxiliares\TFuncoesDeRede.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
//---------------------------------------------------------------------------
TFormPtosEquidistantes *FormPtosEquidistantes;
//---------------------------------------------------------------------------
__fastcall TFormPtosEquidistantes::TFormPtosEquidistantes(TComponent* Owner, VTApl* apl)
	: TForm(Owner)
{
	this->apl = apl;
}
//---------------------------------------------------------------------------
void __fastcall TFormPtosEquidistantes::ActionCalcularExecute(TObject *Sender)
{
//   double distancia;
//	String codBarra;
//	TFuncoesDeRede* funcRede = new TFuncoesDeRede(apl);
//   TList* listaCaminhos = new TList();
//   TList* listaBarrasTerm = new TList();
//   VTBarra* barraTerm;
//	VTLigacao* ligaTerm;
//
// 	// Obtém os dados
//   codBarra = EditCodBarraIni->Text;
//   distancia = EditDistancia->Text.ToDouble();
//	// Executa o algoritmo
//   funcRede->GetCaminhosDistancia(codBarra, distancia, listaCaminhos);
//
//   // Proteção
//   if(listaCaminhos->Count == 0) return;
//
//   // Limpa o Memo
//   MemoBarrasFinais->Lines->Clear();
//
//	// Salva as barras terminais
//   for(int i=0; i<listaCaminhos->Count; i++)
//   {
//    	TList* caminho = (TList*) listaCaminhos->Items[i];
//		ligaTerm = (VTLigacao*) caminho->Items[caminho->Count-1];
//      barraTerm = ligaTerm->Barra(1);
//      listaBarrasTerm->Add(barraTerm);
//      // Adiciona ao Memo
//      MemoBarrasFinais->Lines->Add(barraTerm->Codigo);
//   }


   double distancia;
	String codBarra;
	TFuncoesDeRede* funcRede = new TFuncoesDeRede(apl);
   TList* listaCaminhos = new TList();
   TList* listaBarrasTerm = new TList();
   VTBarra* barraTerm;
	VTLigacao* ligaTerm;

 	// Obtém os dados
   codBarra = EditCodBarraIni->Text;
   distancia = EditDistancia->Text.ToDouble();
	// Executa o algoritmo
//   funcRede->GetCaminhosDistancia(codBarra, distancia, listaCaminhos);
   funcRede->GetBarrasDistancia(codBarra, distancia, listaBarrasTerm);

   // Proteção
   if(listaBarrasTerm->Count == 0) return;

   // Limpa o Memo
   MemoBarrasFinais->Lines->Clear();

	// Salva as barras terminais
   for(int i=0; i<listaBarrasTerm->Count; i++)
   {
    	VTBarra* barra = (VTBarra*) listaBarrasTerm->Items[i];
      // Adiciona ao Memo
      MemoBarrasFinais->Lines->Add(barra->Codigo);
   }
}
//---------------------------------------------------------------------------
