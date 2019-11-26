//---------------------------------------------------------------------------
#ifndef TFormLoginH
#define TFormLoginH

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>

//---------------------------------------------------------------------------
class VTApl;
class VTEmpresa;

//---------------------------------------------------------------------------
class TFormLogin : public TForm
{
__published:	// IDE-managed Components
   TLabel *Label7;
   TEdit *EditOracleTNSname;
   TLabel *Label5;
   TEdit *EditOracleUsername;
   TLabel *Label6;
   TEdit *EditOraclePassword;
   TBitBtn *ButConfirma;
   TBitBtn *ButCancela;
	TBitBtn *ButTesta;
   void __fastcall ButConfirmaClick(TObject *Sender);
   void __fastcall ButCancelaClick(TObject *Sender);
	void __fastcall ButTestaClick(TObject *Sender);

public:		// User declarations
   __fastcall  TFormLogin(TComponent* Owner, VTApl *apl);
   __fastcall ~TFormLogin(void);

private:	//métodos
   void __fastcall DadosConexaoRead(void);
   void __fastcall DadosConexaoWrite(void);
   bool __fastcall IniciaConexaoOracle(void);
   bool __fastcall ValidaDadosConexaoOracle(void);

private:	//objetos externos
   VTApl *apl;

private:    //objeto interno
   struct
   {
	  AnsiString TNSname;
	  AnsiString Username;
	  AnsiString Password;
   }empresa;

};

//---------------------------------------------------------------------------
#endif
//eof

