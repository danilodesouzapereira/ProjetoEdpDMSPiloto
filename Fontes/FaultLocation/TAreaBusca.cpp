//---------------------------------------------------------------------------
#pragma hdrstop
#include "TAreaBusca.h"
#include "TBlocosRedeRadial.h"
#include "TCluster.h"
#include "TThreadFaultLocation.h"
#include "Auxiliares\TConfiguracoes.h"
#include "Auxiliares\TDados.h"
#include "Auxiliares\TFuncoesDeRede.h"
#include "Auxiliares\FuncoesFL.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <System.StrUtils.hpp>
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Ordena.h>
#include <PlataformaSinap\DLL_Inc\Radial.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Ordena\VTOrdena.h>
#include <PlataformaSinap\Fontes\Radial\VTPrimario.h>
#include <PlataformaSinap\Fontes\Radial\VTRadial.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTCarga.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
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
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TITrafo.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TMedidorInteligenteBalanco.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TQualimetro.h>
#include <ProjetoEdpDMSPiloto\Fontes\FaultLocation\Equipamentos\TSensor.h>
//---------------------------------------------------------------------------
bool __fastcall Nulo(std::complex<double> valor)
{
	std::complex<double> czero = std::complex<double>(0., 0.);

   if(valor != czero)
      return false;

   return true;
}
//---------------------------------------------------------------------------
__fastcall TAreaBusca::TAreaBusca(VTApl* apl, TDados* dados, TFuncoesDeRede* funcoesRede)
{
	// Salva ponteiros
	this->apl = apl;
   path = (VTPath*) apl->GetObject(__classid(VTPath));
   redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
   this->dados = dados;
	this->funcoesRede = funcoesRede;
	configuracoes = NULL;

   // Inicializa listas
   lisBlocosPesquisa = new TList();
	lisTrechosDiscret = new TList();
	lisClusteres = new TList();
	lisBlocosRedeRadial = new TList();
	lisLigacoesAreaBusca_RompCabo = new TList();

   // Inicializações
	redeMT = NULL;
	chaveRef_FLOffline = NULL;
 	eqptoCampo_DeterminaAreaBusca = NULL;
   ConsiderarCoordenacaoProtecao = false;

   // Inicializa configurações
	IniciaConfigs();
}
//---------------------------------------------------------------------------
__fastcall TAreaBusca::~TAreaBusca(void)
{
	// Destroi objetos
//	if(lisBlocosPesquisa != NULL)    {delete lisBlocosPesquisa; lisBlocosPesquisa = NULL;}
	if(lisClusteres != NULL)         {delete lisClusteres; lisClusteres = NULL;}
	if(lisTrechosDiscret != NULL)    {delete lisTrechosDiscret; lisTrechosDiscret = NULL;}
	if(lisBlocosRedeRadial != NULL)  {delete lisBlocosRedeRadial; lisBlocosRedeRadial = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Compara os blocos candidatos e define os blocos que efetivamente serão
 * pesquisados.
 */
void __fastcall TAreaBusca::DefineBlocosMaisJusante()
{
	// Percorre apenas "Chaves Monitoradas"
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if((eqptoCampo->GetTipo() != chaveDJ) &&
			(eqptoCampo->GetTipo() != chaveRE) &&
			(eqptoCampo->GetTipo() != chaveSC))
		{
			continue;
		}

      // Se blocos à jusante ainda não definidos
		if(lisBlocosPesquisa->Count == 0)
      {
			lisBlocosPesquisa = eqptoCampo->GetBlocosJusante();
         eqptoCampo_DeterminaAreaBusca = eqptoCampo;
      }
      else
      {
			// Se já definidos, verifica se nova lista é subconjunto
			if(Subconjunto(lisBlocosPesquisa, eqptoCampo->GetBlocosJusante()))
			{
				lisBlocosPesquisa = eqptoCampo->GetBlocosJusante();
				eqptoCampo_DeterminaAreaBusca = eqptoCampo;
			}
		}
	}

	// Se foi determinado um DJ ou RE mais próximo do defeito, ele pode ser utilizado
	// como eqpto de referência (para FL#2) no caso do FL Offline
	if(eqptoCampo_DeterminaAreaBusca)
	{
		TChaveMonitorada* chaveMon = (TChaveMonitorada*)eqptoCampo_DeterminaAreaBusca;
		chaveRef_FLOffline = chaveMon->GetChaveAssociada();
	}

	// Se o equipamento (DJ ou RE) só teve tentativas de religamentos e não entrou
	// em autobloqueio, é possível restringir a área de busca ainda mais, procurando
	// pelos trafos MT/BT com alarme de Last Gasp após chaves fusíveis.
	TChaveMonitorada* chaveMon = (TChaveMonitorada*)eqptoCampo_DeterminaAreaBusca;
	if(chaveMon->ApenasReligamentos())
	{
		AtualizaBlocosPesquisa_LastGaspAposFusivel();
	}
	// Se o equipamento (DJ ou RE) entrou em lockout (autobloqueio), então assume-se
	// a hipótese de que a falta está na zona de proteção do equipamento (DJ ou RE)
	else if(chaveMon->Autobloqueio())
	{
		AtualizaBlocosPesquisa_LastGaspAntesFusivel();
   }

//	//debug
//	TStringList* listatrechos = new TStringList;
//	for(int i=0; i<lisBlocosPesquisa->Count; i++)
//	{
//		VTBloco* bloco = (VTBloco*)lisBlocosPesquisa->Items[i];
//		for(int j=0; j<bloco->LisLigacao()->Count; j++)
//		{
//			VTLigacao* liga = (VTLigacao*) bloco->LisLigacao()->Items[j];
//			if(liga->Tipo() == eqptoTRECHO)
//			{
//				listatrechos->Add(liga->Codigo);
//         }
//		}
//	}
//	listatrechos->SaveToFile("c:\\users\\sinapsis\\desktop\\listaTrechos.csv");
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::AtualizaBlocosPesquisa_LastGaspAposFusivel()
{
	// Transfere blocos da área de pesquisa inicial para lista auxiliar
	TList* lisBlocosPesqInicial = new TList;
	for(int i=lisBlocosPesquisa->Count-1; i>=0; i--)
	{
		VTBloco* bloco = (VTBloco*)lisBlocosPesquisa->Items[i];
		lisBlocosPesquisa->Remove(bloco);
		lisBlocosPesqInicial->Add(bloco);
	}

	// Pega apenas os blocos dos medidores inteligentes com last gasp e que
	// pertencem à área de busca determinada inicialmente.
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoMI) continue;
		TMedidorInteligenteBalanco* miBalanco = (TMedidorInteligenteBalanco*)eqptoCampo;

		// Considera medidor inteligente de balanço com last gasp e que esteja na
		// área de busca determinada inicialmente
		if(lisBlocosPesqInicial->IndexOf(miBalanco->blocoCarga) == -1) continue;

		// Acrescenta o bloco à área de busca atualizada
		if(lisBlocosPesquisa->IndexOf(miBalanco->blocoCarga) == -1)
			lisBlocosPesquisa->Add(miBalanco->blocoCarga);
	}
	delete lisBlocosPesqInicial;
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::AtualizaBlocosPesquisa_LastGaspAntesFusivel()
{
	// Transfere blocos da área de pesquisa inicial para lista auxiliar
	TList* lisBlocosPesqInicial = new TList;
	for(int i=lisBlocosPesquisa->Count-1; i>=0; i--)
	{
		VTBloco* bloco = (VTBloco*)lisBlocosPesquisa->Items[i];
		lisBlocosPesquisa->Remove(bloco);
		lisBlocosPesqInicial->Add(bloco);
	}

	// Pega apenas o bloco do equipamento (DJ ou RE) atuante
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != chaveDJ && eqptoCampo->GetTipo() != chaveRE) continue;
		TChaveMonitorada* chaveMon = (TChaveMonitorada*)eqptoCampo;

		lisBlocosPesquisa->Add(chaveMon->blocoChave);

//		// Considera medidor inteligente de balanço com last gasp e que esteja na
//		// área de busca determinada inicialmente
//		if(lisBlocosPesqInicial->IndexOf(miBalanco->blocoCarga) == -1) continue;
//
//		// Acrescenta o bloco à área de busca atualizada
//		if(lisBlocosPesquisa->IndexOf(miBalanco->blocoCarga) == -1)
//			lisBlocosPesquisa->Add(miBalanco->blocoCarga);
	}
	delete lisBlocosPesqInicial;
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::DefineClusteres()
{
	funcoesRede->SetRedeMT(redeMT);
   funcoesRede->GetClusteres(lisBlocosPesquisa, lisClusteres);
}
//---------------------------------------------------------------------------
/***
 * Método para fazer a discretização dos trechos da área de busca
 */
