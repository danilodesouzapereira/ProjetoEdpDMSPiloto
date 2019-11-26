//---------------------------------------------------------------------------
#ifndef TFormExportaLigacoesH
#define TFormExportaLigacoesH
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
class VTOrdena;
class VTRadial;
class VTRede;
class VTRedes;
class VTTronco;
//---------------------------------------------------------------------------
class TFormExportaLigacoes : public TForm
{
__published:	// IDE-managed Components
	TButton *Button1;
	TEdit *edtCont;
	void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFormExportaLigacoes(TComponent* Owner, VTApl* apl);

   VTApl *apl;
   VTOrdena *ordena;
   VTRadial* radial;
   VTRedes *redes;
   VTTronco* tronco;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormExportaLigacoes *FormExportaLigacoes;
//---------------------------------------------------------------------------
#endif
