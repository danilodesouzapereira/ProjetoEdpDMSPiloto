//---------------------------------------------------------------------------
#ifndef TQualimetroH
#define TQualimetroH
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
#include "TEqptoCampo.h"
//---------------------------------------------------------------------------
class VTBarra;
class VTCarga;
class VTLigacao;
class VTTrecho;
//---------------------------------------------------------------------------
class TQualimetro : public TEqptoCampo
{
public:
	// Dados
	VTCarga*   cargaAssociada;
   VTLigacao* ligacaoAssociada;
	VTBarra*   barraAssociada;
   VTTrecho*  trechoJusante;
	bool       candidatoEqptoRef;

   struct
	{
      String timestamp;

   	struct
      {
         std::complex<double> V[3];
         std::complex<double> I[3];
      }pre; //< Medi��es (V e I) pr�-falta

      struct
      {
         std::complex<double> V[3];
         std::complex<double> I[3];
      }falta; //< Medi��es (V e I) da falta

   }medicaoVI;

	// Construtor e destrutor
	__fastcall TQualimetro(String Codigo);
   __fastcall ~TQualimetro(void);

   // M�todos
   VTLigacao* __fastcall GetLigacaoAssociada();
	void       __fastcall SetLigacaoAssociada(VTLigacao* ligacaoAssociada);
   void       __fastcall SetTrechoJusante(VTTrecho*  trechoJusante);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
