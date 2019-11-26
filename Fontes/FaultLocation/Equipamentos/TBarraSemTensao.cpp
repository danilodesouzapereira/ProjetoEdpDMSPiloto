//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TBarraSemTensao.h"
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TBarraSemTensao::TBarraSemTensao(String Codigo) : TEqptoCampo(Codigo, eqptoBARRA_SEM_TENSAO)
{

}
//---------------------------------------------------------------------------
__fastcall TBarraSemTensao::~TBarraSemTensao(void)
{

}
//---------------------------------------------------------------------------
VTBarra* __fastcall TBarraSemTensao::GetBarraAssociada(void)
{
	return this->barraAssociada;
}
//---------------------------------------------------------------------------
void __fastcall TBarraSemTensao::SetBarraAssociada(VTBarra* barraAssociada)
{
	this->barraAssociada = barraAssociada;
}
//---------------------------------------------------------------------------
