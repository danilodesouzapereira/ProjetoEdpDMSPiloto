// ---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TEstrategiaEvolutiva.h"
#include "Forms\TFormProgressBar.h"
#include "TIndividuoEE.h"
#include "TSorteio.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TConfiguracoes.h"
#include "..\Auxiliares\TDados.h"
#include "..\Auxiliares\TFuncoesDeRede.h"
#include "..\Auxiliares\TLog.h"
#include "..\DSS\TTryFault.h"
#include "..\TAreaBusca.h"
#include "..\TClassificacao.h"
#include "..\TCluster.h"
#include "..\TFormFaultLocation.h"
#include "..\TThreadFaultLocation.h"
// ---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Radial.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Radial\VTRadial.h>
#include <PlataformaSinap\Fontes\Radial\VTPrimario.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
// ---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
/**
 * Construtor 1
 */
__fastcall TEstrategiaEvolutiva::TEstrategiaEvolutiva() {
	// Inicializa par�metros
	this->areaBusca = NULL;
	this->classificacao = NULL;
	this->config = NULL;
	this->dados = NULL;
	this->formFL = NULL;
	ultimoID = 0;

	// Inicializa objetos
	sorteio = new TSorteio(); // < sorteio de n�meros
	log = new TLog(); // < logs para debug
	tryFault = new TTryFault();
	// < objeto para testes de defeitos utilizando o OpenDSS

	// Cria listas
	lisIndividuos = new TList();
	lisNovosIndividuosMutacao = new TList();
	lisNovosIndividuosCruzamento = new TList();
	lisMelhoresIndividuosClusteres = new TList();
}

// ---------------------------------------------------------------------------
/**
 * Construtor 2
 */
__fastcall TEstrategiaEvolutiva::TEstrategiaEvolutiva(TConfiguracoes* config,
	 TDados* dados) {
	// Inicializa par�metros
	this->areaBusca = NULL;
	this->classificacao = NULL;
	this->config = config;
	this->dados = dados;
	this->formFL = NULL;
	ultimoID = 0;

	// Inicializa objetos
	sorteio = new TSorteio(); // < sorteio de n�meros
	log = new TLog(); // < logs para debug
	tryFault = new TTryFault();
	// < objeto para testes de defeitos utilizando o OpenDSS

	// Cria listas
	lisIndividuos = new TList();
	lisNovosIndividuosMutacao = new TList();
	lisNovosIndividuosCruzamento = new TList();
	lisMelhoresIndividuosClusteres = new TList();
}

// ---------------------------------------------------------------------------
__fastcall TEstrategiaEvolutiva::~TEstrategiaEvolutiva(void) {
	// Destroi objetos
	if (lisIndividuos) {
		delete lisIndividuos;
		lisIndividuos = NULL;
	}
	if (lisNovosIndividuosMutacao) {
		delete lisNovosIndividuosMutacao;
		lisNovosIndividuosMutacao = NULL;
	}
	if (lisNovosIndividuosCruzamento) {
		delete lisNovosIndividuosCruzamento;
		lisNovosIndividuosCruzamento = NULL;
	}
	if (lisMelhoresIndividuosClusteres) {
		delete lisMelhoresIndividuosClusteres;
		lisMelhoresIndividuosClusteres = NULL;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::AtribuiIDsIndividuos() {
	TIndividuoEE* indiv;

	for (int i = 0; i < lisIndividuos->Count; i++) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];
		if (indiv->ID > -1)
			continue;

		ultimoID += 1;
		indiv->ID = ultimoID;
	}
}

// ---------------------------------------------------------------------------
/***
 * Percorre a lista de indiv�duos. Para cada indiv�duo, testa o curto que ele representa,
 * obt�m os valores calculados, compara-os com os valores medidos e obt�m o desvio/erro
 * entre eles. Por fim, chama SetAvaliacao para gerar a nota de avalia��o do indiv�duo
 */
