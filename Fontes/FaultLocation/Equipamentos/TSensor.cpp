/****
 *
 * Esta classe representa um Sensor de corrente, instalado associado a uma chave
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TSensor.h"
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TSensor::TSensor(String Codigo) : TEqptoCampo(Codigo, eqptoSENSOR)
{
   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
      this->medicaoI.pre.I[nf]   = std::complex<double>(0., 0.);
      this->medicaoI.falta.I[nf] = std::complex<double>(0., 0.);
   }

   qualidadeOK  = false;
   faltaJusante = false;
   // Inicializa lista de blocos a jusante
   lisBlocosJusante = new TList();
}
//---------------------------------------------------------------------------
__fastcall TSensor::~TSensor(void)
{
	// Destroi objetos
   if(lisBlocosJusante) {delete lisBlocosJusante; lisBlocosJusante = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Retorna lista de blocos à jusante do sensor, excetuando-se o bloco que contém
 * o sensor.
 **/
TList* __fastcall TSensor::GetBlocosJusante_SemBlocoSensor()
{
	TList* listaResp = new TList();

	for(int i=0; i<lisBlocosJusante->Count; i++)
   {
		VTBloco* bloco = (VTBloco*) lisBlocosJusante->Items[i];
   	TList* listaLigacoes = bloco->LisLigacao();
      if(listaLigacoes->IndexOf(ligacaoAssociada) < 0)
      {
         listaResp->Add(bloco);
      }
   }

   return listaResp;
}
//---------------------------------------------------------------------------
VTLigacao* __fastcall TSensor::GetLigacaoAssociada()
{
   return (this->ligacaoAssociada);
}
//---------------------------------------------------------------------------
bool __fastcall TSensor::Sensibilizado()
{
	bool resp;

   resp = false;
   for(int nf = 0; nf < 3; nf++)
   {
      if(std::abs(medicaoI.pre.I[nf]) > 0) resp = true;
      if(std::abs(medicaoI.falta.I[nf]) > 0) resp = true;
   }

   return resp;
}
//---------------------------------------------------------------------------
void __fastcall TSensor::SetLigacaoAssociada(VTLigacao* ligacaoAssociada)
{
	this->ligacaoAssociada = ligacaoAssociada;
}
//---------------------------------------------------------------------------
