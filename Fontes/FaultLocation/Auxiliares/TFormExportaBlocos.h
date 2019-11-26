//---------------------------------------------------------------------------
#ifndef TFormExportaBlocosH
#define TFormExportaBlocosH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
//---------------------------------------------------------------------------
class TFuncoesDeRede;
class VTApl;
class VTBloco;
class VTBlocos;
class VTRede;
class VTRedes;
//---------------------------------------------------------------------------
class TFormExportaBlocos : public TForm
{
__published:	// IDE-managed Components
	TButton *Button1;
	TActionList *ActionList1;
	TAction *ActionExportaBlocos;
	void __fastcall ActionExportaBlocosExecute(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFormExportaBlocos(TComponent* Owner, VTApl* apl);

   VTApl *apl;
   VTRedes *redes;
   TFuncoesDeRede* funcoesRede;
   TList* listaRedes;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormExportaBlocos *FormExportaBlocos;
//---------------------------------------------------------------------------
#endif