void __fastcall TAreaBusca::DiscretizaAreaBusca(TList* lisBlocos)
{
	double compAcum = 0.;             //< soma dos comprimentos (metros) dos trechos
   double compTotal, xPorcIni, xPorcFin;
	TList *lisLiga, *listaLigacoes;
   VTBloco* bloco;
   VTLigacao* liga;
   VTTrecho* trecho;
	TTrechoDiscret* trechoDiscret;


   if(lisBlocos == NULL)
		lisBlocos = lisBlocosPesquisa;

   // :::::::::::::::::::::::::::
 	// Obtém o comprimento total dos trechos
   // :::::::::::::::::::::::::::
   compTotal = 0.;
   for(int i=0; i<lisBlocos->Count; i++)
   {
		bloco = (VTBloco*) lisBlocos->Items[i];

      lisLiga = bloco->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Tipo() != eqptoTRECHO) continue;
         trecho = (VTTrecho*) liga;

         compTotal += trecho->Comprimento_m;
      }
   }

   // Verificação
   if(compTotal == 0.) return;

   // Limpa lista de destino
	lisTrechosDiscret->Clear();

   // :::::::::::::::::::::::::::
 	// Varre os trechos da área de busca para computar xIni e xFin
	// :::::::::::::::::::::::::::
	for(int i=0; i<lisBlocos->Count; i++)
   {
		bloco = (VTBloco*) lisBlocos->Items[i];

      lisLiga = bloco->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Tipo() != eqptoTRECHO) continue;
         trecho = (VTTrecho*) liga;

			// Obtém os limites xIni e xFin (em %)
			xPorcIni = 100. * compAcum / compTotal;
			compAcum += trecho->Comprimento_m;
			xPorcFin = 100. * compAcum / compTotal;

			// Cria objeto de TTrechoDiscret para o trecho
			trechoDiscret = new TTrechoDiscret(trecho);
			trechoDiscret->SetXiniXfin(xPorcIni, xPorcFin);

			// Armazena o objeto em lista
			lisTrechosDiscret->Add(trechoDiscret);
      }
	}
}
//---------------------------------------------------------------------------
/***
 * Método para fazer a discretização dos trechos da área de busca
 */
void __fastcall TAreaBusca::DiscretizaAreaBusca_EE(VTChave* chvMontante, TList* lisBlocos)
{
	double compAcum = 0.;             //< soma dos comprimentos (metros) dos trechos
   double compTotal, xPorcIni, xPorcFin;
	TList *lisLiga, *listaLigacoes;
	VTBloco* bloco;
   VTLigacao* liga;
   VTTrecho* trecho;
	TTrechoDiscret* trechoDiscret;


   if(lisBlocos == NULL)
		lisBlocos = lisBlocosPesquisa;

   // :::::::::::::::::::::::::::
 	// Obtém o comprimento total dos trechos
   // :::::::::::::::::::::::::::
   compTotal = 0.;
   for(int i=0; i<lisBlocos->Count; i++)
   {
		bloco = (VTBloco*) lisBlocos->Items[i];

      lisLiga = bloco->LisLigacao();
      for(int j=0; j<lisLiga->Count; j++)
      {
			liga = (VTLigacao*) lisLiga->Items[j];
         if(liga->Tipo() != eqptoTRECHO) continue;
         trecho = (VTTrecho*) liga;

         compTotal += trecho->Comprimento_m;
      }
   }

   // Verificação
   if(compTotal == 0.) return;

   // Limpa lista de destino
   lisTrechosDiscret->Clear();

	//debug
	TList* lisTrechosOrdenados = new TList();
	VTTrecho* trechoRef = funcoesRede->GetTrechoJusanteLigacao(chvMontante, chvMontante->rede);
	funcoesRede->GetLigacoesOrdenadas_JusanteLigacao(trechoRef, lisTrechosOrdenados);
	for(int i=0; i<lisTrechosOrdenados->Count; i++)
	{
		VTTrecho* trecho = (VTTrecho*) lisTrechosOrdenados->Items[i];
	}

   // :::::::::::::::::::::::::::
 	// Varre os trechos da área de busca para computar xIni e xFin
	// :::::::::::::::::::::::::::

	for(int i=0; i<lisTrechosOrdenados->Count; i++)
	{
		VTTrecho* trecho = (VTTrecho*) lisTrechosOrdenados->Items[i];

		// Obtém os limites xIni e xFin (em %)
		xPorcIni = 100. * compAcum / compTotal;
		compAcum += trecho->Comprimento_m;
		xPorcFin = 100. * compAcum / compTotal;

		// Cria objeto de TTrechoDiscret para o trecho
		trechoDiscret = new TTrechoDiscret(trecho);
		trechoDiscret->SetXiniXfin(xPorcIni, xPorcFin);

		// Armazena o objeto em lista
		lisTrechosDiscret->Add(trechoDiscret);
	}
}
//---------------------------------------------------------------------------
/***
 * Executa o algoritmo de determinação da área de busca
 */
