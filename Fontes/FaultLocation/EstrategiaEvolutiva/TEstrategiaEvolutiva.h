//---------------------------------------------------------------------------
#ifndef TEstrategiaEvolutivaH
#define TEstrategiaEvolutivaH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
class TAreaBusca;
class TClassificacao;
class TConfiguracoes;
class TDados;
class TFormFaultLocation;
class TFormProgressBar;
class TFuncoesDeRede;
class TIndividuoEE;
class TLog;
class TSorteio;
class TTryFault;
class VTBarra;
//---------------------------------------------------------------------------
class TEstrategiaEvolutiva
{
private:
	// Dados elementares
   int                 ultimoID;
   String              pathConfiguracoes;
   TAreaBusca*         areaBusca;
	TClassificacao*     classificacao;
   TConfiguracoes*     config;   //< Parâmetros de config do algo de EE
   TDados*             dados;    //< Conjunto dos dados acerca do defeito
   TFormFaultLocation* formFL;   //< Referência para o form de Fault Location
   TTryFault*          tryFault; //< objeto para testar um curto-circuito

   // Dados de operação
   TSorteio* sorteio;
   TList* lisIndividuos;
   TList* lisMelhoresIndividuosClusteres;  //< Lista para os melhores indivíduos dos clusteres
   TList* lisNovosIndividuosMutacao;       //< Lista para os novos indivíduos, gerados por mutação
   TList* lisNovosIndividuosCruzamento;    //< Lista para os novos indivíduos, gerados por cruzamento

   // Dados auxiliares
   String strTipoFalta;
   TFormProgressBar* FormPG;               //< Form para visualização do processo de otimização
   TLog* log;

public:
	// Construtores e destrutor
   __fastcall TEstrategiaEvolutiva();
   __fastcall TEstrategiaEvolutiva(TConfiguracoes* config, TDados* dados);
   __fastcall ~TEstrategiaEvolutiva();

   // Métodos de operação
	void __fastcall AtribuiIDsIndividuos();
	void __fastcall AvaliaIndividuos(TList* lisIndiv, bool Inicial = false);
   void __fastcall Executa();
   void __fastcall ExecutaEvolutivo();
	TAreaBusca* __fastcall GetAreaBusca();
	void __fastcall GetSolucoes(TFuncoesDeRede* funcoesRede, TList* lisEXT);
	void __fastcall RemoveIndivImproprios();
	void __fastcall SelecionaMelhoresIndividuos();
	void __fastcall VerificaConsistenciaFilhoCruzamento(TIndividuoEE* filho);
	void __fastcall VerificaConsistenciaFilhosMutacao(TList* filhosMutacao);
	bool __fastcall VerificaCriterioParada();

   // Métodos de Preparação
   void __fastcall GeraIndividuosIniciais();
	void __fastcall IniciaTryFault(String caminhoDSS);
	void __fastcall SetParametros(TConfiguracoes* config,
                                 TClassificacao* classificacao,
                                 TAreaBusca* areaBusca,
                                 TDados* dados,
                                 TFormFaultLocation* formFL);

private:
	// Métodos chamados internamente
   void __fastcall OrdenaMelhoresIndividuos();
	void __fastcall SalvaMelhorIndividuoCluster();
   void __fastcall SetAreaBusca(TAreaBusca* areaBusca);
	void __fastcall SetClassificacao(TClassificacao* classificacao);
   void __fastcall SetConfiguracoes(TConfiguracoes* config);
   void __fastcall SetDados(TDados* dados);
	void __fastcall SetFormFL(TFormFaultLocation* formFL);

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
