//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TMedidorInteligente.h"
#include "TMedidorInteligenteBalanco.h"
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TMedidorInteligenteBalanco::TMedidorInteligenteBalanco(String Codigo) : TMedidorInteligente(Codigo)
{
   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
		this->medicaoV.pos.V[nf]   = std::complex<double>(0., 0.);
	}

	tipoMedidorInteligente = miBALANCO;
	cargaAssociada = NULL;
	blocoCarga = NULL;
}
//---------------------------------------------------------------------------
__fastcall TMedidorInteligenteBalanco::~TMedidorInteligenteBalanco()
{

}
//---------------------------------------------------------------------------
