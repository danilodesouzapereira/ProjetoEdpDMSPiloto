//---------------------------------------------------------------------------
#ifndef TFrameElpaBASH
#define TFrameElpaBASH

//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <System.Actions.hpp>

//---------------------------------------------------------------------------
class VTApl;

//---------------------------------------------------------------------------
class TFrameElpaBAS : public TFrame
{
__published:   // IDE-managed Components
   TImageList *ImageList;
	TActionList *ActionList;
   TAction *ActionImportaRedeGIS;
	TAction *ActionImportaRedeANEEL;
	TAction *ActionExportaRedeANEEL;
	TAction *ActionExportaRedeOpenDSS;
   void __fastcall ActionImportaRedeGISExecute(TObject *Sender);
	void __fastcall ActionImportaRedeANEELExecute(TObject *Sender);
	void __fastcall ActionExportaRedeANEELExecute(TObject *Sender);
	void __fastcall ActionExportaRedeOpenDSSExecute(TObject *Sender);

public:      //property
   __property bool Ativo = {write=PM_SetAtivo};

public:      // User declarations
        __fastcall  TFrameElpaBAS(TComponent* Owner, VTApl *apl_owner);
        __fastcall ~TFrameElpaBAS(void);
   void __fastcall  ConfiguraPlanejamento(void);

private:   //metodos
   void __fastcall PM_SetAtivo(bool ativo);

private:   //objetos externos
   VTApl *apl;
};

//---------------------------------------------------------------------------
#endif
//eof

