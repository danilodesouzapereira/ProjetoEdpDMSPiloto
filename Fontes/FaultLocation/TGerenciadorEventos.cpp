//---------------------------------------------------------------------------
#pragma hdrstop
#include <Windows.h>
#include <vector>
#include "TGerenciadorEventos.h"
#include "TFormFaultLocation.h"
#include "TThreadFaultLocation.h"
#include "Auxiliares\FuncoesFL.h"
#include "ComunicacaoXML\TAlarme.h"
#include "ComunicacaoXML\TXMLParser.h"
#include "ComunicacaoXML\TXMLComunicacao.h"
//---------------------------------------------------------------------------
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
TDateTime __fastcall GetTimeStamp(TAlarme* alarme)
{
   unsigned short ano, mes, dia, hora, minuto, segundo, mseg;
   String str;

	String strTimeStamp = alarme->GetTimeStamp();
	str = strTimeStamp.SubString(1,4); ano = str.ToInt();
	str = strTimeStamp.SubString(5,2); mes = str.ToInt();
	str = strTimeStamp.SubString(7,2); dia = str.ToInt();
	str = strTimeStamp.SubString(9,2); hora = str.ToInt();
	str = strTimeStamp.SubString(11,2); minuto = str.ToInt();
	str = strTimeStamp.SubString(13,2); segundo = str.ToInt();
	mseg = 0;

	TDateTime timeStamp = TDateTime(ano, mes, dia, hora, minuto, segundo, mseg);

   return timeStamp;
}
//---------------------------------------------------------------------------
__fastcall TGerenciadorEventos::TGerenciadorEventos(VTApl* apl, TFormFaultLocation* formFL)
{
	// Salva ponteiros
   this->formFL = formFL;
   this->apl = apl;
   path = (VTPath*) apl->GetObject(__classid(VTPath));
	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));

   // Referência para a thread em execução
   FL_Executando = NULL;

   // Inicializa flag
   AguardandoSensores = false;

   // Inicializa listas
   listaAlarmes = new TList();
   listaFL = new TList();
   listaArquivosAdicionados = new TStringList();
   listaArquivosNovos = new TStringList();
	listaXMLCadSensores = new TStringList();

   // Cria obj de parser do XML de alarme
   xmlParser = new TXMLParser(apl, formFL);

   // Resseta diretórios
   ResetDir();

   // Lê configurações gerais do gerenciador
   LeConfigGerais();

   // Ajusta/atualiza arquivos de apoio (pasta Dat/FaultLocation)
   InicializaArquivosApoio();

   // Verifica diretórios auxiliares
   VerificaDiretoriosAuxiliares();
}
//---------------------------------------------------------------------------
__fastcall TGerenciadorEventos::~TGerenciadorEventos(void)
{
	// Destroi objetos
   if(listaAlarmes) {delete listaAlarmes; listaAlarmes = NULL;}
   if(listaFL) {delete listaFL; listaFL = NULL;}
   if(listaArquivosAdicionados) {delete listaArquivosAdicionados; listaArquivosAdicionados = NULL;}
   if(listaArquivosNovos) {delete listaArquivosNovos; listaArquivosNovos = NULL;}
   if(listaXMLCadSensores) {delete listaXMLCadSensores; listaXMLCadSensores = NULL;}
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::VerificaDiretoriosAuxiliares()
{
   String pathDiretorio;

   // Verifica se existe os diretório na pasta Importa
   pathDiretorio = path->DirImporta() + "\\FaultLocation";
   if(!DirectoryExists(pathDiretorio))
   {
      try
      {
         CreateDir(pathDiretorio);
         CreateDir(pathDiretorio + "\\Alarmes");
         CreateDir(pathDiretorio + "\\Mensagens");
      }
      catch(Exception &e){}
	}

   // Verifica se existe os diretório na pasta Exporta
   pathDiretorio = path->DirExporta() + "\\FaultLocation";
   if(!DirectoryExists(pathDiretorio))
   {
      try
      {
         CreateDir(pathDiretorio);
         CreateDir(pathDiretorio + "\\Mensagens");
      }
      catch(Exception &e){}
   }
   else
   {
      // Verifica se existe o diretório Exporta\FaultLocation\Mensagens
      pathDiretorio = path->DirExporta() + "\\FaultLocation\\Mensagens";
      if(!DirectoryExists(pathDiretorio))
      {
         try
         {
            CreateDir(pathDiretorio);
         }
         catch(Exception &e){}
      }
   }

   // Resseta diretório "Logs"
   pathDiretorio = path->DirDat() + "\\FaultLocation\\Logs";
   TStringList* lisNomesArquivos = new TStringList();
   get_all_files_names_within_folder(pathDiretorio, lisNomesArquivos);

   for(int i=0; i<lisNomesArquivos->Count; i++)
   {
      String nomeArquivo  = lisNomesArquivos->Strings[i];
      String pathCompleto = pathDiretorio + "\\" + nomeArquivo;
      try{DeleteFile(pathCompleto);}catch(Exception &e){}
   }
   delete lisNomesArquivos; lisNomesArquivos = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::InicializaArquivosApoio()
{
   // Verifica arquivo de cadastro de disjuntores
   TStringList* lisLinhas = new TStringList();
   String pathCadastroDisjuntores = path->DirDat() + "\\FaultLocation\\CadastroDisjuntores.csv";
   String pathCadastroReligadoras = path->DirDat() + "\\FaultLocation\\CadastroReligadoras.csv";

   // Tenta ler arquivo com o cadastro de disjuntores
   try
   {
      lisLinhas->LoadFromFile(pathCadastroDisjuntores);
   }
   catch(Exception &e) { }

   // Acrescenta os disjuntores das redes abertas e salva o arquivo.
   AtualizaCadastroDisjuntores(lisLinhas);
   lisLinhas->SaveToFile(pathCadastroDisjuntores);


   // Verifica cadastro de religadoras
   lisLinhas->Clear();
   // Tenta ler arquivo com o cadastro de religadoras
   try
   {
      lisLinhas->LoadFromFile(pathCadastroReligadoras);
   }
   catch(Exception &e) { }

   // Acrescenta as religadoras das redes abertas e salva o arquivo.
   AtualizaCadastroReligadoras(lisLinhas);
   lisLinhas->SaveToFile(pathCadastroReligadoras);

   // Destroi lista de linhas
   delete lisLinhas; lisLinhas = NULL;
}
//---------------------------------------------------------------------------
bool __fastcall TGerenciadorEventos::AguardandoCadastroSensores()
{
	return AguardandoSensores;
}
//---------------------------------------------------------------------------
bool __fastcall TGerenciadorEventos::AlarmeConsistente(TAlarme* alarme)
{
	bool resp = false;

	if(!alarme)
   	return false;

   switch(alarme->GetTipoEqpto())
   {
	case 10: // DJ
	case 11: // RE
		resp = (alarme->GetTipoAlarme() == 20 || alarme->GetTipoAlarme() == 21);
		break;

	case 12: // Medidor inteligente trafo
	case 13: // Medidor inteligente consumidor MT
	case 14: // Medidor inteligente consumidor BT
		resp = (alarme->GetTipoAlarme() == 22);
		break;

	default:
   	resp = false;
      break;
   }

   return resp;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::AtualizaCadastroDisjuntores(TStringList* lisLinhas)
{
   if(!lisLinhas) return;

   String linhaFinal, codigoRede;
   TList* lisChaves = new TList();
   VTChave *chave, *chaveDisjuntorRede;

   // Se o disjuntor de uma rede não consta do arquivo, inclui
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
      VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
      if(!rede->Carregada || rede->TipoRede->Segmento != redePRI) continue;
      if(CadastroContemRede(lisLinhas, rede)) continue;

      // Código da rede MT
      codigoRede = rede->Codigo;
      codigoRede = StrReplace(codigoRede, " ", "");
      linhaFinal = codigoRede + ";";

		// Código do disjuntor
		codigoRede = StrReplace(codigoRede, "-", "");
		if(codigoRede.Length() == 8) // redes da EDP-SP têm o formato: RABC1234
			linhaFinal += codigoRede.SubString(1,4) + "_" + codigoRede.SubString(5,4) + ";";
		else if(codigoRede.Length() == 5 ) // redes da EDP-ES têm o formato: ABC12
			linhaFinal += codigoRede.SubString(1,3) + "_" + codigoRede.SubString(4,2) + ";";
		else if(codigoRede.Length() == 7 ) // demais redes
			linhaFinal += codigoRede.SubString(1,3) + "_" + codigoRede.SubString(4,4) + ";";

		// Código da chave do disjuntor
      chaveDisjuntorRede = DisjuntorRede(rede);
      if(chaveDisjuntorRede) linhaFinal += chaveDisjuntorRede->Codigo; else continue;

      // Inclui na lista de linhas
      lisLinhas->Add(linhaFinal);
   }

   delete lisChaves;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::AtualizaCadastroReligadoras(TStringList* lisLinhas)
{
   if(!lisLinhas) return;

   String linhaFinal, codigoRede;
   TList* lisReligadoras = new TList();
   VTChave *chaveReligadora, *chaveDisjuntorRede;

   // Se as religadoras de uma rede não consta do arquivo, inclui
   for(int i=0; i<redes->LisRede()->Count; i++)
   {
      VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
      if(!rede->Carregada || rede->TipoRede->Segmento != redePRI) continue;

      // Pega as religadoras fechadas da rede em questão
      lisReligadoras->Clear();
      ReligadorasRede(rede, lisReligadoras);

      // Verifica se as religadoras já estão no cadastro
      for(int i=0; i<lisReligadoras->Count ;i++)
      {
         chaveReligadora = (VTChave*) lisReligadoras->Items[i];
         if(CadastroContemReligadora(lisLinhas, chaveReligadora)) continue;

         // Forma a linha do cadastro da religadora
         // 1o campo:
         codigoRede = rede->Codigo;
         codigoRede = StrReplace(codigoRede, " ", "");
         linhaFinal = codigoRede + ";";

         // 2o campo:
         codigoRede = StrReplace(codigoRede, "-", "");
         linhaFinal += codigoRede.SubString(1,3) + "_" + chaveReligadora->Codigo + ";";

         // 3o campo:
         linhaFinal += chaveReligadora->Codigo;

         // Adiciona nova linha à lista de cadastro de religadoras.
         lisLinhas->Add(linhaFinal);
      }
   }

   delete lisReligadoras;
}
//---------------------------------------------------------------------------
/***
 * Método para atualizar os timers de cada processo de LF
 */
void __fastcall TGerenciadorEventos::AtualizaTimers()
{
	String rootDir = "";
	TThreadFaultLocation* FL = NULL;

	for(int i=0; i<listaFL->Count; i++)
   {
   	FL = (TThreadFaultLocation*) listaFL->Items[i];

		if(FL->JanelaAberta())
      {
         // Objeto de FL ainda tem a janela de tempo aberta
         // ou seja, aguarda a chegada de mais alarmes.
	      FL->AtualizaTimer(1);
      }
      else if(!FL->JanelaAberta() && !FL->iniciado && FL_Executando == NULL)
      {
         if(FL->timerTimeout < TimeoutSegundos)
         {
            // Verifica se já temos medições do barramento
            FL->AtualizaTimer(2);
         }
         else
         {  // Timeout
         	String evento = ((TAlarme*)FL->GetAlarmes()->Items[0])->GetTimeStamp();
            formFL->MemoProcessosLF->Lines->Add("Encerrando o processo do evento " + evento + " por timeout.");
            FL->Terminate();
            listaFL->Remove(FL);
            delete FL;
         }
      }
      else if(FL->FLfinalizado)
      {
      	FL_Executando = NULL;
	      rootDir = FL->GetPathRootDir();
      	FL->Terminate();
         listaFL->Remove(FL);
         delete FL;
      }
   }
}
//---------------------------------------------------------------------------
/***
 * Retorna os nomes dos arquivos em uma determinada pasta
 */
void get_all_files_names_within_folder(String folder, TStringList* listaArquivos)
{
   if(listaArquivos == NULL) return;

   String search_path = folder + "/*.*";
   WIN32_FIND_DATA fd;
   HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
   if(hFind != INVALID_HANDLE_VALUE) {
      do {
         // read all (real) files in current folder
         // , delete '!' read other 2 default folder . and ..
         if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
         listaArquivos->Add(fd.cFileName);
      	}
      }while(::FindNextFile(hFind, &fd));
      ::FindClose(hFind);
   }
}
//---------------------------------------------------------------------------
/***
 * Monitora o diretório Importa/FaultLocation/Mensagens para verificar se chegou
 * resposta à solicitação do cadastro de sensores.
 */
bool __fastcall TGerenciadorEventos::ExisteRespostaCadSensores()
{
	bool resp;
   int indexXmlNome;
   String dirRespostaCadSensores = path->DirImporta() + "\\FaultLocation\\Mensagens\\";
   TStringList* listaAux = new TStringList();

   // Lista de strings com os nomes dos arquivos do diretório
   get_all_files_names_within_folder(dirRespostaCadSensores, listaAux);

   resp = false;

	// Verifica se a resposta é relativa à solicitação de cadastro
	for(int i=0; i<listaAux->Count; i++)
   {
      String xmlNome = listaAux->Strings[i];
      if(xmlNome.SubString(1,11) != "CADSENSORES") continue;

      // Remove ".xml"
      xmlNome = xmlNome.SubString(1, xmlNome.Length()-4);

      if((indexXmlNome = listaXMLCadSensores->IndexOf(xmlNome)) >= 0)
      {
         xmlRespCadSensores = xmlNome;
         listaXMLCadSensores->Delete(indexXmlNome);
         resp = true;
         break;
      }
   }

   delete listaAux;

   return resp;
}
//---------------------------------------------------------------------------
VTChave* __fastcall TGerenciadorEventos::DisjuntorRede(VTRede* rede)
{
   if(!rede) return(NULL);

   TList* lisChaves = new TList();
   rede->LisChave(lisChaves);
   VTChave* chave = NULL;

   for(int j=0; j<lisChaves->Count; j++)
   {
      chave = (VTChave*) lisChaves->Items[j];
      if(chave->TipoDisjuntor)
         break;
      else
         chave = NULL;
   }
   delete lisChaves;
   return(chave);
}
//---------------------------------------------------------------------------
/***
 * Método monitora o diretório Importa/FaultLocation/Alarmes para verificar
 * se chegou XML de alarme.
 *
 *   listaArquivosNovos: lista com os arquivos XML novos (ainda não lidos)
 *   listaArquivosAdicionados: lista com todos os arquivos XML lidos
 */
bool __fastcall TGerenciadorEventos::ExisteAlarmeNovo()
{
	bool resp = false;
	String nomeArquivo = "";
   String dirDadosBarramento = path->DirImporta() + "\\FaultLocation\\Alarmes\\";
   TStringList* listaAux = new TStringList();

   // Lista de strings com os nomes dos arquivos do diretório
 	listaArquivosNovos->Clear();
   get_all_files_names_within_folder(dirDadosBarramento, listaAux);

   for(int i=listaAux->Count-1; i>=0; i--)
   {
		nomeArquivo = listaAux->Strings[i];

      // Remove a extensão do arquivo
      nomeArquivo = nomeArquivo.SubString(1, nomeArquivo.Length()-4);

      if(listaArquivosAdicionados->IndexOf(nomeArquivo) < 0)
      {
         listaArquivosNovos->Add(nomeArquivo);
      	listaArquivosAdicionados->Add(nomeArquivo);
         resp = true;
      }
   }

   return resp;
}
//---------------------------------------------------------------------------
String __fastcall TGerenciadorEventos::GetStrTimeStamp()
{
	String timestamp;
   TDateTime horario = Now();
   unsigned short hora, minuto, segundo, mseg;
   unsigned short ano, mes, dia;

   horario.DecodeDate(&ano, &mes, &dia);
   horario.DecodeTime(&hora, &minuto, &segundo, &mseg);

   timestamp = String(ano);
   if(mes < 10)
      timestamp += "0" + String(mes);
   else
      timestamp += String(mes);

   if(dia < 10)
		timestamp += "0" + String(dia);
   else
      timestamp += String(dia);

   if(hora < 10)
		timestamp += "0" + String(hora);
   else
      timestamp += String(hora);

   if(minuto < 10)
		timestamp += "0" + String(minuto);
   else
      timestamp += String(minuto);

   if(segundo < 10)
		timestamp += "0" + String(segundo);
   else
      timestamp += String(segundo);

   return timestamp;
}
//---------------------------------------------------------------------------
/***
 * Lê arquivo INI com configurações gerais dos processos de monitoramento de alarmes.
 */
void __fastcall TGerenciadorEventos::LeConfigGerais()
{
	String pathConfigGerais = path->DirDat() + "\\FaultLocation\\ConfigGerais.ini";

	TIniFile* file = new TIniFile(pathConfigGerais);

   PassoMonitDirImporta = file->ReadInteger("GERAL", "PassoMonitDirImporta", 0);
   MaxJanelaDados = file->ReadInteger("GERAL", "MaxJanelaDados", 0);
   CadSensoresInicio = file->ReadBool("GERAL", "SolicitarCadastroSensoresInicio", 0);
   TimeoutSegundos = file->ReadInteger("GERAL", "TimeoutSegundos", 30);
   AgrupamentoAlarmesSegundos = file->ReadInteger("GERAL", "AgrupamentoAlarmesSegundos", 120);

   formFL->SetPassoMonitDirImporta(PassoMonitDirImporta);

   file->Free();
//   // Destroi obj
//   if(file) {delete file; file = NULL;}
}
//---------------------------------------------------------------------------
/***
 * Método para listar os objetos de FL na list view (LViewEventos)
 */
void __fastcall TGerenciadorEventos::ListaObjFaultLocation(TListView* LViewEventos)
{
   for(int i=0; i<listaFL->Count; i++)
   {
		TThreadFaultLocation* FL = (TThreadFaultLocation*) listaFL->Items[i];
      if(FL->JanelaAberta()) continue;

      bool achou = false;
      for(int j=0; j<LViewEventos->Items->Count; j++)
      {
	      TListItem* item = LViewEventos->Items->Item[j];
         if(((TThreadFaultLocation*) item->Data) == FL)
         {
            achou = true;
            break;
         }
      }
      if(!achou)
      {
         TListItem* item = LViewEventos->Items->Add();
         item->Data = (void*) FL;
         item->Caption = String(i+1);
			item->SubItems->Add(((TAlarme*)FL->GetListaAlarmes()->Items[0])->GetCodAlimentador());
         item->SubItems->Add(String(FL->GetListaAlarmes()->Count));
         item->SubItems->Add(String(FL->GetCodigoAlimentador()));
      }

   }
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::MostrarAreaBusca(void* objFL)
{
	if(objFL == NULL) return;

   TThreadFaultLocation* FL = (TThreadFaultLocation*) objFL;
   FL->MostraResultados();
}
//---------------------------------------------------------------------------
/***
 * 1- Procura obj TThreadFaultLocation do mesmo alimentador
 * 2- Verifica se a janela de inserção de dados está aberta
 */
TThreadFaultLocation* __fastcall TGerenciadorEventos::ProcuraObjFaultLocation(String CodigoEvento)
{
	TThreadFaultLocation* FL = NULL;

	for(int i=0; i<listaFL->Count; i++)
   {
   	FL = (TThreadFaultLocation*) listaFL->Items[i];
      if((FL->GetCodigoAlimentador() == CodigoEvento.SubString(1, 7)) && (FL->JanelaAberta()))
			break;
      else
      	FL = NULL;
   }

   return FL;
}
//---------------------------------------------------------------------------
/***
 * Método que retorna um objeto de TThreadFaultLocation que tenha:
 *     1) Alarmes do mesmo alimentador em questão
 *     2) Janela de tempo ainda aberta
 *     3) Alarme em questão com timestamp próximo dos demais do obj de FL
 */
TThreadFaultLocation* __fastcall TGerenciadorEventos::ProcuraObjFaultLocation(TAlarme* alarme)
{
   TAlarme* alarmeGatilho = NULL;
	TThreadFaultLocation* FL = NULL;

	for(int i=0; i<listaFL->Count; i++)
   {
   	FL = (TThreadFaultLocation*) listaFL->Items[i];

      alarmeGatilho = FL->GetAlarmeGatilho();
      if((alarmeGatilho->GetCodAlimentador() == alarme->GetCodAlimentador()) &&
          FL->JanelaAberta() &&
          Sincronismo(FL, alarme))
      {
      	break;
      }
		else
      {
      	FL = NULL;
      }
   }

   return FL;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::ReligadorasRede(VTRede* rede, TList* lisEXT)
{
   if(!rede || !lisEXT) return;

   rede->LisChave(lisEXT);

   for(int i=lisEXT->Count-1; i>=0; i--)
   {
      VTChave* chave = (VTChave*) lisEXT->Items[i];
      if(!chave->TipoReligadora || chave->Aberta)
         lisEXT->Remove(chave);
   }
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::ResetDir()
{
	AnsiString pathDirBase, comando;

   pathDirBase = path->DirDat() + "\\FaultLocation\\Dados";
   comando = "rd /s /q " + pathDirBase;

   // Comando para remoção do diretório
   system(comando.c_str());

   // Recria diretório Dados
   CreateDir(pathDirBase);
}
//---------------------------------------------------------------------------
/***
 * Método acessa o arquivo XML persistido pelo Mód. de Supervisão, atualizando
 * o arquivo local de cadastro de sensores.
 */
void __fastcall TGerenciadorEventos::SalvaArquivoCadastroSensores()
{
	StrCadSensor* str_cad;
   String linha, pathSaida;
   TList* listaCadastro;
   TStringList* CSVListaSensores;

   // Lê XML com o cadastro dos sensores
   listaCadastro = new TList();
	xmlParser->GetCadSensores(xmlRespCadSensores, listaCadastro);

   // Gera o CSV de saída. Formato: COD_ALIMENTADOR;COD_SENSOR;COD_CHAVE
   CSVListaSensores = new TStringList();
   for(int i=0; i<listaCadastro->Count; i++)
   {
		str_cad = (StrCadSensor*) listaCadastro->Items[i];

		linha = str_cad->Alimentador + ";";
      linha += str_cad->CodSensor + ";";
      linha += str_cad->CodChave + ";";

      CSVListaSensores->Add(linha);
   }

   // Ajusta o caminho de saída do CSV de cadastro
   pathSaida = path->DirDat() + "\\FaultLocation";
   CSVListaSensores->SaveToFile(pathSaida + "\\CadastroSensores.csv");

   delete listaCadastro;
	delete CSVListaSensores;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::SetMemoProcessosLF(TMemo* MemoProcessosLF)
{
	this->MemoProcessosLF = MemoProcessosLF;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::SetMemoResultados(TMemo* MemoResultados)
{
	this->MemoResultados = MemoResultados;
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::SetLViewMonitores(TListView* LViewMonitores)
{
	this->LViewMonitores = LViewMonitores;
}
//---------------------------------------------------------------------------
/***
 *  Verifica se dois alarmes são relativos a um mesmo evento
 */
bool __fastcall TGerenciadorEventos::Sincronismo(TAlarme* alarmeFL, TAlarme* alarme)
{
 	double deltaMin, deltaSeg, delta;
   int ano1, ano2, mes1, mes2, dia1, dia2, hora1, hora2, minuto1, minuto2, segundo1, segundo2;
	String t1, t2;

	if(!alarmeFL || !alarme)
   	return false;

   t1 = alarmeFL->GetTimeStamp();
   t2 = alarme->GetTimeStamp();

   // pega timestamp do alarmeFL
   ano1 = t1.SubString(1,4).ToInt();
   mes1 = t1.SubString(5,2).ToInt();
	dia1 = t1.SubString(7,2).ToInt();
   hora1 = t1.SubString(9,2).ToInt();
   minuto1 = t1.SubString(11,2).ToInt();
   segundo1 = t1.SubString(13,2).ToInt();

   // pega timestamp do alarme
   ano2 = t2.SubString(1,4).ToInt();
   mes2 = t2.SubString(5,2).ToInt();
	dia2 = t2.SubString(7,2).ToInt();
   hora2 = t2.SubString(9,2).ToInt();
   minuto2 = t2.SubString(11,2).ToInt();
   segundo2 = t2.SubString(13,2).ToInt();

   if(ano1 != ano2) return false;
   if(mes1 != mes2) return false;
   if(dia1 != dia2) return false;
   if(hora1 != hora2) return false;

   // Analisa minutos e segundos
   delta = 0.;
   if(minuto1 != minuto2)
   {
      if(minuto1 > minuto2)
      {
         deltaMin = minuto1 - minuto2;
         deltaSeg = segundo1 - segundo2;
      }
      else if(minuto2 > minuto1)
      {
         deltaMin = minuto2 - minuto1;
         deltaSeg = segundo2 - segundo1;
      }

      if(deltaSeg >= 0)
      	delta = deltaSeg + deltaMin*60.;
      else
      	delta = 60. + deltaSeg + (deltaMin-1)*60.;
   }
   else if(minuto1 == minuto2)
   {
      delta = abs(segundo1 - segundo2);
   }

   if(delta > AgrupamentoAlarmesSegundos)
   	return false;

   return true;
}
//---------------------------------------------------------------------------
bool __fastcall TGerenciadorEventos::Sincronismo(TThreadFaultLocation* FL, TAlarme* alarme)
{
	TDateTime lim1, lim2, menor, maior;
   int deltaMinutos;

	if(!FL || !alarme || !FL->GetAlarmes()) return false;
   if(FL->GetAlarmes()->Count == 0) return false;

   deltaMinutos = (int)(AgrupamentoAlarmesSegundos / 60.);

   // Define os limites inferior e superior de timestamps
   if(FL->GetAlarmes()->Count == 1)
   {
   	TAlarme* alarmeLista = (TAlarme*) FL->GetAlarmes()->Items[0];
      TDateTime base = GetTimeStamp(alarmeLista);
		lim1 = base - TDateTime((unsigned short)0, (unsigned short)deltaMinutos, (unsigned short)0, (unsigned short)0);
      lim2 = base + TDateTime((unsigned short)0, (unsigned short)deltaMinutos, (unsigned short)0, (unsigned short)0);
   }
   else
   {
      menor = GetTimeStamp(FL->GetAlarmeGatilho());
      maior = menor;
		for(int i=0; i<FL->GetAlarmes()->Count; i++)
      {
			TAlarme* alarmeLista = (TAlarme*) FL->GetAlarmes()->Items[i];
         if(alarmeLista == FL->GetAlarmeGatilho())
         	continue;

         if(GetTimeStamp(alarmeLista) > lim2)
         	maior = GetTimeStamp(alarmeLista);
      }
      lim1 = maior - TDateTime((unsigned short)0, (unsigned short)deltaMinutos, (unsigned short)0, (unsigned short)0);
      lim2 = menor + TDateTime((unsigned short)0, (unsigned short)deltaMinutos, (unsigned short)0, (unsigned short)0);
   }

   if(GetTimeStamp(alarme) >= lim1 && GetTimeStamp(alarme) <= lim2)
   	return true;
   else
   	return false;
}
//---------------------------------------------------------------------------
/***
 * Verifica se o mod. de superv. retornou o XML com o cadastro de sensores
 */
void __fastcall TGerenciadorEventos::VerificaCadastroSensores()
{
	if(ExisteRespostaCadSensores())
   {
   	// Desliga a flag
      AguardandoSensores = false;

      SalvaArquivoCadastroSensores();
   }
}
//---------------------------------------------------------------------------
/******
 * Periodicamente, este método é chamado, para verificar se existem novos alarmes.
 * 1) Procura-se obj de FL do mesmo alimentador.
 * 2) Se houver e ainda tiver a janela de tempo aberta, adiciona arquivos.
 * 3) Se não houver obj de FL, cria obj de FL e guarda em lista (listaFL).
 ***/
void __fastcall TGerenciadorEventos::VerificaNovosDados()
{
   String NomeArquivo, LinhaNovosAlarmes, LinhaAlarmesInconsistentes;
   TAlarme* alarme;

   // Limpa lista de alarmes
   listaAlarmes->Clear();

	// Inicializa Strings
   NomeArquivo = "";
   LinhaNovosAlarmes = "";
   LinhaAlarmesInconsistentes = "";

	// Se houver arquivo novo, retorna o nome dele
   // Exemplo: FREL_CPC01P1_20170927104430  (Sintaxe: <CódAlarme>_<CódAlimentador>_<TimeStamp>)
   // Gera obj do tipo TAlarme e salva em lista
   if(ExisteAlarmeNovo())
   {
   	// Gera um alarme (TAlarme) para cada arquivo XML novo
   	for(int i=0; i<listaArquivosNovos->Count; i++)
      {
      	NomeArquivo = listaArquivosNovos->Strings[i];
         alarme = xmlParser->GetAlarme(NomeArquivo);

         // Verifica consistência dos dados do alarme
         if(AlarmeConsistente(alarme))
      	{
            listaAlarmes->Add(alarme);
            LinhaNovosAlarmes += NomeArquivo;
            if(i < listaArquivosNovos->Count-1) LinhaNovosAlarmes += ", ";
         }
         else
         {
	         LinhaAlarmesInconsistentes += NomeArquivo + "; ";
         }
      }
      // Adiciona dados ao Memo de Processos de LF
      if(LinhaNovosAlarmes != "")
      	MemoProcessosLF->Lines->Add("Novos alarmes: " + LinhaNovosAlarmes);
      if(LinhaAlarmesInconsistentes != "")
      	MemoProcessosLF->Lines->Add("Alarmes inconsistentes: " + LinhaAlarmesInconsistentes);
   }

	// Para cada alarme, verifica se já existe TThreadFaultLocation associada
   for(int i=0; i<listaAlarmes->Count; i++)
   {
   	alarme = (TAlarme*) listaAlarmes->Items[i];
		TThreadFaultLocation* FL = ProcuraObjFaultLocation(alarme);

      // Se não achou FL, cria obj de FL para o evento, passando o código do evento = timestamp
		if(FL == NULL)
      {
      	// Gera objeto de FL com o obj de alarme associado
         FL = new TThreadFaultLocation(true, apl, formFL, this, alarme);

         // Guarda obj de FL em lista
         listaFL->Add(FL);

         // Seta parâmetros auxiliares
         FL->SetLViewMonitores(LViewMonitores);
         FL->SetMemoResultados(MemoResultados);
         FL->SetMemoProcessosLF(MemoProcessosLF);
         FL->SetMaxJanelaDados(MaxJanelaDados);
      }
      else
      {
      	// Associa o novo alarme ao obj de FL pré-existente
			FL->AddAlarme(alarme);
      }
   }
}
//---------------------------------------------------------------------------
/******
 * Método que escreve XML para solicitar o cadastro atualizado dos sensores
 * das redes que estão carregadas.
 ***/
void __fastcall TGerenciadorEventos::XMLCadastroSensores()
{
	String pathFinal, timestamp, nomeXML;
   TList* lisRedes;
   TXMLComunicacao* xmlCom;
   VTRede* rede;

	// Ativa flag
	AguardandoSensores = true;

   // Obtém o timestamp
   timestamp = GetStrTimeStamp();

   // :::::::::::::::::::::::::::::::::::::::::::::::::
   // Composição do nome do arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::
   nomeXML = "CADSENSORES_" + timestamp;

	pathFinal = path->DirExporta() + "\\FaultLocation\\Mensagens\\";
	pathFinal += nomeXML;
   pathFinal += ".xml";

   // :::::::::::::::::::::::::::::::::::::::::::::::::
   // Geração de arquivo XML
	// :::::::::::::::::::::::::::::::::::::::::::::::::

   // Cria objeto XML de solicitação de cadastro de sensores, passando os alimentadores
	xmlCom = new TXMLComunicacao(pathFinal, XMLcadSensores);

   // Pega a lista de redes (alimentadores) carregadas
   lisRedes = redes->LisRede();
   for(int i=0; i<lisRedes->Count; i++)
   {
		rede = (VTRede*) lisRedes->Items[i];
      if(rede->TipoRede->Segmento != redePRI) continue;

   	// Insere o código do alimentador do qual se quer a lista de sensores
   	xmlCom->AddAlimentador(rede->Codigo);
   }

   if(xmlCom->GetContItens() > 0)
   {
      // Persiste o arquivo XML
      xmlCom->Salvar();
      // Adiciona nome do XML à lista listaXMLCadSensores
      listaXMLCadSensores->Add(nomeXML);
   }


//   //debug
//   CadSensores_Fake(timestamp);
}
//---------------------------------------------------------------------------
bool __fastcall TGerenciadorEventos::CadastroContemReligadora(TStringList* lisLinhas, VTChave* chaveReligadora)
{
   if(!lisLinhas || !chaveReligadora) return(false);
   for(int i=0; i<lisLinhas->Count; i++)
   {
      String linha = lisLinhas->Strings[i];
      String codigoReligadoraCadastrada = GetCampoCSV(linha, 2, ";");

      if(chaveReligadora->Codigo == codigoReligadoraCadastrada)
         return(true);
   }
   return(false);
}
//---------------------------------------------------------------------------
bool __fastcall TGerenciadorEventos::CadastroContemRede(TStringList* lisLinhas, VTRede* rede)
{
   if(!lisLinhas || !rede) return(false);

   String codigoRede, codigoRedeCadastro;

   codigoRede = rede->Codigo;
   codigoRede = StrReplace(codigoRede, "-", "");
   codigoRede = StrReplace(codigoRede, " ", "");

   for(int i=0; i<lisLinhas->Count; i++)
   {
      String linha = lisLinhas->Strings[i];
      codigoRedeCadastro = GetCampoCSV(linha, 0, ";");

      codigoRedeCadastro = StrReplace(codigoRedeCadastro, "-", "");
      codigoRedeCadastro = StrReplace(codigoRedeCadastro, " ", "");

      if(codigoRede == codigoRedeCadastro)
         return(true);
   }
   return(false);
}
//---------------------------------------------------------------------------
void __fastcall TGerenciadorEventos::CadSensores_Fake(String TimeStamp)
{
	AnsiString pathIni = "", pathFinal = "";
	String dirImportaMsg, nomeArq;
	TStringList* listaArq;


   // Lista de strings com os nomes dos arquivos do diretório
   listaArq = new TStringList();
   dirImportaMsg = path->DirImporta() + "\\FaultLocation\\Mensagens";
   get_all_files_names_within_folder(dirImportaMsg, listaArq);

   // Pega arquivo de cadastro de sensores
   nomeArq = "";
   for(int i=0; i<listaArq->Count; i++)
   {
		nomeArq = listaArq->Strings[i];
      if(nomeArq.SubString(1, 11) == "CADSENSORES")
      {

         pathIni = path->DirImporta() + "\\FaultLocation\\Mensagens\\" + nomeArq;
         pathFinal = path->DirImporta() + "\\FaultLocation\\Mensagens\\CADSENSORES_" + TimeStamp + ".xml";

         const char* ini = pathIni.c_str();
         const char* final = pathFinal.c_str();
         rename(ini, final);

      	break;
      }
   }
}
//---------------------------------------------------------------------------
//eof
