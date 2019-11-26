//---------------------------------------------------------------------------
#ifndef TFusivelH
#define TFusivelH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class VTChave;
//---------------------------------------------------------------------------
class TFusivel : TEqptoCampo
{
private:
    // Dados
    VTChave* chaveAssociada;
    int Estado;

public:
	// Construtor e destrutor
   __fastcall TFusivel(String Codigo);
   __fastcall ~TFusivel(void);

   // Métodos
   int      __fastcall GetEstado();
	VTChave* __fastcall GetChaveAssociada();
   void     __fastcall SetEstado(int Estado);
   void     __fastcall SetChaveAssociada(VTChave* chave);

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
