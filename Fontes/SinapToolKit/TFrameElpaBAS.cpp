//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <Fontes\Apl\VTApl.h>
#include <Fontes\ImportaRedeGIS\VTImportaRedeGIS.h>
#include <DLL_Inc\ImportaRedeGIS.h>
#include <DLL_Inc\InterfaceANEEL.h>
#include <DLL_Inc\InterfaceODSS.h>
#include "TFrameElpaBAS.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TFrameElpaBAS::TFrameElpaBAS(TComponent* Owner, VTApl *apl_owner)
   : TFrame(Owner)
   {
   //salva ponteiro
   apl = apl_owner;
   }

//---------------------------------------------------------------------------
__fastcall TFrameElpaBAS::~TFrameElpaBAS(void)
   {
   //nada a fazer
   }

//---------------------------------------------------------------------------
void __fastcall TFrameElpaBAS::ActionExportaRedeANEELExecute(TObject *Sender)
	{
	//variáveis locais
	TForm *form;

	if ((form = DLL_NewFormExportaANEEL(this, apl)) != NULL)
		{
		form->ShowModal();
		}
	}

//---------------------------------------------------------------------------
void __fastcall TFrameElpaBAS::ActionExportaRedeOpenDSSExecute(TObject *Sender)
   {
   //variáveis locais
   TForm *form;

   //cria FormWsdl e exibe como Form modal
   form = DLL_NewFormExportaODSS(this, apl);
   if (form)
      {
      form->ShowModal();
      delete form;
      }
   }

//---------------------------------------------------------------------------
void __fastcall TFrameElpaBAS::ActionImportaRedeANEELExecute(TObject *Sender)
	{
	//variáveis locais
	TForm *form;

	if ((form = DLL_NewFormImportaANEEL(this, apl)) != NULL)
		{
		form->ShowModal();
		}
	}

//---------------------------------------------------------------------------
void __fastcall TFrameElpaBAS::ActionImportaRedeGISExecute(TObject *Sender)
	{
	//variáveis locais
	VTImportaRedeGIS *ImportaRedeGIS;

	if ((ImportaRedeGIS = DLL_NewObjImportaRedeGIS(this, apl)) != NULL)
		{
		ImportaRedeGIS->ShowModalFormImportaRedeGIS();
		}
	}

//---------------------------------------------------------------------------
void __fastcall TFrameElpaBAS::PM_SetAtivo(bool ativo)
	{
	//nada a fazer
	}

//---------------------------------------------------------------------------
//eof


