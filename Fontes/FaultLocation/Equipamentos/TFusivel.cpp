/****
 *
 * Esta classe representa um fusível, para o algoritmo de Localização de Faltas
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TFusivel.h"
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TFusivel::TFusivel(String Codigo) : TEqptoCampo(Codigo, chaveFU)
{
	// Inicializa parâmetros
	this->Estado = estadoINDEF;
}
//---------------------------------------------------------------------------
__fastcall TFusivel::~TFusivel(void)
{
	// Nada a fazer
}
//---------------------------------------------------------------------------
int __fastcall TFusivel::GetEstado()
{
	return this->Estado;
}
//---------------------------------------------------------------------------
VTChave* __fastcall TFusivel::GetChaveAssociada()
{
   return (this->chaveAssociada);
}
//---------------------------------------------------------------------------
void __fastcall TFusivel::SetEstado(int Estado)
{
	this->Estado = Estado;
}
//---------------------------------------------------------------------------
void __fastcall TFusivel::SetChaveAssociada(VTChave* chaveAssociada)
{
   this->chaveAssociada = chaveAssociada;
}
//---------------------------------------------------------------------------
