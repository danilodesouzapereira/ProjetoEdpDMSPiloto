/****
 *  Por chave monitorada, entedemos:
 *
 *  DISJUNTOR, RELIGADORA, SECCIONADORA
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
#include "TChaveMonitorada.h"
#include "..\ComunicacaoXML\TAlarme.h"
#include <complex>
#include <cmath>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
/***
 * Construtor 1 - Cria obj com código apenas
 */
__fastcall TChaveMonitorada::TChaveMonitorada(String Codigo) : TEqptoCampo(Codigo, tipoINDEF)
{
	// Inicializa parâmetros
	this->Estado = estadoINDEF;

   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
      this->medicaoVI.pre.V[nf]   = 0.;
      this->medicaoVI.pre.I[nf]   = 0.;
      this->medicaoVI.falta.V[nf] = 0.;
      this->medicaoVI.falta.I[nf] = 0.;
   }

   // Inicializa parâmetros
   DistFalta = 0.;
	TipoAtuacao = "";
	faseAfetada = "";
	blocoChave = NULL;
}
//---------------------------------------------------------------------------
/***
 * Construtor 2 - Cria obj com código e tipo
 */
__fastcall TChaveMonitorada::TChaveMonitorada(String Codigo, int TipoEqptoCampo) : TEqptoCampo(Codigo, TipoEqptoCampo)
{
	// Inicializa parâmetros
	this->Estado = estadoINDEF;

   // Inicializa dados de medição
   for(int nf = 0; nf < 3; nf++)
   {
      this->medicaoVI.pre.V[nf]   = 0.;
      this->medicaoVI.pre.I[nf]   = 0.;
      this->medicaoVI.falta.V[nf] = 0.;
      this->medicaoVI.falta.I[nf] = 0.;
   }

   // Inicializa parâmetros
   DistFalta = 0.;
	TipoAtuacao = "";
	faseAfetada = "";
	blocoChave = NULL;
}
//---------------------------------------------------------------------------
__fastcall TChaveMonitorada::~TChaveMonitorada(void)
{
	// Nada a fazer
}
//---------------------------------------------------------------------------
bool __fastcall TChaveMonitorada::ApenasReligamentos()
{
	for(int i=0; i<lisAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) lisAlarmes->Items[i];
		if(alarme->TipoAlarme == 20) continue; // alarme de tentativa de religamento
		return(false);
	}
	return(true);
}
//---------------------------------------------------------------------------
bool __fastcall TChaveMonitorada::Autobloqueio()
{
	for(int i=0; i<lisAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) lisAlarmes->Items[i];
		if(alarme->TipoAlarme == 21) // alarme de autobloqueio
			return(true);
	}
	return(false);
}
//---------------------------------------------------------------------------
VTChave* __fastcall TChaveMonitorada::GetChaveAssociada()
{
	return (this->chaveAssociada);
}
//---------------------------------------------------------------------------
int __fastcall TChaveMonitorada::GetEstado()
{
	return this->Estado;
}
//---------------------------------------------------------------------------
void __fastcall TChaveMonitorada::SetChaveAssociada(VTChave* chaveAssociada)
{
	this->chaveAssociada = chaveAssociada;
}
//---------------------------------------------------------------------------
void __fastcall TChaveMonitorada::SetEstado(int Estado)
{
	this->Estado = Estado;
}
//---------------------------------------------------------------------------
