//---------------------------------------------------------------------------
#ifndef TFuncoesDeRedeH
#define TFuncoesDeRedeH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TCluster;
class VTApl;
class VTBarra;
class VTBloco;
class VTBlocos;
class VTCarga;
class VTChave;
class VTLigacao;
class VTOrdena;
class VTPath;
class VTRede;
class VTRedes;
class VTTrecho;
//---------------------------------------------------------------------------
class TBifurcacao
{
public:
   // Construtor e destrutor
	__fastcall TBifurcacao(VTLigacao* ligaPai);
	__fastcall ~TBifurcacao();

   // Métodos
	void __fastcall SetCaminho(TList* lisCaminho);
   void __fastcall SetCaminhos(TList* lisCaminhos);
   void __fastcall SetLigaIni(VTLigacao* ligaIni);
	TList* __fastcall GetLigaIni();

   // Dados
	VTLigacao* ligaPai;
   TList* lisCaminhos;
   TList* lisLigaIni;
};
//---------------------------------------------------------------------------
class TBloco_Radial
{
public:
   // Construtor e destrutor
	__fastcall TBloco_Radial(VTBloco* bloco);
	__fastcall ~TBloco_Radial();

   // Métodos
//	void __fastcall SetCaminho(TList* lisCaminho);


   // Dados
	VTBloco* bloco;
   VTChave* chvMont;
   TList* lisBlocosRadJus;
	TBloco_Radial* BlocoRadMont;
};
//---------------------------------------------------------------------------
class TFuncoesDeRede : public TObject
{
public:

	// Construtor e destrutor
	__fastcall TFuncoesDeRede(VTApl* apl);
	__fastcall ~TFuncoesDeRede(void);

   // Métodos
   bool __fastcall AreaTemBarra(TList* lisBlocos, VTBarra* barra);
   bool __fastcall AreaTemLigacao(TList* lisBlocos, VTLigacao* ligacao);
   TBifurcacao* __fastcall Bifurcacao(VTLigacao* ligaRef);
	bool __fastcall ExisteBifurcacao(TList* lisFilhas, double PorcMinLigacoes, TList* lisCaminhos);
   bool __fastcall ExisteCaminhoLista(TList* listaExt, TList* caminho);
   void __fastcall FiltraClusteres(TList* lisBlocosAreaBusca, TList* lisClusteresOrdenados);
	void __fastcall GetBarrasBlocos(TList* listaBlocos, TList* listaExt);
	VTBarra* __fastcall GetBarra_ChaveTrafoMontante(VTBarra* barraRef);
	VTBarra* __fastcall GetBarraMontanteArea(TList* lisBlocos);
	VTBarra* __fastcall GetBarraMontanteLigacao(VTLigacao* liga);
   void __fastcall GetBarrasDistancia(String codBarraRef, double Dist, TList* listaExt); //< Lista de barras que distam L=Dist da barra de ref.
   void __fastcall GetBarrasDistancia(VTRede* redeMT, String codBarraRef, double Dist, TList* listaExt); //< Lista de barras que distam L=Dist da barra de ref.
   void __fastcall GetBifurcacoesBlocos(TList* lisBlocos, TList* listaExt);
	void __fastcall GetBlocosRede(VTRede* rede, TList* listaExt);
   void __fastcall GetBlocosRede(String CodigoAlimentador, TList* listaExt);
	void __fastcall GetBlocosJusanteLigacao(VTLigacao* ligacaoRef, TList* lisExt);
	void __fastcall GetBlocosJusanteBarra(VTRede* redeMT, VTBarra* barraRef, TList* lisExt);
	VTBloco* __fastcall GetBlocoAssociadoCarga(VTCarga* carga);
	VTBloco* __fastcall GetBlocoAssociadoChave(VTChave* chave);
   void __fastcall GetCaminhoLigacoes_BarraRef_Barra(VTRede* rede, VTBarra* barra0, VTBarra* barraRef, TList* lisExt);
	void __fastcall GetCaminhoBarrasSE_Barra(VTRede* rede, VTBarra* barraRef, TList* lisExt);
   void __fastcall GetClusteres(TList* lisBlocos, TList* lisEXT);
	void __fastcall GetCaminhoLigacoesSE_Barra(VTRede* rede, VTBarra* barraRef, TList* lisExt);
	void __fastcall GetCaminhoComprimento(TList* caminho, double Dist);
	void __fastcall GetCaminhosDistancia(VTBarra* barra, double Dist, TList* listaExt);
 	void __fastcall GetCaminhosDistancia(String codBarra, double Dist, TList* listaExt);  //< Caminho, a partir de barra, com L=Dist.
 	void __fastcall GetCaminhosDistancia(VTRede* redeMT, String codBarra, double Dist, TList* listaExt);  //< Caminho, a partir de barra, com L=Dist.
   void __fastcall GetCargasBlocosRede(VTRede* redeRef, int numCargasPorBloco, TStringList* lisCodCargasMT);
   void __fastcall GetCargasMT(TStringList* lisCodEqptosCampo, int numCargasPorBloco, TStringList* lisCodCargasMT);
   void __fastcall GetCargasJusanteBarra(VTRede* redeRef, VTBarra* barraRef, TList* lisEXT);
   void __fastcall GetCargasJusanteLigacao(VTLigacao* ligaRef, TList* lisEXT);
	void __fastcall GetChavesBlocos(TList* listaBlocos, TList* listaExt);
	void __fastcall GetChavesJusanteLigacao(VTLigacao* liga, TList* listaChavesJusLiga);
	void __fastcall GetChavesJusanteBarra(VTRede* redeRef, VTBarra* barraRef, TList* listaChavesJusLiga);
	VTChave* __fastcall GetChaveMontante(String CodChaveRef);
	VTChave* __fastcall GetChaveMontante(VTBarra* barraRef);
	VTChave* __fastcall GetChaveMontante(VTBloco* blocoRef);
   double __fastcall GetDistanciaMetros(VTBarra* barra1, VTBarra* barra2);
	double __fastcall GetDistancia_KM_DaSubestacao(VTBarra* barraRef);

