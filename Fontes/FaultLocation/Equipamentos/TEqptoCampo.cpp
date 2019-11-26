/****
 *
 * ESta classe representa os equipamentos de campo: chaves monitoradas e não
 * monitoradas, sensor, qualímetro e transforamdor inteligente.
 *
 ***/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TEqptoCampo.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Rede\VTEqpto.h>
//---------------------------------------------------------------------------
__fastcall TEqptoCampo::TEqptoCampo(String Codigo, int Tipo)
{
	// Seta parâmetros elementares
	this->Codigo = Codigo;
	this->Tipo = Tipo;
	lisAlarmes = new TList;
	lisBlocosJusante = new TList;
}
//---------------------------------------------------------------------------
__fastcall TEqptoCampo::~TEqptoCampo(void)
{
	// Destroi objetos
	delete lisAlarmes;
	delete lisBlocosJusante;
}
//---------------------------------------------------------------------------
TList* __fastcall TEqptoCampo::GetBlocosJusante()
{
	return this->lisBlocosJusante;
}
//---------------------------------------------------------------------------
String __fastcall TEqptoCampo::GetCodigo()
{
	return this->Codigo;
}
//---------------------------------------------------------------------------
int __fastcall TEqptoCampo::GetTipo()
{
    return this->Tipo;
}
//---------------------------------------------------------------------------
void __fastcall TEqptoCampo::SetBlocosJusante(TList* lisBlocosJusante)
{
	for(int i=0; i<lisBlocosJusante->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosJusante->Items[i];
		this->lisBlocosJusante->Add(bloco);
	}
}
//---------------------------------------------------------------------------
void __fastcall TEqptoCampo::SetTipo(int Tipo)
{
    this->Tipo = Tipo;
}
//---------------------------------------------------------------------------
