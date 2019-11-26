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
      }pre; //< Medições (V) pré-falta

      struct
      {
         std::complex<double> V[3];
      }falta; //< Medições (V) da falta

   }medicaoV;


	// Construtor e destrutor
	__fastcall TITrafo(String Codigo);
	__fastcall ~TITrafo(void);


   // Métodos
   VTCarga* __fastcall GetCargaAssociada();
   void     __fastcall SetCargaAssociada(VTCarga* cargaAssociada);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
