//---------------------------------------------------------------------------
#ifndef TMedidorInteligenteBalancoH
#define TMedidorInteligenteBalancoH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
#include "TMedidorInteligente.h"
//---------------------------------------------------------------------------
class VTBloco;
class VTCarga;
//---------------------------------------------------------------------------
class TMedidorInteligenteBalanco : public TMedidorInteligente
{
public:

	// Dados
	int tipoMedidorInteligente;
	VTBloco* blocoCarga;
	VTCarga* cargaAssociada;

	// Métodos
	__fastcall TMedidorInteligenteBalanco(String Codigo);
	__fastcall ~TMedidorInteligenteBalanco();

};
//---------------------------------------------------------------------------
#endif
