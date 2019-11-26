//---------------------------------------------------------------------------
#pragma hdrstop
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TFuncoesDeRede.h"
#include "..\Auxiliares\TDados.h"
#include "..\Equipamentos\TChaveMonitorada.h"
#include "..\Equipamentos\TQualimetro.h"
#include "TConversorSinapDSS.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <complex>
#include <math.h>
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Radial.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Arranjo\VTArranjo.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Constante\Fases.h>
#include <PlataformaSinap\Fontes\Curvas\VTCurva.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Radial\VTPrimario.h>
#include <PlataformaSinap\Fontes\Radial\VTRadial.h>
#include <PlataformaSinap\Fontes\Rede\Estrutura.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTEqbar.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTSuprimento.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
#include <System.StrUtils.hpp>
//---------------------------------------------------------------------------
__fastcall TConversorSinapDSS::TConversorSinapDSS(VTApl* apl)
{
	// Salva ponteiros
	this->apl = apl;
	path  = (VTPath*)  apl->GetObject(__classid(VTPath));
	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	dados = NULL;
	qualimetroEqptoRef      = NULL;
   chaveMonitoradaEqptoRef = NULL;
	lisLigacoesConsideradas = NULL;
	lisBarrasConsideradas   = NULL;
	funcoesRede = new TFuncoesDeRede(apl);

	// Objeto radial para ordenação das ligações da rede MT
	radial = (VTRadial*) apl->GetObject(__classid(VTRadial));
	if(radial == NULL)
	{
		radial = DLL_NewObjRadial(apl);
	}
	radial->Inicia(redes);

	// Cria listas
	lisArranjos        = new TList();
	lisTrafosSEsFilhas = new TList();
	lisRedesFilhas     = new TList();
	posicaoSuprimento  = "AT";
}
//---------------------------------------------------------------------------
__fastcall TConversorSinapDSS::~TConversorSinapDSS(void)
{
	// Destroi objetos
	if(lisArranjos)
   {
      delete lisArranjos; lisArranjos = NULL;
   }
	if(lisTrafosSEsFilhas)
   {
      delete lisTrafosSEsFilhas; lisTrafosSEsFilhas = NULL;
   }
	if(lisLigacoesConsideradas)
   {
      delete lisLigacoesConsideradas; lisLigacoesConsideradas = NULL;
   }
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::AjustaArranjos()
{
	TStringList* lisAux = new TStringList(); // lista para os IDs dos arranjos

	for(int i=lisArranjos->Count-1; i>=0; i--)
	{
    	VTArranjo* arranjo = (VTArranjo*) lisArranjos->Items[i];

		// Se o arranjo já existe, remove da lista de arranjos
		if(lisAux->IndexOf(String(arranjo->Id)) < 0)
      {
      	lisAux->Add(String(arranjo->Id));
      }
      else
      {
         lisArranjos->Delete(i);
      }
	}

   for(int i=lisAux->Count-1; i>=0; i--) lisAux->Delete(i);
	delete lisAux;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::EquivalenteTheveninMT(VTRede* redeSE, VTRede* redeMT, StrEquivalenteThevenin* strEqThv)
{
	if(!redeSE || !redeMT || !strEqThv) return;

	String codigoSE, linha, linhaIdentificacao;
	double nivelTensaoMT;
	TStringList* lisLinhasArquivoPotCC = new TStringList();
	TStringList* lisLinhasAuxiliar = new TStringList();

	// Pesquisa o equivalente no arquivo de potências de curto
	codigoSE = redeSE->Codigo;
	nivelTensaoMT = Round(redeMT->BarraInicial()->vnom,1);
	String pathArqPotCC = path->DirDat() + "\\FaultLocation\\PotenciasCurtoCircuito.csv";

   //debug
   int numeroAjustes = 0;

	try
	{
		lisLinhasArquivoPotCC->LoadFromFile(pathArqPotCC);

      // Ajusta as linhas do arquivo
      int indiceUltimoVazio = -1;
		for(int i=lisLinhasArquivoPotCC->Count-1; i>=0; i--)
		{
         linha = lisLinhasArquivoPotCC->Strings[i];
         if(linha == "")
         {
            if(indiceUltimoVazio == -1)
               indiceUltimoVazio = i;
            else if(i == indiceUltimoVazio-1)
            {
               indiceUltimoVazio = i;
               lisLinhasArquivoPotCC->Delete(indiceUltimoVazio);
               numeroAjustes += 1;
            }
         }
         else
         {
            indiceUltimoVazio = -1;
         }
      }

		for(int i=0; i<lisLinhasArquivoPotCC->Count; i++)
		{
			linha = lisLinhasArquivoPotCC->Strings[i];

			if(linha == "")
			{
				if(AnsiContainsStr(AnsiString(linhaIdentificacao), AnsiString(codigoSE)))
					break;
				else
					lisLinhasAuxiliar->Clear();
			}
			else
				lisLinhasAuxiliar->Add(linha);
		}
	}
	catch(Exception &e)
	{
		Aviso("PotenciascurtoCircuito.csv não foi encontrado. Valores padrão serão utilizados");
		delete lisLinhasArquivoPotCC; lisLinhasArquivoPotCC = NULL;
		delete lisLinhasAuxiliar; lisLinhasAuxiliar = NULL;
		return;
	}

	if(lisLinhasAuxiliar->Count <= 3)
	{
		Aviso("PotenciascurtoCircuito.csv apresenta padrão incompatível. Valores padrão serão utilizados");
		delete lisLinhasArquivoPotCC; lisLinhasArquivoPotCC = NULL;
		delete lisLinhasAuxiliar; lisLinhasAuxiliar = NULL;
		return;
	}

	// Apaga as 3 primeiras linhas (cabeçalhos)
	for(int i=0; i<3; i++) lisLinhasAuxiliar->Delete(0);

	// Os valores de impedância estão em pu da base vnom, 100MVA
	for(int i=0; i<lisLinhasAuxiliar->Count; i++)
	{
		String linha = lisLinhasAuxiliar->Strings[i];

		double nivelTensaoMT_aux = GetCampo(linha, 1, ";").ToDouble();
		if(nivelTensaoMT_aux == nivelTensaoMT)
		{
			strEqThv->r1 = GetCampo(linha, 2, ";").ToDouble();
			strEqThv->x1 = GetCampo(linha, 3, ";").ToDouble();
			strEqThv->r0 = GetCampo(linha, 6, ";").ToDouble();
			strEqThv->x0 = GetCampo(linha, 7, ";").ToDouble();
			break;
		}
	}

	// Destroi listas
   for(int i=lisLinhasArquivoPotCC->Count-1; i>=0; i--) lisLinhasArquivoPotCC->Delete(i);
	delete lisLinhasArquivoPotCC; lisLinhasArquivoPotCC = NULL;
   for(int i=lisLinhasAuxiliar->Count-1; i>=0; i--) lisLinhasAuxiliar->Delete(i);
	delete lisLinhasAuxiliar; lisLinhasAuxiliar = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::DeterminaAreaJusanteEqptoRef(VTLigacao* ligacaoReferencia)
{
	if(!ligacaoReferencia) return;

	// Pega os blocos á jusante do eqpto de referência
	TList* lisBlocosJusanteEqptoRef = new TList();
	funcoesRede->GetBlocosJusanteLigacao(ligacaoReferencia, lisBlocosJusanteEqptoRef);


	TList* lisChavesRedeMT = new TList();
	redeMT->LisChave(lisChavesRedeMT, chvFECHADA);

	lisLigacoesConsideradas = new TList();
	lisBarrasConsideradas   = new TList();
	for(int i=0; i<lisBlocosJusanteEqptoRef->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosJusanteEqptoRef->Items[i];
		if(!bloco) continue;

		// Pega todas as ligações à jusante do eqpto de referência
		if(!bloco->LisLigacao()) continue;
		TList* lisLigacoesBloco = bloco->LisLigacao();
		for(int j=0; j<lisLigacoesBloco->Count; j++)
		{
			VTLigacao* ligacao = (VTLigacao*) lisLigacoesBloco->Items[j];
			lisLigacoesConsideradas->Add(ligacao);
		}

		// Pega todas as barras à jusante do eqpto de referência
		if(!bloco->LisBarra()) continue;
		TList* lisBarrasBloco = bloco->LisBarra();
		for(int j=0; j<lisBarrasBloco->Count; j++)
		{
			VTBarra* barra = (VTBarra*) lisBarrasBloco->Items[j];
			lisBarrasConsideradas->Add(barra);
		}
	}

	// Pega todas as chaves à jusante do eqpto de referência
	for(int i=0; i<lisChavesRedeMT->Count; i++)
	{
		VTChave* chave = (VTChave*) lisChavesRedeMT->Items[i];
		VTBarra* barra0 = chave->Barra(0);
		VTBarra* barra1 = chave->Barra(1);
		if(lisBarrasConsideradas->IndexOf(barra0) >= 0 || lisBarrasConsideradas->IndexOf(barra1) >= 0)
			lisLigacoesConsideradas->Add(chave);
	}

	delete lisBlocosJusanteEqptoRef;
	delete lisChavesRedeMT;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::EquivalenteTheveninEqptoRef_FLOffline(double baseKV, StrEquivalenteThevenin* strEqThv)
{
	if(!strEqThv) return;

	StrFasor* fasorVref = new StrFasor();
	StrFasor* fasorIref = new StrFasor();

   chaveMonitoradaEqptoRef = dados->GetFasoresVI_Referencia_FLOffline(fasorVref, fasorIref);
   DeterminaAreaJusanteEqptoRef(chaveMonitoradaEqptoRef->GetChaveAssociada());

	strEqThv->r1 = 0.0001;
	strEqThv->x1 = 0.0001;
	strEqThv->r0 = 0.0001;
	strEqThv->x0 = 0.0001;

	strEqThv->Va_pu = abs(fasorVref->faseA) / (1000. * baseKV / sqrt(3.));
	strEqThv->Vb_pu = abs(fasorVref->faseB) / (1000. * baseKV / sqrt(3.));
	strEqThv->Vc_pu = abs(fasorVref->faseC) / (1000. * baseKV / sqrt(3.));

	strEqThv->pVa = std::arg(fasorVref->faseA) * 180./M_PI;
	strEqThv->pVb = std::arg(fasorVref->faseB) * 180./M_PI;
	strEqThv->pVc = std::arg(fasorVref->faseC) * 180./M_PI;

   strEqThv->barra = chaveMonitoradaEqptoRef->GetChaveAssociada()->Barra(1);

   delete fasorVref; delete fasorIref;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::EquivalenteTheveninEqptoRef(double baseKV, StrEquivalenteThevenin* strEqThv)
{
//	if(!strEqThv) return;
//
//	StrFasor* fasorVref = new StrFasor();
//	StrFasor* fasorIref = new StrFasor();
//
//   qualimetroEqptoRef = dados->GetFasoresVI_QualimetroEqptoRef(fasorVref, fasorIref);
//   DeterminaAreaJusanteEqptoRef(qualimetroEqptoRef->ligacaoAssociada);
//
//	strEqThv->r1 = 0.0001;
//	strEqThv->x1 = 0.0001;
//	strEqThv->r0 = 0.0001;
//	strEqThv->x0 = 0.0001;
//
//	strEqThv->Va_pu = abs(fasorVref->faseA) / (1000. * baseKV / sqrt(3.));
//	strEqThv->Vb_pu = abs(fasorVref->faseB) / (1000. * baseKV / sqrt(3.));
//	strEqThv->Vc_pu = abs(fasorVref->faseC) / (1000. * baseKV / sqrt(3.));
//
//	strEqThv->pVa = std::arg(fasorVref->faseA) * 180./M_PI;
//	strEqThv->pVb = std::arg(fasorVref->faseB) * 180./M_PI;
//	strEqThv->pVc = std::arg(fasorVref->faseC) * 180./M_PI;
//
//   if(qualimetroEqptoRef->ligacaoAssociada)
//      strEqThv->barra = qualimetroEqptoRef->ligacaoAssociada->Barra(1);
//   else if(qualimetroEqptoRef->cargaAssociada)
//      strEqThv->barra = qualimetroEqptoRef->cargaAssociada->pbarra;
//
//   delete fasorVref; delete fasorIref;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::Executa(bool locRompCabo)
{
	ExportaMaster();

	ExportaTrafos();

	ExportaLigacoes();

   ExportaArranjos(locRompCabo);

   ExportaCargas();
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::Executa_FLOffline(bool locRompCabo)
{
	ExportaMaster_FLOffline();

	ExportaTrafos();

	ExportaLigacoes();

   ExportaArranjos(locRompCabo);

   ExportaCargas();
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Linecodes.dss
 */
void __fastcall TConversorSinapDSS::ExportaArranjos(bool InsereArranjosAuxiliares)
{
	int numFases;
	strIMP z0, z1;
   String linhaArranjo;
	TStringList* linhasDSS = new TStringList();

	AjustaArranjos(); // remove repetidos
	for(int i=0; i<lisArranjos->Count; i++)
	{
    	VTArranjo* arranjo = (VTArranjo*) lisArranjos->Items[i];

      numFases = GetNumFases(arranjo->Fases); // número de fases do arranjo
      z0 = arranjo->z0; // ohms/km
      z1 = arranjo->z1; // ohms/km

		linhaArranjo = "New Linecode." + String(arranjo->Id) + " nphases=" + String(numFases) + " basefreq=60 units=km";
      linhaArranjo += " r0=" + DoubleToString(z0.r, 4) + " x0=" + DoubleToString(z0.x, 4);
		linhaArranjo += " r1=" + DoubleToString(z1.r, 4) + " x1=" + DoubleToString(z1.x, 4);

		// Gera linha do linecodes.dss
      linhasDSS->Add(linhaArranjo);
	}

	// Exporta arranjos auxiliares, para teste de rompimento de cabo
	if(InsereArranjosAuxiliares)
	{
		linhaArranjo = "New Linecode.Aux1F nphases=1 basefreq=60 units=km";
		linhaArranjo += " r0=0.0010 x0=0.0050";
		linhaArranjo += " r1=0.0010 x1=0.0050";
		linhasDSS->Add(linhaArranjo);

		linhaArranjo = "New Linecode.Aux2F nphases=2 basefreq=60 units=km";
		linhaArranjo += " r0=0.0010 x0=0.0050";
		linhaArranjo += " r1=0.0010 x1=0.0050";
		linhasDSS->Add(linhaArranjo);
   }


   // Exporta arquivo linecodes.dss
   linhasDSS->SaveToFile(pathSaida + "\\linecodes.dss");

   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
   delete linhasDSS;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::FiltraCargasAreaConsiderada(TList* lisCargasTodas)
{
	if(!lisCargasTodas) return;

	VTBarra* barra;
	VTCarga* carga;

	// Remove as cargas que não estiverem na área considerada (à jusante do eqpto de referência)
	for(int i=lisCargasTodas->Count-1; i>=0; i--)
	{
		carga = (VTCarga*) lisCargasTodas->Items[i];
		barra = carga->pbarra;

		if(lisBarrasConsideradas->IndexOf(barra) < 0)
			lisCargasTodas->Remove(carga);
	}
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Loads.dss
 */
void __fastcall TConversorSinapDSS::ExportaCargas()
{
	int idBarra, enumFases, numFases;
   double vnom;
   double valor[nvCURVA_CAR] = {0., 0.};
   double integralP_dia = 0., integralQ_dia = 0.;
   double Pmed, Qmed;
   double valoresP[24], valoresQ[24];
   for(int i=0; i<24; i++) {valoresP[i] = 0.; valoresQ[i] = 0.;}
	String codCarga, linhaDSS = "", linha1, linha2, indiceFases, connCarga;
   strHM hm;
	TList* lisCargas;
   TStringList* linhasDSS = new TStringList();
   TStringList* lisLinhasLoadshapes = new TStringList();
   VTBarra* barra;
   VTCarga* carga;
   VTEqbar* eqbar;
   VTTrecho* trechoPai;

	// Verificação
	if(redeMT == NULL) return;

   // Obtém a lista de cargas da rede MT
	lisCargas = new TList();
	redeMT->LisEqpto(lisCargas, eqptoCARGA);

	// Obtém também as cargas das redes filhas
	TList* lisCargas_aux = new TList();
	for(int i=0; i<lisRedesFilhas->Count; i++)
	{
		VTRede* redeFilha = (VTRede*) lisRedesFilhas->Items[i];

		lisCargas_aux->Clear();
		redeFilha->LisEqpto(lisCargas_aux, eqptoCARGA);
		for(int j=0; j<lisCargas_aux->Count; j++)
		{
			VTCarga* carga = (VTCarga*) lisCargas_aux->Items[j];
			if(lisCargas->IndexOf(carga) < 0)
				lisCargas->Add(carga);
      }
	}
	delete lisCargas_aux;

	if(posicaoSuprimento == "EqptoRef")
	{
		FiltraCargasAreaConsiderada(lisCargas);
	}

	//debug
	int cont = 0;

   // Para cada carga, gera linha correspondente
	for(int ic=0; ic<lisCargas->Count; ic++)
	{
		carga = (VTCarga*) lisCargas->Items[ic];
      eqbar = (VTEqbar*) carga;

		// Pega parâmetros da carga
      codCarga = eqbar->Codigo;
      barra = eqbar->pbarra;
      idBarra = barra->Id;
      vnom = barra->vnom;
      enumFases = carga->Fases;


		indiceFases = GetIndiceFases(enumFases);
      numFases = GetNumPhasesCarga(enumFases);
      connCarga = GetConnCarga(enumFases);
      if(enumFases == faseA || enumFases == faseB || enumFases == faseC ||
      	enumFases == faseAN || enumFases == faseBN || enumFases == faseCN ||
         enumFases == faseAT || enumFases == faseBT || enumFases == faseCT ||
         enumFases == faseANT || enumFases == faseBNT || enumFases == faseCNT)
      {
         vnom /= 1.7321;
      }

      // Analisa curva de carga
		VTCurva* curvaCarga = carga->curva;
		int idTipoCurva = curvaCarga->Tipo;
		String strTipoCurva = curvaCarga->TipoAsStr;
		int idUnidade = curvaCarga->Unidade;

		if(curvaCarga->Tipica)
		{
			Pmed = carga->Energia_kwhmes / 720.;
			Qmed = Pmed * std::tan(acos(0.92));

//			double integralP_kw_tipico = 0.;
			for(int i=0; i<24; i++)
			{
				hm.hora = i;
				hm.minuto = i;
				curvaCarga->GetValor(hm, valor, sizeof(valor)/sizeof(double));

				// Verifica o tipo de curva
				if(idTipoCurva == curvaPQ_MDP)
				{
					if(idUnidade == unidPU_MED)
					{
						valoresP[i] = valor[0];
						valoresQ[i] = valor[1];
					}
					else if(idUnidade == unidKW)
					{
						valoresP[i] = valor[0] / Pmed;
						valoresQ[i] = valor[1] / Pmed;
					}
				}
				else if(idTipoCurva == curvaPFP)
				{
					if(idUnidade == unidKW)
					{
						valoresP[i] = valor[0];
						valoresQ[i] = valor[0] * std::tan(acos(valor[1]));
					}
				}
				else
				{
					cont++;
            }

//            integralP_kw_tipico += valor[0];
         }

//       	// curva em pu da média
//			for(int i=0; i<24; i++)
//			{
//				valoresP[i] /= (integralP_kw_tipico/24.);
//				valoresQ[i] /= (integralP_kw_tipico/24.);
//			}
      }
      // Os valores da curva medida são: P(MW);Q(MVAR)
      else
      {
         integralP_dia = 0.;
         integralQ_dia = 0.;
         for(int i=0; i<24; i++)
         {
            hm.hora = i;
            hm.minuto = i;
            curvaCarga->GetValor(hm, valor, sizeof(valor)/sizeof(double));

            integralP_dia += valor[0] * 1000.;
            integralQ_dia += valor[1] * 1000.;
            valoresP[i] = valor[0] * 1000.;
            valoresQ[i] = valor[1] * 1000.;

            // Valores médios
            Pmed = integralP_dia / 24.;
            Qmed = integralQ_dia / 24.;

         }

         if((Pmed == 0. && Qmed == 0.)) continue;

         // Valores em pu (da média)
         for(int i=0; i<24; i++)
         {
            if(Pmed > 0. && Qmed > 0.)
            {
               valoresP[i] /= Pmed;
               valoresQ[i] /= Qmed;
            }
         }
      }

		// Formata as linhas
		lisLinhasLoadshapes->Add("New Loadshape." + String(curvaCarga->Id) + " npts=24 interval=1 !código carga: " + carga->Codigo);
      linha1 = "~ pmult=(";
      linha2 = "~ qmult=(";
      for(int i=0; i<24; i++)
		{
      	if(valoresP[i] == 0.)
         	linha1 += "0.00000000";
         else
	         linha1 += DoubleToString(valoresP[i], 8);
         if(valoresQ[i] == 0.)
         	linha2 += "0.00000000";
         else
	         linha2 += DoubleToString(valoresQ[i], 8);
         if(i < 23)
         {
            linha1 += ", ";
            linha2 += ", ";
         }
      }
      linha1 += ")";
      linha2 += ")";

      // Insere as linhas na string list de linhas de curvas de carga
      lisLinhasLoadshapes->Add(linha1);
      lisLinhasLoadshapes->Add(linha2);

      // :::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Linhas relativas à carga (loads.dss)
      // :::::::::::::::::::::::::::::::::::::::::::::::::::::::

      linhaDSS = "New Load." + codCarga;
      linhaDSS += " Bus1=" + String(idBarra) + indiceFases;
      linhaDSS += " Conn=" + connCarga;
      linhaDSS += " Model=5";
      linhaDSS += " Phases=" + String(numFases);
      linhaDSS += " KV=" + DoubleToString(vnom, 3);
      linhaDSS += " KW=" + DoubleToString(Pmed, 3);
      linhaDSS += " KVAR=" + DoubleToString(Qmed, 3);
      linhaDSS += " Daily=" + String(curvaCarga->Id);

      // Adiciona linha à lista de linhas
      linhasDSS->Add(linhaDSS);
   }

   // Exporta arquivo loads.dss
   linhasDSS->SaveToFile(pathSaida + "\\loads.dss");
	// Exporta arquivo loadshapes.dss
   lisLinhasLoadshapes->SaveToFile(pathSaida + "\\loadshapes.dss");


   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
   delete linhasDSS;
   for(int i=lisLinhasLoadshapes->Count-1; i>=0; i--) lisLinhasLoadshapes->Delete(i);
   delete lisLinhasLoadshapes;
}
//---------------------------------------------------------------------------
/***
 * Método para exportar os dados de nível de curto-circuito
 */
void __fastcall TConversorSinapDSS::ExportaDadosPotCurto_suprimentoMT(StrEquivalenteThevenin* strEqThv, VTTrafo* trafoSE)
{
	double r_eq_MT, x_eq_MT, z_eq_MT;
	double vnom_MT, Icc_MT, I_base;
	String pathSaidaPotCurto;

	// Obtém os equivalentes (em pu) de seq positiva na MT
	r_eq_MT = strEqThv->r1;
	x_eq_MT = strEqThv->x1;
	z_eq_MT = sqrt((r_eq_MT*r_eq_MT) + (x_eq_MT*x_eq_MT));

	// Tensão nominal (Volts de linha) no nível de MT
	vnom_MT = trafoSE->sec.vnom;

	// Corrente de base (Ampères) no secundário do trafo
	I_base = (1000. * trafoSE->snom) / (sqrt(3.) * vnom_MT);

   // Corrente de curto-circuito 3F no secundário do trafo
   Icc_MT = I_base * (1. / z_eq_MT);

   // Ajusta o path de saída
	pathSaidaPotCurto = pathSaida + "\\..\\Geral.ini";

   // Insere no Geral.ini dados de potência de curto
	TIniFile* file = new TIniFile(pathSaidaPotCurto);
	file->WriteFloat("NIVELCURTOMT", "Vnom", vnom_MT);
	file->WriteFloat("NIVELCURTOMT", "IccMT", Icc_MT);

//	file->Free();
   delete file; file = NULL;
}
//---------------------------------------------------------------------------
/***
 * Método para exportar os dados de nível de curto-circuito
 */
void __fastcall TConversorSinapDSS::ExportaDadosPotCurto(VTSuprimento* sup, VTTrafo* trafoSE)
{
	double r_eq_MT, x_eq_MT, z_eq_MT;
	double vnom_MT, Icc_MT, I_base;
	String pathSaidaPotCurto;

   // Obtém os equivalentes de seq positiva na MT
	r_eq_MT = sup->zeq1.r + trafoSE->z1.r * (100. / trafoSE->snom);
	x_eq_MT = sup->zeq1.x + trafoSE->z1.x * (100. / trafoSE->snom);
   z_eq_MT = sqrt((r_eq_MT*r_eq_MT) + (x_eq_MT*x_eq_MT));

   // Tensão nominal (Volts de linha) no nível de MT
   vnom_MT = trafoSE->sec.vnom;

   // Corrente de base (Ampères) no secundário do trafo
   I_base = (1000. * trafoSE->snom) / (sqrt(3.) * vnom_MT);

   // Corrente de curto-circuito 3F no secundário do trafo
   Icc_MT = I_base * (1. / z_eq_MT);

   // Ajusta o path de saída
	pathSaidaPotCurto = pathSaida + "\\..\\Geral.ini";

   // Insere no Geral.ini dados de potência de curto
	TIniFile* file = new TIniFile(pathSaidaPotCurto);
	file->WriteFloat("NIVELCURTOMT", "Vnom", vnom_MT);
	file->WriteFloat("NIVELCURTOMT", "IccMT", Icc_MT);

   // Destroi objeto
//	file->Free();
   delete file; file = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::FiltraBarrasAreaConsiderada(TList* lisBarrasTodas)
{
	if(!lisBarrasTodas) return;

	// Remove as barras que não estiverem na área considerada (à jusante do eqpto de referência)
	for(int i=lisBarrasTodas->Count-1; i>=0; i--)
	{
		VTBarra* barra = (VTBarra*) lisBarrasTodas->Items[i];

		if(lisBarrasConsideradas->IndexOf(barra) < 0)
			lisBarrasTodas->Remove(barra);
	}
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::FiltraLigacoesAreaConsiderada(TList* lisLigacoesTodas)
{
	if(!lisLigacoesTodas) return;

   // Remove as ligações que não estiverem na área considerada (à jusante do eqpto de referência)
	for(int i=lisLigacoesTodas->Count-1; i>=0; i--)
	{
		VTLigacao* ligacao = (VTLigacao*) lisLigacoesTodas->Items[i];

		if(lisLigacoesConsideradas->IndexOf(ligacao) < 0)
			lisLigacoesTodas->Remove(ligacao);
	}
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Lines.dss
 */
void __fastcall TConversorSinapDSS::ExportaLigacoes()
{
	int utm_x, utm_y;
	String compTrechoStr, indiceFases, linhaDSS, linhaDSStrechos = "", linhaDSSchaves = "", linhaDSSbarras;
	TStringList* linhasDSS = new TStringList();
   TStringList* linhasDSStrechos = new TStringList();
   TStringList* linhasDSSchaves = new TStringList();
   TStringList* linhasDSSbarras = new TStringList(); // coordenadas das barras (id_barra utm_x(m) utm_y(m))
   TList* lisPrimarios;
	TList *lisEXT_BAR, *lisEXT_BAR_aux, *lisEXT_LIG, *lisEXT_LIG_aux;
	VTArranjo* arranjo;
   VTChave* chave;
   VTPrimario* primario = NULL;
	VTTrecho* trecho;

   // Verificação
   if(redeMT == NULL) return;

   // Pega o primário da rede MT
   lisPrimarios = radial->LisPrimario();
	primario = NULL;
	for(int ip=0; ip<lisPrimarios->Count; ip++)
	{
		primario = (VTPrimario*) lisPrimarios->Items[ip];
		if(primario->Rede == redeMT)
			break;
		else
			primario = NULL;
	}

	// Pega listas ordenadas de barras e ligações da rede MT
	lisEXT_BAR = new TList();
	lisEXT_LIG = new TList();
	primario->Ordena(lisEXT_BAR, lisEXT_LIG);

	if(posicaoSuprimento == "EqptoRef")
	{
		FiltraLigacoesAreaConsiderada(lisEXT_LIG);
		FiltraBarrasAreaConsiderada(lisEXT_BAR);
	}

	// Pega as coordenadas das barras
	for(int i=0; i<lisEXT_BAR->Count; i++)
	{
		VTBarra* barra = (VTBarra*) lisEXT_BAR->Items[i];
		barra->CoordenadasUtm_m(utm_x, utm_y);
		linhaDSSbarras = String(barra->Id) + " " + String(utm_x) + " " + String(utm_y);
		linhasDSSbarras->Add(linhaDSSbarras);
	}

	// Analisa redes filhas
	lisEXT_BAR_aux = new TList();
	lisEXT_LIG_aux = new TList();
	RedesFilhas(redeMT, lisTrafosSEsFilhas, lisRedesFilhas);
	for(int i=0; i<lisRedesFilhas->Count; i++)
	{
		VTRede* rede = (VTRede*) lisRedesFilhas->Items[i];

		primario = NULL;
		for(int ip=0; ip<lisPrimarios->Count; ip++)
		{
			primario = (VTPrimario*) lisPrimarios->Items[ip];
			if(primario->Rede == rede)
				break;
			else
				primario = NULL;
		}
		if(primario == NULL) continue;

		// Pega listas ordenadas de barras e ligações
		lisEXT_BAR_aux->Clear();
		lisEXT_LIG_aux->Clear();
		primario->Ordena(lisEXT_BAR_aux, lisEXT_LIG_aux);

		if(posicaoSuprimento == "EqptoRef")
		{
			FiltraLigacoesAreaConsiderada(lisEXT_LIG_aux);
			FiltraBarrasAreaConsiderada(lisEXT_BAR_aux);
		}

		// Pega as coordenadas das barras
		for(int i=0; i<lisEXT_BAR_aux->Count; i++)
		{
			VTBarra* barra = (VTBarra*) lisEXT_BAR_aux->Items[i];
			barra->CoordenadasUtm_m(utm_x, utm_y);
			linhaDSSbarras = String(barra->Id) + " " + String(utm_x) + " " + String(utm_y);
			linhasDSSbarras->Add(linhaDSSbarras);
		}

		// Inclui as ligações da rede filha na lista total de ligações
		for(int j=0; j<lisEXT_LIG_aux->Count; j++)
		{
			VTLigacao* liga = (VTLigacao*) lisEXT_LIG_aux->Items[j];
			lisEXT_LIG->Add(liga);
      }
	}

	// Exporta arquivo de coordenadas
   linhasDSSbarras->SaveToFile(pathSaida + "\\coords.dat");

   // Gera uma linha para cada trecho e para cada chave
   for(int i=0; i<lisEXT_LIG->Count; i++)
   {
		VTLigacao* liga = (VTLigacao*) lisEXT_LIG->Items[i];

   	switch(liga->Tipo())
      {
			case eqptoCHAVE:
         case eqptoREGULADOR:

				chave = (VTChave*) liga;
				linhaDSSchaves = "New Line." + chave->Codigo;
            linhaDSSchaves += " Bus1=" + String(chave->Barra(0)->Id);
            linhaDSSchaves += " Bus2=" + String(chave->Barra(1)->Id);
            linhaDSSchaves += " Length=0.001 Units=km";
            linhaDSSchaves += " r1=0.001 r0=0.001 x1=0 x0=0 c1=0 c0=0";
            // Adiciona linha à lista de chaves
            linhasDSSchaves->Add(linhaDSSchaves);
         	break;


			case eqptoTRECHO:

            trecho = (VTTrecho*) liga;
				indiceFases = GetIndiceFases(trecho->arranjo->Fases);
            compTrechoStr = DoubleToString(trecho->Comprimento_km, 5);
				linhaDSStrechos = "New Line." + String(liga->Id);
				linhaDSStrechos += " Phases=" + String(InsereInfoFasesTrecho(indiceFases));
				linhaDSStrechos += " Bus1=" + String(liga->Barra(0)->Id);
				linhaDSStrechos += indiceFases;
				linhaDSStrechos += " Bus2=" + String(liga->Barra(1)->Id);
            linhaDSStrechos += indiceFases;
            linhaDSStrechos += " Length=" + compTrechoStr;
            linhaDSStrechos += " LineCode=" + String(((VTTrecho*)liga)->arranjo->Id);
            // Pega o arranjo do trecho e o guarda em lista
            arranjo = ((VTTrecho*)liga)->arranjo;
            lisArranjos->Add(arranjo);
            // Adiciona linha à lista de trechos
            linhasDSStrechos->Add(linhaDSStrechos);
         	break;
      }

   }

   // Insere as informações de fases nas linhas das chaves
	InsereInfoFasesChaves(linhasDSStrechos, linhasDSSchaves);

   // Insere linhas de trechos e de chaves à lista final
   for(int i=0; i<linhasDSStrechos->Count; i++)
   {
		linhaDSS = linhasDSStrechos->Strings[i];
      linhasDSS->Add(linhaDSS);
   }
   for(int i=0; i<linhasDSSchaves->Count; i++)
   {
		linhaDSS = linhasDSSchaves->Strings[i];
      linhasDSS->Add(linhaDSS);
   }

   // Exporta arquivo lines.dss
   linhasDSS->SaveToFile(pathSaida + "\\lines.dss");

   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
   delete linhasDSS;
   for(int i=linhasDSStrechos->Count-1; i>=0; i--) linhasDSStrechos->Delete(i);
   delete linhasDSStrechos;
   for(int i=linhasDSSchaves->Count-1; i>=0; i--) linhasDSSchaves->Delete(i);
   delete linhasDSSchaves;
   for(int i=linhasDSSbarras->Count-1; i>=0; i--) linhasDSSbarras->Delete(i);
   delete linhasDSSbarras;
}
//---------------------------------------------------------------------------
// Dentre os alimentadores primários, verifica aqueles que tem seu trafo de SE
// na lista "lisTrafosSEsFilhas"
void __fastcall TConversorSinapDSS::RedesFilhas(VTRede* redeMT, TList* lisTrafosSEsFilhas, TList* lisEXT)
{
	if(redeMT == NULL || lisTrafosSEsFilhas == NULL || lisEXT == NULL) return;

	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redePRI) continue;
		if(rede == redeMT) continue;

		for(int j=0; j<lisTrafosSEsFilhas->Count; j++)
		{
			VTTrafo* trafo = (VTTrafo*) lisTrafosSEsFilhas->Items[j];
			if(rede->ExisteBarra(trafo->Barra(0)) || rede->ExisteBarra(trafo->Barra(1)))
			{
				if(lisEXT->IndexOf(rede) < 0)
					lisEXT->Add(rede);
         }
		}
   }
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Master.dss
 */
void __fastcall TConversorSinapDSS::ExportaMaster()
{
	if(posicaoSuprimento == "AT")
		ExportaMaster_SuprimentoAT();
	else if(posicaoSuprimento == "MT")
		ExportaMaster_SuprimentoMT();
	else if(posicaoSuprimento == "EqptoRef")
		ExportaMaster_SuprimentoEqptoRef();
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::ExportaMaster_FLOffline()
{
	ExportaMaster_SuprimentoEqptoRef_FLOffline();
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Master.dss, com o suprimento
 * na MT (à jusante do trafo SE)
 */
void __fastcall TConversorSinapDSS::ExportaMaster_SuprimentoAT()
{
	double baseKV, r0_sup, x0_sup, r1_sup, x1_sup;
	double kvaTRAFO, kv1, kv2, rtrafo_perc;
	strIMP z1;
	strIMP zeq0, zeq1;
	String linhaDSS = "";
   TStringList* linhasDSS = new TStringList();
   TList* lisTrafoSE = new TList();
   TList* lisReg = new TList();
   VTBarra *barraPRI, *barraSEC;
   VTSuprimento* sup = NULL;
	VTTrafo* trafoSE = NULL;

   // ::::::::::::::::::::::::::::::::::::::::
   // Obtém os dados do trafo SE
   // ::::::::::::::::::::::::::::::::::::::::
   redeSE->LisEqpto(lisTrafoSE, eqptoTRAFO);
   for(int i=0; i<lisTrafoSE->Count; i++)
   {
		trafoSE = (VTTrafo*) lisTrafoSE->Items[i];

		if(trafoSE->BarraSecundario() == redeMT->BarraInicial())
      {
         break;
      }
      else
      {
         trafoSE = NULL;
      }
   }
   // Verificação
	if(trafoSE == NULL) return;
	// Parâmetros do trafo
	kvaTRAFO = trafoSE->snom * 1000.;
	z1 = trafoSE->z1;
	rtrafo_perc = (100.) * z1.r / 2.;
//	rtrafo_perc = 0.5;
	kv1 = trafoSE->pri.vnom;
	kv2 = trafoSE->sec.vnom;
	barraPRI = trafoSE->BarraPrimario();
	barraSEC = trafoSE->BarraSecundario();

   // ::::::::::::::::::::::::::::::::::::::::
   // Trata do suprimento (eqbar da barra de AT)
   // ::::::::::::::::::::::::::::::::::::::::
   TList* lisEqbar = barraPRI->LisEqbar();
   VTEqbar* eqbarSup = NULL;
   for(int i=0; i<lisEqbar->Count; i++)
   {
		eqbarSup = (VTEqbar*) lisEqbar->Items[i];
      if(eqbarSup->Tipo() == eqptoSUPRIMENTO)
      {
         break;
      }
      else
      {
         eqbarSup = NULL;
      }
   }
   // Verificação
   if(eqbarSup == NULL) return;
   sup = (VTSuprimento*) eqbarSup;
   baseKV = sup->vnom;
   zeq0 = sup->zeq0;
   zeq1 = sup->zeq1;

   if((zeq0.r == 0. && zeq0.x == 0.) || (zeq1.r == 0. && zeq1.x == 0.))
   {
      std::complex<double> s3f = std::complex<double>(sup->pcc_3f.p, sup->pcc_3f.q) / 100.;
      std::complex<double> sft = std::complex<double>(sup->pcc_ft.p, sup->pcc_ft.q) / 100.;

      std::complex<double> zth_1 = std::complex<double>(0., 0.1), zth_0 = std::complex<double>(0., 0.1);
      if(sup->pcc_3f.p != 0. || sup->pcc_3f.q != 0.)
      {
         zth_1 = 1. / std::conj(s3f);
         zth_0 = 3. / std::conj(sft) - 2. / std::conj(s3f);

         r0_sup = std::real(zth_0);
         x0_sup = std::imag(zth_0);
         r1_sup = std::real(zth_1);
         x1_sup = std::imag(zth_1);
      }
   }
   else
   {
      r0_sup = zeq0.r * (baseKV * baseKV / 100.);
      x0_sup = zeq0.x * (baseKV * baseKV / 100.);
      r1_sup = zeq1.r * (baseKV * baseKV / 100.);
      x1_sup = zeq1.x * (baseKV * baseKV / 100.);
   }

   // Exporta potência de curto no início do alimentador
	ExportaDadosPotCurto(sup, trafoSE);


	// Verifica transformadores de SEs filhas
	bool TratarSEsFilhas = true;
	if(TratarSEsFilhas)
	{
		TrafosSEsFilhas(lisTrafosSEsFilhas);
   }

   // Compõe as linhas do arquivo
   linhasDSS->Add("clear");
   linhasDSS->Add("");
   linhasDSS->Add("set datapath=(" + pathSaida + ")");
   linhasDSS->Add("");
   linhasDSS->Add("!suprimento");

	// Insere os dados do equivalente na subestação
	String codigoAlimentador = ReplaceStr(redeMT->Codigo, "-", "");
	codigoAlimentador = ReplaceStr(redeMT->Codigo, " ", "");
	linhaDSS = "New object=Circuit." + codigoAlimentador;
	linhaDSS += " baseKV=" + DoubleToString(baseKV, 1) + " pu=1.000 Bus1=" + String(sup->pbarra->Id);
   linhaDSS += " r0=" + DoubleToString(r0_sup, 4) + " x0=" + DoubleToString(x0_sup, 4) + " r1=" + DoubleToString(r1_sup, 4) + " x1=" + DoubleToString(x1_sup, 4);
   linhasDSS->Add(linhaDSS);
   linhasDSS->Add("");

	// Insere os dados do transformador de subestação
	linhaDSS = "New Transformer.trafoSE Windings=2 Phases=3 Buses=(" + String(barraPRI->Id) + ", " + String(barraSEC->Id) + ")";
	linhasDSS->Add(linhaDSS);
	linhaDSS = "~ Conns=(delta wye)";
	linhasDSS->Add(linhaDSS);
	linhaDSS = "~ kvs=(" + DoubleToString(kv1, 1) + ", " + DoubleToString(kv2, 1) + ") kvas=(" + String(kvaTRAFO) + ", " + String(kvaTRAFO) + ")";
	linhasDSS->Add(linhaDSS);
	linhaDSS = "~ xhl=" + DoubleToString(100. * z1.x, 2);
	linhasDSS->Add(linhaDSS);
	linhaDSS = "~ wdg=1 %r=" + DoubleToString(rtrafo_perc, 3);
	linhasDSS->Add(linhaDSS);
	linhaDSS = "~ wdg=2 %r=" + DoubleToString(rtrafo_perc, 3);
	linhasDSS->Add(linhaDSS);
	linhasDSS->Add("");

	// Redirecionamentos
	if(lisTrafosSEsFilhas->Count > 0)
	{
   	linhasDSS->Add("Redirect trafos.dss");
	}
	linhasDSS->Add("Redirect linecodes.dss");
   linhasDSS->Add("Redirect lines.dss");
   linhasDSS->Add("Redirect loadshapes.dss");
   linhasDSS->Add("Redirect loads.dss");
   linhasDSS->Add("");

   // Auxiliares
//   linhasDSS->Add("Set voltagebases=[" + DoubleToString(kv1, 1) + ", " + DoubleToString(kv2, 1) + "]");
	linhasDSS->Add("Set voltagebases=[" + ReplaceStr(VoltageBases(trafoSE, lisTrafosSEsFilhas), ",", ".") + "]");
   linhasDSS->Add("Calcvoltagebases");
	linhasDSS->Add("!Buscoords coords.dat");
	linhasDSS->Add("");
	linhasDSS->Add("Set mode=daily");
	linhasDSS->Add("!Set number=1");
   linhasDSS->Add("!Solve");
	linhasDSS->Add("");
   linhasDSS->Add("!Show voltage ln nodes");
	linhasDSS->Add("!Show current elements");
   linhasDSS->Add("!Plot circuit power dots=n labels=n subs=y");

	// Exporta arquivo lines.dss
	linhasDSS->SaveToFile(pathSaida + "\\Master.dss");

	// Destroi lista
   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
	delete linhasDSS; linhasDSS = NULL;
   delete lisTrafoSE; lisTrafoSE = NULL;
   delete lisReg; lisReg = NULL;
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Master.dss com o suprimento
 * na MT (à jusante do trafo SE)
 */
void __fastcall TConversorSinapDSS::ExportaMaster_SuprimentoMT()
{
	double baseKV, r0_sup, x0_sup, r1_sup, x1_sup;
	double kvaTRAFO, kv1, kv2, rtrafo_perc;
	strIMP z1;
	strIMP zeq0, zeq1;
	String linhaDSS = "";
	TStringList* linhasDSS = new TStringList();
	TList* lisTrafoSE = new TList();
	TList* lisReg = new TList();
	VTBarra *barraPRI, *barraSEC;
   VTSuprimento* sup = NULL;
	VTTrafo* trafoSE = NULL;

   // ::::::::::::::::::::::::::::::::::::::::
   // Obtém os dados do trafo SE
   // ::::::::::::::::::::::::::::::::::::::::
   redeSE->LisEqpto(lisTrafoSE, eqptoTRAFO);
   for(int i=0; i<lisTrafoSE->Count; i++)
   {
		trafoSE = (VTTrafo*) lisTrafoSE->Items[i];

		if(trafoSE->BarraSecundario() == redeMT->BarraInicial())
      {
         break;
      }
      else
      {
         trafoSE = NULL;
      }
   }
   // Verificação
	if(trafoSE == NULL) return;
	// Parâmetros do trafo
	kvaTRAFO = trafoSE->snom * 1000.;
	z1 = trafoSE->z1;
	rtrafo_perc = (100.) * z1.r / 2.;
	kv1 = trafoSE->pri.vnom;
	kv2 = trafoSE->sec.vnom;
	barraPRI = trafoSE->BarraPrimario();
	barraSEC = trafoSE->BarraSecundario();

	// Define a tensão nominal do início do alimentador
	baseKV = Round(barraSEC->vnom, 1);

	// Obtém o equivalente de Thèvènin (em pu), na barra inicial do alimentador (MT)
	StrEquivalenteThevenin* strEqThv = new StrEquivalenteThevenin();
	strEqThv->r0 = 0.0001;
	strEqThv->x0 = 0.0001;
	strEqThv->r1 = 0.0001;
	strEqThv->x1 = 0.0001;
	EquivalenteTheveninMT(redeSE, redeMT, strEqThv);

	// Exporta potência de curto no início do alimentador
	ExportaDadosPotCurto_suprimentoMT(strEqThv, trafoSE);

	// Verifica transformadores de SEs filhas
	bool TratarSEsFilhas = true;
	if(TratarSEsFilhas)
	{
		TrafosSEsFilhas(lisTrafosSEsFilhas);
   }

   // Compõe as linhas do arquivo
   linhasDSS->Add("clear");
   linhasDSS->Add("");
   linhasDSS->Add("set datapath=(" + pathSaida + ")");
   linhasDSS->Add("");
   linhasDSS->Add("!suprimento");

	// Insere os dados do equivalente na subestação, com o suprimento em MT
	String codigoAlimentador = ReplaceStr(redeMT->Codigo, "-", "");
	codigoAlimentador = ReplaceStr(redeMT->Codigo, " ", "");
	linhaDSS = "New object=Circuit." + codigoAlimentador;
	linhaDSS += " baseKV=" + DoubleToString(baseKV, 1) + " pu=1.000 Bus1=" + String(barraSEC->Id);
	linhaDSS += " r0=" + DoubleToString(strEqThv->r0 * (baseKV*baseKV/100.), 4);
	linhaDSS += " x0=" + DoubleToString(strEqThv->x0 * (baseKV*baseKV/100.), 4);
	linhaDSS += " r1=" + DoubleToString(strEqThv->r1 * (baseKV*baseKV/100.), 4);
	linhaDSS += " x1=" + DoubleToString(strEqThv->x1 * (baseKV*baseKV/100.), 4);
	linhasDSS->Add(linhaDSS);
	linhasDSS->Add("");

	// Redirecionamentos
	if(lisTrafosSEsFilhas->Count > 0)
	{
   	linhasDSS->Add("Redirect trafos.dss");
	}
	linhasDSS->Add("Redirect linecodes.dss");
   linhasDSS->Add("Redirect lines.dss");
   linhasDSS->Add("Redirect loadshapes.dss");
   linhasDSS->Add("Redirect loads.dss");
   linhasDSS->Add("");

	// Auxiliares
	linhasDSS->Add("Set voltagebases=[" + ReplaceStr(VoltageBases_SuprimentoMT(trafoSE, lisTrafosSEsFilhas), ",", ".") + "]");
	linhasDSS->Add("Calcvoltagebases");
	linhasDSS->Add("!Buscoords coords.dat");
	linhasDSS->Add("");
	linhasDSS->Add("Set mode=daily");
	linhasDSS->Add("!Set number=1");
	linhasDSS->Add("Solve");
	linhasDSS->Add("");
	linhasDSS->Add("!Show voltage ln nodes");
	linhasDSS->Add("!Show current elements");
	linhasDSS->Add("!Plot circuit power dots=n labels=n subs=y");

	// Exporta arquivo lines.dss
	linhasDSS->SaveToFile(pathSaida + "\\Master.dss");

	// Destroi lista
   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
	delete linhasDSS;
   delete lisTrafoSE; lisTrafoSE = NULL;
   delete lisReg; lisReg = NULL;
   delete strEqThv; strEqThv = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::ExportaMaster_SuprimentoEqptoRef_FLOffline()
{
   double baseKV, r0_sup, x0_sup, r1_sup, x1_sup;
	double kvaTRAFO, kv1, kv2, rtrafo_perc;
	strIMP z1;
	strIMP zeq0, zeq1;
	String linhaDSS = "";
	TStringList* linhasDSS = new TStringList();
	TList* lisTrafoSE = new TList();
	TList* lisReg = new TList();
	VTBarra *barraPRI, *barraSEC;
   VTSuprimento* sup = NULL;
	VTTrafo* trafoSE = NULL;

   // ::::::::::::::::::::::::::::::::::::::::
   // Obtém os dados do trafo SE
   // ::::::::::::::::::::::::::::::::::::::::
   redeSE->LisEqpto(lisTrafoSE, eqptoTRAFO);
   for(int i=0; i<lisTrafoSE->Count; i++)
   {
		trafoSE = (VTTrafo*) lisTrafoSE->Items[i];

		if(trafoSE->BarraSecundario() == redeMT->BarraInicial())
      {
         break;
      }
      else
      {
         trafoSE = NULL;
      }
   }
   // Verificação
	if(trafoSE == NULL) return;
	// Parâmetros do trafo
	kvaTRAFO = trafoSE->snom * 1000.;
	z1 = trafoSE->z1;
	rtrafo_perc = (100.) * z1.r / 2.;
	kv1 = trafoSE->pri.vnom;
	kv2 = trafoSE->sec.vnom;
	barraPRI = trafoSE->BarraPrimario();
	barraSEC = trafoSE->BarraSecundario();

	// Define a tensão nominal do início do alimentador
	baseKV = Round(barraSEC->vnom, 1);

	// Obtém o equivalente de Thèvènin (em pu), na barra inicial do alimentador (MT)
	StrEquivalenteThevenin* strEqThv = new StrEquivalenteThevenin();
	EquivalenteTheveninEqptoRef_FLOffline(baseKV, strEqThv);

	// Exporta potência de curto no início do alimentador
	ExportaDadosPotCurto_suprimentoMT(strEqThv, trafoSE);

	// Verifica transformadores de SEs filhas
	bool TratarSEsFilhas = true;
	if(TratarSEsFilhas)
	{
		TrafosSEsFilhas(lisTrafosSEsFilhas);
   }

   // Compõe as linhas do arquivo
   linhasDSS->Add("clear");
   linhasDSS->Add("");
   linhasDSS->Add("set datapath=(" + pathSaida + ")");
   linhasDSS->Add("");
   linhasDSS->Add("!suprimento");

	// Insere os dados do equivalente na subestação, com o suprimento em MT
//	linhaDSS = "New object=Circuit." + redeMT->Codigo;
//	linhaDSS += " baseKV=" + DoubleToString(baseKV, 1) + " pu=1.000 Bus1=" + String(barraSEC->Id);
//	linhaDSS += " r0=" + DoubleToString(strEqThv->r0 * (baseKV*baseKV/100.), 4);
//	linhaDSS += " x0=" + DoubleToString(strEqThv->x0 * (baseKV*baseKV/100.), 4);
//	linhaDSS += " r1=" + DoubleToString(strEqThv->r1 * (baseKV*baseKV/100.), 4);
//	linhaDSS += " x1=" + DoubleToString(strEqThv->x1 * (baseKV*baseKV/100.), 4);
//	linhasDSS->Add(linhaDSS);
//	linhasDSS->Add("");

	String codigoAlimentador = ReplaceStr(redeMT->Codigo, "-", "");
	codigoAlimentador = ReplaceStr(redeMT->Codigo, " ", "");
	linhaDSS = "New object=Circuit." + codigoAlimentador;
	linhaDSS += " baseKV=" + DoubleToString(baseKV, 1) + " pu=1.000 Bus1=" + String(strEqThv->barra->Id);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhasDSS->Add(linhaDSS);
	linhasDSS->Add("vsource.source.enabled=false");

	linhaDSS = "new vsource.faseA phases=1 bus1=" + String(strEqThv->barra->Id) + ".1";
	linhaDSS += " baseKV=" + DoubleToString(baseKV/sqrt(3.), 3);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhaDSS += " pu=" + DoubleToString(strEqThv->Va_pu, 4);
	linhaDSS += " angle=" + DoubleToString(strEqThv->pVa, 2);
	linhasDSS->Add(linhaDSS);
	linhaDSS = "new vsource.faseB phases=1 bus1=" + String(strEqThv->barra->Id) + ".2";
	linhaDSS += " baseKV=" + DoubleToString(baseKV/sqrt(3.), 3);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhaDSS += " pu=" + DoubleToString(strEqThv->Vb_pu, 4);
	linhaDSS += " angle=" + DoubleToString(strEqThv->pVb, 2);
	linhasDSS->Add(linhaDSS);
	linhaDSS = "new vsource.faseC phases=1 bus1=" + String(strEqThv->barra->Id) + ".3";
	linhaDSS += " baseKV=" + DoubleToString(baseKV/sqrt(3.), 3);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhaDSS += " pu=" + DoubleToString(strEqThv->Vc_pu, 4);
	linhaDSS += " angle=" + DoubleToString(strEqThv->pVc, 2);
	linhasDSS->Add(linhaDSS);
	linhasDSS->Add("");

	// Redirecionamentos
	if(lisTrafosSEsFilhas->Count > 0)
	{
   	linhasDSS->Add("Redirect trafos.dss");
	}
	linhasDSS->Add("Redirect linecodes.dss");
   linhasDSS->Add("Redirect lines.dss");
   linhasDSS->Add("Redirect loadshapes.dss");
   linhasDSS->Add("Redirect loads.dss");
   linhasDSS->Add("");

	// Auxiliares
	linhasDSS->Add("Set voltagebases=[" + ReplaceStr(VoltageBases_SuprimentoMT(trafoSE, lisTrafosSEsFilhas), ",", ".") + "]");
	linhasDSS->Add("Calcvoltagebases");
	linhasDSS->Add("!Buscoords coords.dat");
	linhasDSS->Add("");
	linhasDSS->Add("Set mode=daily");
	linhasDSS->Add("!Set number=1");
	linhasDSS->Add("!Solve");
	linhasDSS->Add("");
	linhasDSS->Add("!Show voltage ln nodes");
	linhasDSS->Add("!Show current elements");
	linhasDSS->Add("!Plot circuit power dots=n labels=n subs=y");

	// Exporta arquivo lines.dss
	linhasDSS->SaveToFile(pathSaida + "\\Master.dss");

	// Destroi lista
   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
	delete linhasDSS;
	delete lisTrafoSE; lisTrafoSE = NULL;
	delete lisReg; lisReg = NULL;
   delete strEqThv; strEqThv = NULL;
}
//---------------------------------------------------------------------------
/***
 * Método para gerar as linhas do arquivo Master.dss com o suprimento
 * na posição do EqptoRef (qualímetro com medição de V, I)
 */
void __fastcall TConversorSinapDSS::ExportaMaster_SuprimentoEqptoRef()
{
	double baseKV, r0_sup, x0_sup, r1_sup, x1_sup;
	double kvaTRAFO, kv1, kv2, rtrafo_perc;
	strIMP z1;
	strIMP zeq0, zeq1;
	String linhaDSS = "";
	TStringList* linhasDSS = new TStringList();
	TList* lisTrafoSE = new TList();
	TList* lisReg = new TList();
	VTBarra *barraPRI, *barraSEC;
   VTSuprimento* sup = NULL;
	VTTrafo* trafoSE = NULL;

   // ::::::::::::::::::::::::::::::::::::::::
   // Obtém os dados do trafo SE
   // ::::::::::::::::::::::::::::::::::::::::
   redeSE->LisEqpto(lisTrafoSE, eqptoTRAFO);
   for(int i=0; i<lisTrafoSE->Count; i++)
   {
		trafoSE = (VTTrafo*) lisTrafoSE->Items[i];

		if(trafoSE->BarraSecundario() == redeMT->BarraInicial())
      {
         break;
      }
      else
      {
         trafoSE = NULL;
      }
   }
   // Verificação
	if(trafoSE == NULL) return;
	// Parâmetros do trafo
	kvaTRAFO = trafoSE->snom * 1000.;
	z1 = trafoSE->z1;
	rtrafo_perc = (100.) * z1.r / 2.;
	kv1 = trafoSE->pri.vnom;
	kv2 = trafoSE->sec.vnom;
	barraPRI = trafoSE->BarraPrimario();
	barraSEC = trafoSE->BarraSecundario();

	// Define a tensão nominal do início do alimentador
	baseKV = Round(barraSEC->vnom, 1);

	// Obtém o equivalente de Thèvènin (em pu), na barra inicial do alimentador (MT)
	StrEquivalenteThevenin* strEqThv = new StrEquivalenteThevenin();
	EquivalenteTheveninEqptoRef(baseKV, strEqThv);

	// Exporta potência de curto no início do alimentador
	ExportaDadosPotCurto_suprimentoMT(strEqThv, trafoSE);

	// Verifica transformadores de SEs filhas
	bool TratarSEsFilhas = true;
	if(TratarSEsFilhas)
	{
		TrafosSEsFilhas(lisTrafosSEsFilhas);
   }

   // Compõe as linhas do arquivo
   linhasDSS->Add("clear");
   linhasDSS->Add("");
   linhasDSS->Add("set datapath=(" + pathSaida + ")");
   linhasDSS->Add("");
   linhasDSS->Add("!suprimento");

	// Insere os dados do equivalente na subestação, com o suprimento em MT
//	linhaDSS = "New object=Circuit." + redeMT->Codigo;
//	linhaDSS += " baseKV=" + DoubleToString(baseKV, 1) + " pu=1.000 Bus1=" + String(barraSEC->Id);
//	linhaDSS += " r0=" + DoubleToString(strEqThv->r0 * (baseKV*baseKV/100.), 4);
//	linhaDSS += " x0=" + DoubleToString(strEqThv->x0 * (baseKV*baseKV/100.), 4);
//	linhaDSS += " r1=" + DoubleToString(strEqThv->r1 * (baseKV*baseKV/100.), 4);
//	linhaDSS += " x1=" + DoubleToString(strEqThv->x1 * (baseKV*baseKV/100.), 4);
//	linhasDSS->Add(linhaDSS);
//	linhasDSS->Add("");
	String codigoAlimentador = ReplaceStr(redeMT->Codigo, "-", "");
	codigoAlimentador = ReplaceStr(redeMT->Codigo, " ", "");
	linhaDSS = "New object=Circuit." + codigoAlimentador;
	linhaDSS += " baseKV=" + DoubleToString(baseKV, 1) + " pu=1.000 Bus1=" + String(strEqThv->barra->Id);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhasDSS->Add(linhaDSS);
	linhasDSS->Add("vsource.source.enabled=false");

	linhaDSS = "new vsource.faseA phases=1 bus1=" + String(strEqThv->barra->Id) + ".1";
	linhaDSS += " baseKV=" + DoubleToString(baseKV/sqrt(3.), 3);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhaDSS += " pu=" + DoubleToString(strEqThv->Va_pu, 4);
	linhaDSS += " angle=" + DoubleToString(strEqThv->pVa, 2);
	linhasDSS->Add(linhaDSS);
	linhaDSS = "new vsource.faseB phases=1 bus1=" + String(strEqThv->barra->Id) + ".2";
	linhaDSS += " baseKV=" + DoubleToString(baseKV/sqrt(3.), 3);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhaDSS += " pu=" + DoubleToString(strEqThv->Vb_pu, 4);
	linhaDSS += " angle=" + DoubleToString(strEqThv->pVb, 2);
	linhasDSS->Add(linhaDSS);
	linhaDSS = "new vsource.faseC phases=1 bus1=" + String(strEqThv->barra->Id) + ".3";
	linhaDSS += " baseKV=" + DoubleToString(baseKV/sqrt(3.), 3);
	linhaDSS += " r0=0.00001 x0=0.00001 r1=0.00001 x1=0.00001";
	linhaDSS += " pu=" + DoubleToString(strEqThv->Vc_pu, 4);
	linhaDSS += " angle=" + DoubleToString(strEqThv->pVc, 2);
	linhasDSS->Add(linhaDSS);
	linhasDSS->Add("");

	// Redirecionamentos
	if(lisTrafosSEsFilhas->Count > 0)
	{
   	linhasDSS->Add("Redirect trafos.dss");
	}
	linhasDSS->Add("Redirect linecodes.dss");
   linhasDSS->Add("Redirect lines.dss");
   linhasDSS->Add("Redirect loadshapes.dss");
   linhasDSS->Add("Redirect loads.dss");
   linhasDSS->Add("");

	// Auxiliares
	linhasDSS->Add("Set voltagebases=[" + ReplaceStr(VoltageBases_SuprimentoMT(trafoSE, lisTrafosSEsFilhas), ",", ".") + "]");
	linhasDSS->Add("Calcvoltagebases");
	linhasDSS->Add("!Buscoords coords.dat");
	linhasDSS->Add("");
	linhasDSS->Add("Set mode=daily");
	linhasDSS->Add("!Set number=1");
	linhasDSS->Add("!Solve");
	linhasDSS->Add("");
	linhasDSS->Add("!Show voltage ln nodes");
	linhasDSS->Add("!Show current elements");
	linhasDSS->Add("!Plot circuit power dots=n labels=n subs=y");

	// Exporta arquivo lines.dss
	linhasDSS->SaveToFile(pathSaida + "\\Master.dss");

	// Destroi lista
   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
	delete linhasDSS;
	delete lisTrafoSE; lisTrafoSE = NULL;
	delete lisReg; lisReg = NULL;
   delete strEqThv; strEqThv = NULL;
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::VoltageBases_SuprimentoMT(VTTrafo* trafoSE, TList* lisTrafosSEsFilhas)
{
	String tensoesBase = "";
	TStringList* lisaux = new TStringList();

	if(trafoSE == NULL && lisTrafosSEsFilhas == NULL) return "";

	InsereOrdenado(lisaux, String(Round(trafoSE->sec.vnom,2)));

	for(int i=0; i<lisTrafosSEsFilhas->Count; i++)
	{
		VTTrafo* trafo = (VTTrafo*) lisTrafosSEsFilhas->Items[i];
		InsereOrdenado(lisaux, String(Round(trafo->pri.vnom,2)));
		InsereOrdenado(lisaux, String(Round(trafo->sec.vnom,2)));
	}

	// Compõe string de tensões de base
	for(int i=0; i<lisaux->Count; i++)
	{
		String valor = lisaux->Strings[i];
		tensoesBase += valor;
		if(i < lisaux->Count-1) tensoesBase += " ";
	}

   for(int i=lisaux->Count-1; i>=0; i--) lisaux->Delete(i);
	delete lisaux;
	return(tensoesBase);
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::VoltageBases(VTTrafo* trafoSE, TList* lisTrafosSEsFilhas)
{
	String tensoesBase = "";
	TStringList* lisaux = new TStringList();

	if(trafoSE == NULL && lisTrafosSEsFilhas == NULL) return "";

	InsereOrdenado(lisaux, String(Round(trafoSE->pri.vnom,2)));
	InsereOrdenado(lisaux, String(Round(trafoSE->sec.vnom,2)));

	for(int i=0; i<lisTrafosSEsFilhas->Count; i++)
	{
		VTTrafo* trafo = (VTTrafo*) lisTrafosSEsFilhas->Items[i];
		InsereOrdenado(lisaux, String(Round(trafo->pri.vnom,2)));
		InsereOrdenado(lisaux, String(Round(trafo->sec.vnom,2)));
	}

	// Compõe string de tensões de base
	for(int i=0; i<lisaux->Count; i++)
	{
		String valor = lisaux->Strings[i];
		tensoesBase += valor;
		if(i < lisaux->Count-1) tensoesBase += " ";
	}

   for(int i=lisaux->Count-1; i>=0; i--) lisaux->Delete(i);
	delete lisaux;
	return(tensoesBase);
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::InsereOrdenado(TStringList* lista, String valor)
{
	if(lista == NULL || valor == "") return;

	if(lista->IndexOf(valor) >= 0)
		return;

	if(lista->Count == 0)
	{
		lista->Add(valor);
	}
	else if(lista->Count == 1)
	{
		if(valor.ToDouble() < lista->Strings[0].ToDouble())
		{
			lista->Add(valor);
		}
		else if(valor.ToDouble() > lista->Strings[0].ToDouble())
		{
			lista->Insert(0, valor);
		}
	}
	else
	{
		for(int i=0; i<lista->Count; i++)
		{
			if(i == 0 && valor.ToDouble() > lista->Strings[0].ToDouble())
			{
				lista->Insert(0, valor);
				break;
			}
			else if(i > 0 && i < lista->Count-1)
			{
				if(valor.ToDouble() < lista->Strings[i-1].ToDouble() && valor.ToDouble() > lista->Strings[i].ToDouble())
				{
					lista->Insert(i, valor);
					break;
				}
			}
			else if(i == lista->Count-1)
			{
				if(valor.ToDouble() < lista->Strings[i-1].ToDouble() && valor.ToDouble() > lista->Strings[i].ToDouble())
				{
					lista->Insert(i, valor);
				}
				else if(valor.ToDouble() < lista->Strings[i].ToDouble())
				{
					lista->Add(valor);
				}
         }
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::ExportaTrafos()
{
	String linhaDSS = "";
	TStringList* linhasDSS = new TStringList();

	if(lisTrafosSEsFilhas->Count == 0) return;

	for(int i=0; i<lisTrafosSEsFilhas->Count; i++)
	{
		VTTrafo* trafo = (VTTrafo*) lisTrafosSEsFilhas->Items[i];

		// Parâmetros do trafo
		double kvaTRAFO = trafo->snom * 1000.;
		strIMP z1 = trafo->z1;
		double rtrafo_perc = (100.) * z1.r / 2.;
		double kv1 = trafo->pri.vnom;
		double kv2 = trafo->sec.vnom;
		VTBarra* barraPRI = trafo->BarraPrimario();
		VTBarra* barraSEC = trafo->BarraSecundario();


		// Insere os dados do transformador
		linhaDSS = "New Transformer." + String(trafo->Codigo) + " Windings=2 Phases=3 Buses=(" + String(barraPRI->Id) + ", " + String(barraSEC->Id) + ")";
		linhasDSS->Add(linhaDSS);
		linhaDSS = "~ Conns=(delta wye)";
		linhasDSS->Add(linhaDSS);
		linhaDSS = "~ kvs=(" + DoubleToString(kv1, 1) + ", " + DoubleToString(kv2, 1) + ") kvas=(" + String(kvaTRAFO) + ", " + String(kvaTRAFO) + ")";
		linhasDSS->Add(linhaDSS);
		linhaDSS = "~ xhl=" + DoubleToString(100. * z1.x, 2);
		linhasDSS->Add(linhaDSS);
		linhaDSS = "~ wdg=1 %r=" + DoubleToString(rtrafo_perc, 3);
		linhasDSS->Add(linhaDSS);
		linhaDSS = "~ wdg=2 %r=" + DoubleToString(rtrafo_perc, 3);
		linhasDSS->Add(linhaDSS);
		linhasDSS->Add("");
	}

	// Exporta arquivo trafos.dss
	linhasDSS->SaveToFile(pathSaida + "\\trafos.dss");

   for(int i=linhasDSS->Count-1; i>=0; i--) linhasDSS->Delete(i);
	delete linhasDSS;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::TrafosSEsFilhas(TList* lisEXT)
{
	TList*   lisTrafos;
	VTBarra *barra0, *barra1;
	VTTrafo* trafo;

	if(lisEXT == NULL) return;

	lisTrafos = new TList();
	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redeETD || rede == redeSE) continue;

		lisTrafos->Clear();
		rede->LisEqpto(lisTrafos, eqptoTRAFO);

		for(int j=0; j<lisTrafos->Count; j++)
		{
			trafo = (VTTrafo*) lisTrafos->Items[j];

			barra0 = trafo->Barra(0);
			barra1 = trafo->Barra(1);

			if(posicaoSuprimento == "EqptoRef")
			{
				if(redeMT->ExisteBarra(barra0) || redeMT->ExisteBarra(barra1))
				{
					if(lisBarrasConsideradas->IndexOf(barra0) >= 0 || lisBarrasConsideradas->IndexOf(barra1) >= 0)
					{
						lisEXT->Add(trafo);
					}
				}
			}
			else
			{
				if(redeMT->ExisteBarra(barra0) || redeMT->ExisteBarra(barra1))
				{
					lisEXT->Add(trafo);
				}
			}
      }
	}
	delete lisTrafos;
}
//---------------------------------------------------------------------------
VTTrecho* __fastcall TConversorSinapDSS::FindTrechoPai(VTCarga* carga)
{
	TList *lisLiga, *lisEqbar;
	VTBarra *barra1, *barra2;
   VTLigacao* liga;

	//Verificações
   if(carga == NULL) return NULL;

   // Procura a ligação que tenha uma barra contendo a carga procurada
   lisLiga = redeMT->LisLigacao();
   String codigoLiga;
   for(int i=0; i<lisLiga->Count; i++)
   {
		liga = (VTLigacao*) lisLiga->Items[i];
      codigoLiga = liga->Codigo;
      barra2 = liga->pbarra2;

      lisEqbar = barra2->LisEqbar();
      if(lisEqbar != NULL && lisEqbar->IndexOf(carga) >= 0)
			break;
      else
         liga = NULL;
   }

   // Se ainda não encontrou a carga, procura nas barras 1
   if(liga == NULL)
   {
      for(int i=0; i<lisLiga->Count; i++)
      {
         liga = (VTLigacao*) lisLiga->Items[i];
         codigoLiga = liga->Codigo;
         barra1 = liga->pbarra1;

         lisEqbar = barra1->LisEqbar();
         if(lisEqbar != NULL && lisEqbar->IndexOf(carga) >= 0)
            break;
         else
            liga = NULL;
      }
   }

   // Verificação
   if(liga == NULL) return NULL;

   // Pega a ligação pai que seja do tipo TRECHO
   while(liga->Tipo() != eqptoTRECHO)
   {
      liga = liga->ligaPai;
      if(liga == NULL) return NULL;
   }

   return ((VTTrecho*) liga);
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::GetConnCarga(int enumFases)
{
	String resp;

	switch(enumFases)
   {
   case faseA:
   case faseB:
   case faseC:
   case faseAN:
   case faseBN:
   case faseCN:
   case faseAT:
   case faseBT:
   case faseCT:
   	resp = "Wye";
      break;

   case faseAB:
   case faseBC:
   case faseCA:
   case faseABN:
   case faseBCN:
   case faseCAN:
   case faseABT:
   case faseBCT:
   case faseCAT:
		resp = "Delta";
      break;

   case faseABC:
   case faseABCN:
   case faseABCT:
   	resp = "Delta";
      break;

   default:
   	resp = "Wye";
      break;
   }

   return resp;
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::GetIDbus(String linha, int idBus)
{
	int comp, pos = -1, cont1, cont2;
   String substr, idBus_str;

   comp = linha.Length();
	for(int i=1; i<=comp-4; i++)
   {
		substr = linha.SubString(i,4);
		if((idBus == 1 && substr != "Bus1") || (idBus == 2 && substr != "Bus2"))
      	continue;

		pos = i+5;
      break;
   }

   // Verificação
   if(pos == -1) return "";

   // Obtém o ID da barra

   // Procura pelo "."
   cont1 = 0;
   for(int i=pos; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
		if(substr != ".")
      {
      	cont1 += 1;
	      continue;
      }

      break;
   }
   // Procura pelo " "
   cont2 = 0;
   for(int i=pos; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
		if(substr != " ")
      {
      	cont2 += 1;
	      continue;
      }

      break;
   }

   if(cont1 < cont2) // linha tem o faseamento .1, .2, .3, .1.2, ...
   {
		idBus_str = linha.SubString(pos, cont1);
   }
   else if(cont1 > cont2) // linha não tem o faseamento .1, .2, .3, .1.2, ...
   {
      idBus_str = linha.SubString(pos, cont2);
   }
	else
   	return "";

   return idBus_str;
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::GetIndiceFases(int enumFases)
{
	String indiceFases;

	switch(enumFases)
   {
   case faseA:
   case faseAN:
   case faseAT:
   case faseANT:
   	indiceFases = ".1";
		break;

   case faseB:
   case faseBN:
   case faseBT:
   case faseBNT:
   	indiceFases = ".2";
		break;

   case faseC:
   case faseCN:
   case faseCT:
   case faseCNT:
   	indiceFases = ".3";
		break;

   case faseAB:
   case faseABN:
   case faseABT:
   case faseABNT:
		indiceFases = ".1.2";
      break;

   case faseBC:
   case faseBCN:
   case faseBCT:
   case faseBCNT:
		indiceFases = ".2.3";
      break;

   case faseCA:
   case faseCAN:
   case faseCAT:
   case faseCANT:
		indiceFases = ".1.3";
      break;

   case faseABC:
   case faseABCN:
   case faseABCT:
   case faseABCNT:
		indiceFases = ".1.2.3";
      break;

   default:
   	break;
   }

   return indiceFases;
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::GetIndiceFases(String linhaTrecho)
{
	int comp, pos = -1, cont;
   String substr, indiceFases;

   comp = linhaTrecho.Length();
	for(int i=1; i<=comp-4; i++)
   {
		substr = linhaTrecho.SubString(i,4);
		if(substr != "Bus1")
      	continue;

		pos = i;
      break;
   }

   // Verificação
   if(pos == -1) return "";

   // Obtém o ID da barra
   cont = 0;
   for(int i=pos; i<=comp; i++)
   {
		substr = linhaTrecho.SubString(i,1);
		if(substr != ".")
      {
      	cont += 1;
	      continue;
      }

      pos += cont;

      break;
   }

   // Verificação
   if(pos == -1) return "";

   // Obtém os índices de fase do trecho
   cont = 0;
   for(int i=pos; i<=comp; i++)
   {
		substr = linhaTrecho.SubString(i,1);
		if(substr != " ")
      {
      	cont += 1;
	      continue;
      }

		indiceFases = linhaTrecho.SubString(pos, cont);
      break;
   }

   return indiceFases;
}
//---------------------------------------------------------------------------
int __fastcall TConversorSinapDSS::GetNumPhasesCarga(int enumFases)
{
	int numFases;

	switch(enumFases)
   {
   case faseA:
   case faseB:
   case faseC:
   case faseAN:
   case faseBN:
   case faseCN:
   case faseAT:
   case faseANT:
   case faseBNT:
   case faseCNT:
   case faseAB:
   case faseBC:
   case faseCA:
   case faseABN:
   case faseBCN:
   case faseCAN:
   case faseABT:
   case faseBCT:
   case faseCAT:
   case faseABNT:
   case faseBCNT:
   case faseCANT:
   	numFases = 1;
      break;

   case faseABC:
   case faseABCN:
   case faseABCT:
   case faseABCNT:
   	numFases = 3;
      break;

   case faseINV: //fase inválida
   	numFases = 3;

   default:
   	numFases = 3;
      break;
   }

   return numFases;
}
//---------------------------------------------------------------------------
int __fastcall TConversorSinapDSS::GetNumFases(int enumFases)
{
	int numFases;

	switch(enumFases)
   {
   case faseA:
   case faseB:
   case faseC:
   case faseAN:
   case faseBN:
   case faseCN:
   case faseANT:
   case faseBNT:
   case faseCNT:
   	numFases = 1;
      break;

   case faseAB:
   case faseBC:
   case faseCA:
   case faseABN:
   case faseBCN:
   case faseCAN:
   case faseABNT:
   case faseBCNT:
   case faseCANT:
   	numFases = 2;
      break;

   case faseABC:
   case faseABCN:
   case faseABCT:
   case faseABCNT:
   	numFases = 3;
      break;

   case faseINV: //fase inválida
   	numFases = 3;

   default:
   	numFases = 3;
      break;
   }

   return numFases;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TConversorSinapDSS::GetRedeMT()
{
	return redeMT;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TConversorSinapDSS::GetRedeSE()
{
	return redeSE;
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::Inicializa(String pathSaida, VTRede* redeMT, VTRede* redeSE)
{
	this->redeMT = redeMT;
	this->redeSE = redeSE;
	this->pathSaida = pathSaida;

   // Cria diretório para o evento
	CreateDir(pathSaida);
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::InsereIndicesFases(TStringList* linhasDSSChaves, int indiceLinha, String indiceFases)
{
	int numFases;
	int comp, cont, pos;
   String substr, linhaOri, linhaFinal;

	// Verificações
	if(linhasDSSChaves == NULL || indiceLinha < 0) return;

   // Pega a linha que deve ser alterada
	linhaOri = linhasDSSChaves->Strings[indiceLinha];

   if((indiceFases == ".1") || (indiceFases == ".2") || (indiceFases == ".3"))
   {
   	numFases = 1;
   }
   else if((indiceFases == ".1.2") || (indiceFases == ".2.3") || (indiceFases == ".1.3") || (indiceFases == ".3.1"))
   {
   	numFases = 2;
   }
   else if(indiceFases == ".1.2.3")
   {
   	numFases = 3;
   }
   else
   {
      numFases = 0;
   }


   // :::::::::::::::::::::::::::::::::::::::::::::::::::
   // Insere os índices de fases nas posições adequadas, gerando nova string
   // :::::::::::::::::::::::::::::::::::::::::::::::::::

	linhaFinal = "";
   comp = linhaOri.Length();
   cont = 0;
   for(int i=1; i<=comp-5; i++)
   {
		substr = linhaOri.SubString(i,5);
      if(substr != "Bus1=")
      {
         cont += 1;
         continue;
      }

		pos = i;
      break;
   }

   // Insere Phases=... (número de fases)
   if(numFases > 0)
	   linhaFinal = linhaOri.SubString(1, pos-1) + "Phases=" + String(numFases);
   else
      linhaFinal = linhaOri.SubString(1, pos-1);


   // pos = posição do "B" em Bus1=...

   cont = 0;
   for(int i=pos; i<=comp; i++)
   {
		substr = linhaOri.SubString(i,1);
      if(substr != " ")
      {
         cont += 1;
         continue;
      }

      break;
   }

   // Insere a informação de fases
	linhaFinal += " " + linhaOri.SubString(pos, cont) + indiceFases;

   // pos = posição do "B" em Bus2=...
   pos += cont + 1;

   // Insere as posições até o final de Bus2
   cont = 0;
   for(int i=pos; i<=comp; i++)
   {
		substr = linhaOri.SubString(i,1);
      if(substr != " ")
      {
         cont += 1;
         continue;
      }

      break;
   }

   // Insere os índices de fase para o Bus2
   linhaFinal += " " + linhaOri.SubString(pos, cont) + indiceFases;

   // Atualiza o índice de posição "pos"
   pos += cont;

   // Insere o restante da linha original
   linhaFinal += linhaOri.SubString(pos, linhaOri.Length()-pos+1);

	// Remove a string inicial e insere a string final na lista
   linhasDSSChaves->Delete(indiceLinha);
   linhasDSSChaves->Insert(indiceLinha, linhaFinal);
}
//---------------------------------------------------------------------------
void __fastcall TConversorSinapDSS::InsereInfoFasesChaves(TStringList* linhasDSStrechos, TStringList* linhasDSSChaves)
{
	String linhaChave, idBus1, idBus2, indiceFases;

	// Verificação
	if(linhasDSStrechos == NULL || linhasDSSChaves == NULL) return;

   // Para cada linha de chave, insere as fases correspondentes (.1, .2, .3, .1.2, ...)
	for(int i=0; i<linhasDSSChaves->Count; i++)
   {
		linhaChave = linhasDSSChaves->Strings[i];

		// Pega os IDs das barras da chave
      idBus1 = GetIDbus(linhaChave, 1);
      idBus2 = GetIDbus(linhaChave, 2);

      // Procura trechos que tenham pelo menos uma barra compartilhada com a chave
      // e pega as fases desse trecho
      indiceFases = ProcuraFasesTrecho(linhasDSStrechos, idBus1, idBus2);
      // Edita a linha da chave, inserindo a informação de fases
      InsereIndicesFases(linhasDSSChaves, i, indiceFases);
   }
}
//---------------------------------------------------------------------------
int __fastcall TConversorSinapDSS::InsereInfoFasesTrecho(String indiceFases)
{
	if(indiceFases == "") return 3;
	if(indiceFases == ".1.2.3") return 3;
	if(indiceFases == ".1.2" || indiceFases == ".2.3" || indiceFases == ".1.3" || indiceFases == ".3.1") return 2;
	if(indiceFases == ".1" || indiceFases == ".2" || indiceFases == ".3") return 1;
	return 3;
}
//---------------------------------------------------------------------------
String __fastcall TConversorSinapDSS::ProcuraFasesTrecho(TStringList* linhasDSStrechos, String idBus1, String idBus2)
{
	int FaseTrecho, numFases = 0;
	String linha, idBus1_trecho, idBus2_trecho, indiceFases = 0;
	TList* lisLiga;
   VTLigacao* liga;
   VTTrecho* trecho;

	lisLiga = redeMT->LisLigacao();
	for(int i=0; i<lisLiga->Count; i++)
	{
		liga = (VTLigacao*) lisLiga->Items[i];
		if(liga->Tipo() != eqptoTRECHO) continue;
		trecho = (VTTrecho*) liga;

		if((idBus1 == String(liga->Barra(0)->Id)) || (idBus1 == String(liga->Barra(1)->Id)) ||
			 (idBus2 == String(liga->Barra(0)->Id)) || (idBus2 == String(liga->Barra(1)->Id)))
		{
			if(GetNumFases(trecho->arranjo->Fases) > numFases)
			{
				numFases = GetNumFases(trecho->arranjo->Fases);
				indiceFases = GetIndiceFases(trecho->arranjo->Fases);
			}
		}
	}

	// Se ainda não achou, procura nas redes filhas
	if(indiceFases == 0)
	{
		for(int i=0; i<lisRedesFilhas->Count; i++)
		{
			VTRede* rede = (VTRede*) lisRedesFilhas->Items[i];

			lisLiga = rede->LisLigacao();
			for(int i=0; i<lisLiga->Count; i++)
			{
				liga = (VTLigacao*) lisLiga->Items[i];
				if(liga->Tipo() != eqptoTRECHO) continue;
				trecho = (VTTrecho*) liga;

				if((idBus1 == String(liga->Barra(0)->Id)) || (idBus1 == String(liga->Barra(1)->Id)) ||
					 (idBus2 == String(liga->Barra(0)->Id)) || (idBus2 == String(liga->Barra(1)->Id)))
				{
					if(GetNumFases(trecho->arranjo->Fases) > numFases)
					{
						numFases = GetNumFases(trecho->arranjo->Fases);
						indiceFases = GetIndiceFases(trecho->arranjo->Fases);
					}
				}
			}

      }
   }

   return indiceFases;
}
//---------------------------------------------------------------------------
//eof