void __fastcall TEstrategiaEvolutiva::AvaliaIndividuos(TList* lisIndiv,
	 bool Inicial) {
	double erroX, erroRf, indiceErro, nota;
	int patamar;
	String faseDefeito;
	TIndividuoEE* indiv;

	// Verifica��o
	if (lisIndiv == NULL)
		return;

	// Informa��es do evento
	patamar = dados->GetPatamarHorario();
	faseDefeito = classificacao->GetStrTipoFalta();

	// ::::::::::::::::
	// Calcula o �ndice de erro, compara os valores de lisCalcV e de lisCalcI com
	// as medi��es de tens�o e corrente de TDados.
	// ::::::::::::::::

	// //debug
	// String linha;
	TStringList* lisaux = new TStringList();
	unsigned short hora, minuto, segundo, mseg;
	TTime t1;
	TTime t2;
	TTime delta;
	double deltaT;

	for (int i = 0; i < lisIndiv->Count; i++) {
		indiv = (TIndividuoEE*) lisIndiv->Items[i];
		// Se indiceErro > -1, o indiv�duo j� foi avaliado
		if (indiv->GetIndiceErro() > -1)
			continue;

		// debug
		t1 = Time();

		// Usa obj da classe TTryFault para testar a falta representada pelo indiv�duo
		tryFault->TestaCurtoRf(patamar, indiv->GetIDbarra1(), faseDefeito,
			 indiv->GetRf(), Inicial && i == 0);

		if (i == 0) {
			// debug: tempo
			t2 = Time();
			delta = t2 - t1;
			delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
			deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
			lisaux->Add("Testa curto Rf: " + String(deltaT) + " - " +
				 String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" +
				 String(mseg));
		}

		// Calcula erro associado ao teste
		indiceErro = tryFault->CalculaErro();

		if (i == 0) {
			// debug: tempo
			t1 = t2;
			t2 = Time();
			delta = t2 - t1;
			delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
			deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
			lisaux->Add("Calcula Erro: " + String(deltaT) + " - " + String(hora) +
				 ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));
		}

		// Insere �ndice de erro para que seja gerada a nota de avalia��o
		indiv->SetAvaliacao(indiceErro);

		if (i == 0) {
			// debug: tempo
			t1 = t2;
			t2 = Time();
			delta = t2 - t1;
			delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
			deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
			lisaux->Add("Set Avalia��o: " + String(deltaT) + " - " + String(hora) +
				 ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));
			// //debug
			// lisaux->SaveToFile("c:\\users\\user\\desktop\\tempos.txt");
			// delete lisaux;
		}

		// //debug:
		// linha = "X=;" + String(indiv->GetX()) + ";Rf=;" + String(indiv->GetRf()) + ";IDBarra1=;" + String(indiv->GetIDbarra1()) + ";IndiceErro=;" + String(indiceErro) + ";fAval=;" + String(indiv->GetAvaliacao());
		// lisaux->Add(linha);
	}

	// debug: tempo
	t1 = Time();

	RemoveIndivImproprios();

	// //debug: tempo
	// t2 = Time();
	// delta = t2 - t1;
	// delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
	// deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
	// lisaux->Add("Remove indiv improprios: " + String(deltaT) + " - " + String(hora) + ":" + String(minuto) + ":" + String(segundo) + ":" + String(mseg));
	// //debug
	// lisaux->SaveToFile("c:\\users\\user\\desktop\\tempos.txt");
	// delete lisaux;

	// //debug: erros
	// lisaux->SaveToFile("c:\\users\\user\\desktop\\ResAval.csv");
	// delete lisaux;

	// //debug: tempo
	// TTime t2 = Time();
	// TTime delta = t2 - t1;
	// delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
	// double deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::Executa() {
	TCluster* cluster;

	// Inicializa��es
	strTipoFalta = classificacao->GetStrTipoFalta();
	areaBusca->DefineClusteres();

	// Progresso
	FormPG = formFL->CriaFormProgressBar();

	// // Executa um AE para cada cluster da �rea de busca
	// for(int i=0; i<areaBusca->GetLisClusteres()->Count; i++)
	// {
	// cluster = (TCluster*) areaBusca->GetLisClusteres()->Items[i];
	// areaBusca->DiscretizaAreaBusca(cluster->GetBlocos());
	//
	// GeraIndividuosIniciais();
	// ExecutaEvolutivo();
	//
	// // Salva melhor indiv. do cluster e limpa lista de indiv�duos
	// SalvaMelhorIndividuoCluster();
	// }

	cluster = (TCluster*) areaBusca->GetLisClusteres()->Items[0];
	areaBusca->DiscretizaAreaBusca_EE(cluster->chaveMontante, cluster->GetBlocos());

	// //debug
	// TStringList* lisCodTrechosAreaBusca = new TStringList();
	// for(int i=0; i<cluster->GetBlocos()->Count; i++)
	// {
	// VTBloco* blocoAreaBusca = (VTBloco*) cluster->GetBlocos()->Items[i];
	// if(!blocoAreaBusca) continue;
	// TList* lisLigaAreaBusca = blocoAreaBusca->LisLigacao();
	// if(!lisLigaAreaBusca) continue;
	//
	// for(int j=0; j<lisLigaAreaBusca->Count; j++)
	// {
	// VTLigacao* liga = (VTLigacao*) lisLigaAreaBusca->Items[j];
	// if(liga->Tipo() != eqptoTRECHO) continue;
	// lisCodTrechosAreaBusca->Add(liga->Codigo);
	// }
	// }
	// lisCodTrechosAreaBusca->SaveToFile("c:\\users\\usrsnp\\desktop\\codTrechos.txt");
	// delete lisCodTrechosAreaBusca; lisCodTrechosAreaBusca = NULL;
	// // fim debug

	GeraIndividuosIniciais();
	ExecutaEvolutivo();

	// Salva melhor indiv. do cluster e limpa lista de indiv�duos
	SalvaMelhorIndividuoCluster();

	delete FormPG;
	FormPG = NULL;
}

