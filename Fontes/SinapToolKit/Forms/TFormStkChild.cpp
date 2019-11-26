// ---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TFormStkChild.h"
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\ImportaRedeGIS\VTImportaRedeGIS.h>
#include <PlataformaSinap\Fontes\MultiMonitor\VTMultiPanel.h>
#include <PlataformaSinap\Fontes\Sinap\VTSinapChild.h>
#include <PlataformaSinap\DLL_Inc\ImportaRedeGIS.h>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\Dll_Inc\CalcIndCont.h>
#include <PlataformaSinap\Dll_Inc\Compensacoes.h>

// ---------------------------------------------------------------------------
//#include <ProjetoNeoEnergia\DLL_Inc\NeoEnergia_InterfaceOracle.h>

// ---------------------------------------------------------------------------
#include <ModulosOpcionais\Dll_Inc\AlocaChaves.h>

#include <ModulosOpcionais\Dll_Inc\ExtratorOcorrencias.h>
#include <ModulosOpcionais\Dll_Inc\GeralMainFormAlocaChavesP1.h>
// #include <ModulosOpcionais\Dll_Inc\Piscadas.h>
#include <ModulosOpcionais\Dll_Inc\SimTurmas.h>
#include <ModulosOpcionais\Dll_Inc\PerdaSensor.h>

// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

// ---------------------------------------------------------------------------
__fastcall TFormStkChild::TFormStkChild(TComponent *Owner, VTApl *apl, int stk_option)
	: TForm(Owner)
{
	// variáveis locais
	bool expand = true;
	VTSinapChild *sinap_child = (VTSinapChild*) apl->GetObject(__classid(VTSinapChild));

	// salva ponteiro de objetos externos
	this->apl = apl;
	// insere Menu
	sinap_child->FormStkAdd("STK: NeoEnergia", ActionList, expand);
	// verifica se foi definida alguma opção
	switch (stk_option)
	{
	case 1: // executa módulo de importação de rede do GIS
		ImportaRedeBaseAccess();
		break;
	case 2: // executa módulo de importação de rede do GIS
		ImportaRedeBaseOracle();
		break;
	default: // nada a fazer
		break;
	}
}

// ---------------------------------------------------------------------------
__fastcall TFormStkChild::~TFormStkChild(void)
{
	// nada a fazer
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::Atualiza(void)
{
	// nada a fazer
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ImportaRedeBaseAccess(void)
{
	// variáveis locais
	VTImportaRedeGIS *ImportaRedeGIS;

	if ((ImportaRedeGIS = DLL_NewObjImportaRedeGIS(this, apl)) != NULL)
	{
		ImportaRedeGIS->ShowModalFormImportaRedeGIS();
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ImportaRedeBaseOracle(void)
{
	// variáveis locais
   /*	TForm *Form;

	// cria TFormOracle como janela modal
	Form = DLL_NewObjImportaRedeGIS(this, apl);
	if (Form != NULL)
	{
		Form->ShowModal();
		delete Form;
	}    */

	// variáveis locais
	VTImportaRedeGIS *ImportaRedeGIS;

	if ((ImportaRedeGIS = DLL_NewObjImportaRedeGIS(this, apl)) != NULL)
	{
		ImportaRedeGIS->ShowModalFormImportaRedeGIS();
	}

}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionAlocaChavesExecute(TObject *Sender)
{
	TForm *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormAlocaChaves(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionCompensacoesExecute(TObject *Sender)
{
	 TForm        *Form;
   VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
   Form = DLL_NewFormCompensacoes(this, apl, MultiPanel->PanelAtivo, "NeoEnergia");
   Form->Show();
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionContinuidadeExecute(TObject *Sender)
{
	TForm *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormCalcIndCont(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionEnumeracaoExecute(TObject *Sender)
{
	TForm *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_Geral_NewGeralFormAlocaChavesP1(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionOcorrenciasExecute(TObject *Sender)
{
	TForm *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormExtratorDados(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionPiscadasExecute(TObject *Sender)
{
   /*	TForm *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormPiscadas(this, apl, MultiPanel->PanelAtivo);
	Form->Show();  */
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionSimTurmasExecute(TObject *Sender)
{
	TForm *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormSimTurmas(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}

// ---------------------------------------------------------------------------
void __fastcall TFormStkChild::ActionPerdasComExecute(TObject *Sender)
{
	TForm        *Form;
	VTMultiPanel *MultiPanel = (VTMultiPanel*)apl->GetObject(__classid(VTMultiPanel));
	Form = DLL_NewFormPerdaSensor(this, apl, MultiPanel->PanelAtivo);
	Form->Show();
}
//---------------------------------------------------------------------------
// eof

