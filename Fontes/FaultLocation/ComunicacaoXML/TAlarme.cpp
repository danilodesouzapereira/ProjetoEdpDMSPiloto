/*****
 * Classe TAlarme
 *
 * Essa classe modela cada XML de alarme, recebido do módulo de Supervisão.
 * Um objeto dessa classe contém todas os campos do respectivo XML.
 */
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAlarme.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
/***
 * Construtor 1
 */
__fastcall TAlarme::TAlarme()
{
	// Nada a fazer
}
//---------------------------------------------------------------------------
/***
 * Construtor 2
 */
__fastcall TAlarme::TAlarme(String TimeStamp, String CodAlimentador,
									 int TipoAlarme, int TipoEqpto, String CodEqpto, String pathArquivoAlarme,
									 bool funcao50A, bool funcao50B, bool funcao50C, bool funcao50N,
									 bool funcao51A, bool funcao51B, bool funcao51C, bool funcao51N, double correnteFalta)
{
	// Salva parâmetros elementares
   this->TimeStamp = TimeStamp;
   this->CodAlimentador = CodAlimentador;
   this->CodEqpto = CodEqpto;
   this->TipoEqpto = TipoEqpto;
	this->TipoAlarme = TipoAlarme;
	this->pathArquivoAlarme = pathArquivoAlarme;
	this->funcao50A = funcao50A;
	this->funcao50B = funcao50B;
	this->funcao50C = funcao50C;
	this->funcao50N = funcao50N;
	this->funcao51A = funcao51A;
	this->funcao51B = funcao51B;
	this->funcao51C = funcao51C;
	this->funcao51N = funcao51N;
   this->correnteFalta = correnteFalta;
}
//---------------------------------------------------------------------------
__fastcall TAlarme::~TAlarme()
{
	// Nada a fazer
}
//---------------------------------------------------------------------------
String __fastcall TAlarme::GetPathArquivoAlarme()
{
	return pathArquivoAlarme;
}
//---------------------------------------------------------------------------
String __fastcall TAlarme::GetCodAlimentador()
{
	return CodAlimentador;
}
//---------------------------------------------------------------------------
String __fastcall TAlarme::GetCodEqpto()
{
	return CodEqpto;
}
//---------------------------------------------------------------------------
String __fastcall TAlarme::GetTimeStamp()
{
	return TimeStamp;
}
//---------------------------------------------------------------------------
int __fastcall TAlarme::GetTipoAlarme()
{
	return TipoAlarme;
}
//---------------------------------------------------------------------------
int __fastcall TAlarme::GetTipoEqpto()
{
	return TipoEqpto;
}
//---------------------------------------------------------------------------
void __fastcall TAlarme::SetPathArquivoAlarme(String pathArquivoAlarme)
{
	this->pathArquivoAlarme = pathArquivoAlarme;
}
//---------------------------------------------------------------------------
void __fastcall TAlarme::SetCodAlimentador(String CodAlimentador)
{
	this->CodAlimentador = CodAlimentador;
}
//---------------------------------------------------------------------------
void __fastcall TAlarme::SetCodEqpto(String CodEqpto)
{
	this->CodEqpto = CodEqpto;
}
//---------------------------------------------------------------------------
void __fastcall TAlarme::SetTimeStamp(String TimeStamp)
{
    this->TimeStamp = TimeStamp;
}
//---------------------------------------------------------------------------
void __fastcall TAlarme::SetTipoAlarme(int TipoAlarme)
{
    this->TipoAlarme = TipoAlarme;
}
//---------------------------------------------------------------------------
void __fastcall TAlarme::SetTipoEqpto(int TipoEqpto)
{
    this->TipoEqpto = TipoEqpto;
}
//---------------------------------------------------------------------------