// ---------------------------------------------------------------------------
// #3 - Execu��o da Estrat�gia Evolutiva
void __fastcall TEstrategiaEvolutiva::ExecutaEvolutivo() {
	double pc, pm;
	int numMaxGeracoes;
	TIndividuoEE *indiv, *indiv1, *indiv2, *filhoCruzamento;
	TList* filhosMutacao;

	// //debug
	// double delta_fAval = 100.;

	// Obt�m alguns par�metros de configura��o
	pc = config->GetPc();
	pm = config->GetPm();

	// debug: Imprime log da gera��o inicial
	if (config->GetMostrarLogDebug()) {
		log->SetCaminho("c:\\users\\usrsnp\\desktop\\Geracoes.txt");
		log->AddLinha("PopIni");
		for (int i = 0; i < lisIndividuos->Count; i++) {
			indiv = (TIndividuoEE*) lisIndividuos->Items[i];
			log->AddLinha("Indiv#" + String(i + 1) + ";X=" +
				 String(Round(indiv->GetX(), 3)) + ";Rf=" +
				 String(Round(indiv->GetRf(), 3)));
		}
		log->AddLinha(" ");
		// debug
		log->ImprimeLog();
	}

	// unsigned short hora, minuto, segundo, mseg;
	// TTime t1 = Time();

	// Faz avalia��o dos novos indiv�duos gerados pelos operadores (muta��o e cruzamento)
	bool ChamadaInicial = true;
	AvaliaIndividuos(lisIndividuos, ChamadaInicial);

	// //debug: tempo
	// TTime t2 = Time();
	// TTime delta = t2 - t1;
	// delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
	// double deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
	// lisTempos->Add("Avalia��o dos indiv. iniciais. Individuos: " + String(lisIndividuos->Count) + ": " + String(deltaT));

	// Configura o form de progress bar
	FormPG->InicializaProgressBarGeracoes(config->GetNumMaxGeracoes());

	numMaxGeracoes = config->GetNumMaxGeracoes();
	for (int i = 0; i < numMaxGeracoes; i++) {
		if (config->GetMostrarLogDebug()) {
			// debug: Imprime log da gera��o
			log->AddLinha("Ger#" + String(i + 1));
			log->AddLinha("Pais");
			for (int i = 0; i < lisIndividuos->Count; i++) {
				indiv = (TIndividuoEE*) lisIndividuos->Items[i];
				String linha = "Indiv#" + String(i + 1) + ";ID=;" +
					 String(indiv->ID);
				linha += ";IDpaiMut=;" + String(indiv->IDpaiMutacao);
				linha += ";IDpaiCruz1=;" + String(indiv->IDpaisCruzamento[0]);
				linha += ";IDpaiCruz2=;" + String(indiv->IDpaisCruzamento[1]);
				linha += ";X=;" + String(Round(indiv->GetX(), 3)) + ";Rf=;" +
					 String(Round(indiv->GetRf(), 3));
				linha += ";IndiceErro=;" + String(Round(indiv->GetIndiceErro(), 5));
				linha += ";fAvaliacao=;" + String(Round(indiv->GetAvaliacao(), 4));
				log->AddLinha(linha);
			}
		}

		// ::::::::::::::::
		// Aplica��o do operador MUTA��O
		// ::::::::::::::::
		lisNovosIndividuosMutacao->Clear();
		for (int j = 0; j < lisIndividuos->Count; j++) {
			indiv = (TIndividuoEE*) lisIndividuos->Items[j];
			filhosMutacao = indiv->Mutacao(pm);
			VerificaConsistenciaFilhosMutacao(filhosMutacao);
		}
		AvaliaIndividuos(lisNovosIndividuosMutacao);

		// debug: Adiciona os filhos ao log
		if (config->GetMostrarLogDebug()) {
			log->AddLinha(" ");
			log->AddLinha("FilhosMut");
			for (int i = 0; i < lisNovosIndividuosMutacao->Count; i++) {
				indiv = (TIndividuoEE*) lisNovosIndividuosMutacao->Items[i];
				log->AddLinha("Indiv#" + String(i + 1) + ";X=;" +
					 String(indiv->GetX()) + ";Rf=;" + String(indiv->GetRf()) +
					 ";IndiceErro=;" + String(indiv->GetIndiceErro()));
			}
			log->AddLinha(" ");
		}

		// ::::::::::::::::
		// Aplica��o do operador CRUZAMENTO
		// ::::::::::::::::
		lisNovosIndividuosCruzamento->Clear();
		for (int j = 0; j < lisIndividuos->Count - 1; j++) {
			indiv1 = (TIndividuoEE*) lisIndividuos->Items[j];

			for (int k = j + 1; k < lisIndividuos->Count; k++) {
				indiv2 = (TIndividuoEE*) lisIndividuos->Items[k];

				filhoCruzamento = indiv1->Cruzamento(indiv2, pc);
				VerificaConsistenciaFilhoCruzamento(filhoCruzamento);
			}
		}
		AvaliaIndividuos(lisNovosIndividuosCruzamento);

		// debug: Adiciona os filhos ao log
		if (config->GetMostrarLogDebug()) {
			log->AddLinha(" ");
			log->AddLinha("FilhosCruzamento");
			for (int i = 0; i < lisNovosIndividuosCruzamento->Count; i++) {
				indiv = (TIndividuoEE*) lisNovosIndividuosCruzamento->Items[i];
				log->AddLinha("Indiv#" + String(i + 1) + ";X=;" +
					 String(indiv->GetX()) + ";Rf=;" + String(indiv->GetRf()) +
					 ";IndiceErro=;" + String(indiv->GetIndiceErro()));
			}
			log->AddLinha(" ");
			log->AddLinha(" ");
			// debug
			log->ImprimeLog();
		}

		// ::::::::::::::::
		// Aplica��o do operador SELE��O
		// ::::::::::::::::
		SelecionaMelhoresIndividuos();

		// //debug
		// if(config->GetMostrarLogDebug())
		// {
		// String linha;
		// TStringList* lisaux = new TStringList();
		// for(int j=0; j<lisIndividuos->Count; j++)
		// {
		// indiv = (TIndividuoEE*) lisIndividuos->Items[j];
		// linha = "X=;" + String(indiv->GetX()) + ";Rf=;" + String(indiv->GetRf()) + ";IDBarra1=;" + String(indiv->GetIDbarra1()) + ";fAval=;" + String(indiv->GetAvaliacao());
		// lisaux->Add(linha);
		// }
		// //debug: erros
		// lisaux->SaveToFile("c:\\users\\usrsnp\\desktop\\ResAval" + String(i+1) + ".csv");
		// delete lisaux;
		// }

		// Atualiza progresso
		FormPG->StepItProgressBarGeracoes();

		// Verifica se a condi��o de parada foi satisfeita, antes que o n�mero m�ximo
		// de gera��es seja atingido
		if (VerificaCriterioParada())
			break;
	}

	// debug
	if (config->GetMostrarLogDebug()) {
		log->ImprimeLog();
	}
}

