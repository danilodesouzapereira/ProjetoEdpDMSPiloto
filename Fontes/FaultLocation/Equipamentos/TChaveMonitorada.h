//---------------------------------------------------------------------------
#ifndef TChaveMonitoradaH
#define TChaveMonitoradaH
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
class VTBloco;
class VTChave;
//---------------------------------------------------------------------------
class TChaveMonitorada : public TEqptoCampo
{
private:
	// Dados
   int Estado;
   VTChave* chaveAssociada;

public:
   // Construtor e destrutor
	__fastcall TChaveMonitorada(String Codigo);
   __fastcall TChaveMonitorada(String Codigo, int TipoEqptoCampo);
	__fastcall ~TChaveMonitorada(void);

   // Métodos
	bool     __fastcall ApenasReligamentos();
	bool     __fastcall Autobloqueio();
   VTChave* __fastcall GetChaveAssociada();
   int      __fastcall GetEstado();
	void     __fastcall SetChaveAssociada(VTChave* chaveAssociada);
   void     __fastcall SetEstado(int Estado);


   // Dados
   struct
   {
      String timestamp;

   	struct
      {
         std::complex<double> V[3];
         std::complex<double> I[3];
      }pre; //< Medições (V e I) pré-falta

      struct
      {
         std::complex<double> V[3];
         std::complex<double> I[3];
      }falta; //< Medições (V e I) da falta

   }medicaoVI;

	VTBloco* blocoChave;  //< Bloco da chave
   double DistFalta;    //< Distância de falta estimada pelo relé (km)
	String TipoAtuacao;  //< Tipo de atuação ("Fase" ou "Neutro")
   String faseAfetada;  //< A, B, C, AB, BC, CA, ABC
};
//---------------------------------------------------------------------------
#endif
