// Classe que implementa a metodologia de localização de cabo rompido.
//   1- Testar rompimento de cabo em uma barra
//   2- Comparar as medições com os valores calculados
//   3- Atribuir índice de erro
//   4- Determinar a(s) solução(ões)
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAlgoRompCabo.h"
#include "..\Auxiliares\FuncoesFL.h"
#include "..\Auxiliares\TDados.h"
#include "..\Auxiliares\TFuncoesDeRede.h"
#include "..\Auxiliares\TLog.h"
#include "..\DSS\TTryRompCabo.h"
#include "..\DSS\TDSS.h"
#include "..\TAreaBusca.h"
#include "..\TClassificacao.h"
#include "..\TThreadFaultLocation.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
// Construtor
__fastcall TAlgoRompCabo::TAlgoRompCabo(VTApl* apl)
{
	// Salva parâmetros elementares
	this->apl = apl;
	path = (VTPath*) apl->GetObject(__classid(VTPath));
	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	funcoesRede = new TFuncoesDeRede(apl);
	dados = NULL;
	areaBusca = NULL;

	// Objeto para testes de rompimento de cabos utilizando o OpenDSS
	tryRompCabo = new TTryRompCabo();

	// Inicializações
	lisBarrasCandidatas = new TList();
}
//---------------------------------------------------------------------------
// Destrutor
__fastcall TAlgoRompCabo::~TAlgoRompCabo()
{
	// Destroi objetos
	if(lisBarrasCandidatas) {delete lisBarrasCandidatas; lisBarrasCandidatas = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TAlgoRompCabo::ExecutaLocRompCabo()
{
	bool inicial;
	String faseRompimento;
	int patamar, IDbarra1;
	TList* lisTrechosAreaBuscaRompCabo;
	TList* lisAux = new TList();

	faseRompimento = "ABC";
	faseRompimento = classificacao->GetStrTipoRompCabo();

	// Obtém os trechos da área de busca do rompimento de cabo
	lisTrechosAreaBuscaRompCabo = new TList();
	areaBusca->GetAreaBuscaRompCabo_Trechos(lisTrechosAreaBuscaRompCabo);

//	// Para cada trecho da área de busca, executa teste de rompimento
//	for(int i=0; i<lisTrechosAreaBuscaRompCabo->Count; i++)
//	{
//		VTTrecho* trecho = (VTTrecho*) lisTrechosAreaBuscaRompCabo->Items[i];
//		IDbarra1 = trecho->Barra(0)->Id;
//
//		if(lisAux->IndexOf(trecho->Barra(0)) < 0)
//			lisAux->Add(trecho->Barra(0));
//		else
//			continue;
//
//		// Executa o teste de rompimento de cabo
//		tryRompCabo->TestaRompCabo(patamar, IDbarra1, faseRompimento, inicial);
//
//		// Calcula o índice de erro associado ao teste
//		indiceErro = tryRompCabo->CalculaErro_RompCabo();
//
//		// Restaura o rompimento de cabo
//		tryRompCabo->RestauraRompCabo();
//
//		// Resume o teste e salva em lista de barras de solução
//		strBarraSolucaoRompCabo* barraSolucao = new strBarraSolucaoRompCabo();
//		barraSolucao->barra = trecho->Barra(0);
//		barraSolucao->indiceErro = indiceErro;
//		barraSolucao->faseAfetada = faseRompimento;
//		lisBarrasCandidatas->Add(barraSolucao);
//	}

	// Debug - para versão preliminar, não testa as barras candidatas, sendo
	// todas soluções.
	for(int i=0; i<lisTrechosAreaBuscaRompCabo->Count; i++)
	{
		VTTrecho* trecho = (VTTrecho*) lisTrechosAreaBuscaRompCabo->Items[i];
		IDbarra1 = trecho->Barra(0)->Id;

		if(lisAux->IndexOf(trecho->Barra(0)) < 0)
			lisAux->Add(trecho->Barra(0));
		else
			continue;

		// Resume o teste e salva em lista de barras de solução
		strBarraSolucaoRompCabo* barraSolucao = new strBarraSolucaoRompCabo();
		barraSolucao->barra = trecho->Barra(0);
		barraSolucao->indiceErro = 0.001;
		barraSolucao->faseAfetada = faseRompimento;
		lisBarrasCandidatas->Add(barraSolucao);
	}

}
//---------------------------------------------------------------------------
void __fastcall TAlgoRompCabo::GetSolucoes(TFuncoesDeRede* funcoesRede, TList* lisEXT)
{
	double   distanciaChvMontante;
	int      utmx, utmy;
	int      numConsumidoresJusanteLigacao;
	VTBarra* barra;
	VTChave* chaveMontante;

	if(!lisEXT || !funcoesRede) return;

   lisEXT->Clear();

	// Ordena as soluções da mais para a menos provável (em função do índice de erro das soluções)
	OrdenaBarrasSolucao();

   // Para cada alternativa de solução, gera uma struct de solução
	for(int i=0; i<lisBarrasCandidatas->Count; i++)
	{
		strBarraSolucaoRompCabo* barraSolucao = (strBarraSolucaoRompCabo*) lisBarrasCandidatas->Items[i];

		barra = barraSolucao->barra;
		chaveMontante = funcoesRede->GetChaveMontante(barra);
		distanciaChvMontante = funcoesRede->GetDistanciaMetros(chaveMontante->Barra(1), barra);
		numConsumidoresJusanteLigacao = funcoesRede->NumeroConsJusLigacao_SolucoesRompCabo(chaveMontante->Codigo);
		barra->CoordenadasUtm_cm(utmx, utmy);

		double lat = double(utmy * 1e-7) - 100.;
		double lon = double(utmx * 1e-7) - 100.;

      strSolucao* solucao = new strSolucao();
      solucao->Probabilidade = i+1;
		solucao->DefX = String(utmx);
		solucao->DefY = String(utmy);
      solucao->DefLat = String(lat);
      solucao->DefLon = String(lon);
		solucao->barraSolucao = barra;
		solucao->CodBarra = barra->Codigo;
		solucao->IdBarra = barra->Id;
		solucao->DefTipo = "C";
		solucao->ChvMont = chaveMontante->Codigo;
		solucao->DistChvMont = String(Round(distanciaChvMontante, 2));
		solucao->DistRele = funcoesRede->GetDistancia_KM_DaSubestacao(barra);
		solucao->ClientesDepoisChvMont = String(numConsumidoresJusanteLigacao);
		solucao->IndiceErro = barraSolucao->indiceErro;
		solucao->Rfalta_estimado = 0;
		lisEXT->Add(solucao);
	}

//	if(AgruparSolucoes)
//	{
//		// Analisa chave montante para agrupar soluções
//		AgrupaSolucoes(lisEXT);
//	}
}
//---------------------------------------------------------------------------
void __fastcall TAlgoRompCabo::IniciaTryRompCabo(String caminhoDSS)
{
	tryRompCabo->SetCaminhoDSS(caminhoDSS);
	tryRompCabo->IniciaBarras();

   TStringList* lisMedicoesBarras = new TStringList();
   dados->GetMedicoesBarras_AlgoFasorial(lisMedicoesBarras);
	tryRompCabo->SetMedicoesBarras(lisMedicoesBarras);

   for(int i=lisMedicoesBarras->Count-1; i>=0; i--) lisMedicoesBarras->Delete(i);
   delete lisMedicoesBarras; lisMedicoesBarras = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoRompCabo::OrdenaBarrasSolucao()
{
	bool ok = false;

	if(lisBarrasCandidatas->Count == 1)
		return;

   while(!ok)
   {
      ok = true;
		for(int i=1; i<lisBarrasCandidatas->Count; i++)
      {
			strBarraSolucaoRompCabo* barraSol_ant = (strBarraSolucaoRompCabo*) lisBarrasCandidatas->Items[i-1];
			strBarraSolucaoRompCabo* barraSol = (strBarraSolucaoRompCabo*) lisBarrasCandidatas->Items[i];
			if(barraSol->indiceErro < barraSol_ant->indiceErro)
         {
				lisBarrasCandidatas->Remove(barraSol);
            lisBarrasCandidatas->Insert(i-1, barraSol);
            ok = false;
         }
      }
	}
}
//---------------------------------------------------------------------------
void __fastcall TAlgoRompCabo::SetLogRompCabo(TLog* logRompCabo)
{
	this->logRompCabo = logRompCabo;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoRompCabo::SetParametros(TAreaBusca* areaBusca, TClassificacao* classificacao, TDados* dados)
{
	// Salva referências dos objetos
	this->areaBusca = areaBusca;
	this->classificacao = classificacao;
	this->dados = dados;
}
//---------------------------------------------------------------------------
