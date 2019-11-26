//---------------------------------------------------------------------------
#ifndef TFrameEdpUspDMSPilotoH
#define TFrameEdpUspDMSPilotoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ImgList.hpp>
#include <System.ImageList.hpp>
//#include <System.ImageList.hpp>
//---------------------------------------------------------------------------
class VTApl;
//---------------------------------------------------------------------------
class TFrameEdpUspDMSPiloto : public TFrame
{
__published:	// IDE-managed Components
	TImageList *ImageList;
	TActionList *ActionList;
	TAction *ActionFaultLocation;
	TAction *ActionModuloTestes;
	void __fastcall ActionFaultLocationExecute(TObject *Sender);
	void __fastcall ActionModuloTestesExecute(TObject *Sender);


public:      //property
	__property bool Ativo = {write=PM_SetAtivo};

public:		// User declarations
	__fastcall TFrameEdpUspDMSPiloto(TComponent* Owner, VTApl *apl_owner);
	__fastcall ~TFrameEdpUspDMSPiloto(void);


private:   //metodos
	void __fastcall PM_SetAtivo(bool ativo);

private:   //objetos externos
   VTApl *apl;

};
//---------------------------------------------------------------------------
extern PACKAGE TFrameEdpUspDMSPiloto *FrameEdpUspDMSPiloto;
//---------------------------------------------------------------------------
#endif