// ---------------------------------------------------------------------------
// #2 - Gera��o dos indiv�duos iniciais do AE
void __fastcall TEstrategiaEvolutiva::GeraIndividuosIniciais() {
	TIndividuoEE* indiv;
	double X, Rf, maxRfalta;
	double sigmaXini, sigmaRfini;
	int numIndivIni;
	int IDbarra1, IDbarra2;
	double xPorc;

	// Pega alguns par�metros de configura��es
	numIndivIni = config->GetNumIndivIni();
	maxRfalta = config->GetMaxRfalta();

	// debug
	TStringList* lisAux = new TStringList();
	String linha;

	for (int i = 0; i < numIndivIni; i++) {
		// Define os par�metros do indiv�duo
		X = sorteio->Uniforme(0., 100.);
		Rf = sorteio->Uniforme(0., maxRfalta);

		sigmaXini = config->GetSigmaXini();
		sigmaRfini = config->GetSigmaRfini();

		// De-para entre X e barra1, barra2 e xporc
		String resp = areaBusca->GetTrechoBarra1Barra2LocalDiscret(X);
		IDbarra1 = GetCampoCSV(resp, 1, ";").ToInt();
		IDbarra2 = GetCampoCSV(resp, 2, ";").ToInt();
		xPorc = GetCampoCSV(resp, 3, ";").ToDouble();

		// Cria objeto de indiv�duo a partir dos par�metros definidos acima
		indiv = new TIndividuoEE(this, X, Rf, sigmaXini, sigmaRfini);
		indiv->SetSorteio(sorteio);
		indiv->SetConfig(config);
		lisIndividuos->Add(indiv);

		// Cadastra um ID para o indiv�duo
		ultimoID += 1;
		indiv->ID = ultimoID;

		// debug
		linha = "X=;" + String(X) + ";Rf=;" + String(Rf) + ";IDbarra1=;" +
			 String(IDbarra1);
		linha += ";IDbarra2=;" + String(IDbarra2) + ";Xporc=;" + String(xPorc);
		lisAux->Add(linha);
	}

//	// debug
//	if (config->GetMostrarLogDebug()) {
//		lisAux->SaveToFile("c:\\users\\usrsnp\\desktop\\individuosIni.csv");
//		delete lisAux;
//		lisAux = NULL;
//	}
}

