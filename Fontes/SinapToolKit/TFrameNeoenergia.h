//---------------------------------------------------------------------------
#ifndef TFrameNeoenergiaH
#define TFrameNeoenergiaH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ImgList.hpp>
//---------------------------------------------------------------------------
class VTApl;
//---------------------------------------------------------------------------
class TFrameNeoenergia : public TFrame
{
__published:	// IDE-managed Components
	TActionList *ActionList;
	TImageList *ImageList;
	TAction *ActionFaultLocation;
	TAction *ActionVoltVar;
	TAction *ActionAjustesParametros;
	void __fastcall ActionFaultLocationExecute(TObject *Sender);
	void __fastcall ActionVoltVarExecute(TObject *Sender);
	void __fastcall ActionAjustesParametrosExecute(TObject *Sender);

public:      //property
   __property bool Ativo = {write=PM_SetAtivo};

public:		// User declarations
	__fastcall TFrameNeoenergia(TComponent* Owner, VTApl *apl_owner);
   __fastcall ~TFrameNeoenergia(void);

private:   //metodos
	void __fastcall PM_SetAtivo(bool ativo);

private:   //objetos externos
   VTApl *apl;
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