void __fastcall TAreaBusca::Executa()
{
	// Define os blocos à jusante de cada eqpto de dado.
	Inicializa();

	// Define os blocos mais à jusante a partir da proteção (DJ ou RE) atuante.
	// Se DJ ou RE apenas tentou religamentos, a falta foi extinta por atuação de
	// fusível. Nesse caso, considera apenas os blocos dos medidores inteligentes
	// que emitiram Last Gasp.
	DefineBlocosMaisJusante();

	// Aplica lógicas para refinamento da lista de blocos a serem pesquisados
	// (remove blocos à jusante de sensores que não foram sensibilizados)
//   RefinamentoBlocosJusante_Sensores();

	// Se a coordenação de proteção for considerada, remove os blocos à jusante
	// das proteções seguintes
	if(ConsiderarCoordenacaoProtecao)
	{
		FiltrarBlocos_Religadoras();
		FiltrarBlocos_Fusiveis();
	}
}
//---------------------------------------------------------------------------
/***
 * Executa o algoritmo de determinação da área de busca
 */
void __fastcall TAreaBusca::Executa_DMS2()
{
	// Define os blocos à jusante de cada eqpto de dado.
	Inicializa();

	// Define os blocos mais à jusante a partir da proteção (DJ ou RE) atuante.
	// Se DJ ou RE apenas tentou religamentos, a falta foi extinta por atuação de
	// fusível. Nesse caso, considera apenas os blocos dos medidores inteligentes
	// que emitiram Last Gasp.
	DefineBlocosMaisJusante();
}
//---------------------------------------------------------------------------
/***
 * Executa o algoritmo de determinação da área de busca para os problemas
 * de rompimento de cabo
 */
void __fastcall TAreaBusca::Executa_RompCabo()
{
	// Ajustes preliminares
	Inicializa();

	// Define o alarme de qualímetro/trafo mais à montante
	DefineAlarmeQualimetroTrafoMontante();

	// Determina a ligação final (mais à jusante) da área de busca
	DeterminaLigacaoFinal_RompCabo();

	// Percorre o circuito em direção à subestação, procurando trafo ou chave
	// cujos blocos à jusante (descontando o bloco de pesquisa inicial) tem
	// qualímetro/trafo que não emitiu alarme
	DeterminaInicioAreaBusca();

	// Aqui, a barra "barraInicioAreaBusca" determina o início da área de busca

	// Ajusta a área de busca, seguindo as etapas:
	// 1) Determina todos os blocos à jusante da barra "barraInicioAreaBusca";
	// 2) Dos blocos listados em 1), remove os blocos da área de busca inicial;
//	AjustaAreaBusca_RompCabo();

   // Correção: pegando apenas as ligações que estão no caminho entre a barra
   // "barraInicioAreaBusca" e a ligação "ligacaoFinalAreaBusca_RompCabo":
   DeterminaLigacoesAreaBuscaRompCabo();
}
//---------------------------------------------------------------------------
// Entradas:
//    Rede MT em questão
//    barra1 - barra do início da área de busca
//    barra2 - barra do final da área de busca
// Saída: lisLigacoesAreaBusca_RompCabo - lista com as ligações da área de busca
void __fastcall TAreaBusca::DeterminaLigacoesAreaBuscaRompCabo()
{
   VTBarra* barra1 = barraInicioAreaBusca;
   VTBarra* barra2 = ligacaoFinalAreaBusca_RompCabo->Barra(1);
   funcoesRede->GetCaminhoLigacoes_BarraRef_Barra(redeMT, barra1, barra2, lisLigacoesAreaBusca_RompCabo);
}
//---------------------------------------------------------------------------
/***
 *  Para rompimento de cabo, ajusta a área de busca, seguindo as etapas:
 *    1) Determina todos os blocos à jusante da barra "barraInicioAreaBusca";
 *    2) Dos blocos listados em 1), remover os blocos da área de busca inicial;
 */
void __fastcall TAreaBusca::AjustaAreaBusca_RompCabo()
{
	TList* lisBlocosJusante;
	VTBloco* blocoRef;
	VTLigacao* ligaRef;

	// Blocos à jusante da lig ref
	ligaRef = funcoesRede->GetTrechoMontanteBarra(barraInicioAreaBusca, redeMT);
	lisBlocosJusante = new TList();
	funcoesRede->GetBlocosJusanteLigacao(ligaRef, lisBlocosJusante);

	// Remove o bloco que contém a ligação de referência
	blocoRef = NULL;
	for(int i=0; i<lisBlocosJusante->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosJusante->Items[i];
		if(bloco->LisLigacao()->IndexOf(ligaRef) >= 0)
		{
			lisBlocosJusante->Remove(bloco);
			blocoRef = bloco;
			break;
		}
	}

	// Ligações dos blocos à jusante
	for(int i=0; i<lisBlocosJusante->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosJusante->Items[i];
		TList* lisLigacao = bloco->LisLigacao();
		for(int j=0; j<lisLigacao->Count; j++)
		{
			VTLigacao* liga = (VTLigacao*) lisLigacao->Items[j];
			lisLigacoesAreaBusca_RompCabo->Add(liga);
		}
	}

	// Adiciona as ligações do bloco de referência, que estejam à jusante da ligação
	// de referência
	for(int i=0; i<blocoRef->LisLigacao()->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) blocoRef->LisLigacao()->Items[i];
		if(funcoesRede->LigacaoJusante(ligaRef, liga))
			lisLigacoesAreaBusca_RompCabo->Add(liga);
	}

	// Remove as ligações que estejam à jusante da barra final da área de busca
	TList* lisLigacoesJusanteBarraFinal = new TList();
	funcoesRede->GetLigacoesJusanteLigacao(ligacaoFinalAreaBusca_RompCabo, lisLigacoesJusanteBarraFinal);
	for(int i=0; i<lisLigacoesJusanteBarraFinal->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) lisLigacoesJusanteBarraFinal->Items[i];
      if(lisLigacoesAreaBusca_RompCabo->IndexOf(liga) >= 0)
         lisLigacoesAreaBusca_RompCabo->Remove(liga);
	}
	delete lisLigacoesJusanteBarraFinal; lisLigacoesJusanteBarraFinal = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::DeterminaLigacaoFinal_RompCabo()
{
	TList* caminhoAteSE = NULL;
	TList* caminhos = new TList();
	VTBarra* barraQualimetro;

	// Obtém os caminhos desde cada qualímetro até a SE
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

		TQualimetro* qualimetro = (TQualimetro*) eqptoCampo;
		VTLigacao* ligaQualimetro = qualimetro->GetLigacaoAssociada();

		if(ligaQualimetro)
		{
			barraQualimetro = ligaQualimetro->Barra(0);

         // pega o caminho do qualímetro até a SE
         caminhoAteSE = new TList();
         funcoesRede->GetCaminhoLigacoesSE_Barra(ligaQualimetro->rede, barraQualimetro, caminhoAteSE);
		}
		else
		{
			VTCarga* cargaQualimetro = qualimetro->cargaAssociada;
			barraQualimetro = cargaQualimetro->pbarra;

         VTRede* redeQualimetro = RedeBarra(barraQualimetro);

         // pega o caminho do qualímetro até a SE
         caminhoAteSE = new TList();
         funcoesRede->GetCaminhoLigacoesSE_Barra(redeQualimetro, barraQualimetro, caminhoAteSE);
		}

		// guarda o caminho até a SE
		caminhos->Add(caminhoAteSE);

//		// Se blocos à jusante ainda não definidos
//		if(lisBlocosPesquisa->Count == 0)
//		{
//			lisBlocosPesquisa = eqptoCampo->GetBlocosJusante();
//			eqptoCampo_DeterminaAreaBusca = eqptoCampo;
//		}
//		else
//		{
//			// Se já definidos, verifica se nova lista contém a lista inicial de blocos
//			if(Subconjunto(eqptoCampo->GetBlocosJusante(), lisBlocosPesquisa))
//			{
//				lisBlocosPesquisa = eqptoCampo->GetBlocosJusante();
//				eqptoCampo_DeterminaAreaBusca = eqptoCampo;
//			}
//		}
	}

	// Determina a ligação final da área de busca.
	ligacaoFinalAreaBusca_RompCabo = NULL;
	TList* caminho = MaiorCaminho(caminhos);
	for(int i=caminho->Count-1; i>=0; i--)
	{
		VTLigacao* ligacao = (VTLigacao*) caminho->Items[i];
		if(LigacaoComum(caminhos, ligacao))
		{
			ligacaoFinalAreaBusca_RompCabo = ligacao;
			break;
      }
	}
}
//---------------------------------------------------------------------------
bool __fastcall TAreaBusca::LigacaoComum(TList* caminhos, VTLigacao* ligacao)
{
	if(!caminhos || !ligacao) return(false);

	for(int i=0; i<caminhos->Count; i++)
	{
		TList* caminho = (TList*) caminhos->Items[i];
		if(caminho->IndexOf(ligacao) == -1)
			return(false);
	}
	return(true);
}
//---------------------------------------------------------------------------
TList* __fastcall TAreaBusca::MaiorCaminho(TList* caminhos)
{
	TList* maiorCaminho = NULL;
	for(int i=0; i<caminhos->Count; i++)
	{
		TList* caminho = (TList*) caminhos->Items[i];
		if(maiorCaminho == NULL || caminho->Count > maiorCaminho->Count)
			maiorCaminho = caminho;
	}
	return(maiorCaminho);
}
//---------------------------------------------------------------------------
/***
 * Define o alarme de qualímetro/trafo mais à montante
 */
