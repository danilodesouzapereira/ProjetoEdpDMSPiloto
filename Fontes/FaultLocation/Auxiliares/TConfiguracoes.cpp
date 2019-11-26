/***
 *  Classe de configurações do algoritmo de Estratégia Evolutiva
 **/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TConfiguracoes.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
//---------------------------------------------------------------------------
/***
 * Construtor 1
 */
__fastcall TConfiguracoes::TConfiguracoes()
{
	NumIndivIni = 40;
   MinDeltaFAval = 5.;
   sigmaXini = 10.;
	sigmaRfini = 20.;
	ConsiderarQualidadeSensor = true;
}
//---------------------------------------------------------------------------
/***
 * Construtor 2
 */
__fastcall TConfiguracoes::TConfiguracoes(String pathDirDat)
{
	NumIndivIni = 40;
   MinDeltaFAval = 5.;
   sigmaXini = 10.;
	sigmaRfini = 20.;

	// Salva path do arquivo de configurações
	pathConfigGerais = pathDirDat + "\\FaultLocation\\ConfigGerais.ini";

   // Obtém as configurações da Estratégia Evolutiva
	GetConfigEE();

	// Obtém configurações gerais
	GetConfigGerais();
}
//---------------------------------------------------------------------------
/***
 * Método para obter as configurações de Estratégia Evolutiva de arquivo INI externo
 */
void __fastcall TConfiguracoes::GetConfigEE()
{
	TIniFile* file = new TIniFile(pathConfigGerais);

	NumMaxGeracoes = file->ReadInteger("EE", "NumMaxGeracoes", 20);
	MaxIdade = file->ReadInteger("EE", "MaxIdade", 5);
	MaxIndivPorGeracao = file->ReadInteger("EE", "MaxIndivPorGeracao", 15);
	NumFilhosMutacao = file->ReadInteger("EE", "NumFilhosMutacao", 5);
	TipoCruzamento = file->ReadInteger("EE", "TipoCruzamento", 0);
	TipoSelecao = file->ReadInteger("EE", "TipoSelecao", 0);
	Pm = file->ReadFloat("EE", "Pm", 1.0);
	Pc = file->ReadFloat("EE", "Pc", 0.4);
	PesoV = file->ReadFloat("EE", "PesoV", 1.0);
	PesoI = file->ReadFloat("EE", "PesoI", 1.0);
	MaxRfalta = file->ReadFloat("EE", "MaxRfalta", 25.);
	MinDeltaFAval = file->ReadFloat("EE", "MinDeltaFAval", 5.);
	sigmaXini = file->ReadFloat("EE", "sigmaXini", 10.);
	sigmaRfini = file->ReadFloat("EE", "sigmaRfini", 20.);
	NumIndivIni = file->ReadInteger("EE", "NumIndivIni", 40);
	MostrarLogDebug = file->ReadBool("GERAL", "MostrarLogsDebug", 0);

   // Destroi obj de arquivo INI
//	file->Free();
   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Método para obter as configurações gerais dos processos de localização, a
 * partir de arquivo INI externo.
 */
void __fastcall TConfiguracoes::GetConfigGerais()
{
	TIniFile* file = new TIniFile(pathConfigGerais);

	ConsiderarQualidadeSensor = file->ReadBool("GERAL", "ConsiderarQualidadeSensor", 1);

   // Destroi obj de arquivo INI
//	file->Free();
   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguracoes::GetConsiderarQualidadeSensor()
{
   return(ConsiderarQualidadeSensor);
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetNumIndivIni()
{
	return this->NumIndivIni;
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetNumMaxGeracoes()
{
	return this->NumMaxGeracoes;
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetMaxIdade()
{
	return this->MaxIdade;
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetMaxIndivPorGeracao()
{
	return this->MaxIndivPorGeracao;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetMaxRfalta()
{
	return this->MaxRfalta;
}
//---------------------------------------------------------------------------
bool __fastcall TConfiguracoes::GetMostrarLogDebug()
{
	return this->MostrarLogDebug;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetMinDeltaFAval()
{
   return this->MinDeltaFAval;
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetNumFilhosMutacao()
{
	return this->NumFilhosMutacao;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetSigmaRfini()
{
	return this->sigmaRfini;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetSigmaXini()
{
	return this->sigmaXini;
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetTipoCruzamento()
{
	return this->TipoCruzamento;
}
//---------------------------------------------------------------------------
int __fastcall TConfiguracoes::GetTipoSelecao()
{
	return this->TipoSelecao;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetPm()
{
	return this->Pm;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetPc()
{
	return this->Pc;
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetPesoV()
{
   return(PesoV);
}
//---------------------------------------------------------------------------
double __fastcall TConfiguracoes::GetPesoI()
{
   return(PesoI);
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetNumMaxGeracoes(int NumMaxGeracoes)
{
	this->NumMaxGeracoes = NumMaxGeracoes;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetMaxIdade(int MaxIdade)
{
	this->MaxIdade = MaxIdade;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetMaxIndivPorGeracao(int MaxIndivPorGeracao)
{
	this->MaxIndivPorGeracao = MaxIndivPorGeracao;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetMaxRfalta(double MaxRfalta)
{
	this->MaxRfalta = MaxRfalta;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetNumFilhosMutacao(int NumFilhosMutacao)
{
	this->NumFilhosMutacao = NumFilhosMutacao;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetTipoCruzamento(int TipoCruzamento)
{
	this->TipoCruzamento = TipoCruzamento;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetTipoSelecao(int TipoSelecao)
{
	this->TipoSelecao = TipoSelecao;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetPm(double Pm)
{
	this->Pm = Pm;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetPc(double Pc)
{
	this->Pc = Pc;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetPesoV(double PesoV)
{
   this->PesoV = PesoV;
}
//---------------------------------------------------------------------------
void __fastcall TConfiguracoes::SetPesoI(double PesoI)
{
   this->PesoI = PesoI;
}
//---------------------------------------------------------------------------
