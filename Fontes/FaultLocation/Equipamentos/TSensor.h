//---------------------------------------------------------------------------
#ifndef TSensorH
#define TSensorH
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class VTLigacao;
//---------------------------------------------------------------------------
class TSensor : public TEqptoCampo
{
private:
	// Dados
   VTLigacao* ligacaoAssociada;

public:

	// Dados
   struct
   {
      String timestamp;

   	struct
      {
         std::complex<double> I[3];
      }pre; //< Medi��es (I) pr�-falta

      struct
      {
         std::complex<double> I[3];
      }falta; //< Medi��es (I) da falta

   }medicaoI;

   bool qualidadeOK;
   bool faltaJusante;

	// Construtor e destrutor
   __fastcall TSensor(String Codigo);
   __fastcall ~TSensor(void);

   // M�todos
	TList*     __fastcall GetBlocosJusante_SemBlocoSensor();
   VTLigacao* __fastcall GetLigacaoAssociada();
   bool       __fastcall Sensibilizado();
   void       __fastcall SetLigacaoAssociada(VTLigacao* ligacaoAssociada);

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
