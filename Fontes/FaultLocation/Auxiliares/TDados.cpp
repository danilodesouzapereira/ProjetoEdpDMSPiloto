/***
 *  Classe para os dados que serão os inputs do algoritmo
 **/
//---------------------------------------------------------------------------
#include <vcl.h>
#include <System.hpp>
#pragma hdrstop
#include "TDados.h"
#include "TFuncoesDeRede.h"
#include "FuncoesFL.h"
#include "..\ComunicacaoXML\TAlarme.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <complex>
#include <cmath>
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <StrUtils.hpp>
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Ordena.h>
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Ordena\VTOrdena.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTEqpto.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TEqptoCampo.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TBarraSemTensao.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TChaveMonitorada.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TFusivel.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TMedidorInteligente.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TMedidorInteligenteBalanco.h>
//---------------------------------------------------------------------------
__fastcall TDados::TDados(VTApl* apl, TList* listaAlarmes)
{
	// Salva ponteiros
	this->apl = apl;
   path = (VTPath*)apl->GetObject(__classid(VTPath));
	redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
   funcoesRede = new TFuncoesDeRede(apl);

   TAlarme* alarme = (TAlarme*) listaAlarmes->Items[0];
   CodigoEvento = alarme->GetCodAlimentador() + alarme->GetTimeStamp();

   // Inicializa paths
   pathDados = path->DirDat() + "\\FaultLocation\\Dados";
	pathEvento = pathDados + "\\" + CodigoEvento;

   // Inicializa path do arquivo de cadastro dos monitores do alimentador em questão
   CodigoAlimentador = CodigoEvento.SubString(1, 7);

   // Inicializa objetos
   lisEqptosCampo = new TList();  //< Lista com os eqptos de dados (de campo)

   // Inicializa patamar horário
   patamar = -1;
}
//---------------------------------------------------------------------------
__fastcall TDados::TDados(VTApl* apl, String TimeStamp, String CodAlimentador)
{
	// Salva ponteiros
	this->apl = apl;
   path = (VTPath*)apl->GetObject(__classid(VTPath));
	redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
   funcoesRede = new TFuncoesDeRede(apl);

   CodigoEvento = CodAlimentador + TimeStamp;

   // Inicializa paths
   pathDados = path->DirDat() + "\\FaultLocation\\Dados";
	pathEvento = pathDados + "\\" + CodigoEvento;

   // Inicializa path do arquivo de cadastro dos monitores do alimentador em questão
   CodigoAlimentador = CodigoEvento.SubString(1, 7);

   // Inicializa objetos
   lisEqptosCampo = new TList();  //< Lista com os eqptos de dados (de campo)

   // Inicializa patamar horário
   patamar = -1;
}
//---------------------------------------------------------------------------
__fastcall TDados::~TDados(void)
{
	// Destroi objetos
   if(lisEqptosCampo)
   {
      for(int i=lisEqptosCampo->Count-1; i>=0; i--)
      {
         TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
         delete eqptoCampo;
      }
		delete lisEqptosCampo; lisEqptosCampo = NULL;
   }
}
//---------------------------------------------------------------------------
/****
 * Atualiza as medições de disjuntor a partir dos dados de alarme de disjuntor
 **/
void __fastcall TDados::AtualizaMedicoesDisjuntor(TChaveMonitorada* chvDJ, TAlarme* alarmeDisjuntor)
{
	if(!chvDJ || !alarmeDisjuntor) return;

	std::complex<double> vetorV[3], vetorI[3];
	double Ia, Ib, Ic, thetaA, thetaB, thetaC;

	// Proteção de sobrecorrente sensibilizada
	if(alarmeDisjuntor->funcao50N || alarmeDisjuntor->funcao51N)
	{
		chvDJ->TipoAtuacao = "Neutro";
	}
	else
	{
		chvDJ->TipoAtuacao = "Fase";
	}

	// Fase afetada
	bool alarmeFaseA = (alarmeDisjuntor->funcao50A || alarmeDisjuntor->funcao51A);
	bool alarmeFaseB = (alarmeDisjuntor->funcao50B || alarmeDisjuntor->funcao51B);
	bool alarmeFaseC = (alarmeDisjuntor->funcao50C || alarmeDisjuntor->funcao51C);

	if     (alarmeFaseA && !alarmeFaseB && !alarmeFaseC) chvDJ->faseAfetada = "A";
	else if(!alarmeFaseA && alarmeFaseB && !alarmeFaseC) chvDJ->faseAfetada = "B";
	else if(!alarmeFaseA && !alarmeFaseB && alarmeFaseC) chvDJ->faseAfetada = "C";
	else if(alarmeFaseA && alarmeFaseB && !alarmeFaseC)  chvDJ->faseAfetada = "AB";
	else if(alarmeFaseA && !alarmeFaseB && alarmeFaseC)  chvDJ->faseAfetada = "CA";
	else if(!alarmeFaseA && alarmeFaseB && alarmeFaseC)  chvDJ->faseAfetada = "BC";
	else if(alarmeFaseA && alarmeFaseB && alarmeFaseC)   chvDJ->faseAfetada = "ABC";

	// Correntes
	Ia = alarmeDisjuntor->correnteFalta;
	Ib = Ic = 0.;
	thetaA = 0. * M_PI / 180.;
   thetaB = -120. * M_PI / 180.;
	thetaC = 120. * M_PI / 180.;
	vetorI[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
	vetorI[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
	vetorI[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));

	// Tensões
	vetorV[0] = vetorV[1] = vetorV[2] = std::complex<double>(0., 0.);

	// Armazena tensões e correntes de medição
	for(int i=0; i<3; i++)
	{
		chvDJ->medicaoVI.falta.V[i] = vetorV[i];
		chvDJ->medicaoVI.falta.I[i] = vetorI[i];
	}
}
//---------------------------------------------------------------------------
/****
 * Atualiza as medições de disjuntor a partir de dados solicitados
 **/
void __fastcall TDados::AtualizaMedicoesDisjuntor(TChaveMonitorada* chvDJ)
{
   double v_re, v_im, i_re, i_im;
   double dist_falta;
	std::complex<double> I[3], V[3];
   std::complex<double> czero = std::complex<double>(0., 0.);
   for(int i=0; i<3; i++) {V[i] = czero; I[i] = czero;}
	int num_med_V = 0, num_med_I = 0;
   String ev_timestamp = "";
   TIniFile* file;
   String tipo_atuacao;

   // Proteção
   if(chvDJ == NULL)
   {
      return;
   }

   // Arquivo com as medições do disjuntor
   file = new TIniFile(pathDados + "\\" + CodigoEvento + "\\" + chvDJ->GetCodigo() + ".ini");

   ev_timestamp = file->ReadString("INFO", "horario", "");
   num_med_V = file->ReadInteger("INFO", "num_med_V", -1);
   num_med_I = file->ReadInteger("INFO", "num_med_I", -1);
   dist_falta = file->ReadFloat("INFO", "dist_falta", 0.);
   tipo_atuacao = file->ReadString("INFO", "tipo_atuacao", "");

   // Proteção
   if((num_med_V == -1) || (num_med_I == -1)) return;

   // Armazena timestamp do evento
   chvDJ->medicaoVI.timestamp = ev_timestamp;
   SetPatamarHorario(ev_timestamp);

   // Armazena a distância de falta
   chvDJ->DistFalta = dist_falta;

   // Armazena o tipo de atuação
   chvDJ->TipoAtuacao = tipo_atuacao;

   // Medições de Tensão
   if(num_med_V > 0)
   {
      // Va
      v_re = file->ReadFloat("TENSAO", "var", 0.);
      v_im = file->ReadFloat("TENSAO", "vai", 0.);
      V[0] = std::complex<double>(v_re, v_im);
		// Vb
      v_re = file->ReadFloat("TENSAO", "vbr", 0.);
      v_im = file->ReadFloat("TENSAO", "vbi", 0.);
      V[1] = std::complex<double>(v_re, v_im);
      // Vc
      v_re = file->ReadFloat("TENSAO", "vcr", 0.);
      v_im = file->ReadFloat("TENSAO", "vci", 0.);
      V[2] = std::complex<double>(v_re, v_im);

		// Armazena tensões e correntes de medição
		for(int i=0; i<3; i++)
		{
			chvDJ->medicaoVI.falta.V[i] = V[i];
		}
   }

   // Medições de Corrente
   if(num_med_I > 0)
   {
      // Ia
      i_re = file->ReadFloat("CORRENTE", "iar", 0.);
      i_im = file->ReadFloat("CORRENTE", "iai", 0.);
      I[0] = std::complex<double>(i_re, i_im);
      // Ib
      i_re = file->ReadFloat("CORRENTE", "ibr", 0.);
      i_im = file->ReadFloat("CORRENTE", "ibi", 0.);
      I[1] = std::complex<double>(i_re, i_im);
      // Ic
      i_re = file->ReadFloat("CORRENTE", "icr", 0.);
      i_im = file->ReadFloat("CORRENTE", "ici", 0.);
      I[2] = std::complex<double>(i_re, i_im);

      // Armazena tensões e correntes de medição
      for(int i=0; i<3; i++)
      {
         chvDJ->medicaoVI.falta.I[i] = I[i];
      }
   }

//   // Destroi obj de arquivo INI
//   file->Free();
   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
/****
 * Atualiza as medições de religadora a partir dos dados de alarme de religadora
 **/
void __fastcall TDados::AtualizaMedicoesReligadora(TChaveMonitorada* chvRE, TAlarme* alarmeReligadora)
{
	if(!chvRE || !alarmeReligadora) return;

	std::complex<double> vetorV[3], vetorI[3];
	double Ia, Ib, Ic, thetaA, thetaB, thetaC;

	// Proteção de sobrecorrente sensibilizada
	if(alarmeReligadora->funcao50N || alarmeReligadora->funcao51N)
	{
		chvRE->TipoAtuacao = "Neutro";
	}
	else
	{
		chvRE->TipoAtuacao = "Fase";
	}

	// Fase afetada
	bool alarmeFaseA = (alarmeReligadora->funcao50A || alarmeReligadora->funcao51A);
	bool alarmeFaseB = (alarmeReligadora->funcao50B || alarmeReligadora->funcao51B);
	bool alarmeFaseC = (alarmeReligadora->funcao50C || alarmeReligadora->funcao51C);

	if     (alarmeFaseA && !alarmeFaseB && !alarmeFaseC) chvRE->faseAfetada = "A";
	else if(!alarmeFaseA && alarmeFaseB && !alarmeFaseC) chvRE->faseAfetada = "B";
	else if(!alarmeFaseA && !alarmeFaseB && alarmeFaseC) chvRE->faseAfetada = "C";
	else if(alarmeFaseA && alarmeFaseB && !alarmeFaseC)  chvRE->faseAfetada = "AB";
	else if(alarmeFaseA && !alarmeFaseB && alarmeFaseC)  chvRE->faseAfetada = "CA";
	else if(!alarmeFaseA && alarmeFaseB && alarmeFaseC)  chvRE->faseAfetada = "BC";
	else if(alarmeFaseA && alarmeFaseB && alarmeFaseC)   chvRE->faseAfetada = "ABC";

	// Correntes
	Ia = alarmeReligadora->correnteFalta;
	Ib = Ic = 0.;
	thetaA = 0. * M_PI / 180.;
   thetaB = -120. * M_PI / 180.;
	thetaC = 120. * M_PI / 180.;
	vetorI[0] = std::complex<double>(Ia * cos(thetaA), Ia * sin(thetaA));
	vetorI[1] = std::complex<double>(Ib * cos(thetaB), Ib * sin(thetaB));
	vetorI[2] = std::complex<double>(Ic * cos(thetaC), Ic * sin(thetaC));

	// Tensões
	vetorV[0] = vetorV[1] = vetorV[2] = std::complex<double>(0., 0.);

	// Armazena tensões e correntes de medição
	for(int i=0; i<3; i++)
	{
		chvRE->medicaoVI.falta.V[i] = vetorV[i];
		chvRE->medicaoVI.falta.I[i] = vetorI[i];
	}
}
//---------------------------------------------------------------------------
/****
 * Atualiza as medições de religadora a partir de dados solicitados
 **/
void __fastcall TDados::AtualizaMedicoesReligadora(TChaveMonitorada* chvRE)
{
   double v_re, v_im, i_re, i_im;
   std::complex<double> I[3], V[3];
   std::complex<double> czero = std::complex<double>(0., 0.);
   for(int i=0; i<3; i++) {V[i] = czero; I[i] = czero;}
	int num_med_V = 0, num_med_I = 0;
   String ev_timestamp = "";
   TIniFile* file;
   String tipo_atuacao;

   // Proteção
   if(chvRE == NULL)
   {
      return;
   }

   // Arquivo com as medições da religadora
   file = new TIniFile(pathDados + "\\" + CodigoEvento + "\\" + chvRE->GetCodigo() + ".ini");

   ev_timestamp = file->ReadString("INFO", "horario", "");
   num_med_V = file->ReadInteger("INFO", "num_med_V", -1);
   num_med_I = file->ReadInteger("INFO", "num_med_I", -1);
   tipo_atuacao = file->ReadString("INFO", "tipo_atuacao", "");

   // Proteção
   if((num_med_V == -1) || (num_med_I == -1)) return;

   // Armazena timestamp do evento
   chvRE->medicaoVI.timestamp = ev_timestamp;
	SetPatamarHorario(ev_timestamp);

   // Armazena o tipo de atuação
   chvRE->TipoAtuacao = tipo_atuacao;

   // Medições de Tensão
   if(num_med_V > 0)
   {
      // Va
      v_re = file->ReadFloat("TENSAO", "var", 0.);
      v_im = file->ReadFloat("TENSAO", "vai", 0.);
      V[0] = std::complex<double>(v_re, v_im);
      // Vb
      v_re = file->ReadFloat("TENSAO", "vbr", 0.);
      v_im = file->ReadFloat("TENSAO", "vbi", 0.);
      V[1] = std::complex<double>(v_re, v_im);
      // Vc
      v_re = file->ReadFloat("TENSAO", "vcr", 0.);
      v_im = file->ReadFloat("TENSAO", "vci", 0.);
      V[2] = std::complex<double>(v_re, v_im);

      // Armazena tensões e correntes de medição
      for(int i=0; i<3; i++)
      {
         chvRE->medicaoVI.falta.V[i] = V[i];
      }
   }

   // Medições de Corrente
   if(num_med_I > 0)
   {
      // Ia
      i_re = file->ReadFloat("CORRENTE", "iar", 0.);
      i_im = file->ReadFloat("CORRENTE", "iai", 0.);
      I[0] = std::complex<double>(i_re, i_im);
      // Ib
      i_re = file->ReadFloat("CORRENTE", "ibr", 0.);
      i_im = file->ReadFloat("CORRENTE", "ibi", 0.);
      I[1] = std::complex<double>(i_re, i_im);
      // Ic
      i_re = file->ReadFloat("CORRENTE", "icr", 0.);
      i_im = file->ReadFloat("CORRENTE", "ici", 0.);
      I[2] = std::complex<double>(i_re, i_im);

      // Armazena tensões e correntes de medição
      for(int i=0; i<3; i++)
      {
         chvRE->medicaoVI.falta.I[i] = I[i];
      }
   }

   // Destroi obj de arquivo INI
//   file->Free();
   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TDados::TensoesNominais(String codTrafo, double &V_linha_MT_nom, double &V_linha_BT_nom)
{
   AnsiString   codRede1, codRede2;
   String       pathCadTrafosIntel;
   TStringList* linhasCadastro;

   // Obtém a tensão BT nominal de linha, em Volts
   pathCadTrafosIntel = path->DirDat() + "\\FaultLocation\\CadastroTrafosInteligentes.csv";
	linhasCadastro = new TStringList;
	linhasCadastro->LoadFromFile(pathCadTrafosIntel);

   V_linha_BT_nom = 220.;
	for(int i=0; i<linhasCadastro->Count; i++)
	{
		String linha = linhasCadastro->Strings[i];
		String codTrafoLinha = GetCampoCSV(linha, 2, ";");
		if(codTrafoLinha == codTrafo)
		{
         V_linha_BT_nom = GetCampoCSV(linha, 3, ";").ToDouble();
         break;
		}
	}

   // Obtém a tensão MT nominal de linha, em Volts
   TList* lisRedes = redes->LisRede();
   VTRede* rede = NULL;
   for(int i=0; i<lisRedes->Count; i++)
	{
      rede = (VTRede*) lisRedes->Items[i];

      codRede1 = AnsiString(CodigoAlimentador);
      codRede1 = AnsiReplaceStr(codRede1, "-", "");
      codRede1 = AnsiReplaceStr(codRede1, " ", "");

      codRede2 = AnsiString(rede->Codigo);
      codRede2 = AnsiReplaceStr(codRede2, "-", "");
      codRede2 = AnsiReplaceStr(codRede2, " ", "");

      if(codRede1 == codRede2) break;
      else rede = NULL;
   }
   if(!rede)
   {
      V_linha_MT_nom = 13800;
      delete linhasCadastro; linhasCadastro = NULL;
      return;
   }
   V_linha_MT_nom = rede->BarraInicial()->vnom;

	delete linhasCadastro; linhasCadastro = NULL;
}
////---------------------------------------------------------------------------
//void __fastcall TDados::TransfereMedicoesParaMT(TITrafo* eqptoTrafoInteligente)
//{
//	if(eqptoTrafoInteligente == NULL) return;
//
//	double V_linha_MT_nom, V_linha_BT_nom, a;
//	std::complex<double> Vf_bt, Vf_mt;
//	String codTrafoIntel, codTrafo;
//
//   // Pega o nome do trafo MT/BT a partir do código do eqptoTrafoInteligente
//   codTrafoIntel = eqptoTrafoInteligente->GetCodigo();
//   codTrafo = codTrafoIntel.SubString(7, codTrafoIntel.Length());
//
//   // Obtém as tensões nominais em Volts
//   TensoesNominais(codTrafo, V_linha_MT_nom, V_linha_BT_nom);
//
//   // Calcula a relação de tranformação
//   a = V_linha_MT_nom / V_linha_BT_nom;
//
//   // Aplica a relação de transformação para as 3 tensões de fase
//   for(int i=0; i<3; i++)
//   {
//      Vf_bt = eqptoTrafoInteligente->medicaoV.falta.V[i];
//      Vf_mt = a * Vf_bt;
//      eqptoTrafoInteligente->medicaoV.falta.V[i] = Vf_mt;
//   }
//
//
////   if(conexaoTrafo == "DY")
////   {
////      // Determina a relação de transformação para as tensões de fase
////      a = V_linha_MT_nom / V_linha_BT_nom;
////
////      for(int i=0; i<3; i++)
////      {
////         Vf_bt = eqptoTrafoInteligente->medicaoV.falta.V[i];
////         Vf_mt = a * Vf_bt;
////         eqptoTrafoInteligente->medicaoV.falta.V[i] = Vf_mt;
////      }
////
////      infoTrafos->AtualizaRelTransformacaoEfetiva(codTrafo, a);
////   }
////   else if(conexaoTrafo == "AN/AN")
////   {
////      // Determina a relação de transformação para as tensões de fase
////      a = V_linha_MT_nom / V_linha_BT_nom;
////
////      for(int i=0; i<3; i++)
////      {
////         Vf_bt = eqptoTrafoInteligente->medicaoV.falta.V[i];
////         Vf_mt = a * Vf_bt;
////         eqptoTrafoInteligente->medicaoV.falta.V[i] = Vf_mt;
////      }
////
////      infoTrafos->AtualizaRelTransformacaoEfetiva(codTrafo, a);
////   }
////   else if(conexaoTrafo == "AB/AN")
////   {
////      // Determina a relação de transformação para as tensões de fase
////      a = V_linha_MT_nom / V_linha_BT_nom;
////      a /= 1.7321;
////
////      for(int i=0; i<3; i++)
////      {
////         Vf_bt = eqptoTrafoInteligente->medicaoV.falta.V[i];
////         Vf_mt = a * Vf_bt;
////         eqptoTrafoInteligente->medicaoV.falta.V[i] = Vf_mt;
////      }
////
////      infoTrafos->AtualizaRelTransformacaoEfetiva(codTrafo, a);
////   }
////   else if(conexaoTrafo == "AB/ABN")
////   {
////      // Determina a relação de transformação para as tensões de fase
////      a = V_linha_MT_nom / V_linha_BT_nom;
////      a *= 2. / 1.7321;
////
////      for(int i=0; i<3; i++)
////      {
////         Vf_bt = eqptoTrafoInteligente->medicaoV.falta.V[i];
////         Vf_mt = a * Vf_bt;
////         eqptoTrafoInteligente->medicaoV.falta.V[i] = Vf_mt;
////      }
////
////      infoTrafos->AtualizaRelTransformacaoEfetiva(codTrafo, a);
////   }
//}
////---------------------------------------------------------------------------
///****
// * Atualiza as medições do trafo inteligente
// **/
//void __fastcall TDados::AtualizaMedicoesTrafoInteligente(TITrafo* eqptoTrafoInteligente)
//{
//   double v_re, v_im, i_re, i_im;
//   std::complex<double> V[3], I[3];
//	for(int i=0; i<3; i++){V[i] = std::complex<double>(0., 0.);I[i] = std::complex<double>(0., 0.);}
//	int num_med_V = 0, num_med_I = 0;
//	String ev_timestamp = "";
//   TIniFile* file;
//
//   // Proteção
//   if(eqptoTrafoInteligente == NULL)
//   {
//      return;
//   }
//
//   // Arquivo com as medições do trafo inteligente
//   file = new TIniFile(pathDados + "\\" + CodigoEvento + "\\" + eqptoTrafoInteligente->GetCodigo() + ".ini");
//
//   ev_timestamp = file->ReadString("INFO", "horario", "");
//   num_med_V = file->ReadInteger("INFO", "num_med_V", -1);
//
//   // Armazena timestamp do evento
//   eqptoTrafoInteligente->medicaoV.timestamp = ev_timestamp;
//   SetPatamarHorario(ev_timestamp);
//
//   // Medições de Tensão durante a falta
//   if(num_med_V > 0)
//   {
//      // Va
//      v_re = file->ReadFloat("TENSAO", "var", 0.);
//      v_im = file->ReadFloat("TENSAO", "vai", 0.);
//      V[0] = std::complex<double>(v_re, v_im);
//      // Vb
//      v_re = file->ReadFloat("TENSAO", "vbr", 0.);
//      v_im = file->ReadFloat("TENSAO", "vbi", 0.);
//      V[1] = std::complex<double>(v_re, v_im);
//      // Vc
//      v_re = file->ReadFloat("TENSAO", "vcr", 0.);
//      v_im = file->ReadFloat("TENSAO", "vci", 0.);
//      V[2] = std::complex<double>(v_re, v_im);
//
//      // Armazena tensões de medição
//      for(int i=0; i<3; i++)
//      {
//      	eqptoTrafoInteligente->medicaoV.falta.V[i] = V[i];
//      }
//   }
//
//   // Destroi obj de arquivo INI
////   file->Free();
//   if(file) {delete file; file = NULL;}
//}
////---------------------------------------------------------------------------
///****
// * Atualiza as medições dos qualímetros
// **/
//void __fastcall TDados::AtualizaMedicoesQualimetro(TQualimetro* eqptoQualimetro)
//{
//   double v_re, v_im, i_re, i_im;
//   std::complex<double> V[3], I[3];
//   for(int i=0; i<3; i++){V[i] = std::complex<double>(0., 0.);I[i] = std::complex<double>(0., 0.);}
//	int num_med_V = 0, num_med_I = 0;
//   String ev_timestamp = "";
//   TIniFile* file;
//
//   // Proteção
//   if(eqptoQualimetro == NULL)
//   {
//      return;
//   }
//
//   // Arquivo com as medições do qualímetro
//   file = new TIniFile(pathDados + "\\" + CodigoEvento + "\\" + eqptoQualimetro->GetCodigo() + ".ini");
//
//   ev_timestamp = file->ReadString("INFO", "horario", "");
//   num_med_V = file->ReadInteger("INFO", "num_med_V", -1);
//	num_med_I = file->ReadInteger("INFO", "num_med_I", -1);
//
//	// Se tem medições de tensão e corrente, o qualímetro é candidato a eqpto de
//	// referência (para cálculo de Ztotal_defeito)
//	if(num_med_V > 0 && num_med_I > 0)
//		eqptoQualimetro->candidatoEqptoRef = true;
//
//   // Armazena timestamp do evento
//   eqptoQualimetro->medicaoVI.timestamp = ev_timestamp;
//   SetPatamarHorario(ev_timestamp);
//
//   // Medições de Tensão durante a falta
//   if(num_med_V > 0)
//   {
//      // Va
//      v_re = file->ReadFloat("TENSAO", "var", 0.);
//      v_im = file->ReadFloat("TENSAO", "vai", 0.);
//      V[0] = std::complex<double>(v_re, v_im);
//      // Vb
//      v_re = file->ReadFloat("TENSAO", "vbr", 0.);
//      v_im = file->ReadFloat("TENSAO", "vbi", 0.);
//      V[1] = std::complex<double>(v_re, v_im);
//      // Vc
//      v_re = file->ReadFloat("TENSAO", "vcr", 0.);
//      v_im = file->ReadFloat("TENSAO", "vci", 0.);
//      V[2] = std::complex<double>(v_re, v_im);
//
//      // Armazena tensões de medição
//      for(int i=0; i<3; i++)
//      {
//      	eqptoQualimetro->medicaoVI.falta.V[i] = V[i];
//      }
//   }
//
//   // Medições de Corrente durante a falta
//   if(num_med_I > 0)
//   {
//      // Ia
//      i_re = file->ReadFloat("CORRENTE", "iar", 0.);
//      i_im = file->ReadFloat("CORRENTE", "iai", 0.);
//      I[0] = std::complex<double>(i_re, i_im);
//      // Ib
//      i_re = file->ReadFloat("CORRENTE", "ibr", 0.);
//      i_im = file->ReadFloat("CORRENTE", "ibi", 0.);
//      I[1] = std::complex<double>(i_re, i_im);
//      // Ic
//      i_re = file->ReadFloat("CORRENTE", "icr", 0.);
//      i_im = file->ReadFloat("CORRENTE", "ici", 0.);
//      I[2] = std::complex<double>(i_re, i_im);
//
//      // Armazena as correntes de medição
//      for(int i=0; i<3; i++)
//      {
//      	eqptoQualimetro->medicaoVI.falta.I[i] = I[i];
//      }
//   }
//
//	// Destroi obj de arquivo INI
////   file->Free();
//   if(file) {delete file; file = NULL;}
//}
//---------------------------------------------------------------------------
/****
 * Atualiza as medições dos medidores inteligentes
 **/
void __fastcall TDados::AtualizaMedicoesMedidorInteligente(TMedidorInteligente* medidorInteligente)
{
	double v_re, v_im;
	std::complex<double> V[3];
	for(int i=0; i<3; i++) V[i] = std::complex<double>(0., 0.);
	String ev_timestamp = "";
	TIniFile* file;

   // Proteção
	if(!medidorInteligente)
		return;

	// Arquivo com as medições do medidor inteligente
	file = new TIniFile(pathDados + "\\" + CodigoEvento + "\\" + medidorInteligente->Codigo + ".ini");

	ev_timestamp = file->ReadString("INFO", "horario", "");

	// Armazena timestamp do evento
	medidorInteligente->medicaoV.timestamp = ev_timestamp;
	SetPatamarHorario(ev_timestamp);

	// Insere as medições de tensão
	// Va
	v_re = file->ReadFloat("POSFALTA", "var", 0.);
	v_im = file->ReadFloat("POSFALTA", "vai", 0.);
	V[0] = std::complex<double>(v_re, v_im);
	// Vb
	v_re = file->ReadFloat("POSFALTA", "vbr", 0.);
	v_im = file->ReadFloat("POSFALTA", "vbi", 0.);
	V[1] = std::complex<double>(v_re, v_im);
	// Vc
	v_re = file->ReadFloat("POSFALTA", "vcr", 0.);
	v_im = file->ReadFloat("POSFALTA", "vci", 0.);
	V[2] = std::complex<double>(v_re, v_im);

	for(int i=0; i<3; i++)
	{
		medidorInteligente->medicaoV.pos.V[i] = V[i];
	}

	delete file;
}
////---------------------------------------------------------------------------
///****
// * Atualiza as medições dos sensores
// **/
//void __fastcall TDados::AtualizaMedicoesSensor(TSensor* eqptoSensor)
//{
//	bool faltaJusante;
//	double i_re, i_im;
//	std::complex<double> I[3];
//	for(int i=0; i<3; i++) I[i] = std::complex<double>(0., 0.);
//	int num_ev_falta = 0;
//   String ev_timestamp = "";
//   TIniFile* file;
//   String qualidade;
//
//   // Proteção
//   if(eqptoSensor == NULL)
//   {
//      return;
//   }
//
//   // Arquivo com as medições do sensor
//   file = new TIniFile(pathDados + "\\" + CodigoEvento + "\\" + eqptoSensor->GetCodigo() + ".ini");
//
//   ev_timestamp = file->ReadString("INFO", "horario", "");
//   num_ev_falta = file->ReadInteger("INFO", "num_ev_falta", -1);
//	faltaJusante = file->ReadBool("INFO", "falta_jusante", 0);
//	qualidade = file->ReadString("INFO", "qualidade", "GOOD");
//
//   // Proteção
//   if(num_ev_falta == -1) return;
//
//   // Armazena o dado de qualidade
//   if(qualidade == "GOOD")
//   	eqptoSensor->qualidadeOK = true;
//   else
//   	eqptoSensor->qualidadeOK = false;
//
//	// Dado que identifica se defeito foi à jusante (AIM) ou não (FPE)
//   eqptoSensor->faltaJusante = faltaJusante;
//
//   // Armazena timestamp do evento
//   eqptoSensor->medicaoI.timestamp = ev_timestamp;
//   SetPatamarHorario(ev_timestamp);
//
//   // Obtém as medições de corrente falta
//   if(num_ev_falta > 0)
//   {
//      // Ia
//      i_re = file->ReadFloat("FALTA", "iar", 0.);
//		i_im = file->ReadFloat("FALTA", "iai", 0.);
//		I[0] = std::complex<double>(i_re, i_im);
//		// Ib
//		i_re = file->ReadFloat("FALTA", "ibr", 0.);
//		i_im = file->ReadFloat("FALTA", "ibi", 0.);
//		I[1] = std::complex<double>(i_re, i_im);
//		// Ic
//      i_re = file->ReadFloat("FALTA", "icr", 0.);
//		i_im = file->ReadFloat("FALTA", "ici", 0.);
//      I[2] = std::complex<double>(i_re, i_im);
//
//      // Armazena correntes de medição
//		for(int i=0; i<3; i++)
//      {
//			eqptoSensor->medicaoI.falta.I[i] = I[i];
//      }
//	}
//
//	// Destroi obj de arquivo INI
////   file->Free();
//   if(file) {delete file; file = NULL;}
//}
//---------------------------------------------------------------------------
/***
 * Verifica se eqpto de monitoramento já foi criado
 */
bool __fastcall TDados::ExisteEqptoCampo(int Tipo, String Codigo)
{
   // Verifica se o eqpto já existe na lista de eqptos de campo
   for(int i=0; i<lisEqptosCampo->Count; i++)
   {
      TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
      if(eqptoCampo->GetTipo() != Tipo) continue;

      // Se o eqpto já existe, termina a procura
      if(eqptoCampo->GetCodigo() == Codigo)
      	return true;
   }

   return false;
}
////---------------------------------------------------------------------------
//// Retorna TRUE (faltam dados) apenas se todos os equipamentos não tiverem
//// registro de nenhuma corrente de defeito
//bool __fastcall TDados::FaltamDados()
//{
//	std::complex<double> Ifalta, Ia, Ib, Ic;
//   for(int i=0; i<lisEqptosCampo->Count; i++)
//	{
//		TEqptoCampo* eqpto = (TEqptoCampo*) lisEqptosCampo->Items[i];
//      if(eqpto->GetTipo() == chaveDJ || eqpto->GetTipo() == chaveRE)
//      {
//       	TChaveMonitorada* chv = (TChaveMonitorada*) eqpto;
//         Ifalta = chv->medicaoVI.falta.I[0];
//         if(std::abs(Ifalta) > 0.)
//         	return false;
//      }
//      else if(eqpto->GetTipo() == eqptoSENSOR)
//      {
//       	TSensor* sensor = (TSensor*) eqpto;
//         Ia = sensor->medicaoI.falta.I[0];
//         Ib = sensor->medicaoI.falta.I[1];
//         Ic = sensor->medicaoI.falta.I[2];
//         if(std::abs(Ia) > 0. || std::abs(Ib) > 0. || std::abs(Ic) > 0.)
//         	return false;
//      }
//   }
//
//   return true;
//}
//---------------------------------------------------------------------------
/***
 * Retorna eqpto de campo, se já tiver sido criado
 */
TEqptoCampo* __fastcall TDados::GetEqptoCampo(int Tipo, String Codigo)
{
   // Verifica se o eqpto já existe na lista de eqptos de campo
   for(int i=0; i<lisEqptosCampo->Count; i++)
   {
      TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
      if(eqptoCampo->GetTipo() != Tipo) continue;

      // Se o eqpto já existe, termina a procura
      if(eqptoCampo->GetCodigo() == Codigo)
			return eqptoCampo;
   }

   return NULL;
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodigoAlimentador()
{
	return CodigoAlimentador;
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodChaveDisjuntor(String CodDisjuntorSage)
{
	String pathCadDisjuntores = path->DirDat() + "\\FaultLocation\\CadastroDisjuntores.csv";
	TStringList* linhas = new TStringList();
	linhas->LoadFromFile(pathCadDisjuntores);

	for(int i=0; i<linhas->Count; i++)
	{
		String linha = linhas->Strings[i];
		String DJ = GetCampoCSV(linha, 1, ";");
		if(DJ == CodDisjuntorSage)
		{
         for(int i=linhas->Count-1; i>=0; i--) linhas->Delete(i);
         delete linhas; linhas = NULL;
			return GetCampoCSV(linha, 2, ";");
		}
	}

   // Destroi lista auxiliar
   for(int i=linhas->Count-1; i>=0; i--) linhas->Delete(i);
	delete linhas; linhas = NULL;
	return "";
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodChaveQualimetro(String CodQualimetro)
{
	String pathCadQualimetros = path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv";
	TStringList* linhas = new TStringList();
	linhas->LoadFromFile(pathCadQualimetros);

	for(int i=0; i<linhas->Count; i++)
	{
		String linha = linhas->Strings[i];
		String QUAL = GetCampoCSV(linha, 1, ";");
		if(QUAL == CodQualimetro)
		{
         for(int i=linhas->Count-1; i>=0; i--) linhas->Delete(i);
			delete linhas; linhas = NULL;
			return (GetCampoCSV(linha, 2, ";"));
		}
	}
   for(int i=linhas->Count-1; i>=0; i--) linhas->Delete(i);
	delete linhas; linhas = NULL;
	return "";
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodChaveReligadora(String CodReligadoraSage)
{
	String pathCadReligadoras = path->DirDat() + "\\FaultLocation\\CadastroReligadoras.csv";
   TStringList* linhas = new TStringList();
   linhas->LoadFromFile(pathCadReligadoras);

   for(int i=0; i<linhas->Count; i++)
   {
		String linha = linhas->Strings[i];
      String RE = GetCampoCSV(linha, 1, ";");
      if(RE == CodReligadoraSage)
      {
         for(int i=0; i<linhas->Count; i++) linhas->Delete(i);
         delete linhas;
      	return GetCampoCSV(linha, 2, ";");
      }
   }

   for(int i=0; i<linhas->Count; i++) linhas->Delete(i);
   delete linhas;
   return "";
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodCargaMIbalanco(String cod_medidor_inteligente_balanco)
{
	String pathCadMIbalanco = path->DirDat() + "\\FaultLocation\\CadastroMedidoresBalanco.csv";
	TStringList* linhas = new TStringList;
	linhas->LoadFromFile(pathCadMIbalanco);

	for(int i=0; i<linhas->Count; i++)
	{
		String linha = linhas->Strings[i];
		String cod_miBalanco = GetCampoCSV(linha, 1, ";");
		if(cod_miBalanco == cod_medidor_inteligente_balanco)
		{
         for(int i=0; i<linhas->Count; i++) linhas->Delete(i);
			delete linhas;
			return GetCampoCSV(linha, 2, ";");
		}
   }

	for(int i=0; i<linhas->Count; i++) linhas->Delete(i);
	delete linhas;
	return "";
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodChaveSensor(String CodSensor)
{
	String pathCadSensores = path->DirDat() + "\\FaultLocation\\CadastroSensores.csv";
	TStringList* linhas = new TStringList();
	linhas->LoadFromFile(pathCadSensores);

	for(int i=0; i<linhas->Count; i++)
	{
		String linha = linhas->Strings[i];
		String sensor = GetCampoCSV(linha, 1, ";");
		if(sensor == CodSensor)
		{
         for(int i=0; i<linhas->Count; i++) linhas->Delete(i);
			delete linhas;
			return GetCampoCSV(linha, 2, ";");
		}
   }

	for(int i=0; i<linhas->Count; i++) linhas->Delete(i);
	delete linhas;
	return "";
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodigoEvento()
{
	return CodigoEvento;
}
////---------------------------------------------------------------------------
//String __fastcall TDados::GetCodChaveDisjuntor(String cod_chave)
//{
//	int num_disjuntores;
//	String Codigo, CodChave = "";
//
//   // Arquivo de cadastro dos monitores do alimentador
//	TIniFile* file = new TIniFile(pathCadastro);
//
//	num_disjuntores = file->ReadInteger("GERAL", "num_disjuntores", -1);
//   if(num_disjuntores != -1)
//   {
//   	for(int i=0; i<num_disjuntores; i++)
//      {
//			Codigo = file->ReadString("DISJUNTOR" + String(i+1), "Codigo", "");
//         if(Codigo == cod_chave)
//         {
//            CodChave = file->ReadString("DISJUNTOR" + String(i+1), "Chave", "");
//            break;
//         }
//         else
//         {
//            CodChave = "";
//         }
//      }
//   }
//   // Destroi obj
//   if(file) {delete file; file = NULL;}
//
// 	return CodChave;
//}
////---------------------------------------------------------------------------
//String __fastcall TDados::GetCodChaveReligadora(String cod_chave)
//{
//	int num_religadoras;
//	String Codigo, CodChave = "";
//
//   // Arquivo de cadastro dos monitores do alimentador
//	TIniFile* file = new TIniFile(pathCadastro);
//
//	num_religadoras = file->ReadInteger("GERAL", "num_religadoras", -1);
//   if(num_religadoras != -1)
//   {
//   	for(int i=0; i<num_religadoras; i++)
//      {
//			Codigo = file->ReadString("RELIGADORA" + String(i+1), "Codigo", "");
//         if(Codigo == cod_chave)
//         {
//            CodChave = file->ReadString("RELIGADORA" + String(i+1), "Chave", "");
//            break;
//         }
//         else
//         {
//            CodChave = "";
//         }
//      }
//   }
//   // Destroi obj
//   if(file) {delete file; file = NULL;}
//
// 	return CodChave;
//}
//---------------------------------------------------------------------------
//String __fastcall TDados::GetCodLigacaoSensor(String cod_sensor)
//{
//	int num_sensores;
//	String Codigo, CodLigacao = "";
//
//   // Arquivo de cadastro dos monitores do alimentador
//	TIniFile* file = new TIniFile(pathCadastro);
//
//	num_sensores = file->ReadInteger("GERAL", "num_sensores", -1);
//   if(num_sensores != -1)
//   {
//   	for(int i=0; i<num_sensores; i++)
//      {
//			Codigo = file->ReadString("SENSOR" + String(i+1), "Codigo", "");
//         if(Codigo == cod_sensor)
//         {
//            CodLigacao = file->ReadString("SENSOR" + String(i+1), "Ligacao", "");
//            break;
//         }
//         else
//         {
//            CodLigacao = "";
//         }
//      }
//   }
//   // Destroi obj
//   if(file) {delete file; file = NULL;}
//
//   return CodLigacao;
//}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodTrechosMonitorados()
{
	String codTrechosMon;
	TEqptoCampo* eqptoCampo;
   VTChave* chaveAssociada;
   VTLigacao* ligacaoAssociada;

   codTrechosMon = "";
	for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      switch(eqptoCampo->GetTipo())
      {
      case chaveDJ:
      case chaveRE:
      case chaveSC:
			chaveAssociada = ((TChaveMonitorada*)eqptoCampo)->GetChaveAssociada();
			codTrechosMon += chaveAssociada->Codigo + ";";
      	break;

//		case eqptoSENSOR:
//		  ligacaoAssociada = ((TSensor*)eqptoCampo)->GetLigacaoAssociada();
//		  if(!ligacaoAssociada) break;
//        codTrechosMon += ligacaoAssociada->Codigo + ";";
//        break;

      default:
      	break;
      }
   }

   return codTrechosMon;
}
//---------------------------------------------------------------------------
TList* __fastcall TDados::GetEqptosCampo()
{
	return this->lisEqptosCampo;
}
////---------------------------------------------------------------------------
//TQualimetro* __fastcall TDados::GetFasoresVI_QualimetroEqptoRef()
//{
//	TChaveMonitorada* chaveMonit = NULL;
//	TEqptoCampo*      eqptoCampo;
//	TQualimetro*      qualEqptoRef = NULL;
//
//	// Seleciona os eqptos de campo candidatos (disjuntores ou religadoras)
//	// Determina o eqpto de campo (DJ ou RE) mais próximo do defeito
//	for(int i=0; i<lisEqptosCampo->Count; i++)
//	{
//		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
//		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;
//		if(!((TQualimetro*)eqptoCampo)->candidatoEqptoRef) continue;
//
//		if(qualEqptoRef == NULL || eqptoCampo->lisBlocosJusante->Count < qualEqptoRef->lisBlocosJusante->Count)
//			qualEqptoRef = (TQualimetro*)eqptoCampo;
//	}
//	return(qualEqptoRef);
//}
////---------------------------------------------------------------------------
//TQualimetro* __fastcall TDados::GetFasoresVI_QualimetroEqptoRef(StrFasor* fasorVref, StrFasor* fasorIref)
//{
//	TChaveMonitorada* chaveMonit = NULL;
//	TEqptoCampo*      eqptoCampo;
//	TQualimetro*      qualEqptoRef = NULL;
//
//	// Seleciona os eqptos de campo candidatos (disjuntores ou religadoras)
//	// Determina o eqpto de campo (DJ ou RE) mais próximo do defeito
//	for(int i=0; i<lisEqptosCampo->Count; i++)
//	{
//		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
//		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;
//		if(!((TQualimetro*)eqptoCampo)->candidatoEqptoRef) continue;
//
//		if(qualEqptoRef == NULL || eqptoCampo->lisBlocosJusante->Count < qualEqptoRef->lisBlocosJusante->Count)
//			qualEqptoRef = (TQualimetro*)eqptoCampo;
//	}
//
//	if(qualEqptoRef)
//	{
//		fasorVref->faseA = qualEqptoRef->medicaoVI.falta.V[0];
//		fasorVref->faseB = qualEqptoRef->medicaoVI.falta.V[1];
//		fasorVref->faseC = qualEqptoRef->medicaoVI.falta.V[2];
//		fasorIref->faseA = qualEqptoRef->medicaoVI.falta.I[0];
//		fasorIref->faseB = qualEqptoRef->medicaoVI.falta.I[1];
//		fasorIref->faseC = qualEqptoRef->medicaoVI.falta.I[2];
//	}
//	else
//	{
//		fasorVref->faseA = fasorVref->faseB = fasorVref->faseC = 0.;
//		fasorIref->faseA = fasorIref->faseB = fasorIref->faseC = 0.;
//		return(NULL);
//	}
//	return(qualEqptoRef);
//}
////---------------------------------------------------------------------------
//void __fastcall TDados::GetFasoresVI_SE(StrFasor* Vse, StrFasor* Ise)
//{
//	TEqptoCampo* eqptoCampo;
//   TQualimetro* qualimetro;
//
//
//   for(int i=0; i<lisEqptosCampo->Count; i++)
//   {
//		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
//
//      if(eqptoCampo->GetTipo() != eqptoQUALIMETRO)
//      	continue;
//
//      qualimetro = (TQualimetro*) eqptoCampo;
//
//      // Verifica se qualímetro está na subestação (início do alimentador)
//      if(!qualimetro->SE)
//      	continue;
//
//		Vse->faseA = qualimetro->medicaoVI.falta.V[0];
//		Vse->faseB = qualimetro->medicaoVI.falta.V[1];
//		Vse->faseC = qualimetro->medicaoVI.falta.V[2];
//
//		Ise->faseA = qualimetro->medicaoVI.falta.I[0];
//		Ise->faseB = qualimetro->medicaoVI.falta.I[1];
//		Ise->faseC = qualimetro->medicaoVI.falta.I[2];
//
//      break;
//   }
//}
//---------------------------------------------------------------------------
void __fastcall TDados::GetFasoresVI_SE_FLOffline(StrFasor* Vse, StrFasor* Ise)
{
	TEqptoCampo* eqptoCampo;
   TChaveMonitorada* disjuntor;

   for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      if(eqptoCampo->GetTipo() != chaveDJ)
      	continue;

      disjuntor = (TChaveMonitorada*) eqptoCampo;

		Vse->faseA = disjuntor->medicaoVI.falta.V[0];
		Vse->faseB = disjuntor->medicaoVI.falta.V[1];
		Vse->faseC = disjuntor->medicaoVI.falta.V[2];

		Ise->faseA = disjuntor->medicaoVI.falta.I[0];
		Ise->faseB = disjuntor->medicaoVI.falta.I[1];
		Ise->faseC = disjuntor->medicaoVI.falta.I[2];

		break;
   }
}
//---------------------------------------------------------------------------
TChaveMonitorada* __fastcall TDados::GetFasoresVI_Referencia_FLOffline(StrFasor* fasorVref, StrFasor* fasorIref)
{
	TChaveMonitorada* chaveMonit = NULL;
	TEqptoCampo*      eqptoCampo;

	// Seleciona os eqptos de campo candidatos (disjuntores ou religadoras)
	// Determina o eqpto de campo (DJ ou RE) mais próximo do defeito
	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != chaveDJ && eqptoCampo->GetTipo() != chaveRE) continue;

		if(chaveMonit == NULL || eqptoCampo->lisBlocosJusante->Count < chaveMonit->lisBlocosJusante->Count)
			chaveMonit = (TChaveMonitorada*)eqptoCampo;
	}

	if(chaveMonit)
	{
		fasorVref->faseA = chaveMonit->medicaoVI.falta.V[0];
		fasorVref->faseB = chaveMonit->medicaoVI.falta.V[1];
		fasorVref->faseC = chaveMonit->medicaoVI.falta.V[2];
		fasorIref->faseA = chaveMonit->medicaoVI.falta.I[0];
		fasorIref->faseB = chaveMonit->medicaoVI.falta.I[1];
		fasorIref->faseC = chaveMonit->medicaoVI.falta.I[2];
	}
	else
	{
		fasorVref->faseA = fasorVref->faseB = fasorVref->faseC = 0.;
		fasorIref->faseA = fasorIref->faseB = fasorIref->faseC = 0.;
		return(NULL);
	}
	return(chaveMonit);
}
//---------------------------------------------------------------------------
void __fastcall TDados::GetMedicoesBarras_AlgoFasorial(TStringList* lisEXT)
{
   std::complex<double> V;
   String               CSVmed;
	TChaveMonitorada*    chaveMonitorada;
	TEqptoCampo*         eqptoCampo;
//	TQualimetro*         qualimetro;
//	TITrafo*             trafointeligente;

	for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      CSVmed = "";

      switch(eqptoCampo->GetTipo())
      {
      case chaveDJ:
      case chaveRE:
      case chaveSC:
      	break;

//      case eqptoQUALIMETRO:
//			qualimetro = (TQualimetro*) eqptoCampo;
//
//			if(qualimetro->GetLigacaoAssociada())
//				CSVmed = String(qualimetro->GetLigacaoAssociada()->Barra(1)->Id) + ";";
//			else
//				CSVmed = String(qualimetro->cargaAssociada->pbarra->Id) + ";";
//
//			for(int nf=0; nf<3; nf++)
//			{
//            V = qualimetro->medicaoVI.falta.V[nf];
//            CSVmed += String(std::abs(V)) + ";";
//         }
//         lisEXT->Add(CSVmed);
//			break;
//
//      case eqptoITRAFO:
//			trafointeligente = (TITrafo*) eqptoCampo;
//
//         CSVmed = String(trafointeligente->cargaAssociada->pbarra->Id) + ";";
//			for(int nf=0; nf<3; nf++)
//			{
//            V = trafointeligente->medicaoV.falta.V[nf];
//            CSVmed += String(std::abs(V)) + ";";
//         }
//         lisEXT->Add(CSVmed);
//			break;
//
//      default:
//      	break;
      }
	}
}
//---------------------------------------------------------------------------
TStringList* __fastcall TDados::GetMedicoesBarras()
{
   std::complex<double> V;
   String CSVmed;
	TChaveMonitorada* chaveMonitorada;
	TEqptoCampo* eqptoCampo;
//	TQualimetro* qualimetro;
	TStringList* lisMedBarras = new TStringList();

	for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      CSVmed = "";

      switch(eqptoCampo->GetTipo())
      {
      case chaveDJ:
      case chaveRE:
      case chaveSC:
//         chaveMonitorada = (TChaveMonitorada*) eqptoCampo;
//         CSVmed = String(chaveMonitorada->GetChaveAssociada()->Barra(1)->Id) + ";";
//         for(int nf=0; nf<3; nf++)
//         {
//            V = chaveMonitorada->medicaoVI.falta.V[nf];
//            CSVmed += String(std::abs(V)) + ";";
//         }
//         lisMedBarras->Add(CSVmed);
      	break;

//		case eqptoQUALIMETRO:
//			qualimetro = (TQualimetro*) eqptoCampo;
//			if(qualimetro->GetLigacaoAssociada())
//				CSVmed = String(qualimetro->GetLigacaoAssociada()->Barra(1)->Id) + ";";
//			else
//				CSVmed = String(qualimetro->cargaAssociada->pbarra->Id) + ";";
//
//			for(int nf=0; nf<3; nf++)
//         {
//            V = qualimetro->medicaoVI.falta.V[nf];
//            CSVmed += String(std::abs(V)) + ";";
//			}
//			lisMedBarras->Add(CSVmed);
//			break;

      default:
      	break;
      }
   }

   return lisMedBarras;
}
//---------------------------------------------------------------------------
TStringList* __fastcall TDados::GetMedicoesTrechos()
{
   std::complex<double> I;
   String CSVmed;
	TChaveMonitorada* chaveMonitorada;
	TEqptoCampo* eqptoCampo;
//   TSensor* sensor;
   TStringList* lisMedTrechos = new TStringList();

	for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      CSVmed = "";

      switch(eqptoCampo->GetTipo())
      {
      case chaveDJ:
      case chaveRE:
      case chaveSC:
			chaveMonitorada = (TChaveMonitorada*) eqptoCampo;
         CSVmed = chaveMonitorada->GetChaveAssociada()->Codigo + ";";
      	for(int nf=0; nf<3; nf++)
         {
            I = chaveMonitorada->medicaoVI.falta.I[nf];
            CSVmed += String(std::abs(I)) + ";";
         }
         lisMedTrechos->Add(CSVmed);
      	break;

//      case eqptoSENSOR:
//		  sensor = (TSensor*) eqptoCampo;
//		  CSVmed = sensor->GetLigacaoAssociada()->Codigo + ";";
//		  for(int nf=0; nf<3; nf++)
//		  {
//			  I = sensor->medicaoI.falta.I[nf];
//			  CSVmed += String(std::abs(I)) + ";";
//        }
//		  lisMedTrechos->Add(CSVmed);
//		  break;

      default:
      	break;
      }
   }

   return lisMedTrechos;
}
//---------------------------------------------------------------------------
void __fastcall TDados::GetMedicoesTrechos_AlgoFasorial(TStringList* lisEXT)
{
   std::complex<double> I;
   String CSVmed;
	TChaveMonitorada* chaveMonitorada;
	TEqptoCampo* eqptoCampo;
//   TSensor* sensor;

	for(int i=0; i<lisEqptosCampo->Count; i++)
   {
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      CSVmed = "";

      switch(eqptoCampo->GetTipo())
      {
      case chaveDJ:
      case chaveRE:
      case chaveSC:
      	break;

//		case eqptoSENSOR:
//        sensor = (TSensor*) eqptoCampo;
//		  CSVmed = sensor->GetLigacaoAssociada()->Codigo + ";";
//		  for(int nf=0; nf<3; nf++)
//		  {
//			  I = sensor->medicaoI.falta.I[nf];
//			  CSVmed += String(std::abs(I)) + ";";
//		  }
//		  lisEXT->Add(CSVmed);
//        break;

      default:
      	break;
      }
   }
}
//---------------------------------------------------------------------------
TStringList* __fastcall TDados::GetMedicoesTrechos_EE()
{
   std::complex<double> I;
   String CSVmed;
	TChaveMonitorada* chaveMonitorada;
	TEqptoCampo* eqptoCampo;
	TStringList* lisMedTrechos = new TStringList();

	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];

      CSVmed = "";

      switch(eqptoCampo->GetTipo())
      {
		case chaveDJ:
      case chaveRE:
      case chaveSC:
			break;

//		case eqptoQUALIMETRO:
//			qualimetro = (TQualimetro*) eqptoCampo;
//			if(!qualimetro->GetLigacaoAssociada()) break;
//
//			CSVmed = qualimetro->GetLigacaoAssociada()->Codigo + ";";
//			for(int nf=0; nf<3; nf++)
//			{
//				I = qualimetro->medicaoVI.falta.I[nf];
//				CSVmed += String(std::abs(I)) + ";";
//			}
//			lisMedTrechos->Add(CSVmed);
//			break;

//		case eqptoSENSOR:
//		  sensor = (TSensor*) eqptoCampo;
//		  CSVmed = sensor->GetLigacaoAssociada()->Codigo + ";";
//		  for(int nf=0; nf<3; nf++)
//		  {
//			  I = sensor->medicaoI.falta.I[nf];
//			  CSVmed += String(std::abs(I)) + ";";
//		  }
//		  lisMedTrechos->Add(CSVmed);
//		  break;

      default:
      	break;
      }
   }

   return lisMedTrechos;
}
//---------------------------------------------------------------------------
int __fastcall TDados::GetPatamarHorario()
{
	return patamar;
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetPathEvento()
{
	return this->pathEvento;
}
////---------------------------------------------------------------------------
//void __fastcall TDados::InsereDadosQualimetro(String CodQualimetro, String linhaDados)
//{
//	double mod, ang;
//	std::complex<double> fasor;
//	TChaveMonitorada* chvDJ;
//	TQualimetro* eqptoQualimetro;
//
//	// Pega o obj do qualímetro
//	eqptoQualimetro = (TQualimetro*) GetEqptoCampo(eqptoQUALIMETRO, CodQualimetro);
//
//	// Insere os fasores de tensão a partir da linha de dados
//	for(int ifase=0; ifase<3; ifase++)
//	{
//		mod = 1000. * GetCampoCSV(linhaDados, 2*ifase, ";").ToDouble();
//		ang = GetCampoCSV(linhaDados, 2*ifase + 1, ";").ToDouble();
//		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
//		eqptoQualimetro->medicaoVI.falta.V[ifase] = fasor;
//	}
//}
////---------------------------------------------------------------------------
//void __fastcall TDados::InsereDadosQualimetro(String CodQualimetro, String linhaV, String linhaI)
//{
//	double mod, ang;
//	int indice;
//	std::complex<double> fasor;
//	TChaveMonitorada* chvDJ;
//	TQualimetro* eqptoQualimetro;
//
//	// Pega o obj do qualímetro
//	eqptoQualimetro = (TQualimetro*) GetEqptoCampo(eqptoQUALIMETRO, CodQualimetro);
//
//	// Insere os fasores de tensão a partir da linha de dados
//	indice = 0;
//	for(int ifase=0; ifase<3; ifase++)
//	{
//		mod = 1000. * GetCampoCSV(linhaV, indice++, ";").ToDouble();
//		ang = GetCampoCSV(linhaV, indice++, ";").ToDouble();
//		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
//		eqptoQualimetro->medicaoVI.falta.V[ifase] = fasor;
//	}
//}
////---------------------------------------------------------------------------
//void __fastcall TDados::InsereDadosSensor(String CodSensor, String linhaI)
//{
//	double mod, ang;
//	int indice;
//	std::complex<double> fasor;
//	TSensor* sensor;
//
//	// Pega o obj do qualímetro
//	sensor = (TSensor*) GetEqptoCampo(eqptoSENSOR, CodSensor);
//
//	// Insere os fasores de tensão a partir da linha de dados
//	indice = 0;
//	for(int ifase=0; ifase<3; ifase++)
//	{
//		mod = GetCampoCSV(linhaI, indice++, ";").ToDouble();
//		ang = GetCampoCSV(linhaI, indice++, ";").ToDouble();
//		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
//		sensor->medicaoI.falta.I[ifase] = fasor;
//	}
//}
//---------------------------------------------------------------------------
void __fastcall TDados::InsereDadosDisjuntor(String CodDisjuntor, String linhaV, String linhaI)
{
	double mod, ang;
	int indice;
	std::complex<double> fasor;
	TChaveMonitorada* chvDJ;

	// Pega o obj do disjuntor
	chvDJ = (TChaveMonitorada*) GetEqptoCampo(chaveDJ, CodDisjuntor);

	// Insere os fasores de tensão a partir da linha de dados
	indice = 0;
	for(int ifase=0; ifase<3; ifase++)
	{
		mod = 1000. * GetCampoCSV(linhaV, indice++, ";").ToDouble();
		ang = GetCampoCSV(linhaV, indice++, ";").ToDouble();
		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
		chvDJ->medicaoVI.falta.V[ifase] = fasor;
	}
	// Insere os fasores de corrente a partir da linha de dados
	indice = 0;
	for(int ifase=0; ifase<3; ifase++)
	{
		mod = GetCampoCSV(linhaI, indice++, ";").ToDouble();
		ang = GetCampoCSV(linhaI, indice++, ";").ToDouble();
		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
		chvDJ->medicaoVI.falta.I[ifase] = fasor;
	}
}
//---------------------------------------------------------------------------
void __fastcall TDados::InsereDadosReligadora(String CodReligadora, String linhaV, String linhaI)
{
	double mod, ang;
	int indice;
	std::complex<double> fasor;
	TChaveMonitorada* chvRE;

	// Pega o obj da religadora
	chvRE = (TChaveMonitorada*) GetEqptoCampo(chaveRE, CodReligadora);

	// Insere os fasores de tensão a partir da linha de dados
	indice = 0;
	for(int ifase=0; ifase<3; ifase++)
	{
		mod = 1000. * GetCampoCSV(linhaV, indice++, ";").ToDouble();
		ang = GetCampoCSV(linhaV, indice++, ";").ToDouble();
		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
		chvRE->medicaoVI.falta.V[ifase] = fasor;
	}
	// Insere os fasores de corrente a partir da linha de dados
	indice = 0;
	for(int ifase=0; ifase<3; ifase++)
	{
		mod = GetCampoCSV(linhaI, indice++, ";").ToDouble();
		ang = GetCampoCSV(linhaI, indice++, ";").ToDouble();
		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
		chvRE->medicaoVI.falta.I[ifase] = fasor;
	}
}
//---------------------------------------------------------------------------
void __fastcall TDados::AcrescentaDadosMedicoes(TList* listaAlarmes)
{
	for(int i=0; i<listaAlarmes->Count; i++)
	{
		TAlarme* alarme = (TAlarme*) listaAlarmes->Items[i];

		if(alarme->TipoEqpto == 10) // Disjuntor
		{
			String codigoDisjuntor = alarme->CodEqpto;
			TChaveMonitorada* chvDJ = (TChaveMonitorada*) GetEqptoCampo(chaveDJ, codigoDisjuntor);
			if(!chvDJ)
			{
				// Gera obj de disjuntor
				chvDJ = new TChaveMonitorada(codigoDisjuntor);
				// Seta as referências dos eqptos associados
				String cod_chave_disjuntor = GetCodChaveDisjuntor(codigoDisjuntor);
				VTChave* chave = (VTChave*) redes->ExisteEqpto(eqptoCHAVE, cod_chave_disjuntor);
				if(chave == NULL) continue;
				// Seta o tipo de chave monitorada
				chvDJ->SetTipo(chaveDJ);
				// Seta o bloco associado à chave
				chvDJ->blocoChave = funcoesRede->GetBlocoAssociadoChave(chave);
				// Associa o alarme ao objeto
				chvDJ->lisAlarmes->Add(alarme);
				// Salva referências
				chvDJ->SetChaveAssociada(chave);
				// Guarda obj de DJ em lista
				lisEqptosCampo->Add(chvDJ);
				// Insere as medições do relé do disjuntor
				AtualizaMedicoesDisjuntor(chvDJ, alarme);
			}
			else
			{
				// Associa o alarme ao objeto de disjuntor
				chvDJ->lisAlarmes->Add(alarme);
				// Insere as medições do relé do disjuntor
				AtualizaMedicoesDisjuntor(chvDJ, alarme);
         }
		}
		else if(alarme->TipoEqpto == 11) // Religadora
		{
			String codigoReligadora = alarme->CodEqpto;
			TChaveMonitorada* chvRE = (TChaveMonitorada*) GetEqptoCampo(chaveRE, codigoReligadora);
			if(!chvRE)
			{
				// Gera obj de religadora
				chvRE = new TChaveMonitorada(codigoReligadora);
				// Seta as referências dos eqptos associados
				String cod_chave_religadora = GetCodChaveReligadora(codigoReligadora);
				VTChave* chave = (VTChave*) redes->ExisteEqpto(eqptoCHAVE, cod_chave_religadora);
				if(chave == NULL) continue;
				// Seta o tipo de chave monitorada
				chvRE->SetTipo(chaveRE);
				// Seta o bloco associado à chave
				chvRE->blocoChave = funcoesRede->GetBlocoAssociadoChave(chave);
				// Associa o alarme ao objeto
				chvRE->lisAlarmes->Add(alarme);
				// Salva referências
				chvRE->SetChaveAssociada(chave);
				// Guarda obj de RE em lista
				lisEqptosCampo->Add(chvRE);
				// Insere as medições do relé do disjuntor
				AtualizaMedicoesReligadora(chvRE, alarme);
			}
			else
			{
				// Associa o alarme ao objeto de religadora
				chvRE->lisAlarmes->Add(alarme);
				// Insere as medições do relé do disjuntor
				AtualizaMedicoesReligadora(chvRE, alarme);
         }
		}
		else if(alarme->TipoEqpto == 12) // Medidor Ingeligente de Balanço
		{
			String codigoMedidor = alarme->CodEqpto;
			TMedidorInteligenteBalanco* eqptoMIbalanco = (TMedidorInteligenteBalanco*) GetEqptoCampo(eqptoMI, codigoMedidor);
			if(!eqptoMIbalanco)
			{
				eqptoMIbalanco = new TMedidorInteligenteBalanco(codigoMedidor);
				String cod_carga_medidor_inteligente_balanco = GetCodCargaMIbalanco(codigoMedidor);
            // Seta as referências dos eqptos associados
				VTCarga* carga = (VTCarga*) redes->ExisteEqpto(eqptoCARGA, cod_carga_medidor_inteligente_balanco);
				if(carga == NULL) continue;
				// Salva referências
				eqptoMIbalanco->cargaAssociada = carga;
				eqptoMIbalanco->blocoCarga = funcoesRede->GetBlocoAssociadoCarga(carga);
				// Associa o alarme ao objeto
				eqptoMIbalanco->lisAlarmes->Add(alarme);
				// Guarda em lista o obj de medidor inteligente de balanço
				lisEqptosCampo->Add(eqptoMIbalanco);
			}
			else
			{
				// Associa o alarme ao objeto de medidor inteligente de balanço
				eqptoMIbalanco->lisAlarmes->Add(alarme);
         }
		}
		else if(alarme->TipoEqpto == 13) // Medidor Ingeligente de Consumidor MT
		{

		}
		else if(alarme->TipoEqpto == 14) // Medidor Ingeligente de Consumidor BT
		{

		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TDados::InsereDadosDisjuntor(String CodDisjuntor, String linhaDados)
{
	double mod, ang;
	std::complex<double> fasor;
   TChaveMonitorada* chvDJ;

	// Pega o obj do disjuntor
	chvDJ = (TChaveMonitorada*) GetEqptoCampo(chaveDJ, CodDisjuntor);

   // Insere os fasores de tensão a partir da linha de dados
	for(int ifase=0; ifase<3; ifase++)
   {
		mod = 1000. * GetCampoCSV(linhaDados, 33 + 2*ifase, ";").ToDouble();
		ang = GetCampoCSV(linhaDados, 33 + 2*ifase + 1, ";").ToDouble();
      fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
      chvDJ->medicaoVI.falta.V[ifase] = fasor;
   }
	// Insere os fasores de corrente a partir da linha de dados
	for(int ifase=0; ifase<3; ifase++)
	{
		mod = GetCampoCSV(linhaDados, 23 + 2*ifase, ";").ToDouble();
		ang = GetCampoCSV(linhaDados, 23 + 2*ifase + 1, ";").ToDouble();
		fasor = std::complex<double>(mod * cos(ang * M_PI/180.), mod * sin(ang * M_PI/180.));
		chvDJ->medicaoVI.falta.I[ifase] = fasor;
	}
}
//---------------------------------------------------------------------------
VTTrecho* __fastcall TDados::DeterminaTrechoJusante(VTLigacao* ligacaoRef)
{
   VTOrdena* ordena = (VTOrdena*) apl->GetObject(__classid(VTOrdena));

   if(ligacaoRef->Tipo() == eqptoTRECHO)
      return((VTTrecho*)ligacaoRef);
   else if(ligacaoRef->Tipo() == eqptoCHAVE)
   {
      if(ordena == NULL)
      {
         ordena = DLL_NewObjOrdena(apl);
         ordena->Executa(redes);
      }
      VTRede* rede = ligacaoRef->rede;
      TList* lisLigacao = rede->LisLigacao();

      // Encontra a ligação à jusante que seja do tipo Trecho
      VTLigacao* ligaJus = ligacaoRef;
      while(ligaJus->Tipo() != eqptoTRECHO)
      {
         for(int i=0; i<lisLigacao->Count; i++)
         {
            VTLigacao* liga = (VTLigacao*) lisLigacao->Items[i];
            if(liga->ligaPai == ligaJus)
            {
               ligaJus = liga;
               break;
            }
         }
      }
      return((VTTrecho*)ligaJus);
   }
}
//---------------------------------------------------------------------------
/***
 * Método para leitura das informações de entrada
 * (arquivos INI do diretório Dat/FaultLocation/Dados/<código evento>)
 */
void __fastcall TDados::LeDadosFormatados(void)
{
	int num_sensores, num_disjuntores, num_religadoras;
	int num_medidores_inteligentes_balanco;
	String cod_disjuntor,         cod_chave_disjuntor;
	String cod_religadora,        cod_chave_religadora;
	String cod_medidor_inteligente_balanco, cod_carga_medidor_inteligente_balanco;
	String pathArqGeral;
	TChaveMonitorada *chvDJ, *chvRE;
	TMedidorInteligenteBalanco* eqptoMIbalanco;
	TIniFile*    file;

   // Caminho do arquivo INI "Geral.ini"
   pathArqGeral = pathDados + "\\" + CodigoEvento + "\\Geral.ini";

   // Abre arquivo Geral.ini do evento
	file = new TIniFile(pathArqGeral);
	num_medidores_inteligentes_balanco = file->ReadInteger("INFO", "num_medidores_inteligentes_balanco", -1);
	num_disjuntores = file->ReadInteger("INFO", "num_disjuntores", -1);
	num_religadoras = file->ReadInteger("INFO", "num_religadoras", -1);

	for(int i=0; i<num_medidores_inteligentes_balanco; i++)
	{
		cod_medidor_inteligente_balanco = file->ReadString("MEDIDORESINTELIGENTESBALANCO", "cod_medidor_inteligente_balanco" + String(i+1), "");
		cod_carga_medidor_inteligente_balanco = GetCodCargaMIbalanco(cod_medidor_inteligente_balanco);
		eqptoMIbalanco = (TMedidorInteligenteBalanco*) GetEqptoCampo(eqptoMI, cod_medidor_inteligente_balanco);

		if(eqptoMIbalanco == NULL)
		{
			// Gera obj de medidor inteligente de balanço
			eqptoMIbalanco = new TMedidorInteligenteBalanco(cod_medidor_inteligente_balanco);
			// Seta as referências dos eqptos associados
			VTCarga* carga = (VTCarga*) redes->ExisteEqpto(eqptoCARGA, cod_carga_medidor_inteligente_balanco);
			if(carga == NULL) continue;
			// Salva referências
			eqptoMIbalanco->cargaAssociada = carga;
			eqptoMIbalanco->blocoCarga = funcoesRede->GetBlocoAssociadoCarga(carga);
			// Guarda em lista o obj de medidor inteligente de balanço
			lisEqptosCampo->Add(eqptoMIbalanco);
		}

		AtualizaMedicoesMedidorInteligente(eqptoMIbalanco);
	}

//	for(int i=0; i<num_sensores; i++)
//	{
//		cod_sensor = file->ReadString("SENSORES", "cod_sensor" + String(i+1), "");
//		cod_ligacao_sensor = GetCodChaveSensor(cod_sensor);
//		eqptoSensor = (TSensor*) GetEqptoCampo(eqptoSENSOR, cod_sensor);
//
//		if(eqptoSensor == NULL)
//		{
//			// Gera obj de sensor
//			eqptoSensor = new TSensor(cod_sensor);
//			// Seta as referências dos eqptos associados
//			VTLigacao* ligacao = (VTLigacao*) redes->ExisteEqpto(eqptoLIGACAO, cod_ligacao_sensor);
//			if(ligacao == NULL) continue;
//			// Salva referências
//			eqptoSensor->SetLigacaoAssociada(ligacao);
//			// Guarda obj de sensor em lista
//			lisEqptosCampo->Add(eqptoSensor);
//		}
//
//		AtualizaMedicoesSensor(eqptoSensor);
//	}

//	for(int i=0; i<num_qualimetros; i++)
//	{
//		cod_qualimetro = file->ReadString("QUALIMETROS", "cod_qualimetro" + String(i+1), "");
//		cod_eqpto_qualimetro = GetCodigoLigacaoAssociadaQualimetro(cod_qualimetro);
//		eqptoQualimetro = (TQualimetro*) GetEqptoCampo(eqptoQUALIMETRO, cod_qualimetro);
//
//		if(eqptoQualimetro == NULL)
//		{
//			eqptoQualimetro = new TQualimetro(cod_qualimetro);
//
//			// Seta as referências dos eqptos associados
//			VTLigacao* ligacao = (VTLigacao*) redes->ExisteEqpto(eqptoLIGACAO, cod_eqpto_qualimetro);
//			VTCarga* carga = (VTCarga*) redes->ExisteEqpto(eqptoCARGA, cod_eqpto_qualimetro);
//			if(!ligacao && !carga)
//         {
//            delete eqptoQualimetro; eqptoQualimetro = NULL;
//            continue;
//         }
//
//         if(ligacao && !carga)
//         {
//            eqptoQualimetro->SetLigacaoAssociada(ligacao);
//            eqptoQualimetro->cargaAssociada = NULL;
//            VTTrecho* trechoJusante = DeterminaTrechoJusante(ligacao);
//            eqptoQualimetro->SetTrechoJusante(trechoJusante);
//         }
//         else if(!ligacao && carga)
//         {
//            eqptoQualimetro->SetLigacaoAssociada(NULL);
//            eqptoQualimetro->cargaAssociada = carga;
//            eqptoQualimetro->SetTrechoJusante(NULL);
//         }
//         else
//         {
//            delete eqptoQualimetro; eqptoQualimetro = NULL;
//            continue;
//         }
//
//			lisEqptosCampo->Add(eqptoQualimetro);
//		}
//
//		AtualizaMedicoesQualimetro(eqptoQualimetro);
//	}

//	for(int i=0; i<num_trafos_inteligentes; i++)
//	{
//		cod_trafo_inteligente = file->ReadString("TRAFOSINTELIGENTES", "cod_trafo_inteligente" + String(i+1), "");
//		cod_eqpto_trafo_inteligente = ReplaceStr(cod_trafo_inteligente, "TRAFO_", "");
//		eqptoTrafoInteligente = (TITrafo*) GetEqptoCampo(eqptoITRAFO, cod_trafo_inteligente);
//
//		if(eqptoTrafoInteligente == NULL)
//		{
//			eqptoTrafoInteligente = new TITrafo(cod_trafo_inteligente);
//
//			// Seta as referências dos eqptos associados
//			VTCarga* carga = (VTCarga*) redes->ExisteEqpto(eqptoCARGA, cod_eqpto_trafo_inteligente);
//			if(!carga) continue;
//
//			eqptoTrafoInteligente->cargaAssociada = carga;
//			lisEqptosCampo->Add(eqptoTrafoInteligente);
//		}
//
//		AtualizaMedicoesTrafoInteligente(eqptoTrafoInteligente);
//
//		if(MedicoesTrafoIntelBT())
//			TransfereMedicoesParaMT(eqptoTrafoInteligente);
//	}

   for(int i=0; i<num_disjuntores; i++)
	{
		cod_disjuntor = file->ReadString("DISJUNTORES", "cod_disjuntor" + String(i+1), "");
		cod_chave_disjuntor = GetCodChaveDisjuntor(cod_disjuntor);
      chvDJ = (TChaveMonitorada*) GetEqptoCampo(chaveDJ, cod_disjuntor);

      if(chvDJ == NULL)
      {
			// Gera obj de disjuntor
			chvDJ = new TChaveMonitorada(cod_disjuntor);
			// Seta as referências dos eqptos associados
			VTChave* chave = (VTChave*) redes->ExisteEqpto(eqptoCHAVE, cod_chave_disjuntor);
			if(chave == NULL) continue;
			// Seta o tipo de chave monitorada
			chvDJ->SetTipo(chaveDJ);
			// Seta o bloco associado à chave
			chvDJ->blocoChave = funcoesRede->GetBlocoAssociadoChave(chave);
			// Salva referências
			chvDJ->SetChaveAssociada(chave);
			// Guarda obj de DJ em lista
			lisEqptosCampo->Add(chvDJ);
      }

      AtualizaMedicoesDisjuntor(chvDJ);
   }

   for(int i=0; i<num_religadoras; i++)
	{
   	cod_religadora = file->ReadString("RELIGADORAS", "cod_religadora" + String(i+1), "");
      cod_chave_religadora = GetCodChaveReligadora(cod_religadora);
      chvRE = (TChaveMonitorada*) GetEqptoCampo(chaveRE, cod_religadora);

      if(chvRE == NULL)
      {
	      // Gera obj de religadora
         chvRE = new TChaveMonitorada(cod_religadora);
         // Seta as referências dos eqptos associados
         VTChave* chave = (VTChave*) redes->ExisteEqpto(eqptoCHAVE, cod_chave_religadora);
         if(chave == NULL) continue;
         // Seta o tipo de chave monitorada
			chvRE->SetTipo(chaveRE);
			// Seta o bloco associado à chave
			chvRE->blocoChave = funcoesRede->GetBlocoAssociadoChave(chave);
         // Salva referências
         chvRE->SetChaveAssociada(chave);
         // Guarda obj de DJ em lista
         lisEqptosCampo->Add(chvRE);
      }

      AtualizaMedicoesReligadora(chvRE);
	}
	delete file;
}
//---------------------------------------------------------------------------
String __fastcall TDados::GetCodigoLigacaoAssociadaQualimetro(String cod_qualimetro)
{
	String pathArqCadastro = path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv";
	TStringList* lisArqCadQual = new TStringList;
	lisArqCadQual->LoadFromFile(pathArqCadastro);

	for(int i=0; i<lisArqCadQual->Count; i++)
	{
		String linha = lisArqCadQual->Strings[i];
		if(GetCampoCSV(linha, 1, ";") == cod_qualimetro)
		{
			delete lisArqCadQual;
			return(GetCampoCSV(linha, 2, ";"));
		}
	}
	delete lisArqCadQual;
	return("");
}
//---------------------------------------------------------------------------
bool __fastcall TDados::MedicoesTrafoIntelBT()
{
   TIniFile* file;
   String pathConfig = path->DirDat() + "\\FaultLocation\\ConfigGerais.ini";
   String ladoMedicoes;

   file = new TIniFile(pathConfig);
   ladoMedicoes = file->ReadString("ALGO_FASORIAL", "LadoMedicoesTrafoIntel", "BT");
   delete file; file = NULL;

   return(ladoMedicoes == "BT");
}
////---------------------------------------------------------------------------
//void __fastcall TDados::GeraEqptoQualimetro()
//{
//	int num_qualimetros;
//	String cod_qualimetro, cod_eqpto_qualimetro;
//	String pathArqGeral;
//	TQualimetro *eqptoQualimetro;
//	TIniFile* file;
//
//	// Caminho do arquivo INI "Geral.ini"
//	pathArqGeral = pathDados + "\\" + CodigoEvento + "\\Geral.ini";
//
//	// Abre arquivo Geral.ini do evento
//	file = new TIniFile(pathArqGeral);
//	num_qualimetros = file->ReadInteger("INFO", "num_qualimetros", -1);
//
//	for(int i=0; i<num_qualimetros; i++)
//	{
//		cod_qualimetro = file->ReadString("QUALIMETROS", "cod_qualimetro" + String(i+1), "");
//		cod_eqpto_qualimetro = ReplaceStr(cod_qualimetro, "QUALIMETRO_", "");
//		eqptoQualimetro = (TQualimetro*) GetEqptoCampo(eqptoQUALIMETRO, cod_qualimetro);
//
//		if(eqptoQualimetro == NULL)
//      {
//			// Gera obj de qualímetro
//			eqptoQualimetro = new TQualimetro(cod_qualimetro);
//			// Seta as referências dos eqptos associados
//			VTLigacao* ligacao = (VTLigacao*) redes->ExisteEqpto(eqptoLIGACAO, cod_eqpto_qualimetro);
//			VTCarga* carga = (VTCarga*) redes->ExisteEqpto(eqptoCARGA, cod_eqpto_qualimetro);
//			if(ligacao == NULL) continue;
//
//			eqptoQualimetro->SetLigacaoAssociada(ligacao);
//			eqptoQualimetro->cargaAssociada = carga;
////			VerificaQualimetroSE(eqptoQualimetro);
//
//			lisEqptosCampo->Add(eqptoQualimetro);
//      }
//	}
//
//   // Destroi arquivo
//   delete file; file = NULL;
//}
//---------------------------------------------------------------------------
void __fastcall TDados::GeraEqptoDisjuntor()
{
	int num_disjuntores;
	String cod_disjuntor, cod_chave_disjuntor;
	String pathArqGeral;
	TChaveMonitorada *chvDJ;
	TIniFile* file;

	// Caminho do arquivo INI "Geral.ini"
	pathArqGeral = pathDados + "\\" + CodigoEvento + "\\Geral.ini";

   // Abre arquivo Geral.ini do evento
   file = new TIniFile(pathArqGeral);
	num_disjuntores = file->ReadInteger("INFO", "num_disjuntores", -1);

   for(int i=0; i<num_disjuntores; i++)
	{
   	cod_disjuntor = file->ReadString("DISJUNTORES", "cod_disjuntor" + String(i+1), "");
      cod_chave_disjuntor = GetCodChaveDisjuntor(cod_disjuntor);
      chvDJ = (TChaveMonitorada*) GetEqptoCampo(chaveDJ, cod_disjuntor);

      if(chvDJ == NULL)
      {
	      // Gera obj de disjuntor
         chvDJ = new TChaveMonitorada(cod_disjuntor);
         // Seta as referências dos eqptos associados
         VTChave* chave = (VTChave*) redes->ExisteEqpto(eqptoCHAVE, cod_chave_disjuntor);
			if(chave == NULL) continue;
			// Seta o tipo de chave monitorada
			chvDJ->SetTipo(chaveDJ);
			// Salva referências
			chvDJ->SetChaveAssociada(chave);
			// Guarda obj de DJ em lista
			lisEqptosCampo->Add(chvDJ);
		}
	}

   // Destroi arquivo
   delete file; file = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDados::GeraEqptoReligadora()
{
	int num_religadoras;
	String cod_religadora, cod_chave_religadora;
	String pathArqGeral;
	TChaveMonitorada *chvRE;
	TIniFile* file;

	// Caminho do arquivo INI "Geral.ini"
	pathArqGeral = pathDados + "\\" + CodigoEvento + "\\Geral.ini";

	// Abre arquivo Geral.ini do evento
	file = new TIniFile(pathArqGeral);
	num_religadoras = file->ReadInteger("INFO", "num_religadoras", -1);

	for(int i=0; i<num_religadoras; i++)
	{
		cod_religadora = file->ReadString("RELIGADORAS", "cod_religadora" + String(i+1), "");
		cod_chave_religadora = GetCodChaveReligadora(cod_religadora);
		chvRE = (TChaveMonitorada*) GetEqptoCampo(chaveRE, cod_religadora);

		if(chvRE == NULL)
      {
			// Gera obj de religadora
			chvRE = new TChaveMonitorada(cod_religadora);
			// Seta as referências dos eqptos associados
			VTChave* chave = (VTChave*) redes->ExisteEqpto(eqptoCHAVE, cod_chave_religadora);
			if(chave == NULL) continue;
			// Seta o tipo de chave monitorada
			chvRE->SetTipo(chaveRE);
			// Salva referências
			chvRE->SetChaveAssociada(chave);
			// Guarda obj de RE em lista
			lisEqptosCampo->Add(chvRE);
		}
	}

   // Destroi arquivo
   delete file; file = NULL;
}
//---------------------------------------------------------------------------
//void __fastcall TDados::GeraEqptoSensor()
//{
//	int num_sensores;
//	String cod_sensor, cod_ligacao_sensor;
//	String pathArqGeral;
//	TSensor* eqptoSensor;
//	TIniFile* file;
//
//	// Caminho do arquivo INI "Geral.ini"
//	pathArqGeral = pathDados + "\\" + CodigoEvento + "\\Geral.ini";
//
//	// Abre arquivo Geral.ini do evento
//	file = new TIniFile(pathArqGeral);
//	num_sensores = file->ReadInteger("INFO", "num_sensores", -1);
//
//	for(int i=0; i<num_sensores; i++)
//	{
//		cod_sensor = file->ReadString("SENSORES", "cod_sensor" + String(i+1), "");
//		cod_ligacao_sensor = GetCodChaveSensor(cod_sensor);
//		eqptoSensor = (TSensor*) GetEqptoCampo(eqptoSENSOR, cod_sensor);
//
//		if(eqptoSensor == NULL)
//      {
//			// Gera obj de sensor
//			eqptoSensor = new TSensor(cod_sensor);
//			// Seta as referências dos eqptos associados
//			VTLigacao* ligacao = (VTLigacao*) redes->ExisteEqpto(eqptoLIGACAO, cod_ligacao_sensor);
//			if(ligacao == NULL) continue;
//			// Salva referências
//			eqptoSensor->SetLigacaoAssociada(ligacao);
//			// Guarda obj de sensor em lista
//			lisEqptosCampo->Add(eqptoSensor);
//		}
//	}
//
//   // Destroi arquivo
//   delete file; file = NULL;
//}
//---------------------------------------------------------------------------
void __fastcall TDados::SetPatamarHorario(String ev_timestamp)
{
	int posIni, cont;
	String substr;

   // Pega o horário do timestamp
   posIni = 9;
   cont = 2;
   substr = ev_timestamp.SubString(posIni, cont);

   if(substr != "")
   {
      // Salva o patamar horário
      patamar = substr.ToInt();
   }
}
////---------------------------------------------------------------------------
//// Método para verificar se qualímetro está instalado na SE (início do alimentador)
//void __fastcall TDados::VerificaQualimetroSE(TQualimetro* qualimetro)
//{
//	VTLigacao* ligacaoAssociada;
//	VTRede* rede;
//
//	if(!qualimetro) return;
//
//	ligacaoAssociada = qualimetro->GetLigacaoAssociada();
//	if(!ligacaoAssociada) return;
//
//   rede = NULL;
//   for(int i=0; i<redes->LisRede()->Count; i++)
//   {
//      rede = (VTRede*) redes->LisRede()->Items[i];
//      if(!rede->Carregada) continue;
//
//      if(rede->ExisteLigacao(ligacaoAssociada))
//      	break;
//      else
//      	rede = NULL;
//   }
//	if(!rede) return;
//
//	if(rede->BarraInicial() == ligacaoAssociada->Barra(0) ||
//		rede->BarraInicial() == ligacaoAssociada->Barra(1))
//	{
//		qualimetro->SE = true;
//	}
//}
//---------------------------------------------------------------------------
//eof
