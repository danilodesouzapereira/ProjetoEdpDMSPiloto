//---------------------------------------------------------------------------
#pragma hdrstop
#include "TOpcoesGraficas.h"
#include "..\DSS\TFaultStudyFL.h"
#include "..\TThreadFaultLocation.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Grafico.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Grafico\VTGrafico.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
// ---------------------------------------------------------------------------
__fastcall TOpcoesGraficas::TOpcoesGraficas(VTApl* apl)
{
   // Salva parâmetros elementares
   this->apl = apl;
  	graf = (VTGrafico*) apl->GetObject(__classid(VTGrafico));
}
// ---------------------------------------------------------------------------
__fastcall TOpcoesGraficas::~TOpcoesGraficas()
{
   // nada a fazer
}
// ---------------------------------------------------------------------------
/**
 * Método para conferir aspecto visual ao resultado de localização
 */
void __fastcall TOpcoesGraficas::InsereMoldurasBarras(TList* lisBarras)
{
	//Aplica moldura às barras limites
	graf->Moldura();
	graf->Moldura(lisBarras);
	graf->Refresh();
}
// ---------------------------------------------------------------------------
/**
 * Método para conferir aspecto visual ao resultado de localização
 */
void __fastcall TOpcoesGraficas::InsereMoldurasBarrasFaultStudy(TList* lisSolucoes)
{
	TList* lisBarras = new TList();

   for(int i=0; i<lisSolucoes->Count; i++)
   {
     	strSolucao* solucao = (strSolucao*) lisSolucoes->Items[i];
      lisBarras->Add(solucao->barraSolucao);
   }

   if(lisBarras->Count > 0)
   {
   	//Aplica moldura às barras limites
      graf->Moldura();
      graf->Moldura(lisBarras);
      graf->Refresh();
   }

}
// ---------------------------------------------------------------------------
//eof
