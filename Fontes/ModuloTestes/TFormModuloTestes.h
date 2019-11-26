//---------------------------------------------------------------------------
#ifndef TFormModuloTestesH
#define TFormModuloTestesH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class VTApl;
class VTGrafico;
class VTPath;
class VTRedes;
class VTTrecho;
//---------------------------------------------------------------------------
class TFormModuloTestes : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TButton *btnSelecionaArquivoTrechos;
	TOpenDialog *OpenDialog1;
	TButton *Button1;
	void __fastcall btnSelecionaArquivoTrechosClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations

   // Métodos
	          __fastcall TFormModuloTestes(TComponent *Owner, VTApl *apl_owner, TWinControl *parent);
				 __fastcall ~TFormModuloTestes();

	void __fastcall TFormModuloTestes::LeTrechos(TStringList* lisLinhas, TList* lisEXT);

	// Dados
	VTApl* apl;
	VTGrafico* grafico;
	VTPath* path;
	VTRedes* redes;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormModuloTestes *FormModuloTestes;
//---------------------------------------------------------------------------
#endif
