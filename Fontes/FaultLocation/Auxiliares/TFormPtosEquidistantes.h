//---------------------------------------------------------------------------
#ifndef TFormPtosEquidistantesH
#define TFormPtosEquidistantesH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Graphics.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.Menus.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
//---------------------------------------------------------------------------
class VTApl;
//---------------------------------------------------------------------------
class TFormPtosEquidistantes : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *GroupBox1;
	TEdit *EditCodBarraIni;
	TLabel *Label1;
	TLabel *Label2;
	TEdit *EditDistancia;
	TGroupBox *GroupBox2;
	TMemo *MemoBarrasFinais;
	TButton *Button1;
	TActionList *ActionList;
	TAction *ActionCalcular;
	void __fastcall ActionCalcularExecute(TObject *Sender);
private:	// User declarations

	VTApl* apl;

public:		// User declarations
	__fastcall TFormPtosEquidistantes(TComponent* Owner, VTApl* apl);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormPtosEquidistantes *FormPtosEquidistantes;
//---------------------------------------------------------------------------
#endif
