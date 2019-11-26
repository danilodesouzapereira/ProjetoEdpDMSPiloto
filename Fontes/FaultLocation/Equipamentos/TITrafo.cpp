/****
 *
 * Esta classe representa um iTrafo. Nos algoritmos de FL, estará associado
 * a uma carga equivalente (condensando a BT).
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TITrafo.h"
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TITrafo::TITrafo(String Codigo) : TEqptoCampo(Codigo, eqptoITRAFO)
{
   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
      this->medicaoV.pre.V[nf]   = 0.;
      this->medicaoV.falta.V[nf] = 0.;
   }

   cargaAssociada = NULL;
}
//---------------------------------------------------------------------------
__fastcall TITrafo::~TITrafo(void)
{

}
//---------------------------------------------------------------------------
VTCarga* __fastcall TITrafo::GetCargaAssociada()
{
   return (this->cargaAssociada);
}
//---------------------------------------------------------------------------
void __fastcall TITrafo::SetCargaAssociada(VTCarga* cargaAssociada)
{
	this->cargaAssociada = cargaAssociada;
}
//---------------------------------------------------------------------------
