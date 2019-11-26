//---------------------------------------------------------------------------
#ifndef TBarraSemTensaoH
#define TBarraSemTensaoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class VTBarra;
//---------------------------------------------------------------------------
class TBarraSemTensao : public TEqptoCampo
{
private:
   // Dados
   VTBarra* barraAssociada;

public:
	// Construtor e destrutor
   __fastcall TBarraSemTensao(String Codigo);
   __fastcall ~TBarraSemTensao(void);

   // Métodos
	VTBarra* __fastcall GetBarraAssociada(void);
   void __fastcall SetBarraAssociada(VTBarra* barraAssociada);

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof