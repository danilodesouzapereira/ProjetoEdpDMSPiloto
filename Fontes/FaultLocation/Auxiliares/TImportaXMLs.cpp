//---------------------------------------------------------------------------
#pragma hdrstop
#include "TImportaXMLs.h"
#include "..\Auxiliares\TLog.h"
#include "..\ComunicacaoXML\TAlarme.h"
#include "..\ComunicacaoXML\TXMLParser.h"
#include "..\ComunicacaoXML\TXMLComunicacao.h"
#include "..\TThreadFaultLocation.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <StrUtils.hpp>
#include <cmath>
#include <complex>
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
//---------------------------------------------------------------------------
bool __fastcall Nulo(std::complex<double>* vetor, int num_posicoes)
{
	std::complex<double> czero = std::complex<double>(0., 0.);
	for(int i=0; i<num_posicoes; i++)
   {
		if(vetor[i] != czero)
      	return false;
   }
   return true;
}
//---------------------------------------------------------------------------
double __fastcall Round(double valIni, int ndigitos)
{
	double resp;
	double base = 1.;
   for(int i=0; i<ndigitos; i++) base *= 10.;

   resp = (int)(valIni * base)/base;
   return resp;
}
//---------------------------------------------------------------------------
__fastcall TImportaXMLs::TImportaXMLs(VTApl* apl, TForm* formFL, TList* listaAlarmes)
{
	// Salva ponteiro para classes elementares
	this->apl = apl;
   this->formFL = formFL;
   path = (VTPath*) apl->GetObject(__classid(VTPath));
   this->listaAlarmes = listaAlarmes;
   lisResponses = NULL;
   logErros = NULL;

   // Caminho para o diretório com os XMLs de alarmes
   pathArqBarramento = path->DirImporta() + "\\FaultLocation\\Alarmes";

   // Código do evento = nome do primeiro arquivo XML
   TAlarme* alarme = (TAlarme*) listaAlarmes->Items[0];
   CodigoEvento = alarme->GetCodAlimentador() + alarme->GetTimeStamp();
   // Caminho para o diretório com os arquivos INI formatados (dados de campo)
  	pathArqIni = path->DirDat() + "\\FaultLocation\\Dados\\" + CodigoEvento;
}
//---------------------------------------------------------------------------
__fastcall TImportaXMLs::~TImportaXMLs(void)
{
	// Destroi objetos
   if(lisResponses)
   {
      for(int i=lisResponses->Count-1; i>=0; i--)
      {
         StrReqResponse* resp = (StrReqResponse*) lisResponses->Items[i];
         delete resp;
      }
      delete lisResponses; lisResponses = NULL;
   }
}
//---------------------------------------------------------------------------
/***
 * Adiciona atuação e medição de disjuntor
 */
void __fastcall TImportaXMLs::AddDadosDisjuntor(String CodigoDisjuntor, String timestamp, std::complex<double>* V, std::complex<double>* I, double DistFalta, String TipoAtuacao)
{
	String pathArqDisjuntor;

	// Arquivo do Disjuntor
   pathArqDisjuntor = pathArqIni + "\\" + CodigoDisjuntor + ".ini";
  	TIniFile* file = new TIniFile(pathArqDisjuntor);

   // Insere informações
   file->WriteString("INFO", "horario", timestamp);
   if(Nulo(V, 3))
		file->WriteInteger("INFO", "num_med_V", 0);
   else
      file->WriteInteger("INFO", "num_med_V", 1);
   if(Nulo(I, 3))
		file->WriteInteger("INFO", "num_med_I", 0);
   else
      file->WriteInteger("INFO", "num_med_I", 1);
   if(DistFalta > 0.)
   	file->WriteFloat("INFO", "dist_falta", DistFalta);
   else
		file->WriteFloat("INFO", "dist_falta", 0.);
	file->WriteString("INFO", "tipo_atuacao", TipoAtuacao);

   // Adiciona tensões
   file->WriteFloat("TENSAO", "var", Round(real(V[0]) ,3));
   file->WriteFloat("TENSAO", "vai", Round(imag(V[0]) ,3));
   file->WriteFloat("TENSAO", "vbr", Round(real(V[1]) ,3));
   file->WriteFloat("TENSAO", "vbi", Round(imag(V[1]) ,3));
   file->WriteFloat("TENSAO", "vcr", Round(real(V[2]) ,3));
   file->WriteFloat("TENSAO", "vci", Round(imag(V[2]) ,3));

   // Adiciona correntes
   file->WriteFloat("CORRENTE", "iar", Round(real(I[0]) ,3));
   file->WriteFloat("CORRENTE", "iai", Round(imag(I[0]) ,3));
   file->WriteFloat("CORRENTE", "ibr", Round(real(I[1]) ,3));
   file->WriteFloat("CORRENTE", "ibi", Round(imag(I[1]) ,3));
   file->WriteFloat("CORRENTE", "icr", Round(real(I[2]) ,3));
   file->WriteFloat("CORRENTE", "ici", Round(imag(I[2]) ,3));


   // Atualiza arquivo Geral de equipamentos
   AddGeral("DISJUNTOR", CodigoDisjuntor);

   file->Free();
//   // Destroi arquivo INI
//   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Adiciona atuação e medição de disjuntor
 */
void __fastcall TImportaXMLs::AddGeral(String TipoEqpto, String CodigoEqpto)
{
	int num_disjuntores, num_religadoras, num_medidores_intel_balanco;
	String pathArqGeral;

	// Arquivo do Sensor
   pathArqGeral = pathArqIni + "\\Geral.ini";
  	TIniFile* file = new TIniFile(pathArqGeral);

	if(TipoEqpto == "MEDIDORINTELIGENTEBALANCO")
   {
		num_medidores_intel_balanco = file->ReadInteger("INFO", "num_medidores_inteligentes_balanco", -1);
		if(num_medidores_intel_balanco != -1)
		{
			num_medidores_intel_balanco += 1;
			file->WriteInteger("INFO", "num_medidores_inteligentes_balanco", num_medidores_intel_balanco);
			file->WriteString("MEDIDORESINTELIGENTESBALANCO", "cod_medidor_inteligente_balanco" + String(num_medidores_intel_balanco), CodigoEqpto);
		}
	}
   else if(TipoEqpto == "DISJUNTOR")
   {
   	num_disjuntores = file->ReadInteger("INFO", "num_disjuntores", -1);
		if(num_disjuntores != -1)
      {
         num_disjuntores += 1;
         file->WriteInteger("INFO", "num_disjuntores", num_disjuntores);
         file->WriteString("DISJUNTORES", "cod_disjuntor" + String(num_disjuntores), CodigoEqpto);
      }
   }
   else if(TipoEqpto == "RELIGADORA")
   {
   	num_religadoras = file->ReadInteger("INFO", "num_religadoras", -1);
		if(num_religadoras != -1)
      {
         num_religadoras += 1;
			file->WriteInteger("INFO", "num_religadoras", num_religadoras);
         file->WriteString("RELIGADORAS", "cod_religadora" + String(num_religadoras), CodigoEqpto);
      }
	}
	delete file;
}
//---------------------------------------------------------------------------
/***
 * Adiciona atuação e medição de religadora
 */
void __fastcall TImportaXMLs::AddDadosReligadora(String CodigoReligadora, String timestamp, std::complex<double>* V, std::complex<double>* I, String TipoAtuacao)
{
	String pathArqReligadora;

	// Arquivo da religadora
   pathArqReligadora = pathArqIni + "\\" + CodigoReligadora + ".ini";
  	TIniFile* file = new TIniFile(pathArqReligadora);

   file->WriteString("INFO", "horario", timestamp);
   if(Nulo(V, 3))
		file->WriteInteger("INFO", "num_med_V", 0);
   else
      file->WriteInteger("INFO", "num_med_V", 1);
   if(Nulo(I, 3))
		file->WriteInteger("INFO", "num_med_I", 0);
   else
      file->WriteInteger("INFO", "num_med_I", 1);
   file->WriteString("INFO", "tipo_atuacao", TipoAtuacao);

   // Adiciona tensões
   file->WriteFloat("TENSAO", "var", Round(real(V[0]), 3));
   file->WriteFloat("TENSAO", "vai", Round(imag(V[0]), 3));
   file->WriteFloat("TENSAO", "vbr", Round(real(V[1]), 3));
   file->WriteFloat("TENSAO", "vbi", Round(imag(V[1]), 3));
   file->WriteFloat("TENSAO", "vcr", Round(real(V[2]), 3));
   file->WriteFloat("TENSAO", "vci", Round(imag(V[2]), 3));

   // Adiciona correntes
   file->WriteFloat("CORRENTE", "iar", Round(real(I[0]), 3));
   file->WriteFloat("CORRENTE", "iai", Round(imag(I[0]), 3));
   file->WriteFloat("CORRENTE", "ibr", Round(real(I[1]), 3));
   file->WriteFloat("CORRENTE", "ibi", Round(imag(I[1]), 3));
   file->WriteFloat("CORRENTE", "icr", Round(real(I[2]), 3));
   file->WriteFloat("CORRENTE", "ici", Round(imag(I[2]), 3));

   // Atualiza arquivo Geral de equipamentos
   AddGeral("RELIGADORA", CodigoReligadora);

   file->Free();
//   // Destroi arquivo INI
//   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
bool __fastcall TImportaXMLs::FaltaJusante(String CodigoSensor)
{
   if(!listaAlarmes)
   	return false;

	for(int i=0; i<listaAlarmes->Count; i++)
   {
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];
      if(alarme->GetCodEqpto() == CodigoSensor)
      {
         if(alarme->GetTipoAlarme() == 3)
         	return false;
			else if(alarme->GetTipoAlarme() == 4)
         	return true;
      }
   }

   return false;
}
//---------------------------------------------------------------------------
/***
 * Adiciona medições de correntes registradas por um medidor inteligente
 */
void __fastcall TImportaXMLs::AddDadosMedidorInteligente(String CodigoMedidor, String timestamp, std::complex<double>* Vposfalta)
{
	String pathArqMedInteligente;

	// Arquivo do Medidor Inteligente
	pathArqMedInteligente = pathArqIni + "\\" + CodigoMedidor + ".ini";
	TIniFile* file = new TIniFile(pathArqMedInteligente);

   file->WriteString("INFO", "horario", timestamp);
   file->WriteInteger("INFO", "num_ev_falta", 1);
	file->WriteBool("INFO", "falta_jusante", false);
	file->WriteString("INFO", "qualidade", "GOOD");

	// Medições de tensões de fase pós-falta
	file->WriteFloat("POSFALTA", "var", Round(real(Vposfalta[0]), 3));
	file->WriteFloat("POSFALTA", "vai", Round(imag(Vposfalta[0]), 3));
	file->WriteFloat("POSFALTA", "vbr", Round(real(Vposfalta[1]), 3));
	file->WriteFloat("POSFALTA", "vbi", Round(imag(Vposfalta[1]), 3));
	file->WriteFloat("POSFALTA", "vcr", Round(real(Vposfalta[2]), 3));
	file->WriteFloat("POSFALTA", "vci", Round(imag(Vposfalta[2]), 3));

	// Atualiza arquivo Geral de equipamentos
	AddGeral("MEDIDORINTELIGENTEBALANCO", CodigoMedidor);

	delete file;
}
//---------------------------------------------------------------------------
/***
 * Adiciona medições de correntes registradas por um sensor
 */
void __fastcall TImportaXMLs::AddDadosSensor(String CodigoSensor, String timestamp, std::complex<double>* Ifalta, std::complex<double>* Ifalta_jusante, String Qualidade)
{
	String cod_sensor, pathArqSensor;

	// Verifica se a falta foi à jusante (AIM) ou não (FPE), com base nos alarmes recebidos
	bool faltaJusante = FaltaJusante(CodigoSensor);

	// Arquivo do Sensor
   pathArqSensor = pathArqIni + "\\" + CodigoSensor + ".ini";
  	TIniFile* file = new TIniFile(pathArqSensor);

   file->WriteString("INFO", "horario", timestamp);
   file->WriteInteger("INFO", "num_ev_falta", 1);
	file->WriteBool("INFO", "falta_jusante", faltaJusante);
   file->WriteString("INFO", "qualidade", Qualidade);

	// Medições de corrente de falta (com desligamento)
	file->WriteFloat("FALTA", "iar", Round(real(Ifalta[0]), 3));
	file->WriteFloat("FALTA", "iai", Round(imag(Ifalta[0]), 3));
	file->WriteFloat("FALTA", "ibr", Round(real(Ifalta[1]), 3));
	file->WriteFloat("FALTA", "ibi", Round(imag(Ifalta[1]), 3));
	file->WriteFloat("FALTA", "icr", Round(real(Ifalta[2]), 3));
   file->WriteFloat("FALTA", "ici", Round(imag(Ifalta[2]), 3));

	// Atualiza arquivo Geral de equipamentos
	AddGeral("SENSOR", CodigoSensor);

   file->Free();
//   // Destroi arquivo INI
//   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::AddDadosTrafoInteligente(String CodigoTrafoInteligente, String timestamp, std::complex<double>* V)
{
	String pathArqTrafoInteligente;

	// Arquivo do trafo inteligente
   pathArqTrafoInteligente = pathArqIni + "\\" + CodigoTrafoInteligente + ".ini";
  	TIniFile* file = new TIniFile(pathArqTrafoInteligente);

   file->WriteString("INFO", "horario", timestamp);
   if(Nulo(V, 3))
		file->WriteInteger("INFO", "num_med_V", 0);
   else
      file->WriteInteger("INFO", "num_med_V", 1);

   // Adiciona tensões
   file->WriteFloat("TENSAO", "var", Round(real(V[0]), 3));
   file->WriteFloat("TENSAO", "vai", Round(imag(V[0]), 3));
   file->WriteFloat("TENSAO", "vbr", Round(real(V[1]), 3));
   file->WriteFloat("TENSAO", "vbi", Round(imag(V[1]), 3));
   file->WriteFloat("TENSAO", "vcr", Round(real(V[2]), 3));
   file->WriteFloat("TENSAO", "vci", Round(imag(V[2]), 3));

   // Atualiza arquivo Geral de equipamentos
   AddGeral("TRAFOINTELIGENTE", CodigoTrafoInteligente);

   file->Free();
}
//---------------------------------------------------------------------------
/***
 * Adiciona medições de tensões registradas por um qualímetro
 */
void __fastcall TImportaXMLs::AddDadosQualimetro(String CodigoQualimetro, String timestamp, std::complex<double>* V, std::complex<double>* I)
{
	String pathArqQualimetro;

	// Arquivo do qualímetro
   pathArqQualimetro = pathArqIni + "\\" + CodigoQualimetro + ".ini";
  	TIniFile* file = new TIniFile(pathArqQualimetro);

   file->WriteString("INFO", "horario", timestamp);
   if(Nulo(V, 3))
		file->WriteInteger("INFO", "num_med_V", 0);
   else
      file->WriteInteger("INFO", "num_med_V", 1);
   if(Nulo(I, 3))
		file->WriteInteger("INFO", "num_med_I", 0);
   else
      file->WriteInteger("INFO", "num_med_I", 1);

   // Adiciona tensões
   file->WriteFloat("TENSAO", "var", Round(real(V[0]), 3));
   file->WriteFloat("TENSAO", "vai", Round(imag(V[0]), 3));
   file->WriteFloat("TENSAO", "vbr", Round(real(V[1]), 3));
   file->WriteFloat("TENSAO", "vbi", Round(imag(V[1]), 3));
   file->WriteFloat("TENSAO", "vcr", Round(real(V[2]), 3));
   file->WriteFloat("TENSAO", "vci", Round(imag(V[2]), 3));

   // Adiciona correntes
   file->WriteFloat("CORRENTE", "iar", Round(real(I[0]), 3));
   file->WriteFloat("CORRENTE", "iai", Round(imag(I[0]), 3));
   file->WriteFloat("CORRENTE", "ibr", Round(real(I[1]), 3));
   file->WriteFloat("CORRENTE", "ibi", Round(imag(I[1]), 3));
   file->WriteFloat("CORRENTE", "icr", Round(real(I[2]), 3));
   file->WriteFloat("CORRENTE", "ici", Round(imag(I[2]), 3));

   // Atualiza arquivo Geral de equipamentos
   AddGeral("QUALIMETRO", CodigoQualimetro);

   file->Free();
}
//---------------------------------------------------------------------------
/***
 * Problema: quando da atuação de disjuntor (DJ) ou religadora (RE), o SAGE
 * só registra o valor de corrente que disparou a proteção. Ou seja, não existe
 * a informação das fases afetadas.
 *
 * APROXIMAÇÃO ADOTADA:
 * Se houver atuação da proteção de neutro     ===>  mantém corrente medida na fase A
 * Se NÃO houver atuação da proteção de neutro ===> supõe-se defeito AB (Ia = Ib)
 */
void __fastcall TImportaXMLs::AjustaCorrentesMedicao()
{
	for(int i=0; i<lisResponses->Count; i++)
   {
		StrReqResponse* resp = (StrReqResponse*) lisResponses->Items[i];

      if(resp->TipoEqpto != "DJ" && resp->TipoEqpto != "RE") continue;

      if(resp->TipoAtuacao == "Fase")
      {
       	resp->IB = resp->IA;
      }
   }
}
//---------------------------------------------------------------------------
/***
 * Verifica se pelo menos uma das funções de proteção foi sensibilizada pelo defeito
 */
bool __fastcall TImportaXMLs::AtuacaoProtecao(StrReqResponse* resp)
{
   double horaE, minutoE, segundoE;
   double hora, minuto, segundo;
   double deltaMinuto;
   int anoE, mesE, diaE;
   int ano, mes, dia;


   if(resp->Funcao50 == "False" &&
   	resp->Funcao51 == "False" &&
      resp->Funcao50N == "False" &&
      resp->Funcao51N == "False")
   {
      return false;
   }
   else
   {
   	return true;
   }

//   // **********
//   double deltaMinutoMax = 2.;
//	// **********
//
//	// TimeStamp do evento
//   anoE = (resp->TimeStampEvento.SubString(1,4)).ToInt();
//   mesE = (resp->TimeStampEvento.SubString(5,2)).ToInt();
//   diaE = (resp->TimeStampEvento.SubString(7,2)).ToInt();
//   horaE = (resp->TimeStampEvento.SubString(9,2)).ToDouble();
//   minutoE = (resp->TimeStampEvento.SubString(11,2)).ToDouble();
//   segundoE = (resp->TimeStampEvento.SubString(13,2)).ToDouble();
//
//   // função 50
//   ano = (resp->TimeStamp50.SubString(1,4)).ToInt();
//   mes = (resp->TimeStamp50.SubString(5,2)).ToInt();
//   dia = (resp->TimeStamp50.SubString(7,2)).ToInt();
//   if(ano == anoE && mes == mesE && dia == diaE)
//   {
//   	hora = (resp->TimeStamp50.SubString(9,2)).ToInt();
//      minuto = (resp->TimeStamp50.SubString(11,2)).ToInt();
//      segundo = (resp->TimeStamp50.SubString(13,2)).ToInt();
//		deltaMinuto = 60.*(hora - horaE) + (minuto - minutoE) + (segundo - segundoE)/60.;
//
//      if(deltaMinuto <= deltaMinutoMax)
//      	return true;
//   }
//
//   // função 51
//   ano = (resp->TimeStamp51.SubString(1,4)).ToInt();
//   mes = (resp->TimeStamp51.SubString(5,2)).ToInt();
//   dia = (resp->TimeStamp51.SubString(7,2)).ToInt();
//   if(ano == anoE && mes == mesE && dia == diaE)
//   {
//   	hora = (resp->TimeStamp51.SubString(9,2)).ToInt();
//      minuto = (resp->TimeStamp51.SubString(11,2)).ToInt();
//      segundo = (resp->TimeStamp51.SubString(13,2)).ToInt();
//		deltaMinuto = 60.*(hora - horaE) + (minuto - minutoE) + (segundo - segundoE)/60.;
//
//      if(deltaMinuto <= deltaMinutoMax)
//      	return true;
//   }
//
//   // função 50N
//   ano = (resp->TimeStamp50N.SubString(1,4)).ToInt();
//   mes = (resp->TimeStamp50N.SubString(5,2)).ToInt();
//   dia = (resp->TimeStamp50N.SubString(7,2)).ToInt();
//   if(ano == anoE && mes == mesE && dia == diaE)
//   {
//   	hora = (resp->TimeStamp50N.SubString(9,2)).ToInt();
//      minuto = (resp->TimeStamp50N.SubString(11,2)).ToInt();
//      segundo = (resp->TimeStamp50N.SubString(13,2)).ToInt();
//		deltaMinuto = 60.*(hora - horaE) + (minuto - minutoE) + (segundo - segundoE)/60.;
//
//      if(deltaMinuto <= deltaMinutoMax)
//      	return true;
//   }
//
//   // função 51N
//   ano = (resp->TimeStamp51N.SubString(1,4)).ToInt();
//   mes = (resp->TimeStamp51N.SubString(5,2)).ToInt();
//   dia = (resp->TimeStamp51N.SubString(7,2)).ToInt();
//   if(ano == anoE && mes == mesE && dia == diaE)
//   {
//   	hora = (resp->TimeStamp51N.SubString(9,2)).ToInt();
//      minuto = (resp->TimeStamp51N.SubString(11,2)).ToInt();
//      segundo = (resp->TimeStamp51N.SubString(13,2)).ToInt();
//		deltaMinuto = 60.*(hora - horaE) + (minuto - minutoE) + (segundo - segundoE)/60.;
//
//      if(deltaMinuto <= deltaMinutoMax)
//      	return true;
//   }
//
//
//   return false;
}
//---------------------------------------------------------------------------
//void __fastcall TImportaXMLs::AvaliaMedicoesIndisponiveis(StrDadosLog* strLog)
//{
//	String item;
//	StrReqResponse* resp;
//
//	for(int i=0; i<lisResponses->Count; i++)
//   {
//   	resp = (StrReqResponse*) lisResponses->Items[i];
//      if(resp->TipoEqpto == "DJ")
//      {
//         if(resp->IA == "0")
//         {
//				item = resp->CodEqpto + ";I;" + resp->Resposta_mensagem;
//            strLog->lisUnavailableMeasurements->Add(item);
//         }
//      }
//      else if(resp->TipoEqpto == "SR")
//      {
//         if(resp->IA == "0" && resp->IB == "0" && resp->IC == "0")
//         {
//            item = resp->CodEqpto + ";I" + resp->Resposta_mensagem;
//            strLog->lisUnavailableMeasurements->Add(item);
//         }
//      }
//
//
//      // CONTINUAR PARA DEMAIS EQUIPAMENTOS
//      // ...
//   }
//}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::AvaliaErrosAquisicao(StrDadosLog* strLog)
{
	StrReqResponse* resp;

	for(int i=0; i<lisResponses->Count; i++)
   {
   	resp = (StrReqResponse*) lisResponses->Items[i];

		if(resp->Resposta_erro != "False")
      {
      	strLog->lisEqptosErrosAquisicao->Add(resp->TipoEqpto + ";" + resp->CodEqpto);
      }

   }
}
//---------------------------------------------------------------------------
//void __fastcall TImportaXMLs::AvaliaSensoresIndisponíveis(StrDadosLog* strLog)
//{
//	StrReqResponse* resp;
//   TStringList* lisUnavailableSensors;
//
//   lisUnavailableSensors = new TStringList();
//
//	for(int i=0; i<lisResponses->Count; i++)
//   {
//   	resp = (StrReqResponse*) lisResponses->Items[i];
//      if(resp->TipoEqpto != "SR") continue;
//
//      if(resp->Qualidade == "BAD")
//      {
//         lisUnavailableSensors->Add(resp->CodEqpto);
//      }
//   }
//
//   // Insere código do sensor na lista de sensores indisponíveis
//   strLog->lisUnavailableSensors = lisUnavailableSensors;
//}
//---------------------------------------------------------------------------
/***
 * Método para leitura dos XML
 *
 *   !!!!   INCREMENTAR NESTE MÉTODO A DESSEREALIZAÇÃO   !!!!
 */
void __fastcall TImportaXMLs::ConverteArquivosXML()
{

	// !!!  PROVISÓRIO:
   double Ia, Ib, Ic;
   double Va, Vb, Vc;
   double thetaA, thetaB, thetaC, DistFalta;

   std::complex<double> vetorV[3], vetorI[3];
   std::complex<double> vetorIfalta[3], vetorIfalta_jusante[3];

   // ::::::::::::::::::::::::::::
   // Lê um arquivo XML de sensor
   // ::::::::::::::::::::::::::::
//	Ia = 98;
//   Ib = 1.84;
//   Ic = 1.43;
//	thetaA = 117 * M_PI / 180.;
//	thetaB = 32 * M_PI / 180.;
//   thetaC = -80 * M_PI / 180.;
//	Ia = 280.;
//   Ib = 280.;
//   Ic = 1.43;
//	thetaA = -30. * M_PI / 180.;
//	thetaB = 150. * M_PI / 180.;
//   thetaC = -110. * M_PI / 180.;
//   vetorIfalta[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
//   vetorIfalta[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
//   vetorIfalta[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));
//
//   vetorIfalta_jusante[0] = vetorIfalta_jusante[1] = vetorIfalta_jusante[2] = std::complex<double>(0., 0.);
//   // Gera obj TSensor com as medições do arquivo
//   AddDadosSensor("SensorA45176", "20170928101010", vetorIfalta, vetorIfalta_jusante);



   // ::::::::::::::::::::::::::::
   // Lê XML de disjuntor
   // ::::::::::::::::::::::::::::
	Va = 7882.5;
   Vb = 7447.5;
   Vc = 7965.5;
	thetaA = -34 * M_PI/180.;
	thetaB = -151 * M_PI/180.;
   thetaC = 89 * M_PI/180.;
   vetorV[0] = std::complex<double>(Va * cos(thetaA), Va * sin(thetaA));
   vetorV[1] = std::complex<double>(Vb * cos(thetaB), Vb * sin(thetaB));
   vetorV[2] = std::complex<double>(Vc * cos(thetaC), Vc * sin(thetaC));
	Ia = 330.81;
   Ib = 327.9;
   Ic = 4.94;
	thetaA = -36.2 * M_PI / 180.;
	thetaB = 144.7 * M_PI / 180.;
   thetaC = 81.3 * M_PI / 180.;
   vetorI[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
   vetorI[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
   vetorI[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));
   DistFalta = 20.834;
   AddDadosDisjuntor("A79416", "20170928101010", vetorV, vetorI, DistFalta, "Fase");

   /*
	Va = 7882.5;
   Vb = 7447.5;
   Vc = 7965.5;
	thetaA = -34 * M_PI/180.;
	thetaB = -151 * M_PI/180.;
   thetaC = 89 * M_PI/180.;
   vetorV[0] = std::complex<double>(Va * cos(thetaA), Va * sin(thetaA));
   vetorV[1] = std::complex<double>(Vb * cos(thetaB), Vb * sin(thetaB));
   vetorV[2] = std::complex<double>(Vc * cos(thetaC), Vc * sin(thetaC));
	Ia = 135.;
   Ib = 131.;
   Ic = 4.;
	thetaA = -36.2 * M_PI / 180.;
	thetaB = 144.7 * M_PI / 180.;
   thetaC = 81.3 * M_PI / 180.;
   vetorI[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
   vetorI[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
   vetorI[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));
   DistFalta = 20.834;
   AddDadosDisjuntor("DisjuntorA79416", "20170928101010", vetorV, vetorI, DistFalta);
   */

//   // Lê XML de religadora
//   vetorV[0] = vetorV[1] = vetorV[2] = std::complex<double>(0., 0.);
////   vetorI[0] = vetorI[1] = vetorI[2] = std::complex<double>(3800., -80.);
//	theta1 = -30.9 * M_PI/180.;
//	theta2 = 149 * M_PI/180.;
//   theta3 = 176.7 * M_PI/180.;
//   vetorI[0] = std::complex<double>(176.27 * cos(theta1), 176.27 * sin(theta1));
//   vetorI[1] = std::complex<double>(175.44 * cos(theta2), 175.44 * sin(theta2));
//   vetorI[2] = std::complex<double>(0.704 * cos(theta3), 0.704 * sin(theta3));
//   AddDadosReligadora("ReligadoraA45488", "20170928101010", vetorV, vetorI);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::DeterminaAtuacaoProtecao()
{
   double horaE, minutoE, segundoE;
   double hora, minuto, segundo;
   double deltaMinuto;
   int anoE, mesE, diaE;
   int ano, mes, dia;
   String TipoAtuacao;

//   // **********
//   double deltaMinutoMax = 2.;
//   // **********

	for(int i=0; i<lisResponses->Count; i++)
   {
		StrReqResponse* resp = (StrReqResponse*) lisResponses->Items[i];

      if(resp->TipoEqpto != "DJ" && resp->TipoEqpto != "RE") continue;

      TipoAtuacao = "";

      if(resp->Funcao50 == "True" || resp->Funcao51 == "True")
      {
      	TipoAtuacao = "Fase";
      }

      if(resp->Funcao50N == "True" || resp->Funcao51N == "True")
      {
         TipoAtuacao = "Neutro";
      }
      // Salva o Tipo de Atuação determinado
      resp->TipoAtuacao = TipoAtuacao;
   }

}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraBaseDados()
{
   // Cria diretório para o evento
   CreateDir(pathArqIni);

   // Cria arquivo geral para os nomes dos eqptos com dados
   String pathArqGeral = pathArqIni + "\\Geral.ini";
  	TIniFile* file = new TIniFile(pathArqGeral);

	file->WriteInteger("INFO", "num_disjuntores", 0);
	file->WriteInteger("INFO", "num_religadoras", 0);
	file->WriteInteger("INFO", "num_medidores_inteligentes_balanco", 0);

   file->Free();
//   // Destroi objeto
//   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::InsereDadosAdicionais(StrDadosLog* strLog)
{
	StrReqResponse* reqResponse;

   for(int i=0; i<lisResponses->Count; i++)
   {
   	reqResponse = (StrReqResponse*) lisResponses->Items[i];

      if(reqResponse->TipoEqpto == "DJ")
      {
      	strLog->CodigoReleSubestacao = reqResponse->CodEqpto;
      	strLog->DistFaltaRele = reqResponse->DistFaltaRele;
      }
   }
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::InsereInsumosDados(StrDadosLog* strLog)
{
	String item;
	StrReqResponse* reqResponse;
	StrInsumoDado* dataInput;
   StrDadoRuim* badData;
   String type, error, message;

   for(int i=0; i<lisResponses->Count; i++)
   {
   	reqResponse = (StrReqResponse*) lisResponses->Items[i];

		if(reqResponse->TipoEqpto == "MI")
      {
			dataInput = new StrInsumoDado();
	      dataInput->codEqpto = reqResponse->CodEqpto;
			dataInput->tipoEqpto = "Medidor inteligente";

			// Adiciona as tensões instantâneas pós-falta medidas por medidor inteligente
			dataInput->VA_posfalta_medidor_inteligente = reqResponse->VA;
			dataInput->VB_posfalta_medidor_inteligente = reqResponse->VB;
			dataInput->VC_posfalta_medidor_inteligente = reqResponse->VC;

			// Salva na lista de insumos de dados do log
			strLog->lisInputData->Add(dataInput);
		}
		else if(reqResponse->TipoEqpto == "DJ")
      {
      	dataInput = new StrInsumoDado();
	      dataInput->codEqpto = reqResponse->CodEqpto;
         dataInput->tipoEqpto = "Relay";
         // Adiciona a corrente de falta medida e utilizada pelo algoritmo
         dataInput->Ifalta_DJ_RE = reqResponse->IA;

         // Salva na lista de insumos de dados do log
         strLog->lisInputData->Add(dataInput);
      }
      else if(reqResponse->TipoEqpto == "RE")
      {
      	dataInput = new StrInsumoDado();
	      dataInput->codEqpto = reqResponse->CodEqpto;
         dataInput->tipoEqpto = "Relay";
         // Adiciona a corrente de falta medida e utilizada pelo algoritmo
         dataInput->Ifalta_DJ_RE = reqResponse->IA;

         // Salva na lista de insumos de dados do log
         strLog->lisInputData->Add(dataInput);
      }
      else if(reqResponse->TipoEqpto == "QUALIM")
      {
      	dataInput = new StrInsumoDado();
	      dataInput->codEqpto = reqResponse->CodEqpto;
         dataInput->tipoEqpto = "Qualimetro";
         // Adiciona a corrente de falta medida e utilizada pelo algoritmo
         dataInput->VA_falta_qualimetro = reqResponse->VA;
         dataInput->VB_falta_qualimetro = reqResponse->VB;
         dataInput->VC_falta_qualimetro = reqResponse->VC;
         dataInput->IA_falta_qualimetro = reqResponse->IA;
         dataInput->IB_falta_qualimetro = reqResponse->IB;
         dataInput->IC_falta_qualimetro = reqResponse->IC;

         // Salva na lista de insumos de dados do log
         strLog->lisInputData->Add(dataInput);
      }
      else if(reqResponse->TipoEqpto == "TRAFOINTELIGENTE")
      {
      	dataInput = new StrInsumoDado();
	      dataInput->codEqpto = reqResponse->CodEqpto;
         dataInput->tipoEqpto = "TrafoInteligente";
         // Adiciona a corrente de falta medida e utilizada pelo algoritmo
         dataInput->VA_falta_trafo_inteligente = reqResponse->VA;
         dataInput->VB_falta_trafo_inteligente = reqResponse->VB;
         dataInput->VC_falta_trafo_inteligente = reqResponse->VC;

         // Salva na lista de insumos de dados do log
         strLog->lisInputData->Add(dataInput);
      }
      else if(reqResponse->TipoEqpto == "SR")
      {
      	// Verifica a qualidade do dado do sensor
      	if(reqResponse->Qualidade != "GOOD")
         {
            badData = new StrDadoRuim();
            badData->tipoEqpto = "Sensor";
            badData->codEqpto = reqResponse->CodEqpto;

            // Salva na lista de dados ruins do log
            strLog->lisBadData->Add(badData);
         }
         else
         {
            dataInput = new StrInsumoDado();
            dataInput->codEqpto = reqResponse->CodEqpto;
            dataInput->tipoEqpto = "Sensor";
            // Adiciona as correntes de falta medidas e utilizadas pelo algoritmo
            dataInput->IA_falta_sensor = reqResponse->IA;
            dataInput->IB_falta_sensor = reqResponse->IB;
            dataInput->IC_falta_sensor = reqResponse->IC;

            // Salva na lista de insumos de dados do log
            strLog->lisInputData->Add(dataInput);
         }
      }
   }
}
//---------------------------------------------------------------------------
/***
 * Método para ler e validar os dados que chegaram via XML
 */
void __fastcall TImportaXMLs::LeValidaDadosXML(String nomeXMLresponses, TXMLComunicacao* xmlRequest, StrDadosLog* strLog)
{
   String CodEqpto;
	TXMLParser* xmlParser;

   // Cria obj de parser para XML
   xmlParser = new TXMLParser(apl, formFL);
   xmlParser->SetLogErros(logErros);

	// Faz a leitura do XML de responses
	lisResponses = new TList();
	xmlParser->GetReqResponse(nomeXMLresponses, xmlRequest, lisResponses);

   InsereInsumosDados(strLog);
	InsereDadosAdicionais(strLog);

	// Avalia indisponibilidade de equipamentos
   AvaliaErrosAquisicao(strLog);

	// Faz ajustes nas responses
//	ValidaDados();
//   DeterminaAtuacaoProtecao();

   // Gera arquivos INI
	GeraINIFormatados();
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatados()
{
	StrReqResponse* resp;

	for(int i=0; i<lisResponses->Count; i++)
   {
		resp = (StrReqResponse*) lisResponses->Items[i];

		if(resp->TipoEqpto == "MI")
			GeraINIFormatado_MedidorInteligente(resp);
		else if(resp->TipoEqpto == "DJ")
			GeraINIFormatado_Disjuntor(resp);
      else if(resp->TipoEqpto == "RE")
         GeraINIFormatado_Religadora(resp);
      else if(resp->TipoEqpto == "SR")
         GeraINIFormatado_Sensor(resp);
      else if(resp->TipoEqpto == "QUALIM")
         GeraINIFormatado_Qualimetro(resp);
      else if(resp->TipoEqpto == "TRAFOINTELIGENTE")
         GeraINIFormatado_TrafoInteligente(resp);
   }
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatado_MedidorInteligente(StrReqResponse* resp)
{
	double Va, Vb, Vc;
	double thetaA, thetaB, thetaC;
	std::complex<double> vetorV_posfalta[3];

	for(int i=0; i<3; i++)
		vetorV_posfalta[i] = std::complex<double>(0., 0.);

	if(resp->VA != "") Va = resp->VA.ToDouble(); else Va = 0.;
	if(resp->VB != "") Vb = resp->VB.ToDouble(); else Vb = 0.;
	if(resp->VC != "") Vc = resp->VC.ToDouble(); else Vc = 0.;
	thetaA = 0. * M_PI / 180.;
	thetaB = -120. * M_PI / 180.;
   thetaC = 120. * M_PI / 180.;
	vetorV_posfalta[0] = std::complex<double>(Va * cos(thetaA), Va * sin(thetaA));
	vetorV_posfalta[1] = std::complex<double>(Vb * cos(thetaB), Vb * sin(thetaB));
	vetorV_posfalta[2] = std::complex<double>(Vc * cos(thetaC), Vc * sin(thetaC));

	AddDadosMedidorInteligente(resp->CodEqpto, resp->TimeStampEvento, vetorV_posfalta);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatado_Disjuntor(StrReqResponse* resp)
{
	double Ia, Ib, Ic;
   double Va, Vb, Vc;
   double thetaA, thetaB, thetaC, DistFalta;
   std::complex<double> vetorV[3], vetorI[3];
   std::complex<double> vetorIfalta[3], vetorIfalta_jusante[3];

   // Tensões
   Va = 0;
   Vb = 0;
   Vc = 0;
   thetaA = 0 * M_PI/180.;
   thetaB = -120 * M_PI/180.;
   thetaC = 120 * M_PI/180.;
   vetorV[0] = std::complex<double>(Va * cos(thetaA), Va * sin(thetaA));
   vetorV[1] = std::complex<double>(Vb * cos(thetaB), Vb * sin(thetaB));
   vetorV[2] = std::complex<double>(Vc * cos(thetaC), Vc * sin(thetaC));

   // Correntes
   if(resp->IA != "") Ia = resp->IA.ToDouble(); else Ia = 0.;
   if(resp->IB != "") Ib = resp->IB.ToDouble(); else Ib = 0.;
   if(resp->IC != "") Ic = resp->IC.ToDouble(); else Ic = 0.;
   thetaA = 0. * M_PI / 180.;
   thetaB = -120. * M_PI / 180.;
	thetaC = 120. * M_PI / 180.;
   vetorI[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
   vetorI[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
   vetorI[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));
   DistFalta = resp->DistFaltaRele.ToDouble();

	AddDadosDisjuntor(resp->CodEqpto, resp->TimeStampEvento, vetorV, vetorI, DistFalta, resp->TipoAtuacao);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatado_Religadora(StrReqResponse* resp)
{
   double Ia, Ib, Ic;
   double Va, Vb, Vc;
   double thetaA, thetaB, thetaC;
   std::complex<double> vetorV[3], vetorI[3];
   std::complex<double> vetorIfalta[3], vetorIfalta_jusante[3];

   // Tensões
   Va = 0;
   Vb = 0;
   Vc = 0;
   thetaA = 0 * M_PI/180.;
   thetaB = -120 * M_PI/180.;
   thetaC = 120 * M_PI/180.;
   vetorV[0] = std::complex<double>(Va * cos(thetaA), Va * sin(thetaA));
   vetorV[1] = std::complex<double>(Vb * cos(thetaB), Vb * sin(thetaB));
   vetorV[2] = std::complex<double>(Vc * cos(thetaC), Vc * sin(thetaC));

   // Correntes
   if(resp->IA != "") Ia = resp->IA.ToDouble(); else Ia = 0.;
   if(resp->IB != "") Ib = resp->IB.ToDouble(); else Ib = 0.;
   if(resp->IC != "") Ic = resp->IC.ToDouble(); else Ic = 0.;
   thetaA = 0. * M_PI / 180.;
   thetaB = -120. * M_PI / 180.;
   thetaC = 120. * M_PI / 180.;
   vetorI[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
   vetorI[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
   vetorI[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));

   AddDadosReligadora(resp->CodEqpto, resp->TimeStampEvento, vetorV, vetorI, resp->TipoAtuacao);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatado_Sensor(StrReqResponse* resp)
{
	double Ia, Ib, Ic;
	double thetaA, thetaB, thetaC;
	std::complex<double> vetorIfalta[3], vetorIfalta_jusante[3];

	for(int i=0; i<3; i++)
      vetorIfalta_jusante[i] = std::complex<double>(0., 0.);

   if(resp->IA != "") Ia = resp->IA.ToDouble(); else Ia = 0.;
   if(resp->IB != "") Ib = resp->IB.ToDouble(); else Ib = 0.;
   if(resp->IC != "") Ic = resp->IC.ToDouble(); else Ic = 0.;
   thetaA = 0. * M_PI / 180.;
	thetaB = -120. * M_PI / 180.;
   thetaC = 120. * M_PI / 180.;
	vetorIfalta[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
   vetorIfalta[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
	vetorIfalta[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));

	AddDadosSensor(resp->CodEqpto, resp->TimeStampEvento, vetorIfalta, vetorIfalta_jusante, resp->Qualidade);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatado_TrafoInteligente(StrReqResponse* resp)
{
   double Va, Vb, Vc;
   double thetaVA, thetaVB, thetaVC;
   std::complex<double> vetorVfalta[3];

   // Insere medições dos fasores de tensão
   if(resp->VA != "")
   {
   	Va = resp->VA.ToDouble();
      thetaVA = resp->pVA.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Va = 0.;
      thetaVA = 0.;
   }
   if(resp->VB != "")
   {
   	Vb = resp->VB.ToDouble();
		thetaVB = resp->pVB.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Vb = 0.;
      thetaVB = 0.;
   }
   if(resp->VC != "")
   {
   	Vc = resp->VC.ToDouble();
		thetaVC = resp->pVC.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Vc = 0.;
      thetaVC = 0.;
   }

   vetorVfalta[0] = std::complex<double>(Va * cos(thetaVA), Va * sin(thetaVA));
   vetorVfalta[1] = std::complex<double>(Vb * cos(thetaVB), Vb * sin(thetaVB));
   vetorVfalta[2] = std::complex<double>(Vc * cos(thetaVC), Vc * sin(thetaVC));

	AddDadosTrafoInteligente(resp->CodEqpto, resp->TimeStampEvento, vetorVfalta);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::GeraINIFormatado_Qualimetro(StrReqResponse* resp)
{
   double Va, Vb, Vc, Ia, Ib, Ic;
   double thetaVA, thetaVB, thetaVC, thetaIA, thetaIB, thetaIC;
   std::complex<double> vetorVfalta[3], vetorIfalta[3];


   // Insere medições dos fasores de tensão
   if(resp->VA != "")
   {
   	Va = resp->VA.ToDouble();
      thetaVA = resp->pVA.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Va = 0.;
      thetaVA = 0.;
   }
   if(resp->VB != "")
   {
   	Vb = resp->VB.ToDouble();
		thetaVB = resp->pVB.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Vb = 0.;
      thetaVB = 0.;
   }
   if(resp->VC != "")
   {
   	Vc = resp->VC.ToDouble();
		thetaVC = resp->pVC.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Vc = 0.;
      thetaVC = 0.;
   }

	// Insere medições dos fasores de corrente
   if(resp->IA != "")
   {
   	Ia = resp->IA.ToDouble();
      thetaIA = resp->pIA.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Ia = 0.;
      thetaIA = 0.;
   }
   if(resp->IB != "")
   {
   	Ib = resp->IB.ToDouble();
		thetaIB = resp->pIB.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Ib = 0.;
      thetaIB = 0.;
   }
   if(resp->IC != "")
   {
   	Ic = resp->IC.ToDouble();
		thetaIC = resp->pIC.ToDouble() * M_PI / 180.;
   }
   else
   {
	   Ic = 0.;
      thetaIC = 0.;
   }

   vetorVfalta[0] = std::complex<double>(Va * cos(thetaVA), Va * sin(thetaVA));
   vetorVfalta[1] = std::complex<double>(Vb * cos(thetaVB), Vb * sin(thetaVB));
   vetorVfalta[2] = std::complex<double>(Vc * cos(thetaVC), Vc * sin(thetaVC));

   vetorIfalta[0] = std::complex<double>(Ia * cos(thetaIA), Ia * sin(thetaIA));
   vetorIfalta[1] = std::complex<double>(Ib * cos(thetaIB), Ib * sin(thetaIB));
   vetorIfalta[2] = std::complex<double>(Ic * cos(thetaIC), Ic * sin(thetaIC));

	AddDadosQualimetro(resp->CodEqpto, resp->TimeStampEvento, vetorVfalta, vetorIfalta);
}
//---------------------------------------------------------------------------
void __fastcall TImportaXMLs::SetLogErros(TLog* logErros)
{
	this->logErros = logErros;
}
//---------------------------------------------------------------------------
/***
 *  Verifica se realmente houve atuação de proteção
 */
void __fastcall TImportaXMLs::ValidaDados()
{
	for(int i=lisResponses->Count-1; i>=0; i--)
   {
		StrReqResponse* resp = (StrReqResponse*) lisResponses->Items[i];

      // Funções de proteção
      if(resp->TipoEqpto == "DJ" || resp->TipoEqpto == "RE")
      {
         if(!AtuacaoProtecao(resp))
         {
            lisResponses->Remove(resp);
         }
      }
   }
}
//---------------------------------------------------------------------------
//eof
