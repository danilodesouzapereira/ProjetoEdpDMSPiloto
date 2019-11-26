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

//---------------------------------------------------------------------------
class TFormStkChild : public TForm
{
__published:   // IDE-managed Components
   TImageList *ImageList;
   TActionList *ActionList;
   TAction *ActionOcorrencias;
   TAction *ActionContinuidade;
   TAction *ActionEnumeracao;
   TAction *ActionAlocaChaves;
   TAction *ActionSimTurmas;
	TAction *ActionPerdasCom;
   void __fastcall ActionAlocaChavesExecute(TObject *Sender);
   void __fastcall ActionCompensacoesExecute(TObject *Sender);
   void __fastcall ActionContinuidadeExecute(TObject *Sender);
   void __fastcall ActionEnumeracaoExecute(TObject *Sender);
   void __fastcall ActionOcorrenciasExecute(TObject *Sender);
   void __fastcall ActionPiscadasExecute(TObject *Sender);
   void __fastcall ActionSimTurmasExecute(TObject *Sender);
	void __fastcall ActionPerdasComExecute(TObject *Sender);

public:      // User declarations
		  __fastcall  TFormStkChild(TComponent *Owner, VTApl *apl, int stk_option);
        __fastcall ~TFormStkChild(void);
   void __fastcall  Atualiza(void);

private:   //métodos
	void __fastcall ImportaRedeBaseAccess(void);
	void __fastcall ImportaRedeBaseOracle(void);

private:   //objetos externos
   VTApl *apl;
};

//---------------------------------------------------------------------------
#endif
//eof