	String __fastcall GetFases(VTRede* redeMT, VTBarra* barra);

   VTLigacao* __fastcall GetLigacaoMontanteArea(TList* lisBlocos);
	VTLigacao* __fastcall GetLigacaoMontanteArea(VTRede* redeRef, TList* lisBlocos);
	void __fastcall GetLigacoesBlocos(TList* listaBlocos, TList* listaExt);
	void __fastcall GetLigaFilhas(VTLigacao* ligacao, TList* lisFilhas);
	void __fastcall GetLigacoesJusanteLigacao(VTLigacao* ligacaoRef, TList* lisExt);
	void __fastcall GetLigacoesJusanteBarra(VTBarra* barraRef, TList* lisExt);
	VTLigacao* __fastcall GetLigacaoJusanteBarra(VTBarra* barraRef, VTRede* rede);
	VTTrecho* __fastcall GetTrechoJusanteBarra(VTBarra* barraRef, VTRede* rede);
	VTTrecho* __fastcall GetTrechoMontanteBarra(VTBarra* barraRef, VTRede* rede);
	VTTrecho* __fastcall GetTrechoJusanteLigacao(VTLigacao* ligaRef, VTRede* rede);
	VTLigacao* __fastcall GetLigacaoFilha(VTLigacao* ligacaoRef, VTRede* rede);
	void __fastcall GetLigacoesJusanteLigacao(String codLigacaoRef, TStringList* lisExt);
	void __fastcall GetLigacoesOrdenadas_JusanteLigacao(VTLigacao* ligacaoRef, TList* lisExt);
	void __fastcall GetLigacoesTerminais(VTRede* rede, TList* listaExt);
	int  __fastcall GetNumConsJusLigacao(String CodLigacao);
   void __fastcall GetSensoresJusanteChave(String CodChaveRef, TStringList* lisCadSensores, TStringList* lisSensoresJusante);

