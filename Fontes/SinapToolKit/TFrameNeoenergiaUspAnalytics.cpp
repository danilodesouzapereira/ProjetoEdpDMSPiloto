//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include "TFrameNeoenergiaUspAnalytics.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <ProjetoNeoenergia\DLL_Inc\FaultLocationOffline.h>
#include <ModulosOpcionais\DLL_Inc\PerdaSensor.h>
#include <ProjetoNeoenergia\DLL_Inc\Analytics_QualiProt.h>
#include <ProjetoNeoenergia\DLL_Inc\Analytics_QualiManOtima.h>
#include <ProjetoNeoenergia\DLL_Inc\Analytics_QualiGeral.h>
#include <PlataformaSinap\Fontes\MultiMonitor\VTMultiPanel.h>
//---------------------------------------------------------------------------
TFrameNeoenergiaUspAnalytics *FrameNeoenergiaUspAnalytics;
//---------------------------------------------------------------------------
__fastcall TFrameNeoenergiaUspAnalytics::TFrameNeoenergiaUspAnalytics(TComponent* Owner, VTApl *apl_owner)
	: TFrame(Owner)
{
	//salva ponteiro
	apl = apl_owner;
}
//---------------------------------------------------------------------------
__fastcall TFrameNeoenergiaUspAnalytics::~TFrameNeoenergiaUspAnalytics()
{

}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergiaUspAnalytics::PM_SetAtivo(bool ativo)
{
	// nada a fazer
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergiaUspAnalytics::ActionFaultLocationOfflineExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormFaultLocationOffline(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergiaUspAnalytics::ActionPerdasExecute(TObject *Sender)

{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormPerdaSensor(this, apl, MultiPanel->PanelAtivo);
   Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergiaUspAnalytics::ActionMonitoraQualidadeExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormMonitoramentoQualiProt(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergiaUspAnalytics::ActionManobraOtimaExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormMonitoramentoQManOtima(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameNeoenergiaUspAnalytics::ActionGeralAnalyticsExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormMonitoramentoQGeral(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------

