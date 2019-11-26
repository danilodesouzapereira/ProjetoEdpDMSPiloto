//---------------------------------------------------------------------------
#ifndef TMedidorInteligenteH
#define TMedidorInteligenteH
//---------------------------------------------------------------------------
#include <cmath>
#include <complex>
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TMedidorInteligente : public TEqptoCampo
{

public:

	// Dados
   struct
	{
		String timestamp;
   	struct
      {
			std::complex<double> V[3];
		}pos; //< Medições de tensões pós-falta
	}medicaoV;

	// Métodos
	__fastcall TMedidorInteligente(String Codigo);
	__fastcall ~TMedidorInteligente();

};
//---------------------------------------------------------------------------
#endif