// ---------------------------------------------------------------------------
TAreaBusca* __fastcall TEstrategiaEvolutiva::GetAreaBusca() {
	return areaBusca;
}

// ---------------------------------------------------------------------------
// M�todo para gerar objetos (structs) de solu��es resultantes do processo de
// otimiza��o de Estrat�gia Evolutiva
void __fastcall TEstrategiaEvolutiva::GetSolucoes(TFuncoesDeRede* funcoesRede,
	 TList* lisEXT) {
	double distanciaChvMontante;
	int utmx, utmy;
	int numConsumidoresJusanteLigacao;
	VTBarra* barraSolucao;
	VTChave* chaveMontante;

	if (!lisEXT || !funcoesRede)
		return;

	lisEXT->Clear();

	// Ordena as solu��es da mais para a menos prov�vel (em fun��o da nota do indiv�duo)
	OrdenaMelhoresIndividuos();

	// Para cada melhor indiv�duo de um cluster, gera uma solu��o associada
	for (int i = 0; i < lisMelhoresIndividuosClusteres->Count; i++) {
		TIndividuoEE* indiv =
			 (TIndividuoEE*) lisMelhoresIndividuosClusteres->Items[i];

		barraSolucao = areaBusca->GetBarra(indiv->GetIDbarra1());
		chaveMontante = funcoesRede->GetChaveMontante(barraSolucao);
		distanciaChvMontante = funcoesRede->GetDistanciaMetros
			 (chaveMontante->Barra(1), barraSolucao);
		numConsumidoresJusanteLigacao =
			 funcoesRede->GetNumConsJusLigacao(chaveMontante->Codigo);
		barraSolucao->CoordenadasUtm_cm(utmx, utmy);

		// Ajusta coordenadas "fake" do Sinap
		double lat = double(utmy * 1e-7) - 100.;
		double lon = double(utmx * 1e-7) - 100.;

		strSolucao* solucao = new strSolucao();
		solucao->Probabilidade = i + 1;
		solucao->DefX = String(utmx);
		solucao->DefY = String(utmy);
		solucao->DefLat = String(lat);
		solucao->DefLon = String(lon);
		solucao->barraSolucao = barraSolucao;
		solucao->CodBarra = barraSolucao->Codigo;
		solucao->IdBarra = barraSolucao->Id;
		solucao->DefTipo = strTipoFalta;
		solucao->ChvMont = chaveMontante->Codigo;
		solucao->DistChvMont = String(Round(distanciaChvMontante, 2));
		solucao->DistRele = funcoesRede->GetDistancia_KM_DaSubestacao
			 (barraSolucao);
		solucao->ClientesDepoisChvMont = String(numConsumidoresJusanteLigacao);

		lisEXT->Add(solucao);
	}
}

