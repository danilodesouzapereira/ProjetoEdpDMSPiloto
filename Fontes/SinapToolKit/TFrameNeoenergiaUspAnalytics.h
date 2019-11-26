//---------------------------------------------------------------------------
#ifndef TFrameNeoenergiaUspAnalyticsH
#define TFrameNeoenergiaUspAnalyticsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ImgList.hpp>
#include <System.ImageList.hpp>
//---------------------------------------------------------------------------
class VTApl;
//---------------------------------------------------------------------------
class TFrameNeoenergiaUspAnalytics : public TFrame
{
__published:	// IDE-managed Components
	TImageList *ImageList;
	TActionList *ActionList;
	TAction *ActionFaultLocationOffline;
	TAction *ActionPerdas;
	TAction *ActionMonitoraQualidade;
	TAction *ActionManobraOtima;
	TAction *ActionGeralAnalytics;
	void __fastcall ActionFaultLocationOfflineExecute(TObject *Sender);
	void __fastcall ActionPerdasExecute(TObject *Sender);
	void __fastcall ActionMonitoraQualidadeExecute(TObject *Sender);
	void __fastcall ActionManobraOtimaExecute(TObject *Sender);
	void __fastcall ActionGeralAnalyticsExecute(TObject *Sender);

public:      //property
	__property bool Ativo = {write=PM_SetAtivo};

public:		// User declarations
	__fastcall TFrameNeoenergiaUspAnalytics(TComponent* Owner, VTApl *apl_owner);
	__fastcall ~TFrameNeoenergiaUspAnalytics(void);


private:   //metodos
	void __fastcall PM_SetAtivo(bool ativo);

private:   //objetos externos
	VTApl *apl;
};
//---------------------------------------------------------------------------
extern PACKAGE TFrameNeoenergiaUspAnalytics *FrameNeoenergiaUspAnalytics;
//---------------------------------------------------------------------------
#endif
