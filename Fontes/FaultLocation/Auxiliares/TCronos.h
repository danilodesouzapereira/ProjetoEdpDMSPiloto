//---------------------------------------------------------------------------
#ifndef TCronosH
#define TCronosH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TCronos
{
private:
	// Dados
    TDateTime horarioIni;

public:
	// Construtor e destrutor
   __fastcall TCronos();
   __fastcall ~TCronos();

   // Métodos
   double __fastcall GetSegundos();
   double __fastcall GetMinutos();
   void __fastcall Reinicia();
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