// ---------------------------------------------------------------------------
// #1 - Inicializa��o do objeto de testes de faltas (TTryFault)
void __fastcall TEstrategiaEvolutiva::IniciaTryFault(String caminhoDSS) {
	// Prepara o TryFault
	tryFault->SetCaminhoDSS(caminhoDSS);
	tryFault->SetNivelCurtoMT();
	tryFault->IniciaBarrasTrechos();
	tryFault->SetMedicoesBarras(dados->GetMedicoesBarras());
	tryFault->SetMedicoesTrechos(dados->GetMedicoesTrechos_EE());
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::OrdenaMelhoresIndividuos() {
	bool ok = false;

	if (lisMelhoresIndividuosClusteres->Count == 1)
		return;

	while (!ok) {
		ok = true;
		for (int i = 1; i < lisMelhoresIndividuosClusteres->Count; i++) {
			TIndividuoEE* indiv1 =
				 (TIndividuoEE*) lisMelhoresIndividuosClusteres->Items[i - 1];
			TIndividuoEE* indiv2 =
				 (TIndividuoEE*) lisMelhoresIndividuosClusteres->Items[i];

			if (indiv2->GetAvaliacao() > indiv1->GetAvaliacao()) {
				lisMelhoresIndividuosClusteres->Remove(indiv2);
				lisMelhoresIndividuosClusteres->Insert(i - 1, indiv2);
				ok = false;
			}
		}
	}
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::RemoveIndivImproprios() {
	TIndividuoEE* indiv;

	for (int i = lisIndividuos->Count - 1; i >= 0; i--) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];

		// Se a nota de avalia��o do indiv�duo estiver fora do intervalo (0,100), remove
		if (indiv->GetAvaliacao() < 0. || indiv->GetAvaliacao() > 100.) {
			lisIndividuos->Remove(indiv);
		}
	}
}

