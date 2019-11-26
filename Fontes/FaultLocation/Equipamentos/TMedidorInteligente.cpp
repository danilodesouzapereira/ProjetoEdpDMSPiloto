/****
 *
 * Esta classe representa um Medidor Inteligente, associado a um transformador MT/BT
 * ou a uma carga MT ou a uma carga BT
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TMedidorInteligente.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TMedidorInteligente::TMedidorInteligente(String Codigo) : TEqptoCampo(Codigo, eqptoMI)
{
   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
		this->medicaoV.pos.V[nf]   = std::complex<double>(0., 0.);
	}
}
//---------------------------------------------------------------------------
__fastcall TMedidorInteligente::~TMedidorInteligente()
{

}
//---------------------------------------------------------------------------
