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
	// Inicializa parâmetros
	this->areaBusca = NULL;
	this->classificacao = NULL;
	this->config = NULL;
	this->dados = NULL;
	this->formFL = NULL;
	ultimoID = 0;

	// Inicializa objetos
	sorteio = new TSorteio(); // < sorteio de números
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
	// Inicializa parâmetros
	this->areaBusca = NULL;
	this->classificacao = NULL;
	this->config = config;
	this->dados = dados;
	this->formFL = NULL;
	ultimoID = 0;

	// Inicializa objetos
	sorteio = new TSorteio(); // < sorteio de números
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
 * Percorre a lista de indivíduos. Para cada indivíduo, testa o curto que ele representa,
 * obtém os valores calculados, compara-os com os valores medidos e obtém o desvio/erro
 * entre eles. Por fim, chama SetAvaliacao para gerar a nota de avaliação do indivíduo
 */
void __fastcall TEstrategiaEvolutiva::AvaliaIndividuos(TList* lisIndiv,
	 bool Inicial) {
	double erroX, erroRf, indiceErro, nota;
	int patamar;
	String faseDefeito;
	TIndividuoEE* indiv;

	// Verificação
	if (lisIndiv == NULL)
		return;

	// Informações do evento
	patamar = dados->GetPatamarHorario();
	faseDefeito = classificacao->GetStrTipoFalta();

	// ::::::::::::::::
	// Calcula o índice de erro, compara os valores de lisCalcV e de lisCalcI com
	// as medições de tensão e corrente de TDados.
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
		// Se indiceErro > -1, o indivíduo já foi avaliado
		if (indiv->GetIndiceErro() > -1)
			continue;

		// debug
		t1 = Time();

		// Usa obj da classe TTryFault para testar a falta representada pelo indivíduo
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

		// Insere índice de erro para que seja gerada a nota de avaliação
		indiv->SetAvaliacao(indiceErro);

		if (i == 0) {
			// debug: tempo
			t1 = t2;
			t2 = Time();
			delta = t2 - t1;
			delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
			deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
			lisaux->Add("Set Avaliação: " + String(deltaT) + " - " + String(hora) +
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

	// Inicializações
	strTipoFalta = classificacao->GetStrTipoFalta();
	areaBusca->DefineClusteres();

	// Progresso
	FormPG = formFL->CriaFormProgressBar();

	// // Executa um AE para cada cluster da área de busca
	// for(int i=0; i<areaBusca->GetLisClusteres()->Count; i++)
	// {
	// cluster = (TCluster*) areaBusca->GetLisClusteres()->Items[i];
	// areaBusca->DiscretizaAreaBusca(cluster->GetBlocos());
	//
	// GeraIndividuosIniciais();
	// ExecutaEvolutivo();
	//
	// // Salva melhor indiv. do cluster e limpa lista de indivíduos
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

	// Salva melhor indiv. do cluster e limpa lista de indivíduos
	SalvaMelhorIndividuoCluster();

	delete FormPG;
	FormPG = NULL;
}

// ---------------------------------------------------------------------------
// #3 - Execução da Estratégia Evolutiva
void __fastcall TEstrategiaEvolutiva::ExecutaEvolutivo() {
	double pc, pm;
	int numMaxGeracoes;
	TIndividuoEE *indiv, *indiv1, *indiv2, *filhoCruzamento;
	TList* filhosMutacao;

	// //debug
	// double delta_fAval = 100.;

	// Obtém alguns parâmetros de configuração
	pc = config->GetPc();
	pm = config->GetPm();

	// debug: Imprime log da geração inicial
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

	// Faz avaliação dos novos indivíduos gerados pelos operadores (mutação e cruzamento)
	bool ChamadaInicial = true;
	AvaliaIndividuos(lisIndividuos, ChamadaInicial);

	// //debug: tempo
	// TTime t2 = Time();
	// TTime delta = t2 - t1;
	// delta.DecodeTime(&hora, &minuto, &segundo, &mseg);
	// double deltaT = 3600. * hora + 60. * minuto + segundo + 0.001 * mseg;
	// lisTempos->Add("Avaliação dos indiv. iniciais. Individuos: " + String(lisIndividuos->Count) + ": " + String(deltaT));

	// Configura o form de progress bar
	FormPG->InicializaProgressBarGeracoes(config->GetNumMaxGeracoes());

	numMaxGeracoes = config->GetNumMaxGeracoes();
	for (int i = 0; i < numMaxGeracoes; i++) {
		if (config->GetMostrarLogDebug()) {
			// debug: Imprime log da geração
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
		// Aplicação do operador MUTAÇÃO
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
		// Aplicação do operador CRUZAMENTO
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
		// Aplicação do operador SELEÇÃO
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

		// Verifica se a condição de parada foi satisfeita, antes que o número máximo
		// de gerações seja atingido
		if (VerificaCriterioParada())
			break;
	}

	// debug
	if (config->GetMostrarLogDebug()) {
		log->ImprimeLog();
	}
}

// ---------------------------------------------------------------------------
// #2 - Geração dos indivíduos iniciais do AE
void __fastcall TEstrategiaEvolutiva::GeraIndividuosIniciais() {
	TIndividuoEE* indiv;
	double X, Rf, maxRfalta;
	double sigmaXini, sigmaRfini;
	int numIndivIni;
	int IDbarra1, IDbarra2;
	double xPorc;

	// Pega alguns parâmetros de configurações
	numIndivIni = config->GetNumIndivIni();
	maxRfalta = config->GetMaxRfalta();

	// debug
	TStringList* lisAux = new TStringList();
	String linha;

	for (int i = 0; i < numIndivIni; i++) {
		// Define os parâmetros do indivíduo
		X = sorteio->Uniforme(0., 100.);
		Rf = sorteio->Uniforme(0., maxRfalta);

		sigmaXini = config->GetSigmaXini();
		sigmaRfini = config->GetSigmaRfini();

		// De-para entre X e barra1, barra2 e xporc
		String resp = areaBusca->GetTrechoBarra1Barra2LocalDiscret(X);
		IDbarra1 = GetCampoCSV(resp, 1, ";").ToInt();
		IDbarra2 = GetCampoCSV(resp, 2, ";").ToInt();
		xPorc = GetCampoCSV(resp, 3, ";").ToDouble();

		// Cria objeto de indivíduo a partir dos parâmetros definidos acima
		indiv = new TIndividuoEE(this, X, Rf, sigmaXini, sigmaRfini);
		indiv->SetSorteio(sorteio);
		indiv->SetConfig(config);
		lisIndividuos->Add(indiv);

		// Cadastra um ID para o indivíduo
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
// Método para gerar objetos (structs) de soluções resultantes do processo de
// otimização de Estratégia Evolutiva
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

	// Ordena as soluções da mais para a menos provável (em função da nota do indivíduo)
	OrdenaMelhoresIndividuos();

	// Para cada melhor indivíduo de um cluster, gera uma solução associada
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
// #1 - Inicialização do objeto de testes de faltas (TTryFault)
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

		// Se a nota de avaliação do indivíduo estiver fora do intervalo (0,100), remove
		if (indiv->GetAvaliacao() < 0. || indiv->GetAvaliacao() > 100.) {
			lisIndividuos->Remove(indiv);
		}
	}
}

// ---------------------------------------------------------------------------
// 1) Pega o primeiro elemento da lista ordenada (melhor indivíduo) e o salva
// numa lista de melhores indivíduos por cluster
// 2) Limpa lista de indivíduos
void __fastcall TEstrategiaEvolutiva::SalvaMelhorIndividuoCluster() {
	TIndividuoEE* indiv = (TIndividuoEE*) lisIndividuos->Items[0];
	lisIndividuos->Remove(indiv);
	lisMelhoresIndividuosClusteres->Add(indiv);

	// Limpa lista de indivíduos
	// LimpaTList(lisIndividuos);

	for (int i = 0; i < lisIndividuos->Count; i++) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];
		delete indiv;
	}
	lisIndividuos->Clear();
}

// ---------------------------------------------------------------------------
/***
 * Este método considera:
 * 1) Lista de indivíduos pais
 * 2) Lista de novos indivíduos gerados por mutação
 * 3) Lista de novos indivíduos gerados por cruzamento
 *
 * Então, atualiza a lista de indivíduos pais apenas com os melhores indivíduos.
 */
void __fastcall TEstrategiaEvolutiva::SelecionaMelhoresIndividuos() {
	bool isOK;
	int MaxIdade;
	TIndividuoEE *indiv_ant, *indiv;
	TList* lisTotal;

	// Obtém a idade máxima configurada para um indivíduo
	MaxIdade = config->GetMaxIdade();

	// Lista para o conjunto total de indivíduos
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

	// Ordena os indivíduos do melhor para o pior, com base nas suas funções de avaliação.
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

	// ::::: OBS: Verificar a idade máxima dos indivíduos aqui

	// Limpa lista de indivíduos pais e insere apenas os melhores (dentre pais e filhos)
	lisIndividuos->Clear();
	for (int i = 0; i < lisTotal->Count; i++) {
		indiv = (TIndividuoEE*) lisTotal->Items[i];

		// Se indivíduo superar idade máxima, é desconsiderado
		if (indiv->GetIdade() > MaxIdade)
			continue;

		// Adiciona os melhores indivíduos até a qtde máxima de indivíduos por geração
		if (lisIndividuos->Count < config->GetMaxIndivPorGeracao()) {
			indiv->IncrementaIdade();
			lisIndividuos->Add(indiv);
		}
	}

	// Limpa lista de todos os indivíduos. Se o indivíduo tiver sido
	// desconsiderado, é destruído
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
 * Verifica se os parâmetros do filho do cruzamento são consistentes para utilização
 */
void __fastcall TEstrategiaEvolutiva::VerificaConsistenciaFilhoCruzamento
	 (TIndividuoEE* filho) {
	double X, Rf;

	// Verificação
	if (filho == NULL)
		return;

	X = filho->GetX();
	Rf = filho->GetRf();

	if (X < 0. || X > 100.)
		return;
	if (Rf < 0. || Rf > config->GetMaxRfalta())
		return;

	// Se os parâmetros do filho forem adequados, adiciona à lista de novos indiv por mutação
	lisNovosIndividuosCruzamento->Add(filho);
}

// ---------------------------------------------------------------------------
/***
 * Verifica se os parâmetros dos filhos da mutação são consistentes para utilização
 */
void __fastcall TEstrategiaEvolutiva::VerificaConsistenciaFilhosMutacao
	 (TList* filhosMutacao) {
	double X, Rf;

	// Verificação
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

		// Se os parâmetros do filho forem adequados, adiciona à lista de novos indiv por mutação
		lisNovosIndividuosMutacao->Add(filho);
	}
}

// ---------------------------------------------------------------------------
/***
 * Com os indivíduos da geração ordenados por ordem decrescente de função de avaliação,
 * compara a função de avaliação do melhor indivíduo com a função de avaliação média da geração.
 */
bool __fastcall TEstrategiaEvolutiva::VerificaCriterioParada() {
	double fAval_media, fAval_melhor, delta_fAval, min_delta_fAval;
	TIndividuoEE* indiv;

	// Pega o valor pré-configurado de menor valor do delta de avaliação
	min_delta_fAval = config->GetMinDeltaFAval();

	// Obtém o valor médio das funções de avaliação dos indivíduos da geração corrente
	fAval_media = 0.;
	for (int i = 0; i < lisIndividuos->Count; i++) {
		indiv = (TIndividuoEE*) lisIndividuos->Items[i];

		// Pega a função de avaliação do melhor indivíduo
		if (i == 0)
			fAval_melhor = indiv->GetAvaliacao();

		// Incrementa fAval_media
		fAval_media += indiv->GetAvaliacao();
	}
	if (lisIndividuos->Count > 0)
		fAval_media /= lisIndividuos->Count;
	else
		return false;

	// Computa a diferença entre fAval do melhor indivíduo e fAval média da geração
	delta_fAval = fAval_melhor - fAval_media;

	// Verifica se o delta de fAval é menor que o mínimo
	if (delta_fAval < min_delta_fAval) {
		return true;
	}

	return false;
}
// ---------------------------------------------------------------------------