// ---------------------------------------------------------------------------
// 1) Pega o primeiro elemento da lista ordenada (melhor indiv�duo) e o salva
// numa lista de melhores indiv�duos por cluster
// 2) Limpa lista de indiv�duos
void __fastcall TEstrategiaEvolutiva::SalvaMelhorIndividuoCluster() {
	TIndividuoEE* indiv = (TIndividuoEE*) lisIndividuos->Items[0];
	lisIndividuos->Remove(indiv);
	lisMelhoresIndividuosClusteres->Add(indiv);

	// Limpa lista de indiv�duos
	// LimpaTList(lisIndividuos);

	for (int i = 0; i < lisIndividuos->Count; i++) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];
		delete indiv;
	}
	lisIndividuos->Clear();
}

// ---------------------------------------------------------------------------
/***
 * Este m�todo considera:
 * 1) Lista de indiv�duos pais
 * 2) Lista de novos indiv�duos gerados por muta��o
 * 3) Lista de novos indiv�duos gerados por cruzamento
 *
 * Ent�o, atualiza a lista de indiv�duos pais apenas com os melhores indiv�duos.
 */
void __fastcall TEstrategiaEvolutiva::SelecionaMelhoresIndividuos() {
	bool isOK;
	int MaxIdade;
	TIndividuoEE *indiv_ant, *indiv;
	TList* lisTotal;

	// Obt�m a idade m�xima configurada para um indiv�duo
	MaxIdade = config->GetMaxIdade();

	// Lista para o conjunto total de indiv�duos
	lisTotal = new TList();
	for (int i = 0; i < lisIndividuos->Count; i++) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];
		lisTotal->Add(indiv);
	}
	for (int i = 0; i < lisNovosIndividuosMutacao->Count; i++) {
		indiv = (TIndividuoEE*) lisNovosIndividuosMutacao->Items[i];
		lisTotal->Add(indiv);
	}
	for (int i = 0; i < lisNovosIndividuosCruzamento->Count; i++) {
		indiv = (TIndividuoEE*) lisNovosIndividuosCruzamento->Items[i];
		lisTotal->Add(indiv);
	}

	// Ordena os indiv�duos do melhor para o pior, com base nas suas fun��es de avalia��o.
	isOK = false;
	while (!isOK) {
		for (int i = 1; i < lisTotal->Count; i++) {
			indiv_ant = (TIndividuoEE*) lisTotal->Items[i - 1];
			indiv = (TIndividuoEE*) lisTotal->Items[i];

			isOK = true;
			if (indiv->GetAvaliacao() > indiv_ant->GetAvaliacao()) {
				lisTotal->Delete(i);
				lisTotal->Insert(i - 1, indiv);
				isOK = false;
				break;
			}
		}
	}

	// ::::: OBS: Verificar a idade m�xima dos indiv�duos aqui

	// Limpa lista de indiv�duos pais e insere apenas os melhores (dentre pais e filhos)
	lisIndividuos->Clear();
	for (int i = 0; i < lisTotal->Count; i++) {
		indiv = (TIndividuoEE*) lisTotal->Items[i];

		// Se indiv�duo superar idade m�xima, � desconsiderado
		if (indiv->GetIdade() > MaxIdade)
			continue;

		// Adiciona os melhores indiv�duos at� a qtde m�xima de indiv�duos por gera��o
		if (lisIndividuos->Count < config->GetMaxIndivPorGeracao()) {
			indiv->IncrementaIdade();
			lisIndividuos->Add(indiv);
		}
	}

	// Limpa lista de todos os indiv�duos. Se o indiv�duo tiver sido
	// desconsiderado, � destru�do
	for (int i = lisTotal->Count - 1; i >= 0; i--) {
		indiv = (TIndividuoEE*) lisTotal->Items[i];
		lisTotal->Remove(indiv);
		if (lisIndividuos->IndexOf(indiv) < 0)
			delete indiv;
	}
	delete lisTotal;
	lisTotal = NULL;

	AtribuiIDsIndividuos();
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::SetAreaBusca(TAreaBusca* areaBusca) {
	this->areaBusca = areaBusca;
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::SetClassificacao
	 (TClassificacao* classificacao) {
	this->classificacao = classificacao;
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::SetConfiguracoes(TConfiguracoes* config) {
	this->config = config;
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::SetDados(TDados* dados) {
	this->dados = dados;
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::SetFormFL(TFormFaultLocation* formFL) {
	this->formFL = formFL;
}

// ---------------------------------------------------------------------------
void __fastcall TEstrategiaEvolutiva::SetParametros(TConfiguracoes* config,
	 TClassificacao* classificacao, TAreaBusca* areaBusca, TDados* dados,
	 TFormFaultLocation* formFL) {
	this->classificacao = classificacao;
	this->config = config;
	this->areaBusca = areaBusca;
	this->dados = dados;
	this->formFL = formFL;
	tryFault->SetConfig(config);
}

// ---------------------------------------------------------------------------
/***
 * Verifica se os par�metros do filho do cruzamento s�o consistentes para utiliza��o
 */
void __fastcall TEstrategiaEvolutiva::VerificaConsistenciaFilhoCruzamento
	 (TIndividuoEE* filho) {
	double X, Rf;

	// Verifica��o
	if (filho == NULL)
		return;

	X = filho->GetX();
	Rf = filho->GetRf();

	if (X < 0. || X > 100.)
		return;
	if (Rf < 0. || Rf > config->GetMaxRfalta())
		return;

	// Se os par�metros do filho forem adequados, adiciona � lista de novos indiv por muta��o
	lisNovosIndividuosCruzamento->Add(filho);
}

// ---------------------------------------------------------------------------
/***
 * Verifica se os par�metros dos filhos da muta��o s�o consistentes para utiliza��o
 */
void __fastcall TEstrategiaEvolutiva::VerificaConsistenciaFilhosMutacao
	 (TList* filhosMutacao) {
	double X, Rf;

	// Verifica��o
	if (filhosMutacao == NULL)
		return;

	for (int i = 0; i < filhosMutacao->Count; i++) {
		TIndividuoEE* filho = (TIndividuoEE*) filhosMutacao->Items[i];

		X = filho->GetX();
		Rf = filho->GetRf();

		if (X < 0. || X > 100.)
			continue;
		if (Rf < 0. || Rf > config->GetMaxRfalta())
			continue;

		// Se os par�metros do filho forem adequados, adiciona � lista de novos indiv por muta��o
		lisNovosIndividuosMutacao->Add(filho);
	}
}

// ---------------------------------------------------------------------------
/***
 * Com os indiv�duos da gera��o ordenados por ordem decrescente de fun��o de avalia��o,
 * compara a fun��o de avalia��o do melhor indiv�duo com a fun��o de avalia��o m�dia da gera��o.
 */
bool __fastcall TEstrategiaEvolutiva::VerificaCriterioParada() {
	double fAval_media, fAval_melhor, delta_fAval, min_delta_fAval;
	TIndividuoEE* indiv;

	// Pega o valor pr�-configurado de menor valor do delta de avalia��o
	min_delta_fAval = config->GetMinDeltaFAval();

	// Obt�m o valor m�dio das fun��es de avalia��o dos indiv�duos da gera��o corrente
	fAval_media = 0.;
	for (int i = 0; i < lisIndividuos->Count; i++) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];

		// Pega a fun��o de avalia��o do melhor indiv�duo
		if (i == 0)
			fAval_melhor = indiv->GetAvaliacao();

		// Incrementa fAval_media
		fAval_media += indiv->GetAvaliacao();
	}
	if (lisIndividuos->Count > 0)
		fAval_media /= lisIndividuos->Count;
	else
		return false;

	// Computa a diferen�a entre fAval do melhor indiv�duo e fAval m�dia da gera��o
	delta_fAval = fAval_melhor - fAval_media;

	// Verifica se o delta de fAval � menor que o m�nimo
	if (delta_fAval < min_delta_fAval) {
		return true;
	}

	return false;
}
// ---------------------------------------------------------------------------
