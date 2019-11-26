//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <PlataformaSinap\Fontes\Licenca\VTLicenca.h>
#include <PlataformaSinap\Fontes\Licenca\VTLicencas.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\ImportaRedeGIS\VTImportaRedeGIS.h>
#include <PlataformaSinap\Fontes\Sinap\VTSinapChild.h>
#include <PlataformaSinap\DLL_Inc\ImportaRedeGIS.h>
#include <ModulosOpcionais\DLL_Inc\ImportaRedeAccess.h>
#include "TFormStkChild.h"
//---------------------------------------------------------------------------
#include "TFrameEdpUspDMSPiloto.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TFormStkChild::TFormStkChild(TComponent *Owner, VTApl *apl, int stk_option)
   : TForm(Owner)
{
	//variáveis locais
	bool         expand = false;
	VTSinapChild *sinap_child = (VTSinapChild*) apl->GetObject(__classid(VTSinapChild));
	// variáveis locais
	VTLicenca *licenca;
	VTLicencas *licencas = (VTLicencas*)apl->GetObject(__classid(VTLicencas));

   //salva ponteiro de objetos externos
   this->apl = apl;
	//-------------------
	//obtem dados da licenca
	licenca = licencas->ExisteLicenca();
	//trata caso de licença MASTER
	if(licenca != NULL)
	{//habilita importador Access
		if(licenca->Tipo == tipoMASTER)
			ActionImportaAccess->Visible = true;
		else
			ActionImportaAccess->Visible = false;
	}
	sinap_child->ActionRedeCarregada = ActionRedeCarregada;
	// verifica se foi definida alguma opção
	switch (stk_option)
	{
	case 1: // executa módulo de importação de rede do GIS
		ActionImportaRedeGIS->Execute();
		break;
	default: // nada a fazer
		break;
	}
   //-------------------
	// Cria frame Edp Usp - DMS Piloto
	FrameEdpUspDMSPiloto = new TFrameEdpUspDMSPiloto(this, apl);

	// Insere menu para ADA
	sinap_child->FormStkAdd("STK: EDP USP - DMS Piloto", FrameEdpUspDMSPiloto->ActionList, expand);
}
//---------------------------------------------------------------------------
__fastcall TFormStkChild::~TFormStkChild(void)
{
   //destrói objetos
	if (FrameEdpUspDMSPiloto) {delete FrameEdpUspDMSPiloto; FrameEdpUspDMSPiloto = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TFormStkChild::Atualiza(void)
{
	//nada a fazer
}
//---------------------------------------------------------------------------
//eof

void __fastcall TFormStkChild::ActionImportaAccessExecute(TObject *Sender)
{
	// variaveis locais
	VTImportaRedeGIS *ImportaRedeGIS;

	if ((ImportaRedeGIS = DLL_NewObjImportaRedeAccess(this, apl)) != NULL)
	{
		ImportaRedeGIS->ShowModalFormImportaRedeGIS();
	}

}
//---------------------------------------------------------------------------

void __fastcall TFormStkChild::ActionImportaRedeGISExecute(TObject *Sender)
{
 	// variáveis locais
	VTImportaRedeGIS *ImportaRedeGIS;

	if ((ImportaRedeGIS = DLL_NewObjImportaRedeGIS(this, apl)) != NULL)
	{
		ImportaRedeGIS->ShowModalFormImportaRedeGIS();
	}
	//conecta chaves vis e subestações mae/filha
	//ActionRedeCarregadaExecute(NULL);

}
//---------------------------------------------------------------------------

void __fastcall TFormStkChild::ActionRedeCarregadaExecute(TObject *Sender)
{
//   VTConecta *conecta;
//   VTConectaMaeFilha *conectaMaeFilha;
//
//   try
//   {//cria objeto para conectar as chaves de socorro
//	  conecta = NewObjConecta(apl);
//	  if (conecta != NULL)
//	  {
////		 conecta->Executa();
//		 //destrói objeto
//		 delete conecta;
//	  }
//      //cria objeto para conectar subestações mãe/filha
//	  conectaMaeFilha = NewObjConectaMaeFilha(apl);
//	  if (conectaMaeFilha != NULL)
//	  {
//		 conectaMaeFilha->Executa();
//		 //destrói objeto
//		 delete conectaMaeFilha;
//	  }
//   }
//   catch(Exception &e)
//		{//nada a fazer
//		}

}
//---------------------------------------------------------------------------

