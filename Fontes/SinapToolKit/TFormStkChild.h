//---------------------------------------------------------------------------
#ifndef TFormStkChildH
#define TFormStkChildH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <System.Actions.hpp>
//---------------------------------------------------------------------------
class VTApl;
class TFrameEdpUspDMSAda;
//---------------------------------------------------------------------------
class TFormStkChild : public TForm
{
__published:   // IDE-managed Components
	TActionList *ActionList;
	TAction *ActionImportaRedeGIS;
	TAction *ActionOcorrencias;
	TAction *ActionEnumeracao;
	TAction *ActionAlocaChaves;
	TAction *ActionCompensacoes;
	TAction *ActionPiscadas;
	TAction *ActionSimTurmas;
	TAction *ActionPerdasComerciais;
	TAction *ActionImportaAccess;
	TAction *ActionAlocaIdentificadorFalta;
	TAction *ActionCalcIndContTelecom;
	TAction *ActionGerenciadorCenarioTelecom;
	TActionList *ActionList1;
	TAction *ActionRedeCarregada;
	void __fastcall ActionImportaAccessExecute(TObject *Sender);
	void __fastcall ActionImportaRedeGISExecute(TObject *Sender);
	void __fastcall ActionRedeCarregadaExecute(TObject *Sender);

public:      // User declarations
		  __fastcall  TFormStkChild(TComponent *Owner, VTApl *apl, int stk_option);
        __fastcall ~TFormStkChild(void);
   void __fastcall  Atualiza(void);

private:   //métodos

private:   //objetos externos
   VTApl *apl;

private:   //dados locais
	TFrameEdpUspDMSAda* FrameEdpUspDMSAda;
};

//---------------------------------------------------------------------------
#endif
//eof