void __fastcall TAreaBusca::DefineAlarmeQualimetroTrafoMontante()
{
	// Percorre os qualímetros
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

		// Se blocos à jusante ainda não definidos
		if(lisBlocosPesquisa->Count == 0)
		{
			lisBlocosPesquisa = eqptoCampo->GetBlocosJusante();
			eqptoCampo_DeterminaAreaBusca = eqptoCampo;
		}
		else
		{
			// Se já definidos, verifica se nova lista contém a lista inicial de blocos
			if(Subconjunto(eqptoCampo->GetBlocosJusante(), lisBlocosPesquisa))
			{
				lisBlocosPesquisa = eqptoCampo->GetBlocosJusante();
				eqptoCampo_DeterminaAreaBusca = eqptoCampo;
			}
		}
	}
}
//---------------------------------------------------------------------------
/***
 * Percorre o circuito em direção à subestação, procurando trafo ou chave
 * cujos blocos à jusante (descontando o bloco de pesquisa inicial) tem
 * qualímetro/trafo que não emitiu alarme
 */
void __fastcall TAreaBusca::DeterminaInicioAreaBusca()
{
	VTBarra *barraRef, *barraAnterior;
	VTLigacao* ligaJusante;

	// Inicializações
	barraInicioAreaBusca = NULL;
	barraRef = funcoesRede->GetBarraMontanteLigacao(ligacaoFinalAreaBusca_RompCabo);

	while(barraInicioAreaBusca == NULL && barraRef != NULL)
	{
		if(ExisteEqptoJusanteSemAlarme(barraRef))
		{
			barraInicioAreaBusca = barraAnterior;
		}
		else
		{
			barraAnterior = barraRef;
			barraRef = GetBarra_ChaveTrafoMontante(barraRef);
		}
	}
}
//---------------------------------------------------------------------------
/***
 *  Verifica se existe eqpto de campo à jusante (qualímetro ou trafo inteligente)
 *  que não emitiu alarme de afundamento de tensão.
 */
