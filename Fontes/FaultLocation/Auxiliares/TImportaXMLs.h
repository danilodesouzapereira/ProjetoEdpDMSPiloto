//---------------------------------------------------------------------------
#ifndef TImportaXMLsH
#define TImportaXMLsH
//---------------------------------------------------------------------------
#include <complex>
#include <cmath>
#include <IniFiles.hpp>
#include <System.Classes.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
struct StrDadosLog;
struct StrReqResponse;
//---------------------------------------------------------------------------
class TLog;
class TXMLComunicacao;
class VTApl;
class VTPath;
//---------------------------------------------------------------------------
class TImportaXMLs
{

private:
	// Dados elementares
   TForm*  formFL;
   TLog*   logErros;
   VTApl*  apl;
   VTPath* path;

   // Dados
   String CodigoEvento;
   String pathArqIni;        //< Caminho para o diretório com os arquivos INI formatados (dados de campo)
   String pathArqBarramento; //< Caminho para o diretório com os arquivos do barramento
//   String pathArqCadastro; //< Caminho para o arquivo com o cadastro dos monitores da rede
//   TStringList* listaArqBarramento; //< Lista com os nomes dos arquivos XML do barramento
	TList* listaAlarmes;     //< Lista com todos os alarmes (XML) associados à Thread de LocFaltas
   TList* lisResponses;     //< Lista com as responses do módulo de supervisão

public:

	// Contrutor e destrutor
   __fastcall TImportaXMLs(VTApl* apl, TForm* formFL, TList* listaAlarmes);
   __fastcall ~TImportaXMLs(void);


	// Atualização dos dados
	void __fastcall AddDadosMedidorInteligente(String CodigoMedidor, String timestamp, std::complex<double>* Vposfalta);
	void __fastcall AddDadosDisjuntor(String CodigoDisjuntor, String timestamp, std::complex<double>* V, std::complex<double>* I, double DistFalta, String TipoAtuacao);
	void __fastcall AddDadosReligadora(String CodigoDisjuntor, String timestamp, std::complex<double>* V, std::complex<double>* I, String TipoAtuacao);
   void __fastcall AddDadosSensor(String CodigoSensor, String timestamp, std::complex<double>* Ifalta, std::complex<double>* Ifalta_jusante, String Qualidade);
   void __fastcall AddDadosQualimetro(String CodigoQualimetro, String timestamp, std::complex<double>* V, std::complex<double>* I);
   void __fastcall AddDadosTrafoInteligente(String CodigoTrafoInteligente, String timestamp, std::complex<double>* V);

	void __fastcall AddGeral(String TipoEqpto, String CodigoEqpto);
	void __fastcall ConverteArquivosXML();
	void __fastcall GeraBaseDados();
	void __fastcall SetLogErros(TLog* logErros);

   void __fastcall LeValidaDadosXML(String nomeXMLresponses, TXMLComunicacao* xmlRequest, StrDadosLog* strLog);  //< Lê e valida os dados que chegaram via XML


private:
   void __fastcall AjustaCorrentesMedicao();
	bool __fastcall AtuacaoProtecao(StrReqResponse* resp);
	void __fastcall DeterminaAtuacaoProtecao();
	void __fastcall GeraINIFormatados();
   void __fastcall ValidaDados();

	void __fastcall GeraINIFormatado_MedidorInteligente(StrReqResponse* resp);
	void __fastcall GeraINIFormatado_Disjuntor(StrReqResponse* resp);
	void __fastcall GeraINIFormatado_Religadora(StrReqResponse* resp);
   void __fastcall GeraINIFormatado_Sensor(StrReqResponse* resp);
	void __fastcall GeraINIFormatado_Qualimetro(StrReqResponse* resp);
   void __fastcall GeraINIFormatado_TrafoInteligente(StrReqResponse* resp);
	void __fastcall InsereDadosAdicionais(StrDadosLog* strLog);
	void __fastcall InsereInsumosDados(StrDadosLog* strLog);

   void __fastcall AvaliaErrosAquisicao(StrDadosLog* strLog);
	bool __fastcall FaltaJusante(String CodigoSensor);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//eof
