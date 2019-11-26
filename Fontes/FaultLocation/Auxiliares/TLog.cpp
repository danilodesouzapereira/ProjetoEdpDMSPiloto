/***
 *  Classe para a gera��o de logs, subsidiando~:
 *     - Debug
 *     - Opera��o
 **/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TLog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
/***
 * Costrutor 1
 */
__fastcall TLog::TLog()
{
	// Inicializa��o de objetos
   this->lisConteudo = new TStringList();
	this->caminho = "";
}
//---------------------------------------------------------------------------
/***
 * Costrutor 2
 */
__fastcall TLog::TLog(String caminho)
{
	// Inicializa��o de objetos
   this->lisConteudo = new TStringList();
	this->caminho = caminho;
}
//---------------------------------------------------------------------------
__fastcall TLog::~TLog(void)
{
	// Destroi objetos
   if(this->lisConteudo) {delete this->lisConteudo; this->lisConteudo = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TLog::AddLinha(String linha, bool pulaLinha)
{
	// Ajusta linha
   if(pulaLinha)
		if(linha.SubString(linha.Length()-1,1) != "\n") linha += "\n";

	// Insere linha � lista de strings
   lisConteudo->Add(linha);
}
//---------------------------------------------------------------------------
void __fastcall TLog::ImprimeLog()
{
	if(caminho != "")
   {
		lisConteudo->SaveToFile(caminho);
   }
}
//---------------------------------------------------------------------------
void __fastcall TLog::Reinicia()
{
   // Limpa lista de conte�do
	this->lisConteudo->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TLog::SetCaminho(String caminho)
{
	this->caminho = caminho;
}
//---------------------------------------------------------------------------
bool __fastcall TLog::Vazio()
{
	return(lisConteudo->Count == 0);
}
//---------------------------------------------------------------------------
//eof
