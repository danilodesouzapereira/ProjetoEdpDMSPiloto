//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAlgoritmoDMS3.h"
#include "TThreadFaultLocation.h"
#include "Auxiliares\FuncoesFL.h"
#include "Auxiliares\TDados.h"
#include "Auxiliares\TFuncoesDeRede.h"
#include "Equipamentos\TChaveMonitorada.h"
#include "Equipamentos\TEqptoCampo.h"
#include "Equipamentos\TMedidorInteligenteBalanco.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TAlgoritmoDMS3::TAlgoritmoDMS3(VTApl* apl, TDados* dados)
{
	// Salva referências
	this->apl = apl;
	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	path = (VTPath*) apl->GetObject(__classid(VTPath));
	this->dados = dados;
	funcoesRede = new TFuncoesDeRede(apl);
	caminhoAteBarraFinal = NULL;
}
//---------------------------------------------------------------------------
__fastcall TAlgoritmoDMS3::~TAlgoritmoDMS3()
{
	// Destroi objetos
   delete funcoesRede;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoritmoDMS3::ExecutaLocRompCabo()
{
	// Detertmina a barra final da área de busca, que é a barra mais a montante com
	// alarme de Last Gasp
	caminhoAteBarraFinal = DefineCaminhoAteBarraFinalAreaBusca();

	// Determina os medidores inteligentes que não emitiram Last Gasp
	TList* lisBarrasMI_SemLastGasp = new TList;
	DeterminaBarrasMedidoresInteligentesSemLastGasp(lisBarrasMI_SemLastGasp);

	// Determina caminhos até as barras dos demais medidores inteligentes de
	// balanço que não emitiram Last Gasp
	TList* lisCaminhosAteMedSemLG = new TList;
	DeterminaCaminhosAteBarrasMedidoresSemLastGasp(lisBarrasMI_SemLastGasp, lisCaminhosAteMedSemLG);

	// Para cada caminho, elimina o caminho comum com "caminhoAteBarraFinal"
	EliminaCaminhosComuns(caminhoAteBarraFinal, lisCaminhosAteMedSemLG);

	delete lisBarrasMI_SemLastGasp;
	delete lisCaminhosAteMedSemLG;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoritmoDMS3::GetAreaBusca(TList* lisEXT)
{
	if(!lisEXT) return;
	for(int i=0; i<caminhoAteBarraFinal->lisLigacoes->Count; i++)
	{
		VTLigacao* ligacao = (VTLigacao*) caminhoAteBarraFinal->lisLigacoes->Items[i];

		// Cria objeto de solução DMS2, passando o bloco e a chave pai
		StrSolucaoDMS3* solucaoDMS3 = new StrSolucaoDMS3;
		solucaoDMS3->ligacao = ligacao;
		lisEXT->Add(solucaoDMS3);
	}
}
//---------------------------------------------------------------------------
void __fastcall TAlgoritmoDMS3::EliminaCaminhosComuns(StrCaminho* caminhoAteBarraFinal, TList* lisCaminhosAteMedSemLG)
{
	if(!caminhoAteBarraFinal || !lisCaminhosAteMedSemLG) return;

	// Remove as ligações que são comuns ao "caminhoAteBarraFinal"
	TList* lisLigacoes = caminhoAteBarraFinal->lisLigacoes;
	TList* lisBarras = caminhoAteBarraFinal->lisBarras;
	for(int i=lisLigacoes->Count-1; i>=0; i--)
	{
		VTLigacao* ligacao = (VTLigacao*) lisLigacoes->Items[i];

		for(int j=0; j<lisCaminhosAteMedSemLG->Count; j++)
		{
			StrCaminho* caminho = (StrCaminho*) lisCaminhosAteMedSemLG->Items[j];
			if(caminho->lisLigacoes->IndexOf(ligacao) != -1)
			{
				lisLigacoes->Remove(ligacao);

				if(lisBarras->IndexOf(ligacao->Barra(0)) != -1)
					lisBarras->Remove(ligacao->Barra(0));
				if(lisBarras->IndexOf(ligacao->Barra(1)) != -1)
					lisBarras->Remove(ligacao->Barra(1));
				break;
			}
		}
	}

	// Atualiza a barra inicial do caminho até a barra final
	VTLigacao* ligaIni = (VTLigacao*) caminhoAteBarraFinal->lisLigacoes->Items[0];
	if(lisBarras->IndexOf(ligaIni->Barra(0)) == -1)
	{
		lisBarras->Add(ligaIni->Barra(0));
		caminhoAteBarraFinal->barraIni = ligaIni->Barra(0);
	}
	else if(lisBarras->IndexOf(ligaIni->Barra(1)) == -1)
	{
		lisBarras->Add(ligaIni->Barra(1));
		caminhoAteBarraFinal->barraIni = ligaIni->Barra(1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TAlgoritmoDMS3::DeterminaCaminhosAteBarrasMedidoresSemLastGasp(TList* lisBarrasMI_SemLastGasp, TList* lisEXT)
{
	if(!lisBarrasMI_SemLastGasp || !lisEXT) return;

	TList* lisLigacoes = new TList;
	for(int i=0; i<lisBarrasMI_SemLastGasp->Count; i++)
	{
		VTBarra* barra = (VTBarra*)lisBarrasMI_SemLastGasp->Items[i];

		// Pega a lista de ligações entre a barra inicial e a barra da referida carga
		lisLigacoes->Clear();
		funcoesRede->GetCaminhoLigacoesSE_Barra(redeMT, barra, lisLigacoes);

		// Gera o objeto de caminho desde a SE até a barra da referida carga
		StrCaminho* caminho = new StrCaminho;
		caminho->barraIni = redeMT->BarraInicial();
		caminho->barraFim = barra;
		for(int j=0; j<lisLigacoes->Count; j++)
		{
			VTLigacao* liga = (VTLigacao*) lisLigacoes->Items[j];
			caminho->lisLigacoes->Add(liga);
		}

		// Insere objeto de caminho na lista externa
		lisEXT->Add(caminho);
	}
	delete lisLigacoes;
}
//---------------------------------------------------------------------------
void __fastcall TAlgoritmoDMS3::DeterminaBarrasMedidoresInteligentesSemLastGasp(TList* lisEXT)
{
	if(!lisEXT) return;

	// Lê arquivo de cadastro de medidores inteligentes
	String pathArqCadastroMI = path->DirDat() + "\\FaultLocation\\CadastroMedidoresBalanco.csv";
	TStringList* lisLinhasCadMI = new TStringList;
	lisLinhasCadMI->LoadFromFile(pathArqCadastroMI);

	// Pega todas as cargas do alimentador em questão
	TList* lisCargas = new TList;
	redeMT->LisEqbar(lisCargas, eqptoCARGA);

	// Percorre todo o cadastro de medidores inteligentes
	for(int i=0; i<lisLinhasCadMI->Count; i++)
	{
		// Verifica o código do alimentador
		String linha =  lisLinhasCadMI->Strings[i];
		String codRede = GetCampoCSV(linha, 0, ";");
		if(redeMT->Codigo != codRede) continue;

		// Verifica se a rede realmente contém aquela carga
		String codCarga = GetCampoCSV(linha, 2, ";");
		VTCarga* carga = NULL;
		for(int j=0; j<lisCargas->Count; j++)
		{
			carga = (VTCarga*) lisCargas->Items[j];
			if(carga->Codigo == codCarga)
				break;
			else
				carga = NULL;
		}

		// Caso a carga realmente exista, acrescenta a sua barra à lista externa
		if(carga) lisEXT->Add(carga->pbarra);
	}

	// Remove as barras cujos medidores inteligentes de balanço emitiram last gasp
	TList* lisEqptosCampo = dados->GetEqptosCampo();
	for(int i=0; i<lisEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) lisEqptosCampo->Items[i];
		if(eqptoCampo->Tipo != eqptoMI) continue;

		TMedidorInteligenteBalanco* miBalanco = (TMedidorInteligenteBalanco*)eqptoCampo;
		VTBarra* barraMI = miBalanco->cargaAssociada->pbarra;
		if(lisEXT->IndexOf(barraMI) != -1)
			lisEXT->Remove(barraMI);
   }

	// Destroi objetos
	delete lisLinhasCadMI;
	delete lisCargas;
}
//---------------------------------------------------------------------------
/***
 * Determina a barra final da área de busca
 */
StrCaminho* __fastcall TAlgoritmoDMS3::DefineCaminhoAteBarraFinalAreaBusca()
{
	TList* listaEqptosCampo = dados->GetEqptosCampo();

	// Percorre os medidores inteligentes de balanço que emitiram Last Gasp, obtendo
	// seus caminhos desde a subestação
	TList* lisCaminhos = new TList;
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoMI) continue;

		TMedidorInteligenteBalanco* miBalanco = (TMedidorInteligenteBalanco*) eqptoCampo;
		StrCaminho* caminho = CaminhoDesdeSubestacao(miBalanco->cargaAssociada->pbarra);
		lisCaminhos->Add(caminho);
	}

	// A partir da lista de caminhos, determina o caminho comum
	StrCaminho* caminhoComum = DeterminaCaminhoComum(lisCaminhos);
	delete lisCaminhos;

	return(caminhoComum);
}
//---------------------------------------------------------------------------
StrCaminho* __fastcall TAlgoritmoDMS3::DeterminaCaminhoComum(TList* lisCaminhos)
{
	if(!lisCaminhos) return(NULL);

	// Comprimento do caminho mais curto
	int menorComp = 0;
	int iMenorComp = -1;
	for(int i=0; i<lisCaminhos->Count; i++)
	{
		StrCaminho* caminho = (StrCaminho*) lisCaminhos->Items[i];
		if(menorComp == 0 || caminho->lisLigacoes->Count < menorComp)
		{
			menorComp = caminho->lisLigacoes->Count;
			iMenorComp = i;
		}
	}


	// Identifica a posição do caminho em que as barras deixam de ser comuns
	StrCaminho* caminhoComum = new StrCaminho;
	caminhoComum->barraIni = redeMT->BarraInicial();
	for(int i=0; i<menorComp; i++)
	{
		// Verifica se, na posição i, todos os caminhos têm a mesma barra
		if(!TodosCaminhosMesmaBarra(lisCaminhos, i))
		{
			caminhoComum->barraFim = (VTBarra*)((StrCaminho*)lisCaminhos->Items[0])->lisBarras->Items[i];
			break;
		}
		else
		{
			VTBarra* barra_i = (VTBarra*)((StrCaminho*)lisCaminhos->Items[0])->lisBarras->Items[i];
			VTLigacao* ligacao_i = (VTLigacao*)((StrCaminho*)lisCaminhos->Items[0])->lisLigacoes->Items[i];
			caminhoComum->lisBarras->Add(barra_i);
			caminhoComum->lisLigacoes->Add(ligacao_i);
		}
	}

	return(caminhoComum);
}
//---------------------------------------------------------------------------
bool __fastcall TAlgoritmoDMS3::TodosCaminhosMesmaBarra(TList* lisCaminhos, int indicePosicao)
{
	if(!lisCaminhos || indicePosicao == -1) return(false);

	VTBarra* barra = NULL;
	for(int i=0; i<lisCaminhos->Count; i++)
	{
		StrCaminho* caminho = (StrCaminho*) lisCaminhos->Items[i];
		VTBarra* barraCaminho = (VTBarra*) caminho->lisBarras->Items[indicePosicao];
		if(!barra)
			barra = barraCaminho;
		else if(barra != barraCaminho)
			return(false);
	}
	return(true);
}
//---------------------------------------------------------------------------
StrCaminho* __fastcall TAlgoritmoDMS3::CaminhoDesdeSubestacao(VTBarra* barraFinal)
{
	TList* lisLigacoesDesdeSE = new TList;
	TList* lisBarrasDesdeSE = new TList;
	funcoesRede->GetCaminhoLigacoesSE_Barra(redeMT, barraFinal, lisLigacoesDesdeSE);
	funcoesRede->GetCaminhoBarrasSE_Barra(redeMT, barraFinal, lisBarrasDesdeSE);

	StrCaminho* caminho = new StrCaminho;
	caminho->barraIni = redeMT->BarraInicial();
	caminho->barraFim = barraFinal;
	for(int i=0; i<lisLigacoesDesdeSE->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) lisLigacoesDesdeSE->Items[i];
		caminho->lisLigacoes->Add(liga);
	}
	for(int i=0; i<lisBarrasDesdeSE->Count; i++)
	{
		VTBarra* barra = (VTBarra*) lisBarrasDesdeSE->Items[i];
		caminho->lisBarras->Add(barra);
	}
	delete lisLigacoesDesdeSE;
	delete lisBarrasDesdeSE;

	return(caminho);
}
//---------------------------------------------------------------------------
void __fastcall TAlgoritmoDMS3::Inicializa()
{
	// Pega lista de eqptos de campo sensibilizados
	TList* listaEqptosCampo = dados->GetEqptosCampo();

	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoMI) continue;
		TMedidorInteligenteBalanco* miBalanco = (TMedidorInteligenteBalanco*)eqptoCampo;

		// Procura a rede que contém a barra à qual a carga está conectada
		TList* lisRedes = redes->LisRede();
		for(int i=0; i<lisRedes->Count ;i++)
		{
			VTRede* rede = (VTRede*) lisRedes->Items[i];
			if(rede->TipoRede->Segmento != redePRI || !rede->Carregada) continue;
			if(rede->ExisteBarra(miBalanco->cargaAssociada->pbarra))
			{
				redeMT = rede;
				break;
			}
		}
      if(redeMT) break;
	}

   // Determina a rede SE (rede da Subestação)
   if(!redeMT) return;

   bool ok = false;
	for(int i=0; i<redes->LisRede()->Count && !ok; i++)
   {
		redeSE = (VTRede*) redes->LisRede()->Items[i];
      if(redeSE->TipoRede->Segmento != redeETD) continue;

      for(int j=0; j<redeSE->LisLigacao()->Count; j++)
      {
         VTLigacao* liga = (VTLigacao*) redeSE->LisLigacao()->Items[j];
         if(liga->Barra(0) == redeMT->BarraInicial() || liga->Barra(1) == redeMT->BarraInicial())
         {
            ok = true;
            break;
         }
      }
	}
}
//---------------------------------------------------------------------------
