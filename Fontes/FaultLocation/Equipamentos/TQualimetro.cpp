/****
 *
 * Esta classe representa um qual�metro, associado a uma liga��o de MT.
 * Ele cont�m medi��es de tens�o e corrente daqule ponto da rede.
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
   // Inicializa dados de medi��o
   for(int nf = 0; nf < 3; nf++)
   {
      this->medicaoVI.pre.V[nf]   = 0.;
      this->medicaoVI.pre.I[nf]   = 0.;
      this->medicaoVI.falta.V[nf] = 0.;
      this->medicaoVI.falta.I[nf] = 0.;
   }
	candidatoEqptoRef = false;

	// Inicializa refer�ncias
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
