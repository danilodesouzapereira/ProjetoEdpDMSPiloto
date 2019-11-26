/****
 *
 * Esta classe representa um qualímetro, associado a uma ligação de MT.
 * Ele contém medições de tensão e corrente daqule ponto da rede.
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TQualimetro.h"
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TQualimetro::TQualimetro(String Codigo) : TEqptoCampo(Codigo, eqptoQUALIMETRO)
{
   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
      this->medicaoVI.pre.V[nf]   = 0.;
      this->medicaoVI.pre.I[nf]   = 0.;
      this->medicaoVI.falta.V[nf] = 0.;
      this->medicaoVI.falta.I[nf] = 0.;
   }
	candidatoEqptoRef = false;

	// Inicializa referências
	cargaAssociada = NULL;
   ligacaoAssociada = NULL;
   trechoJusante = NULL;
}
//---------------------------------------------------------------------------
__fastcall TQualimetro::~TQualimetro(void)
{
	// Nada a fazer
}
//---------------------------------------------------------------------------
VTLigacao* __fastcall TQualimetro::GetLigacaoAssociada()
{
   return (this->ligacaoAssociada);
}
//---------------------------------------------------------------------------
void __fastcall TQualimetro::SetLigacaoAssociada(VTLigacao* ligacaoAssociada)
{
	this->ligacaoAssociada = ligacaoAssociada;
}
//---------------------------------------------------------------------------
void __fastcall TQualimetro::SetTrechoJusante(VTTrecho* trechoJusante)
{
    this->trechoJusante = trechoJusante;
}
//---------------------------------------------------------------------------