bool __fastcall TAreaBusca::ExisteEqptoJusanteSemAlarme(VTBarra* barraRef)
{
	if(barraRef == NULL) return false;

	TList* listaBlocosJusante = new TList();
	TList* lisChavesQualimetros = new TList();
	TList* lisCargasQualimetros = new TList();
	TList* listaChavesJusante = new TList();
   TList* listaCargasJusante = new TList();
	bool resp = false;

	// Pega lista das CHAVES e CARGAS à jusante da barra de referência
	funcoesRede->GetChavesJusanteBarra(redeMT, barraRef, listaChavesJusante);
	funcoesRede->GetCargasJusanteBarra(redeMT, barraRef, listaCargasJusante);

	// Pega lista dos eqptos de campo com QUALÍMETRO (chave ou carga)
	GetChavesAssociadasQualimetros(lisChavesQualimetros);
   GetCargasAssociadasQualimetros(lisCargasQualimetros);

	// Remove da lista os eqptos que não pertencem aos blocos à jusante da barra
	AjustaListaChavesJusante(listaChavesJusante, lisChavesQualimetros);
   AjustaListaCargasJusante(listaCargasJusante, lisCargasQualimetros);

	// Verifica se, na lista acima, há eqptos de campo que não estão na lista de
	// eqptos que emitiram alarme
	for(int i=0; i<lisChavesQualimetros->Count; i++)
	{
		VTChave* chave = (VTChave*) lisChavesQualimetros->Items[i];
		if(!ExisteAlarme(chave))
		{
			resp = true;
			break;
		}
	}
	for(int i=0; i<lisCargasQualimetros->Count; i++)
	{
		VTCarga* carga = (VTCarga*) lisCargasQualimetros->Items[i];
		if(!ExisteAlarme(carga))
		{
			resp = true;
			break;
		}
	}

   // Destroi listas
   delete listaChavesJusante; delete listaCargasJusante;
   delete lisChavesQualimetros; delete lisCargasQualimetros;

	return(resp);
}
//---------------------------------------------------------------------------
// Verifica se houve qualímetro que emitiu alarme de afundamento de tensão, que
// esteja associado à chave.
bool __fastcall TAreaBusca::ExisteAlarme(VTChave* chave)
{
	// Percorre os qualímetros que emitiram alarme de afundamento de tensão
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

		TQualimetro* qualimetro = (TQualimetro*) eqptoCampo;
		VTLigacao* ligacaoAssociada = qualimetro->GetLigacaoAssociada();
		if(!ligacaoAssociada) continue;

		if(ligacaoAssociada->Codigo == chave->Codigo)
			return(true);
	}
	return(false);
}
//---------------------------------------------------------------------------
// Verifica se houve qualímetro que emitiu alarme de afundamento de tensão, que
// esteja associado à carga.
bool __fastcall TAreaBusca::ExisteAlarme(VTCarga* carga)
{
	// Percorre os trafos inteligentes que emitiram alarme de afundamento de tensão
	for(int i=0; i<listaEqptosCampo->Count; i++)
	{
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
		if(eqptoCampo->GetTipo() != eqptoQUALIMETRO) continue;

		TQualimetro* qualimetro = (TQualimetro*) eqptoCampo;

      VTCarga* cargaAssociada = qualimetro->cargaAssociada;
      if(!cargaAssociada) continue;

		if(cargaAssociada->Codigo == carga->Codigo)
			return(true);
	}
	return(false);
}
//---------------------------------------------------------------------------
// Retorna as cargas associadas aos trafos inteligentes instalados na rede em questão.
void __fastcall TAreaBusca::GetCargasAssociadasTrafosIntel(TList* lisEXT)
{
	String codRedeMT, codCargaAssociada;
	TStringList* lisCadTrafosIntel;
	VTCarga* cargaAssociada;

	if(lisEXT == NULL)
		return;

	lisCadTrafosIntel = new TStringList();
	lisCadTrafosIntel->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroTrafosInteligentes.csv");

	for(int i=0; i<lisCadTrafosIntel->Count; i++)
	{
		String linha = lisCadTrafosIntel->Strings[i];
		codRedeMT = GetCampoCSV(linha, 0, ";");
		codCargaAssociada = GetCampoCSV(linha, 2, ";");

		if(ReplaceStr(codRedeMT, "-", "") != ReplaceStr(redeMT->Codigo, "-", "")) continue;

		cargaAssociada = funcoesRede->GetCargaStr(redeMT, codCargaAssociada);
		if(lisEXT->IndexOf(cargaAssociada) < 0)
			lisEXT->Add(cargaAssociada);
   }

   // Destroi lista auxiliar com o cadastro de trafos inteligentes
   if(lisCadTrafosIntel)
   {
      for(int i=lisCadTrafosIntel->Count-1; i>=0; i--) lisCadTrafosIntel->Delete(i);
      delete lisCadTrafosIntel; lisCadTrafosIntel = NULL;
   }
}
//---------------------------------------------------------------------------
// Retorna as chaves associadas aos qualímetros instalados na rede em questão.
void __fastcall TAreaBusca::GetChavesAssociadasQualimetros(TList* lisEXT)
{
	String codRedeMT,codChaveAssociada;
	TStringList* lisCadQualimetros;
	VTChave* chaveAssociada;

	if(lisEXT == NULL)
		return;

	lisCadQualimetros = new TStringList();
	lisCadQualimetros->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv");

	for(int i=0; i<lisCadQualimetros->Count; i++)
	{
		String linha = lisCadQualimetros->Strings[i];
		codRedeMT = GetCampoCSV(linha, 0, ";");
		codChaveAssociada = GetCampoCSV(linha, 2, ";");

		if(ReplaceStr(codRedeMT, "-", "") != ReplaceStr(redeMT->Codigo, "-", "")) continue;

		chaveAssociada = funcoesRede->GetChaveStr(redeMT, codChaveAssociada);
      if(!chaveAssociada) continue;

		if(lisEXT->IndexOf(chaveAssociada) < 0)
			lisEXT->Add(chaveAssociada);
   }

   // Destroi lista auxiliar
   if(lisCadQualimetros)
   {
      for(int i=lisCadQualimetros->Count-1; i>=0; i--) lisCadQualimetros->Delete(i);
      delete lisCadQualimetros; lisCadQualimetros = NULL;
   }
}
//---------------------------------------------------------------------------
// Retorna as cargas associadas aos qualímetros instalados na rede em questão.
void __fastcall TAreaBusca::GetCargasAssociadasQualimetros(TList* lisEXT)
{
	String codRedeMT,codCargaAssociada;
	TStringList* lisCadQualimetros;
	VTCarga* cargaAssociada;

	if(lisEXT == NULL)
		return;

	lisCadQualimetros = new TStringList();
	lisCadQualimetros->LoadFromFile(path->DirDat() + "\\FaultLocation\\CadastroQualimetros.csv");

	for(int i=0; i<lisCadQualimetros->Count; i++)
	{
		String linha = lisCadQualimetros->Strings[i];
		codRedeMT = GetCampoCSV(linha, 0, ";");
		codCargaAssociada = GetCampoCSV(linha, 2, ";");

		if(ReplaceStr(codRedeMT, "-", "") != ReplaceStr(redeMT->Codigo, "-", "")) continue;

		cargaAssociada = funcoesRede->GetCargaStr(redeMT, codCargaAssociada);
      if(!cargaAssociada) continue;

		if(lisEXT->IndexOf(cargaAssociada) < 0)
			lisEXT->Add(cargaAssociada);
   }

   // Destroi lista auxiliar
   if(lisCadQualimetros)
   {
      for(int i=lisCadQualimetros->Count-1; i>=0; i--) lisCadQualimetros->Delete(i);
      delete lisCadQualimetros; lisCadQualimetros = NULL;
   }
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::AjustaListaCargasJusante(TList* lisCargas, TList* lisCargasTrafosIntel)
{
	if(lisCargas == NULL || lisCargasTrafosIntel == NULL)
		return;

	// Remove os trafos inteligentes que não estão na lista das cargas
	for(int i=lisCargasTrafosIntel->Count-1; i>=0; i--)
	{
		VTEqpto* eqpto = (VTEqpto*) lisCargasTrafosIntel->Items[i];
		if(lisCargas->IndexOf(eqpto) < 0)
			lisCargasTrafosIntel->Remove(eqpto);
	}
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::AjustaListaChavesJusante(TList* lisChaves, TList* lisChavesQualimetros)
{
	if(lisChaves == NULL || lisChavesQualimetros == NULL)
		return;

	// remove as chaves que não estão na lista de todas as ligações
	for(int i=lisChavesQualimetros->Count-1; i>=0; i--)
	{
		VTEqpto* eqpto = (VTEqpto*) lisChavesQualimetros->Items[i];
		if(lisChaves->IndexOf(eqpto) < 0)
			lisChavesQualimetros->Remove(eqpto);
	}
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TAreaBusca::GetBarra_ChaveTrafoMontante(VTBarra* barraRef)
{
	if(barraRef == NULL) return NULL;
	VTBarra* barra_ChaveTrafoMont = funcoesRede->GetBarra_ChaveTrafoMontante(barraRef);
	return(barra_ChaveTrafoMont);
}
////---------------------------------------------------------------------------
//VTBarra* __fastcall TAreaBusca::GetBarraMontanteEqptoCampo(TEqptoCampo* eqptoCampo)
//{
//	TQualimetro* qualimetro;
//	VTBarra* barraMontante;
//
//	if(eqptoCampo == NULL) return NULL;
//
//	barraMontante = NULL;
//	if(eqptoCampo->GetTipo() == eqptoQUALIMETRO)
//	{
//		qualimetro = (TQualimetro*) eqptoCampo;
//		barraMontante = funcoesRede->GetBarraMontanteLigacao(qualimetro->GetLigacaoAssociada());
//	}
//
//	return(barraMontante);
//}//---------------------------------------------------------------------------
void __fastcall TAreaBusca::FiltrarBlocos_Religadoras()
{
	TList* lisChaves = NULL;
   TList* lisBlocosJusChave = NULL;
	VTChave* chave;

   lisChaves = new TList();
   lisBlocosJusChave = new TList();

   funcoesRede->GetChavesBlocos(lisBlocosPesquisa, lisChaves);

   for(int i=0; i<lisChaves->Count; i++)
	{
		chave = (VTChave*) lisChaves->Items[i];
      if(chave->TipoReligadora)
      {
      	lisBlocosJusChave->Clear();
         funcoesRede->GetBlocosJusanteLigacao(chave, lisBlocosJusChave);

         if(lisBlocosJusChave->Count == lisBlocosPesquisa->Count)
         	continue;

         RemoveSubconjunto(lisBlocosPesquisa, lisBlocosJusChave);
      }
   }

   if(lisChaves) {delete lisChaves; lisChaves = NULL;}
   if(lisBlocosJusChave) {delete lisBlocosJusChave; lisBlocosJusChave = NULL;}
}
//---------------------------------------------------------------------------
// Verificam-se duas situações:
// 	1) Se DJ ou RE determinou a área de busca, eliminamos dela os blocos à jusante de fusíveis
//    2) Se SR (FPE) determinou a área de busca, eliminamos dela os blocos à jusante de fusíveis
//    3) Se SR (AIM) determinou a área de busca, mantemos nela apenas os blocos à jusante de fusíveis
void __fastcall TAreaBusca::FiltrarBlocos_Fusiveis()
{
	TList* lisChaves = NULL;
   TList* lisBlocosJusChave = NULL;
	VTChave* chave;

   lisChaves = new TList();
   lisBlocosJusChave = new TList();

   funcoesRede->GetChavesBlocos(lisBlocosPesquisa, lisChaves);

   // Pega lista apenas com fusíveis
   for(int i=lisChaves->Count-1; i>=0; i--)
	{
      VTLigacao* liga = (VTLigacao*) lisChaves->Items[i];
      if(liga->Tipo() != eqptoCHAVE)
      {
         lisChaves->Remove(liga);
         continue;
      }

		chave = (VTChave*) liga;

      // Verifica se realmente se trata da rede MT em questão
      if(chave->rede != redeMT || chave->Aberta)
      {
         lisChaves->Remove(chave);
         continue;
      }

      lisBlocosJusChave->Clear();
      funcoesRede->GetBlocosJusanteLigacao(chave, lisBlocosJusChave);
      if(!chave->TipoBaseFusivel || lisBlocosPesquisa->Count == lisBlocosJusChave->Count)
      {
         lisChaves->Remove(chave);
         continue;
      }
   }

   // Verifica eqpto que determinou a área de busca
   if(eqptoCampo_DeterminaAreaBusca->GetTipo() == eqptoSENSOR)
   {
      TSensor* sensorAreaBusca = (TSensor*) eqptoCampo_DeterminaAreaBusca;
      if(sensorAreaBusca->faltaJusante)
      {
         MantemBlocosJusanteFusiveis(lisBlocosPesquisa, lisChaves);
      }
      else
      {
         RemoveBlocosJusanteFusiveis(lisBlocosPesquisa, lisChaves);
      }
   }
   else if(eqptoCampo_DeterminaAreaBusca->GetTipo() == chaveDJ || eqptoCampo_DeterminaAreaBusca->GetTipo() == chaveRE)
   {
      RemoveBlocosJusanteFusiveis(lisBlocosPesquisa, lisChaves);
   }

   if(lisChaves) {delete lisChaves; lisChaves = NULL;}
   if(lisBlocosJusChave) {delete lisBlocosJusChave; lisBlocosJusChave = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::MantemBlocosJusanteFusiveis(TList* lisBlocosPesquisa, TList* lisChaves)
{
	TList* lisAux = NULL;

	if(!lisBlocosPesquisa || !lisChaves)
   	return;

   // Lista auxiliar com os blocos de pesquisa
   lisAux = new TList();
   CopiaTList(lisBlocosPesquisa, lisAux);
   // Remove da lista auxiliar os blocos à jusante de fusíveis
   RemoveBlocosJusanteFusiveis(lisAux, lisChaves);
	// Remove a lista auxiliar da lista original de blocos de pesquisa
   RemoveSubconjunto(lisBlocosPesquisa, lisAux);

   if(lisAux) {delete lisAux; lisAux = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::RemoveBlocosJusanteFusiveis(TList* lisBlocosPesquisa, TList* lisChaves)
{
   TList* lisBlocosJusChave = NULL;

   if(!lisBlocosPesquisa || !lisChaves)
   	return;

   lisBlocosJusChave = new TList();
   for(int i=0; i<lisChaves->Count; i++)
   {
		VTChave* chave = (VTChave*) lisChaves->Items[i];

      lisBlocosJusChave->Clear();
      funcoesRede->GetBlocosJusanteLigacao(chave, lisBlocosJusChave);
      RemoveSubconjunto(lisBlocosPesquisa, lisBlocosJusChave);
   }

   if(lisBlocosJusChave) {delete lisBlocosJusChave; lisBlocosJusChave = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Verifica se eqpto de monitoramento já foi criado
 */
bool __fastcall TAreaBusca::ExisteEqptoCampo(int Tipo, String Codigo, TList* lista)
{
   // Verifica se o eqpto já existe na lista de eqptos de campo
   for(int i=0; i<lista->Count; i++)
   {
      TEqptoCampo* eqptoCampo = (TEqptoCampo*) lista->Items[i];
      if(eqptoCampo->GetTipo() != Tipo) continue;

      // Se o eqpto já existe, termina a procura
      if(eqptoCampo->GetCodigo() == Codigo)
      	return true;
   }

   return false;
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::GetAreaBusca_Barras(TList* listaExt)
{
	String CodigoBarra;

	// Proteção
	if(listaExt == NULL) return;

	// Limpa lista externa
	listaExt->Clear();

	for(int i=0; i<lisBlocosPesquisa->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosPesquisa->Items[i];
		// Lista de barras do bloco
		TList* listaBarras = bloco->LisBarra();
		for(int ib=0; ib<listaBarras->Count; ib++)
		{
			VTBarra* barra = (VTBarra*) listaBarras->Items[ib];
			if(listaExt->IndexOf(barra) < 0)
				listaExt->Add(barra);
		}
	}
}
//---------------------------------------------------------------------------
// Retorna os trechos da área de busca
void __fastcall TAreaBusca::GetAreaBuscaRompCabo_Trechos(TList* listaExt)
{
	// Proteção
	if(listaExt == NULL) return;

   // Limpa lista externa
	listaExt->Clear();

	for(int i=0; i<lisLigacoesAreaBusca_RompCabo->Count; i++)
	{
		VTLigacao* liga = (VTLigacao*) lisLigacoesAreaBusca_RompCabo->Items[i];
		if(liga->Tipo() != eqptoTRECHO) continue;

		VTTrecho* trecho = (VTTrecho*) liga;
		if(listaExt->IndexOf(trecho) < 0)
			listaExt->Add(trecho);
	}
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::GetAreaBusca_Blocos(TList* listaExt)
{
	// Proteção
	if(listaExt == NULL) return;

   for(int i=0; i<lisBlocosPesquisa->Count; i++)
   {
		VTBloco* bloco = (VTBloco*) lisBlocosPesquisa->Items[i];
      if(listaExt->IndexOf(bloco) < 0)
	      listaExt->Add(bloco);
   }
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::GetAreaBusca_DMS2(TList* listaExt)
{
	if(!listaExt) return;
   for(int i=0; i<lisBlocosPesquisa->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosPesquisa->Items[i];
		VTChave* chave = funcoesRede->GetChaveMontante(bloco);
		if(!chave) continue;

		// Cria objeto de solução DMS2, passando o bloco e a chave pai
		StrSolucaoDMS2* solucaoDMS2 = new StrSolucaoDMS2;
		solucaoDMS2->bloco = bloco;
		solucaoDMS2->chaveMontante = chave;

		// Acrescenta os trechos do bloco ao objeto
		TList* lisLigacoes = bloco->LisLigacao();
		for(int j=0; j<lisLigacoes->Count; j++)
		{
			VTLigacao* liga = (VTLigacao*) lisLigacoes->Items[j];
			if(liga->Tipo() != eqptoTRECHO) continue;
			solucaoDMS2->lisTrechosBloco->Add((VTTrecho*)liga);
		}

		listaExt->Add(solucaoDMS2);
	}
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::GetAreaBusca_CodChavesBlocos(TStringList* listaExt)
{
   VTChave* chave;

	if(!listaExt)
		return;

	for(int i=0; i<lisBlocosPesquisa->Count; i++)
	{
		VTBloco* bloco = (VTBloco*) lisBlocosPesquisa->Items[i];
		chave = funcoesRede->GetChaveMontante(bloco);
		if(chave)
			listaExt->Add(chave->Codigo);
	}
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TAreaBusca::GetBarra(int Id)
{
	for(int i=0; i<redeMT->LisBarra()->Count; i++)
   {
    	VTBarra* barra = (VTBarra*) redeMT->LisBarra()->Items[i];
      if(barra->Id == Id)
      	return barra;
   }
   return NULL;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TAreaBusca::RedeBarra(VTBarra* barra)
{
	for(int i=0; i<redes->LisRede()->Count; i++)
   {
    	VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->ExisteBarra(barra))
         return(rede);
   }
   return(NULL);
}
//---------------------------------------------------------------------------
VTBarra* __fastcall TAreaBusca::GetBarraMontante()
{
	VTBarra* barraMontante;
   barraMontante = funcoesRede->GetBarraMontanteArea(lisBlocosPesquisa);
   return(barraMontante);
}
//---------------------------------------------------------------------------
TBlocosRedeRadial* __fastcall TAreaBusca::GetBlocosRedeRadial(VTRede* rede)
{
	TBlocosRedeRadial* blocosRedeRad;

	if(rede == NULL) return NULL;

	// Verifica se já existe um objeto TBlocosRedeRadial associado à rede de interesse
	blocosRedeRad = NULL;
	for(int i=0; i<lisBlocosRedeRadial->Count; i++)
	{
		blocosRedeRad = (TBlocosRedeRadial*) lisBlocosRedeRadial->Items[i];
		if(blocosRedeRad->rede == rede)
			break;
		else
			blocosRedeRad = NULL;
	}

	// Se o objeto de TBlocosRedeRadial não existe, cria um
	if(blocosRedeRad == NULL)
	{
		blocosRedeRad = new TBlocosRedeRadial(apl, redes, rede);
		lisBlocosRedeRadial->Add(blocosRedeRad);
	}
	return(blocosRedeRad);
}
//---------------------------------------------------------------------------
TList* __fastcall TAreaBusca::GetLisClusteres()
{
	return lisClusteres;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TAreaBusca::GetRedeMT()
{
   // Retorna referência para a rede MT em questão
   return redeMT;
}
//---------------------------------------------------------------------------
VTRede* __fastcall TAreaBusca::GetRedeSE()
{
   // Retorna referência para a rede da subestação associada à redeMT em questão
   return redeSE;
}
//---------------------------------------------------------------------------
/***
 * Retorna CSV: códTrecho;IDbarra1;IDbarra2;xLocal(%);
 */
String __fastcall TAreaBusca::GetTrechoBarra1Barra2LocalDiscret(double xPorc)
{
	double xLocal;
	String strFinal = "";

	for(int i=0; i<lisTrechosDiscret->Count; i++)
   {
		TTrechoDiscret* trechoDiscret = (TTrechoDiscret*) lisTrechosDiscret->Items[i];

      if(trechoDiscret->xIni < xPorc && xPorc <= trechoDiscret->xFin)
      {
         strFinal = trechoDiscret->trecho->Codigo + ";";
         strFinal += String(trechoDiscret->trecho->Barra(0)->Id) + ";";
         strFinal += String(trechoDiscret->trecho->Barra(1)->Id) + ";";
         xLocal = 100. * (xPorc - trechoDiscret->xIni) / (trechoDiscret->xFin - trechoDiscret->xIni);
         strFinal += String(xLocal) + ";";

         break;
      }
   }

   return strFinal;
}
//---------------------------------------------------------------------------
/***
 * Retorna o código do trecho que contém um valor de x (%)
 */
String __fastcall TAreaBusca::GetTrechoDiscret(double xPorc)
{
	for(int i=0; i<lisTrechosDiscret->Count; i++)
   {
		TTrechoDiscret* trechoDiscret = (TTrechoDiscret*) lisTrechosDiscret->Items[i];

      if(trechoDiscret->xIni < xPorc && xPorc <= trechoDiscret->xFin)
         return trechoDiscret->trecho->Codigo;
   }

   return "";
}
//---------------------------------------------------------------------------
/***
 * Retorna CSV: códTrecho;xLocal(%);
 */
String __fastcall TAreaBusca::GetTrechoLocalDiscret(double xPorc)
{
	double xLocal;
	String strFinal = "";

	for(int i=0; i<lisTrechosDiscret->Count; i++)
   {
		TTrechoDiscret* trechoDiscret = (TTrechoDiscret*) lisTrechosDiscret->Items[i];

      if(trechoDiscret->xIni < xPorc && xPorc <= trechoDiscret->xFin)
      {
         strFinal = trechoDiscret->trecho->Codigo + ";";
         xLocal = 100. * (xPorc - trechoDiscret->xIni) / (trechoDiscret->xFin - trechoDiscret->xIni);
         strFinal += String(xLocal) + ";";

         break;
      }
   }

   return strFinal;
}
//---------------------------------------------------------------------------
/***
 * Início do algoritmo de determinação da área de busca
 *   1) Obtém a rede MT afetada
 *   2) Inicializa blocos à jusante dos eqptos de dados
 */
void __fastcall TAreaBusca::Inicializa()
{
	TChaveMonitorada* chaveMon = NULL;
   TList* lisBlocosJusante = NULL;
   VTChave* chave = NULL;
	VTLigacao* ligacao = NULL;

	// Pega lista de eqptos de campo sensibilizados
	listaEqptosCampo = dados->GetEqptosCampo();

   for(int i=0; i<listaEqptosCampo->Count; i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];

      switch(eqptoCampo->GetTipo())
      {
			case chaveDJ:
         case chaveRE:
         case chaveSC:
				chaveMon = (TChaveMonitorada*) eqptoCampo;
				chave = chaveMon->GetChaveAssociada();

            if(!chave) continue;

				// Obtém ponteiro para a rede MT em questão
				if(redeMT == NULL)
					redeMT = chave->rede;

				// Obtém os blocos à jusante do eqpto
				lisBlocosJusante = new TList();
				funcoesRede->GetBlocosJusanteLigacao(chave, lisBlocosJusante);

            // Salva ponteiro para a lista de blocos à jusante da chave monitorada
            chaveMon->SetBlocosJusante(lisBlocosJusante);
				break;

			default:
				break;
      }
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
VTRede* __fastcall TAreaBusca::RedeCarga(VTCarga* carga)
{
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
      VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
      if(rede->TipoRede->Segmento != redePRI || !rede->Carregada) continue;
      if(rede->ExisteBarra(carga->pbarra))
         return(rede);
   }
   return(NULL);
}
//---------------------------------------------------------------------------
/***
 * Método para inicializar configurações do objeto Área de Busca
 */
void __fastcall TAreaBusca::IniciaConfigs()
{
   String pathConfigGerais = path->DirDat() + "\\FaultLocation\\ConfigGerais.ini";
	TIniFile* file = new TIniFile(pathConfigGerais);
	ConsiderarCoordenacaoProtecao = file->ReadBool("PROTECAO", "ConsiderarCoordenacaoProtecao", 0);

	delete file; file = NULL;
}
//---------------------------------------------------------------------------
/***
 * 1) Se sensor dentro da área de busca não foi sensibilizado, remove seus blocos
 */
void __fastcall TAreaBusca::RefinamentoBlocosJusante_Sensores()
{
   TList* listaLigacoes;
	TList* listaChaves;
	bool ConsideraQualidadeSensor = configuracoes->GetConsiderarQualidadeSensor();

 	// Lista de ligações dos blocos de pesquisa
	listaLigacoes = new TList();
	for(int i=0; i<lisBlocosPesquisa->Count; i++)
   {
		VTBloco* bloco = (VTBloco*) lisBlocosPesquisa->Items[i];
      TList* lisLiga = bloco->LisLigacao();
      CopiaTList(lisLiga, listaLigacoes);
   }
   // Acrescenta as chaves relativas ao blocos
   listaChaves = new TList();
	funcoesRede->GetChavesBlocos(lisBlocosPesquisa, listaChaves);
   CopiaTList(listaChaves, listaLigacoes);

   // Verifica sensores que não foram sensibilizados
   for(int i=0; i<listaEqptosCampo->Count; i++)
   {
		TEqptoCampo* eqptoCampo = (TEqptoCampo*) listaEqptosCampo->Items[i];
      if(eqptoCampo->GetTipo() != eqptoSENSOR) continue;

      TSensor* eqptoSensor = (TSensor*) eqptoCampo;

      // Verifica se as medições das correntes de fase são todas nulas
      if(!(Nulo(eqptoSensor->medicaoI.falta.I[0]) && Nulo(eqptoSensor->medicaoI.falta.I[1]) && Nulo(eqptoSensor->medicaoI.falta.I[2])))
      	continue;

      // Verifica ainda a qualidade dos dados do sensor
      if(ConsideraQualidadeSensor && !eqptoSensor->qualidadeOK)
      	continue;

      VTLigacao* ligacaoSensor = eqptoSensor->GetLigacaoAssociada();

      // Se a área de busca contém a ligação do sensor, ignora seus blocos à jusante
      if(listaLigacoes->IndexOf(ligacaoSensor) >= 0)
      {
      	TList* listaBlocosJus = eqptoSensor->GetBlocosJusante_SemBlocoSensor();
         if(Subconjunto(lisBlocosPesquisa, listaBlocosJus))
         {
            RemoveSubconjunto(lisBlocosPesquisa, listaBlocosJus);
         }
      }
   }

	// Destroi listas
	delete listaLigacoes; listaLigacoes = NULL;
	delete listaChaves; listaChaves = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TAreaBusca::SetConfiguracoes(TConfiguracoes* configuracoes)
{
	this->configuracoes = configuracoes;
}
//---------------------------------------------------------------------------
/***
 * Remove itens da listaMenor na listaMaior
 */
void __fastcall TAreaBusca::RemoveSubconjunto(TList* listaMaior, TList* listaMenor)
{
	for(int i=0; i<listaMenor->Count; i++)
   {
		void* listItem = listaMenor->Items[i];
      listaMaior->Remove(listItem);
   }
}
//---------------------------------------------------------------------------
/***
 * Verifica se listaMenor está contida em listaMaior
 */
bool __fastcall TAreaBusca::Subconjunto(TList* listaMaior, TList* listaMenor)
{

	for(int i=0; i<listaMenor->Count; i++)
	{
		void* listItem = listaMenor->Items[i];

		if(listaMaior->IndexOf(listItem) < 0) return false;
	}

	return true;
}
//---------------------------------------------------------------------------
//==========================  TTrechoDiscret ==================================
//---------------------------------------------------------------------------
__fastcall TTrechoDiscret::TTrechoDiscret(VTTrecho* trecho)
{
	// Inicializa objeto
	this->trecho = trecho;
   this->xIni = 0.;
	this->xFin = 0.;
}
//---------------------------------------------------------------------------
void __fastcall TTrechoDiscret::SetXiniXfin(double xIni, double xFin)
{
   this->xIni = xIni;
	this->xFin = xFin;
}
//---------------------------------------------------------------------------
