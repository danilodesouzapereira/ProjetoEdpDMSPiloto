//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TFormModuloTestes.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Grafico\VTGrafico.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormModuloTestes *FormModuloTestes;
//---------------------------------------------------------------------------
__fastcall TFormModuloTestes::TFormModuloTestes(TComponent *Owner, VTApl *apl_owner, TWinControl *parent)
	: TForm(Owner)
{
	// Salva parâmetros
	apl = apl_owner;
	path = (VTPath*) apl->GetObject(__classid(VTPath));
	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	grafico = (VTGrafico*) apl->GetObject(__classid(VTGrafico));

	this->Parent = parent;
}
//---------------------------------------------------------------------------
__fastcall TFormModuloTestes::~TFormModuloTestes()
{

}
//---------------------------------------------------------------------------
void __fastcall TFormModuloTestes::btnSelecionaArquivoTrechosClick(TObject *Sender)
{
	String filePath = "";
	if(!OpenDialog1->Execute()) return;

	TStringList* linhasArquivo = new TStringList;
	TList* lisTrechos = new TList;

	filePath = OpenDialog1->FileName;
	linhasArquivo->LoadFromFile(filePath);
	LeTrechos(linhasArquivo, lisTrechos);

	grafico->DestacaEqpto(NULL, clBlue, 10);
	grafico->DestacaEqpto(lisTrechos, clBlue, 10);

	delete linhasArquivo;
	delete lisTrechos;
}
//---------------------------------------------------------------------------
void __fastcall TFormModuloTestes::LeTrechos(TStringList* lisLinhas, TList* lisEXT)
{
	if(!lisLinhas || !lisEXT) return;

	for(int i=0; i<lisLinhas->Count; i++)
	{
		String linha = lisLinhas->Strings[i];

		TList* lisRedes = redes->LisRede();
		VTTrecho* trecho = NULL;
		for(int j=0; j<lisRedes->Count; j++)
		{
			VTRede* rede = (VTRede*) lisRedes->Items[j];
			if(!rede->Carregada) continue;
			if(rede->TipoRede->Segmento != redePRI) continue;

			if(trecho = (VTTrecho*)rede->ExisteLigacao(eqptoTRECHO, linha))
				break;
			else
				trecho = NULL;
		}
		if(trecho) lisEXT->Add(trecho);
	}
}
//---------------------------------------------------------------------------
void __fastcall TFormModuloTestes::Button1Click(TObject *Sender)
{
	grafico->DestacaEqpto(NULL, clBlue, 10);
}
//---------------------------------------------------------------------------

