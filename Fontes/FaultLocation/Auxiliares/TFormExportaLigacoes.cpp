//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TFormExportaLigacoes.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Ordena.h>
#include <PlataformaSinap\DLL_Inc\Radial.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Ordena\VTOrdena.h>
#include <PlataformaSinap\Fontes\Radial\VTPrimario.h>
#include <PlataformaSinap\Fontes\Radial\VTRadial.h>
#include <PlataformaSinap\Fontes\Radial\VTTronco.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
//---------------------------------------------------------------------------
TFormExportaLigacoes *FormExportaLigacoes;
//---------------------------------------------------------------------------
__fastcall TFormExportaLigacoes::TFormExportaLigacoes(TComponent* Owner, VTApl* apl)
	: TForm(Owner)
{
	this->apl = apl;
   redes = (VTRedes*) apl->GetObject(__classid(VTRedes));

  	ordena = (VTOrdena*) apl->GetObject(__classid(VTOrdena));
   if(ordena == NULL)
   {
     	ordena = DLL_NewObjOrdena(apl);
   }

  	this->radial = DLL_NewObjRadial(this->apl);
	this->radial->Inicia(this->redes);

   this->tronco = DLL_NewObjTronco(apl);
}
//---------------------------------------------------------------------------
void __fastcall TFormExportaLigacoes::Button1Click(TObject *Sender)
{
   TList* lisRedes = NULL;
   TList* lisLigacoes = NULL;
   TList* lisLigaOrd = new TList();
	VTRede* rede = NULL;

	// rede MT
   lisRedes = redes->LisRede();
   for(int i=0; i<lisRedes->Count; i++)
   {
      rede = (VTRede*) lisRedes->Items[i];
      if(rede->Codigo == "CMC-01P1")
      {
         break;
      }
      else
      {
         rede = NULL;
      }
   }

   if(rede == NULL) return;

   ordena->Executa(rede);

   lisLigacoes = rede->LisLigacao();

   TList* lisPrimarios = radial->LisPrimario();
   VTPrimario* prim = NULL;
   for(int i=0; i<lisPrimarios->Count; i++)
   {
   	prim =  (VTPrimario*) lisPrimarios->Items[i];
      if(prim->Rede == rede) break;
      prim = NULL;
   }

   // proteção
  	if(prim == NULL) return;


   // lista de ligações do primário
   tronco->DefinePrimario(prim, 0);
   TList* lisLigaTronco = tronco->LisLigacaoTronco();
   TList* lisLigaRamal = tronco->LisLigacaoRamal();


   TStringList* lisCodigos = new TStringList();
   for(int i=0; i<lisLigaTronco->Count; i++)
   {
		VTLigacao* liga = (VTLigacao*) lisLigaTronco->Items[i];
      lisCodigos->Add(liga->Codigo);
   }
   lisCodigos->Add("");
   for(int i=0; i<lisLigaRamal->Count; i++)
   {
		VTLigacao* liga = (VTLigacao*) lisLigaRamal->Items[i];
      lisCodigos->Add(liga->Codigo);
   }
   String caminho = "c:\\users\\user\\desktop\\ligacoes2.txt";
	lisCodigos->SaveToFile(caminho);
}
//---------------------------------------------------------------------------
