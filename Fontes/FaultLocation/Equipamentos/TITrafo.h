//---------------------------------------------------------------------------
#ifndef TITrafoH
#define TITrafoH
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class VTCarga;
//---------------------------------------------------------------------------
class TITrafo : public TEqptoCampo
{
public:
	// Dados
   VTCarga* cargaAssociada;

   struct
   {
      String timestamp;

   	struct
      {
         std::complex<double> V[3];
      }pre; //< Medi��es (V) pr�-falta

      struct
      {
         std::complex<double> V[3];
      }falta; //< Medi��es (V) da falta

   }medicaoV;


	// Construtor e destrutor
	__fastcall TITrafo(String Codigo);
	__fastcall ~TITrafo(void);


   // M�todos
   VTCarga* __fastcall GetCargaAssociada();
   void     __fastcall SetCargaAssociada(VTCarga* cargaAssociada);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
