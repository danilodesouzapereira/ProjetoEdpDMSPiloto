//---------------------------------------------------------------------------
#include <vcl.h>
#include <cmath>
#include <complex>
#pragma hdrstop
#include "TFormFaultLocation.h"
#include "TThreadFaultLocation.h"
#include "TGerenciadorEventos.h"
#include "Auxiliares\TDados.h"
#include "Auxiliares\TFormPtosEquidistantes.h"
#include "Auxiliares\TFormExportaBlocos.h"
#include "Auxiliares\TFormExportaLigacoes.h"
#include "Auxiliares\TFuncoesDeRede.h"
#include "ConfiguracoesRede\TConfigRede.h"
#include "DSS\TFaultStudyFL.h"
#include "EstrategiaEvolutiva\Forms\TFormProgressBar.h"
#include "GeraDefeito\TGeraDefeito.h"
#include "AlgoFasorial\TAlgoFasorial.h"
#include "Lote\TFaultLocationLote.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Bloco.h>
#include <PlataformaSinap\DLL_Inc\Grafico.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Grafico\VTGrafico.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTSuprimento.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
#include <StrUtils.hpp>
//---------------------------------------------------------------------------
TFormFaultLocation *FormFaultLocation;
//---------------------------------------------------------------------------
__fastcall TFormFaultLocation::TFormFaultLocation(TComponent *Owner, VTApl *apl_owner, TWinControl *parent)
	: TForm(Owner)
{
	// Parâmetros básicos
	this->apl = apl_owner;
	path = (VTPath*) apl->GetObject(__classid(VTPath));
   this->Parent = parent;
   graf = (VTGrafico*) apl->GetObject(__classid(VTGrafico));
   FormPG = NULL;

   // Obj para auxiliar varreduras na rede
   funcoesRede = new TFuncoesDeRede(apl);

   // Gerador de defeitos (para efeito de testes)
   geraDefeito = new TGeraDefeito(apl);

	// Inicializações
   tempo = 0;
   tempoTotal = 0.;
   PassoMonitDirImporta = 5;

   // Inicializa gerenciador de eventos
	gerEventos = new TGerenciadorEventos(apl, this);
   gerEventos->SetMemoProcessosLF(MemoProcessosLF);

   // Seta a LF em tempo real como a tab sheet ativa
   PageControl->ActivePage = TabSheetLFTempoReal;

   // Incializa objeto de localização de faltas em lote (TFaultLocationLote)
   FLLote = new TFaultLocationLote(apl);

   // Incializa objeto de localização de faltas baseado em distância
   AlgoFasorial = new TAlgoFasorial(apl);

   // Configurações de pot. de curto e param. de trafos SE
   configRede = new TConfigRede(apl);

   // ********** INICIALIZA O MONITORAMENTO
	Timer->Enabled = true;
   MemoProcessosLF->Lines->Add("Aguardando alarmes");
   // *********
}
//---------------------------------------------------------------------------
__fastcall TFormFaultLocation::~TFormFaultLocation(void)
{
   //Remove molduras
   graf->DestacaEqpto(NULL, clRed, 10);
	graf->Moldura();
	graf->Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::butAlignClick(TObject *Sender)
{
   Align  = alNone;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::mnuAlignLeftClick(TObject *Sender)
{
   //verifica se a janela estava alinhada à direita
   if (Align == alRight) Align = alNone;
   //alinha janela à esquerda
   Align = alLeft;
}
//---------------------------------------------------------------------------
TFormProgressBar* __fastcall TFormFaultLocation::CriaFormProgressBar()
{
	FormPG = new TFormProgressBar(NULL);
   return FormPG;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::TimerTimer(TObject *Sender)
{
   // ::::::::::::::::::::::::::::::::::::::::::::
   // Solicitação inicial do cadastro dos sensores
   // ::::::::::::::::::::::::::::::::::::::::::::
   if(tempo == 0 && tempoTotal == 0. && gerEventos->CadSensoresInicio)
   	gerEventos->XMLCadastroSensores();
   if(gerEventos->AguardandoCadastroSensores())
   	gerEventos->VerificaCadastroSensores();

   tempo += 1;
   gerEventos->AtualizaTimers();

   // Passo de monitoramento
   if(tempo < PassoMonitDirImporta) return;

   // ::::::::::::::::::::::::::::::::::::::::::::
	// Pede o cadastro de sensores a cada 24 horas
   // ::::::::::::::::::::::::::::::::::::::::::::
   tempoTotal += tempo / 3600.;
   // Reinicia tempo em segundos
   tempo = 0;
   if((int)tempoTotal >= 24 && gerEventos->CadSensoresInicio)
   {
		tempoTotal = 0.;
      gerEventos->XMLCadastroSensores();
   }
   if(gerEventos->AguardandoCadastroSensores())
      gerEventos->VerificaCadastroSensores();

   // ::::::::::::::::::::::::::::::::::::::::::
   // Verifica novos dados de alarme
   // ::::::::::::::::::::::::::::::::::::::::::
   gerEventos->VerificaNovosDados();

   // Atualiza tempos configurados em ConfigGerais.ini
   gerEventos->LeConfigGerais();
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::SetPassoMonitDirImporta(int PassoMonitDirImporta)
{
	this->PassoMonitDirImporta = PassoMonitDirImporta;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::ToolButtonDistanciaClick(TObject *Sender)
{
   TFormPtosEquidistantes* formPtos = new TFormPtosEquidistantes(this, apl);

	formPtos->ShowModal();

   delete formPtos;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::ToolButtonExpBlocosClick(TObject *Sender)
{
   TFormExportaBlocos* formExpBlocos = new TFormExportaBlocos(this, apl);

	formExpBlocos->ShowModal();

   delete formExpBlocos;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::ToolButtonExpLigacoesClick(TObject *Sender)
{
   TFormExportaLigacoes* formExpLiga = new TFormExportaLigacoes(this, apl);

	formExpLiga->ShowModal();

   delete formExpLiga;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::mnuAlignRightClick(TObject *Sender)
{
   //verifica se a janela estava alinhada à esquerda
   if (Align == alLeft) Align = alNone;
   //alinha janela à esquerda
   Align = alRight;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::mnuAlignDownClick(TObject *Sender)
{
   //verifica se a janela estava alinhada em cima
   if (Align == alTop) Align = alNone;
   //alinha janela à esquerda
   Align = alBottom;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::mnuAlignUpClick(TObject *Sender)
{
   //verifica se a janela estava alinhada em baixo
   if (Align == alBottom) Align = alNone;
   //alinha janela à esquerda
   Align = alTop;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::mnuAlignNodeClick(TObject *Sender)
{
	// cancela alinhamento
   Align = alNone;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::tbIniciaLFClick(TObject *Sender)
{
	// Dispara timer
	Timer->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::FormClose(TObject *Sender, TCloseAction &Action)
{
   // Desliga o timer
	Timer->Enabled = false;

	//Remove molduras
	graf->Moldura();
	graf->DestacaEqpto(NULL, clBlue, 10);
	graf->Refresh();

   delete gerEventos; gerEventos = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::TreeViewProcessosLFMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y)
{
	// Obtém o nó pela posição do mouse
	TTreeNode* node = TreeViewProcessosLF->GetNodeAt(X,Y);

	int idAlgoritmo = CodigoAlgoritmo(node);
	if(idAlgoritmo == 1)
		DestacarGrafico_DMS1(node);
	else if(idAlgoritmo == 2)
		DestacarGrafico_DMS2(node);
	else if(idAlgoritmo == 3)
		DestacarGrafico_DMS3(node);
	else
	{
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();
   }
}
//---------------------------------------------------------------------------
int __fastcall TFormFaultLocation::CodigoAlgoritmo(TTreeNode* node)
{
	if(node == NULL) return(0);

	if(node->Level == 2)
	{
		TTreeNode* nodePai = node->Parent->Parent;
		TTreeNode* nodeLevel1 = nodePai->getFirstChild();
		nodeLevel1 = nodeLevel1->GetNext();
		nodeLevel1 = nodeLevel1->GetNext();

		if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 1"))
			return(1);
		else if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 2"))
			return(2);
		else if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 3"))
			return(3);
	}
	else if(node->Level == 1)
	{
		TTreeNode* nodePai = node->Parent;
		TTreeNode* nodeLevel1 = nodePai->getFirstChild();
		nodeLevel1 = nodeLevel1->GetNext();
		nodeLevel1 = nodeLevel1->GetNext();

		if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 1"))
			return(1);
		else if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 2"))
			return(2);
		else if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 3"))
			return(3);
	}
	else if(node->Level == 0)
	{
		TTreeNode* nodeLevel1 = node->getFirstChild();
		nodeLevel1 = nodeLevel1->GetNext();
		nodeLevel1 = nodeLevel1->GetNext();
		if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 1"))
			return(1);
		else if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 2"))
			return(2);
		else if(AnsiContainsStr(nodeLevel1->Text, "Algoritmo 3"))
			return(3);
	}
	return(0);
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::DestacarGrafico_DMS1(TTreeNode* node)
{
	if(node == NULL)
	{
		// Remove molduras e destaques
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();

		return;
	}

	if(node->Level == 0)
	{
		// Remove molduras
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();
	}
	else if(node->Level == 1)  // Destaca todas as barras candidatas a solução
	{
		TList* lisTodasBarras = new TList;

		TTreeNode* childNode = node->getFirstChild();
		strSolucao* solucaoDMS1 = (strSolucao*) childNode->Data;
		lisTodasBarras->Add(solucaoDMS1->barraSolucao);

		while(childNode = node->GetNextChild(childNode))
		{
			solucaoDMS1 = (strSolucao*) childNode->Data;
			lisTodasBarras->Add(solucaoDMS1->barraSolucao);
		}

		// Destaca todos os trechos
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->MolduraSolida(lisTodasBarras);
		graf->Refresh();

		delete lisTodasBarras;
	}
	else if(node->Level == 2) // Destaca apenas a barra da solução
   {
		strSolucao* solucaoDMS1 = (strSolucao*) node->Data;

		// Exibe a barra associada à solução
		if(solucaoDMS1->barraSolucao)
		{
			graf->Moldura();
			graf->Moldura(solucaoDMS1->barraSolucao);
			graf->Refresh();
		}
   }

}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::DestacarGrafico_DMS2(TTreeNode* node)
{
	if(node == NULL)
	{
		// Remove molduras e destaques
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();

		return;
	}

	if(node->Level == 0)
	{
		// Remove molduras
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();
	}
	else if(node->Level == 1)  // Lista todos os trechos dos blocos da solução
	{
		TList* lisTodosTrechos = new TList;

		TTreeNode* childNode = node->getFirstChild();
		StrSolucaoDMS2* solucaoDMS2 = (StrSolucaoDMS2*) childNode->Data;
		for(int i=0; i<solucaoDMS2->lisTrechosBloco->Count; i++)
		{
			VTTrecho* trecho = (VTTrecho*) solucaoDMS2->lisTrechosBloco->Items[i];
			lisTodosTrechos->Add(trecho);
		}

		while(childNode = node->GetNextChild(childNode))
		{
			StrSolucaoDMS2* solucaoDMS2 = (StrSolucaoDMS2*) childNode->Data;
			for(int i=0; i<solucaoDMS2->lisTrechosBloco->Count; i++)
			{
				VTTrecho* trecho = (VTTrecho*) solucaoDMS2->lisTrechosBloco->Items[i];
				lisTodosTrechos->Add(trecho);
			}
		}

		// Destaca todos os trechos
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->DestacaEqpto(lisTodosTrechos, clBlue, 20);
		graf->Refresh();

		delete lisTodosTrechos;
	}
	else if(node->Level == 2) // Apenas um bloco da solução
   {
		StrSolucaoDMS2* solucaoDMS2 = (StrSolucaoDMS2*) node->Data;

		// Exibe um bloco da solução
		if(solucaoDMS2->bloco)
		{
			graf->DestacaEqpto(NULL, clBlue, 20);
			graf->DestacaEqpto(solucaoDMS2->lisTrechosBloco, clBlue, 20);
			graf->Refresh();
		}
   }
   else if(node->Level == 3) // Parâmetros de uma solução
   {

	}
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::DestacarGrafico_DMS3(TTreeNode* node)
{
	if(node == NULL)
	{
		// Remove molduras e destaques
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();
		return;
	}

   if(node->Level == 0)
	{
		// Remove molduras
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();
   }
	else if(node->Level == 1)  // Lista todos os trechos dos blocos da solução
	{
		TList* lisTodasLigacoes = new TList;

		TTreeNode* nodeParent = node->Parent;
		TTreeNode* nodeLevel1 = nodeParent->getFirstChild();
		nodeLevel1 = nodeLevel1->GetNext();
		nodeLevel1 = nodeLevel1->GetNext();
		nodeLevel1 = nodeLevel1->GetNext();

		TTreeNode* nodeLevel2 = nodeLevel1->getFirstChild();
		StrSolucaoDMS3* solucaoDMS3 = (StrSolucaoDMS3*) nodeLevel2->Data;
		VTLigacao* ligacao = solucaoDMS3->ligacao;
		lisTodasLigacoes->Add(ligacao);

		while(nodeLevel2 = nodeLevel2->GetNext())
		{
			StrSolucaoDMS3* solucaoDMS3 = (StrSolucaoDMS3*) nodeLevel2->Data;
			VTLigacao* ligacao = solucaoDMS3->ligacao;
			lisTodasLigacoes->Add(ligacao);
		}

		// Destaca todos os trechos
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->DestacaEqpto(lisTodasLigacoes, clBlue, 20);
		graf->Refresh();

		// Destroi lista
		delete lisTodasLigacoes;
	}
	else if(node->Level == 2) // Apenas uma ligação da solução
	{
		TList* lisLigacoes = new TList;
		StrSolucaoDMS3* solucaoDMS3 = (StrSolucaoDMS3*) node->Data;
		VTLigacao* ligacao = solucaoDMS3->ligacao;
		lisLigacoes->Add(ligacao);

		// Destaca todos os trechos
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->DestacaEqpto(lisLigacoes, clBlue, 20);
		graf->Refresh();

		// Destroi lista
		delete lisLigacoes;
	}
   else if(node->Level == 3) // Parâmetros de uma solução
   {
   	// Remove molduras
		graf->Moldura();
		graf->DestacaEqpto(NULL, clBlue, 20);
		graf->Refresh();
	}
}
//---------------------------------------------------------------------------
VTChave* __fastcall TFormFaultLocation::GetChave(String codChave)
{
	VTRedes* redes;
   VTRede* redeMT;

	redes = (VTRedes*) apl->GetObject(__classid(VTRedes));

   for(int i=0; i<redes->LisRede()->Count; i++)
	{
     	redeMT = (VTRede*) redes->LisRede()->Items[i];

      TList* lisLigacao = redeMT->LisLigacao();
      for(int j=0; j<lisLigacao->Count; j++)
		{
			VTLigacao* liga = (VTLigacao*) lisLigacao->Items[j];

         if(liga->Tipo() != eqptoCHAVE) continue;

         if(liga->Codigo == codChave)
         {
            return ((VTChave*) liga);
         }
   	}
   }

   return NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::ToolButton1Click(TObject *Sender)
{
	// Dispara timer
	Timer->Enabled = true;

   MemoProcessosLF->Lines->Add("Aguardando alarmes");
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::tbCarregarArquivosClick(TObject *Sender)
{
	FLLote->CarregarDados();
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::btnCalcBarrasClick(TObject *Sender)
{
//	AlgoFasorial->ExecutaFL_FT(edtCodigoRede->Text, edtRtotal->Text.ToDouble(), edtXtotal->Text.ToDouble());
}
//---------------------------------------------------------------------------

void __fastcall TFormFaultLocation::btnCalcZtotalClick(TObject *Sender)
{
	AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
	AlgoFasorial->CalculaZtotal(edtModVa->Text.ToDouble(), edtModVb->Text.ToDouble(), edtModVc->Text.ToDouble(),
                            	 edtFaseVa->Text.ToDouble(), edtFaseVb->Text.ToDouble(), edtFaseVc->Text.ToDouble(),
                            	 edtModIa->Text.ToDouble(), edtModIb->Text.ToDouble(), edtModIc->Text.ToDouble(),
                            	 edtFaseIa->Text.ToDouble(), edtFaseIb->Text.ToDouble(), edtFaseIc->Text.ToDouble());

   double reZtotal, imZtotal;
	AlgoFasorial->GetZTotal(reZtotal, imZtotal);
	edtZtotal->Text = String(reZtotal) + " +j " + String(imZtotal);
}
//---------------------------------------------------------------------------

void __fastcall TFormFaultLocation::btnFiltroRtotalClick(TObject *Sender)
{
   AlgoFasorial->ExecutaFiltroRf(edtRtotal->Text.ToDouble());
}
//---------------------------------------------------------------------------

void __fastcall TFormFaultLocation::Button1Click(TObject *Sender)
{
	AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
	AlgoFasorial->CalculaZtotal_3F(edtModVa->Text.ToDouble(), edtModVb->Text.ToDouble(), edtModVc->Text.ToDouble(),
                            	 edtFaseVa->Text.ToDouble(), edtFaseVb->Text.ToDouble(), edtFaseVc->Text.ToDouble(),
                            	 edtModIa->Text.ToDouble(), edtModIb->Text.ToDouble(), edtModIc->Text.ToDouble(),
                            	 edtFaseIa->Text.ToDouble(), edtFaseIb->Text.ToDouble(), edtFaseIc->Text.ToDouble());

   double reZtotal, imZtotal;
	AlgoFasorial->GetZTotal_3F(reZtotal, imZtotal);
	edtZtotal_3F->Text = String(reZtotal) + " +j " + String(imZtotal);
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::Button2Click(TObject *Sender)
{
//	AlgoFasorial->ExecutaFL_3F(edtCodigoRede->Text, edtRtotal->Text.ToDouble(), edtXtotal->Text.ToDouble());
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::Button4Click(TObject *Sender)
{
   VTRedes* redes;
   VTRede* rede;
	String pathArquivo = edtPathArquivo->Text;
   TStringList* linhas = new TStringList();
   linhas->LoadFromFile(pathArquivo);

   TList* lisLigacoes = new TList();
   funcoesRede->GetTrechos(linhas, lisLigacoes);

   //Remove molduras
	graf->Moldura();
	graf->Refresh();

   graf->DestacaEqpto(lisLigacoes, clBlue, 10.);
	graf->Refresh();

   delete lisLigacoes; lisLigacoes = NULL;
   delete linhas; linhas = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::Button5Click(TObject *Sender)
{
	String pathArquivo = edtPathArquivoChaves->Text;
   TList* lisChaves = NULL;
   TStringList* linhas = NULL;
   VTRedes* redes;
   VTRede* rede;

   linhas = new TStringList();
   linhas->LoadFromFile(pathArquivo);

   lisChaves = new TList();
   funcoesRede->GetChaves(linhas, lisChaves);

   //Remove molduras
   graf->Moldura();
   graf->Moldura(lisChaves);
	graf->Refresh();

   delete lisChaves; lisChaves = NULL;
   delete linhas; linhas = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::Button6Click(TObject *Sender)
{
	double kmTrechos;

	String pathArquivo = edtCodTrechos->Text;
   TStringList* linhas = new TStringList();
   linhas->LoadFromFile(pathArquivo);

   TList* lisTrechos = new TList();
   funcoesRede->GetTrechos(linhas, lisTrechos);

   kmTrechos = 0.;
   for(int i=0; i<lisTrechos->Count; i++)
   {
		VTTrecho* trecho = (VTTrecho*) lisTrechos->Items[i];
      kmTrechos += trecho->Comprimento_km;
   }

   lblCompTotal->Caption = "Total: " + String(kmTrechos);

   delete lisTrechos; lisTrechos = NULL;
   delete linhas; linhas = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TFormFaultLocation::Button7Click(TObject *Sender)
{
	double Va, Vb, Vc, pVa, pVb, pVc, Ia, Ib, Ic, pIa, pIb, pIc;
	double reZtotal, imZtotal;
	StrFasor* Vse = new StrFasor();
   StrFasor* Ise = new StrFasor();

	Va = edtModVa->Text.ToDouble(); pVa = edtFaseVa->Text.ToDouble();
	Vb = edtModVb->Text.ToDouble(); pVb = edtFaseVb->Text.ToDouble();
	Vc = edtModVc->Text.ToDouble(); pVc = edtFaseVc->Text.ToDouble();

	Ia = edtModIa->Text.ToDouble(); pIa = edtFaseIa->Text.ToDouble();
	Ib = edtModIb->Text.ToDouble(); pIb = edtFaseIb->Text.ToDouble();
	Ic = edtModIc->Text.ToDouble(); pIc = edtFaseIc->Text.ToDouble();


   Vse->faseA = std::complex<double>(Va * cos(pVa * 3.14/180.), Va * sin(pVa * 3.14/180.));
   Vse->faseB = std::complex<double>(Vb * cos(pVb * 3.14/180.), Vb * sin(pVb * 3.14/180.));
   Vse->faseC = std::complex<double>(Vc * cos(pVc * 3.14/180.), Vc * sin(pVc * 3.14/180.));

   Ise->faseA = std::complex<double>(Ia * cos(pIa * 3.14/180.), Ia * sin(pIa * 3.14/180.));
   Ise->faseB = std::complex<double>(Ib * cos(pIb * 3.14/180.), Ib * sin(pIb * 3.14/180.));
   Ise->faseC = std::complex<double>(Ic * cos(pIc * 3.14/180.), Ic * sin(pIc * 3.14/180.));

	AlgoFasorial->InicializaConfiguracoes(path->DirDat() + "\\FaultLocation");
	AlgoFasorial->CalculaVI_Sequencias012(Vse, Ise);
   AlgoFasorial->CalculaZtotal_seq1_2F(Vse, Ise);
   AlgoFasorial->GetZTotal_1(reZtotal, imZtotal);
   edtZtotal_2F->Text = String(reZtotal) + " +j " + String(imZtotal);
}
//---------------------------------------------------------------------------


void __fastcall TFormFaultLocation::btnGeraDefeitoClick(TObject *Sender)
{
	StrGeraDefeito* strGeraDefeito = new StrGeraDefeito();
	strGeraDefeito->caminhoDSS = edtCaminhoDSS->Text;
   strGeraDefeito->CodigoAlimentador = edtCodigoAlimentador->Text;
   strGeraDefeito->CodigoBarra = edtCodigoBarraDefeito->Text;
   strGeraDefeito->TipoDefeito = edtTipoDefeito->Text;
   strGeraDefeito->Rfalta = edtRfalta->Text.ToDouble();

   geraDefeito->SetConfiguracoes(strGeraDefeito);
   geraDefeito->SetMemo(memoResultadosGeraDefeito);
   geraDefeito->Executa();
}
//---------------------------------------------------------------------------

void __fastcall TFormFaultLocation::Button8Click(TObject *Sender)
{
	geraDefeito->ExportarResultados(edtPathTxtResultados->Text);
}
//---------------------------------------------------------------------------

void __fastcall TFormFaultLocation::Button9Click(TObject *Sender)
{
	TList* lisBarras = new TList();
	VTBlocos* blocos = (VTBlocos*) apl->GetObject(__classid(VTBlocos));
	if(blocos == NULL)
		apl->Add(blocos = DLL_NewObjBlocos());

	VTRedes* redes = (VTRedes*) apl->GetObject(__classid(VTRedes));
	blocos->Executa(redes);
	TList* lisBlocos = blocos->LisBloco();


	// Pega os blocos à jusante de liga ref
	VTLigacao* ligaRef = NULL;
	TList* lisBlocosJusante = new TList();
	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redePRI) continue;

		for(int j=0; j<rede->LisLigacao()->Count; j++)
		{
			ligaRef = (VTLigacao*) rede->LisLigacao()->Items[j];
			if(ligaRef->Tipo() != eqptoTRECHO) continue;

			if(ligaRef->Codigo == edtLigaRef->Text)
				break;
			else
				ligaRef = NULL;
      }
	}
	funcoesRede->GetBlocosJusanteLigacao(ligaRef, lisBlocosJusante);


	TStringList* lisLinhas = new TStringList();
	TStringList* lisLinhas2 = new TStringList();
	String linha = "";
	String linha2 = "";
	for(int i=0; i<redes->LisRede()->Count; i++)
	{
		VTRede* rede = (VTRede*) redes->LisRede()->Items[i];
		if(rede->TipoRede->Segmento != redePRI) continue;

		for(int j=0; j<lisBlocos->Count; j++)
		{
			VTBloco* bloco = (VTBloco*) lisBlocos->Items[j];
			if(bloco->LisLigacao() == NULL || bloco->LisLigacao()->Count == 0) continue;

			VTLigacao* liga = (VTLigacao*) bloco->LisLigacao()->Items[0];
			if(liga->rede == rede)
			{
				if(lisBlocosJusante->IndexOf(bloco) >= 0)
					linha2 += ".blo" + String(lisLinhas->Count + 1);

				linha = "Bloco.blo" + String(lisLinhas->Count+1) + " barras.";
				TList* lisBarras = bloco->LisBarra();
				for(int k=0; k<lisBarras->Count; k++)
				{
					VTBarra* barra = (VTBarra*) lisBarras->Items[k];
					linha += barra->Codigo;
					if(k < lisBarras->Count-1) linha += ".";
				}
				lisLinhas->Add(linha);
         }
		}
	}
	lisLinhas2->Add(linha2);
	lisLinhas2->SaveToFile(edtPathSaidaExpBlocos->Text + "-blocosJusante");
	lisLinhas->SaveToFile(edtPathSaidaExpBlocos->Text);
	delete lisLinhas; lisLinhas = NULL;
}
//---------------------------------------------------------------------------

