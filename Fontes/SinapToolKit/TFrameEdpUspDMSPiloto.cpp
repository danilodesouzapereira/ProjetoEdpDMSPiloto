//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include "TFrameEdpUspDMSPiloto.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <ProjetoEdpDMSPiloto\DLL_Inc\FaultLocationDMSPiloto.h>
#include <ProjetoEdpDMSPiloto\DLL_Inc\ModuloTestes.h>
#include <PlataformaSinap\Fontes\MultiMonitor\VTMultiPanel.h>
//---------------------------------------------------------------------------
TFrameEdpUspDMSPiloto *FrameEdpUspDMSPiloto;
//---------------------------------------------------------------------------
__fastcall TFrameEdpUspDMSPiloto::TFrameEdpUspDMSPiloto(TComponent* Owner, VTApl *apl_owner)
	: TFrame(Owner)
{
	//salva ponteiro
	apl = apl_owner;
}
//---------------------------------------------------------------------------
__fastcall TFrameEdpUspDMSPiloto::~TFrameEdpUspDMSPiloto(void)
{
	// nada a fazer
}
//---------------------------------------------------------------------------
void __fastcall TFrameEdpUspDMSPiloto::PM_SetAtivo(bool ativo)
{
	// nada a fazer
}
//---------------------------------------------------------------------------
void __fastcall TFrameEdpUspDMSPiloto::ActionFaultLocationExecute(TObject *Sender)
{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormFaultLocation(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
void __fastcall TFrameEdpUspDMSPiloto::ActionModuloTestesExecute(TObject *Sender)

{
	TForm* Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormModuloTestes(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------

