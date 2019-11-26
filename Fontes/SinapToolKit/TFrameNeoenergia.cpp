//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include "TFrameNeoenergia.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <ProjetoNeoenergia\DLL_Inc\FaultLocation.h>
#include <ProjetoNeoEnergia\DLL_Inc\VoltVAr.h>
#include <ProjetoNeoEnergia\DLL_Inc\AjustesParametros.h>
#include <ProjetoNeoenergia\DLL_Inc\DSSDirect.h>
#include <PlataformaSinap\Fontes\MultiMonitor\VTMultiPanel.h>
//---------------------------------------------------------------------------
__fastcall TFrameNeoenergia::TFrameNeoenergia(TComponent* Owner, VTApl *apl_owner)
	: TFrame(Owner)
{
   //salva ponteiro
   apl = apl_owner;
}
//---------------------------------------------------------------------------
__fastcall TFrameNeoenergia::~TFrameNeoenergia(void)
{
	// nada a fazer
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergia::PM_SetAtivo(bool ativo)
{
	// nada a fazer
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergia::ActionFaultLocationExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormFaultLocation(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergia::ActionVoltVarExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormVoltVAr(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergia::ActionAjustesParametrosExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormAjustesParametros(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------