	void __fastcall GetChaves(TStringList* lisCodChaves, TList* lisEXT);
	void __fastcall GetTrechos(TStringList* lisCodTrechos, TList* lisEXT);
	VTLigacao* __fastcall GetLigacaoMontanteBarra(VTBarra* barraRef);

	bool __fastcall LigacaoJusante(VTLigacao* ligaMont, VTLigacao* ligaJus);

	void __fastcall OrdCaminhosMaiorMenor(TList* lisCaminhos);
   void __fastcall OrdenaBlocosClusteres(TList* lisClusteres);
	void __fastcall OrdenaBlocosCluster(TCluster* cluster);
	void __fastcall OrdenaClusteres(TList* lisClusteres);
	void __fastcall RemoveCaminhosSemelhantes(TList* lisCaminhos, double MaxPorcLigacoesIguais);
   void __fastcall RessetaParametros();

   void __fastcall SetRedeMT(VTRede* redeRef);
	VTRede* __fastcall GetRede(String codEqpto);
	VTRede* __fastcall GetRede_CodRede(String CodRede);


   void __fastcall GetListBlocosRadiais(TList* lisBlocos, TList* lisExt);
   void __fastcall GetLisChavesEntreBlocos(VTRede* rede, TList* lisBlocos, TList* lisExt);
   VTChave* __fastcall GetChaveStr(VTRede* rede, String CodChave);
   VTCarga* __fastcall GetCargaStr(VTRede* rede, String CodCarga);
	TBloco_Radial* __fastcall GetBloco_Radial(TList* lisBloRad, VTBloco* bloco);
	void __fastcall GetChavesIniciaisClusteres(TList* lisExt);

   void __fastcall GetLigacoesCluster(TCluster* cluster, TList* lisExt);
	bool __fastcall ExisteEloComBlocoExistente(TList* lisBloExistentes, VTBloco* bloco);

	// Obtém as barras com certa impedância acumulada, em relação à barra inicial do alimentador
	double __fastcall CalcImpedanciaTrechos(VTBarra* barra1, VTBarra* barra2, double &r0Total, double &r1Total, double &x0Total, double &x1Total);
	void __fastcall GetBarras_CaminhoReatanciaFT(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido, double maxErroX, TList* lisEXT);
	void __fastcall GetBarras_CaminhoReatancia3F(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido, double maxErroX, TList* lisEXT);
	void __fastcall GetBarras_CaminhoReatancia2FT(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT);
	void __fastcall GetBarras_CaminhoReatancia2F(VTRede* redeSE, VTRede* redeMT, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT);

	// Obtém as barras com certa impedância acumulada, em relação a uma barra de referência
	void __fastcall GetBarras_CaminhoReatanciaFT(VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido, double maxErroX, TList* lisEXT);
	void __fastcall GetBarras_CaminhoReatancia3F(VTRede* redeSE, VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido, double maxErroX, TList* lisEXT);
	void __fastcall GetBarras_CaminhoReatancia2FT(VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT);
	void __fastcall GetBarras_CaminhoReatancia2F(VTRede* redeMT, VTBarra* barraRef, double Xtotal_medido_seq1, double maxErroX, TList* lisEXT);


	String __fastcall GetCodChaveSinap(String codEqptoSage, TStringList* lisCadDisjuntores);

	bool __fastcall VerificaBifurcacao(VTLigacao* ligaRef, TList* lisLigacoesRede);
	bool __fastcall TemCarga(VTBarra* barra);

	int __fastcall NumeroConsJusLigacao_SolucoesRompCabo(String CodLigacao);
	void __fastcall BlocosJusanteLigacao_RompCabo(VTLigacao* ligacaoRef, TList* lisExt);

private:

	// Parâmetros básicos
	TList*    listaBlocosRede;
	TList*    listaBlocosRede_RompCabo;
	VTApl*    apl;
	VTBlocos* blocos;
   VTOrdena* ordena;
	VTPath*   path;
	VTRede*   redeRef;
	VTRede*   rede_RompCabo;
	VTRedes*  redes;

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
