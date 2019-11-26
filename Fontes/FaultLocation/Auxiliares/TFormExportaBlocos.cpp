//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TFormExportaBlocos.h"
#include "TFuncoesDeRede.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
//---------------------------------------------------------------------------
TFormExportaBlocos *FormExportaBlocos;
//---------------------------------------------------------------------------
__fastcall TFormExportaBlocos::TFormExportaBlocos(TComponent* Owner, VTApl* apl)
	: TForm(Owner)
{
	this->apl = apl;
   this->funcoesRede = new TFuncoesDeRede(apl);
   redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
   listaRedes = new TList();
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
   	VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->Carregada)
      	listaRedes->Add(rede);
   }
}
//---------------------------------------------------------------------------
void __fastcall TFormExportaBlocos::ActionExportaBlocosExecute(TObject *Sender)
{
	String pathFinal = "c:\\users\\user\\desktop\\blocos.txt";
   String pathFinalFusiveis = "c:\\users\\user\\desktop\\fusiveis.txt";
	TList* listaBlocos = new TList();
   TList* listaBarras = NULL;
   TList* listaChaves = NULL;
   TList* listaFusiveis = new TList();
   TStringList* linhasBlocos = new TStringList();
   TStringList* linhasFusiveis = new TStringList();
   String linha;

   for(int nr=0; nr<listaRedes->Count; nr++)
   {
   	VTRede* rede = (VTRede*) listaRedes->Items[nr];
      listaChaves = rede->LisLigacao();
      for(int i=0; i<listaChaves->Count; i++)
      {
       	VTLigacao* liga = (VTLigacao*) listaChaves->Items[i];
         if(liga->Tipo() != eqptoCHAVE) continue;

         VTChave* chave = (VTChave*) liga;

         if(!chave->TipoBaseFusivel) continue;

         listaFusiveis->Add(chave);
      }


      listaBlocos->Clear();
      funcoesRede->GetBlocosRede(rede, listaBlocos);
      linha = "";
      for(int i=0; i<listaBlocos->Count; i++)
      {
         VTBloco* bloco = (VTBloco*) listaBlocos->Items[i];
         listaBarras = bloco->LisBarra();

         linha = "";
         for(int j=0; j<listaBarras->Count; j++)
         {
            VTBarra* barra = (VTBarra*) listaBarras->Items[j];
            linha += barra->Codigo;
            if(j<listaBarras->Count-1) linha += ".";
         }
         linha = "Bloco.blo" + String(i+1) + " barras." + linha;
         linhasBlocos->Add(linha);
      }
   }

   // Analisa fusíveis
   linha = "";
   linhasFusiveis->Add("[INFO]");
   linhasFusiveis->Add("numfusiveis=" + String(listaFusiveis->Count));
   for(int i=0; i<listaFusiveis->Count; i++)
   {
      VTChave* chave = (VTChave*) listaFusiveis->Items[i];
		linhasFusiveis->Add("[Fusivel" + String(i+1) + "]");
		linhasFusiveis->Add("nome=" + chave->Codigo);
      linhasFusiveis->Add("barra1=" + chave->Barra(0)->Codigo);
      linhasFusiveis->Add("barra2=" + chave->Barra(1)->Codigo);
   }
   linhasFusiveis->SaveToFile(pathFinalFusiveis);

   linhasBlocos->SaveToFile(pathFinal);
   delete linhasBlocos;
   delete listaChaves;
   delete listaFusiveis;
}
//---------------------------------------------------------------------------
