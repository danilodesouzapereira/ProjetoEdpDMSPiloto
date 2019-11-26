//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Sinap\VTSinapMdi.h>
#include "TFormStkMdi.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TFormStkMdi::TFormStkMdi(TComponent *Owner, VTApl *apl_owner)
   : TForm(Owner)
	{
	//salva ponteiro para objeto
	this->apl = apl_owner;
	//configura MainMenu: inclui MenuItem para executar importador de rede
	ConfiguraMainMenu();
	//configura PopupMenuImporta: inclui MenuItem para executar importador de rede
	ConfiguraPopupMenuImporta();
	}

//---------------------------------------------------------------------------
__fastcall TFormStkMdi::~TFormStkMdi(void)
   {
	}

//---------------------------------------------------------------------------
void __fastcall TFormStkMdi::ActionImportaGisExecute(TObject *Sender)
	{
	//vari�veis locais
	VTSinapMdi *sinap_mdi = (VTSinapMdi*)apl->GetObject(__classid(VTSinapMdi));

	//prote��o
	if (sinap_mdi == NULL) return;
	//executa Action para criar um novo FormChild
	sinap_mdi->ActionNewChild->OnExecute((TObject*)1);
	}

//---------------------------------------------------------------------------
void __fastcall TFormStkMdi::ActionImportaANEELExecute(TObject *Sender)
	{
	//vari�veis locais
	VTSinapMdi *sinap_mdi = (VTSinapMdi*)apl->GetObject(__classid(VTSinapMdi));

	//prote��o
	if (sinap_mdi == NULL) return;
	//executa Action para criar um novo FormChild
	sinap_mdi->ActionNewChild->OnExecute((TObject*)2);
	}

//---------------------------------------------------------------------------
void __fastcall TFormStkMdi::ConfiguraMainMenu(void)
	{
	//vari�veis locais
	TAction    *Action;
	TMenuItem  *MenuItem, *MenuItemGis;
	VTSinapMdi *sinap_mdi = (VTSinapMdi*)apl->GetObject(__classid(VTSinapMdi));

	//prote��o
	if (sinap_mdi == NULL) return;
	//inclui MenuItem identificando integra��o com GIS
	MenuItemGis             = new TMenuItem(sinap_mdi->MainMenu);
	MenuItemGis->Action     = NULL;
	MenuItemGis->Hint       = "";
	MenuItemGis->Caption    = "STK: Enerq";
	MenuItemGis->GroupIndex = 0;
	MenuItemGis->RadioItem  = false;
	MenuItemGis->AutoCheck  = false;
	MenuItemGis->Checked    = false;
	sinap_mdi->MainMenu->Items->Insert(3, MenuItemGis);
	//inclui MenuItem para cada Action de ActionList
   for (int n = 0; n < ActionList->ActionCount; n++)
      {
      Action               = (TAction*)ActionList->Actions[n];
      //cria novo MenuItem
		MenuItem             = new TMenuItem(sinap_mdi->MainMenu);
      MenuItem->Action     = Action;
      MenuItem->Hint       = "";
      MenuItem->Caption    = Action->Caption;
      MenuItem->GroupIndex = 0;
      MenuItem->RadioItem  = false;
      MenuItem->AutoCheck  = false;
      MenuItem->Checked    = false;
		MenuItemGis->Add(MenuItem);
      }
   }

//---------------------------------------------------------------------------
void __fastcall TFormStkMdi::ConfiguraPopupMenuImporta(void)
	{
	//vari�veis locais
	TAction    *Action;
	TMenuItem  *MenuItem;
	VTSinapMdi *sinap_mdi = (VTSinapMdi*)apl->GetObject(__classid(VTSinapMdi));

	//prote��o
	if (sinap_mdi == NULL) return;
	//inclui MenuItem para cada Action de ActionList
   for (int n = 0; n < ActionList->ActionCount; n++)
      {
      Action               = (TAction*)ActionList->Actions[n];
      //cria novo MenuItem
		MenuItem             = new TMenuItem(sinap_mdi->PopupMenuImporta);
      MenuItem->Action     = Action;
      MenuItem->Hint       = "";
      MenuItem->Caption    = Action->Caption;
      MenuItem->GroupIndex = 0;
      MenuItem->RadioItem  = false;
      MenuItem->AutoCheck  = false;
      MenuItem->Checked    = false;
		sinap_mdi->PopupMenuImporta->Items->Add(MenuItem);
      }
   }

//---------------------------------------------------------------------------
//eof


