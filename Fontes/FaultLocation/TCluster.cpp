//---------------------------------------------------------------------------
#pragma hdrstop
#include "TCluster.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
__fastcall TCluster::TCluster(VTChave* chaveMontante)
{
   lisBlocos = new TList();
   this->chaveMontante = chaveMontante;
}
//---------------------------------------------------------------------------
__fastcall TCluster::~TCluster()
{
	// Destroi objetos
	if(lisBlocos) {delete lisBlocos; lisBlocos = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TCluster::SetBloco(VTBloco* bloco)
{
	VTLigacao* liga;

   if(lisBlocos->IndexOf(bloco) < 0)
   	lisBlocos->Add(bloco);
}
//---------------------------------------------------------------------------
void __fastcall TCluster::SetBlocos(TList* lisBlocos)
{
	VTBloco* bloco;

   for(int j=0; j<lisBlocos->Count; j++)
   {
   	bloco = (VTBloco*) lisBlocos->Items[j];
		SetBloco(bloco);
   }
}
////---------------------------------------------------------------------------
//void __fastcall TCluster::SetLigacao(VTLigacao* ligacao)
//{
//	if(lisLigacoes->IndexOf(ligacao) < 0)
//   	lisLigacoes->Add(ligacao);
//}
////---------------------------------------------------------------------------
//void __fastcall TCluster::SetLigacoes(TList* lisLigacoes)
//{
//	this->lisLigacoes = lisLigacoes;
//}
////---------------------------------------------------------------------------
//TList* __fastcall TCluster::GetLigacoes()
//{
//	return lisLigacoes;
//}
//---------------------------------------------------------------------------
TList* __fastcall TCluster::GetBlocos()
{
	return lisBlocos;
}
//---------------------------------------------------------------------------
void __fastcall TCluster::ImprimeLigacoes(String pathArquivo)
{
	VTBloco* bloco;
   VTLigacao* ligacao;

	if(pathArquivo == "")
   	return;

   TStringList* LisCodLigacoes = new TStringList();

   for(int i=0; i<lisBlocos->Count; i++)
   {
   	bloco = (VTBloco*) lisBlocos->Items[i];
		TList* LisLigacao = bloco->LisLigacao();
		for(int j=0; j<LisLigacao->Count; j++)
      {
         ligacao = (VTLigacao*) LisLigacao->Items[j];
         LisCodLigacoes->Add(ligacao->Codigo);
      }
   }

   LisCodLigacoes->SaveToFile(pathArquivo);
   delete LisCodLigacoes;
}
//---------------------------------------------------------------------------
