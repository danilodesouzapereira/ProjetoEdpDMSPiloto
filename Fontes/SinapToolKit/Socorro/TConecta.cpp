// ---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <Math.h>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Rede.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Edita\VTEdita.h>
#include <PlataformaSinap\Fontes\Progresso\VTProgresso.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTEqbar.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo.h>
#include "TConecta.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)

// ---------------------------------------------------------------------------
VTConecta* __fastcall NewObjConecta(VTApl *apl_owner)
{
	try
	{
		return (new TConecta(apl_owner));
	}
	catch (Exception &e)
	{
	}
	// erro na criação do objeto
	return (NULL);
}

// ---------------------------------------------------------------------------
__fastcall TConecta::TConecta(VTApl *apl_owner)
{
	// salva ponteiro para objetos
	this->apl = apl_owner;
	// cria listas
	lisCHV = new TList();
	lisBAR_ISO = new TList();
}

// ---------------------------------------------------------------------------
__fastcall TConecta::~TConecta(void)
{
	// destrói lista sem destruir seus objetos
	if (lisCHV)
	{
		delete lisCHV;
		lisCHV = NULL;
	}
	if (lisBAR_ISO)
	{
		delete lisBAR_ISO;
		lisBAR_ISO = NULL;
	}
}

// ---------------------------------------------------------------------------
bool __fastcall TConecta::Executa(void)
{
	// variáveis locais
	TList *lisBAR, *lisLIG, *lisREDE;
	VTBarra *barra;
	VTChave *chave;
	VTLigacao *ligacao;
	VTRede *rede;
	VTEdita *edita = (VTEdita*)apl->GetObject(__classid(VTEdita));
	VTRedes *redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
	VTProgresso *progresso = (VTProgresso*)apl->GetObject(__classid(VTProgresso));

	if (progresso)
	{
		progresso->Start(progTEXTO);
		progresso->Add("Conectando chaves vis.");
	}
	try
	{
		if (progresso)
		{
			progresso->Add("Preparando barras.");
		}
		// reinicia Tag de todas as Barras com valor 0
		lisBAR = redes->LisBarra();
		for (int n = 0; n < lisBAR->Count; n++)
		{
			barra = (VTBarra*)lisBAR->Items[n];
			barra->Tag = 0;
		}
		// loop p/ todas redes
		lisREDE = redes->LisRede();
		if (progresso)
		{
			progresso->Add("Lendo redes.");
		}
		for (int nr = 0; nr < lisREDE->Count; nr++)
		{
			rede = (VTRede*)lisREDE->Items[nr];
			// loop p/ todas Ligacoes da Rede
			lisLIG = rede->LisLigacao();
			for (int nl = 0; nl < lisLIG->Count; nl++)
			{
				ligacao = (VTLigacao*)lisLIG->Items[nl];
				// incrementa Tag de suas Barras
				for (int ind_bar = 0; ind_bar < NBAR_LIG; ind_bar++)
				{
					if ((barra = ligacao->Barra(ind_bar)) == NULL)
						continue;
					// incrementa Tag da Barra
					barra->Tag++;
				}
			}
		}
		// inicia lista de Chaves abertas que podem ser de socorro
		lisCHV->Clear();
		// loop p/ todas redes
		lisREDE = redes->LisRede();
		if (progresso)
		{
			progresso->Add("Listando chaves abertas.");
		}
		for (int nr = 0; nr < lisREDE->Count; nr++)
		{
			rede = (VTRede*)lisREDE->Items[nr];
			// loop p/ todas Ligacoes da Rede
			lisLIG = rede->LisLigacao();
			for (int nl = 0; nl < lisLIG->Count; nl++)
			{
				ligacao = (VTLigacao*)lisLIG->Items[nl];
				if (ligacao->Tipo() != eqptoCHAVE)
					continue;
				// verifica se é uma Chave aberta
				chave = (VTChave*)ligacao;
				if (chave->Aberta)
				{ // Chave aberta: verifica se uma de suas Barras possui Tag igual a 1
					if ((chave->pbarra1->Tag == 1) || (chave->pbarra2->Tag == 1))
					{ // inclui Chave em lisCHV
						lisCHV->Add(chave);
					}
				}
			}
		}
		// loop p/ todas Chaves que podem ser de socorro
		if (progresso)
		{
			progresso->Add("Procurando conexões.");
		}
		for (int n = 0; n < lisCHV->Count; n++)
		{
			chave = (VTChave*)lisCHV->Items[n];
			if (chave->pbarra1->Tag == 1)
			{ // verifica se há uma outra Barra de mesma coordenada
				barra = ExisteBarraDeMesmaCoordenada(chave->pbarra1, redes->LisBarra(), chave);
				if (barra != NULL)
				{ // inclui em lisBAR_ISO a Barra da chave que ficou isolada
					lisBAR_ISO->Add(chave->pbarra1);
					// redefine Barras da Chave de socorro
					// chave->DefineObjBarra(barra, chave->pbarra2);
					// marca que a Chave foi alterada
					// chave->Status[sttALTERADO] = true;
					TransfereEqptosDeAparaB(chave->pbarra1, barra);
				}
			}
			else if (chave->pbarra2->Tag == 1)
			{ // verifica se há uma outra Barra de mesma coordenada
				barra = ExisteBarraDeMesmaCoordenada(chave->pbarra2, redes->LisBarra(), chave);
				if (barra != NULL)
				{ // inclui em lisBAR_ISO a Barra da chave que fixou isolada
					lisBAR_ISO->Add(chave->pbarra2);
					// redefine Barras da Chave de socorro
					// chave->DefineObjBarra(chave->pbarra1, barra);
					// marca que a Chave foi alterada
					// chave->Status[sttALTERADO] = true;
					TransfereEqptosDeAparaB(chave->pbarra2, barra);
				}
			}
		}
		// elimina Barras que ficaram isoladas
		if (progresso)
		{
			progresso->Add("Eliminando barras isoladas.");
		}
		if (edita)
		{
			edita->RetiraLisEqpto(lisBAR_ISO);
		}
		if (progresso)
		{
			progresso->Stop();
		}

	}
	catch (Exception &e)
	{
		if (progresso)
		{
			progresso->Stop();
		}
	}
	return (true);
}

/*
 //---------------------------------------------------------------------------
 VTBarra* __fastcall TConecta::ExisteBarraDeMesmaCoordenada(VTBarra *bar_ref, TList *lisBAR)
 {
 //variáveis locais
 VTBarra *barra;

 //loop p/ todas barras da Lista
 for (int n = 0; n < lisBAR->Count; n++)
 {
 barra = (VTBarra*)lisBAR->Items[n];
 //proteção: desconsidera a própria Barra de referência
 if (barra == bar_ref) continue;
 //verifica se a Barra possui as mesmas coordenadas da Barra de referência
 if ((barra->utm.x == bar_ref->utm.x) && (barra->utm.y == bar_ref->utm.y)) return(barra);
 }
 return(NULL);
 }
 */
// ---------------------------------------------------------------------------
VTBarra* __fastcall TConecta::ExisteBarraDeMesmaCoordenada(VTBarra *bar_ref, TList *lisBAR,
	VTChave *chave)
{
	// variáveis locais
	int dx, dy;
	VTBarra *barra;

	// protecao
	// loop p/ todas barras da Lista
	for (int n = 0; n < lisBAR->Count; n++)
	{
		barra = (VTBarra*)lisBAR->Items[n];
		// proteção: desconsidera a própria Barra de referência
		if (barra == bar_ref)
			continue;
		// verifica se a Barra possui as mesmas coordenadas da Barra de referência
		dx = abs(barra->utm.x - bar_ref->utm.x);
		dy = abs(barra->utm.y - bar_ref->utm.y);
		if ((dx <= 2) && (dy <= 2))
		{
			// verifica a tensao nominal da barra  //FKM
			if (IsDoubleZero(bar_ref->vnom - barra->vnom))
			{
				// teste
				// if(barra->Codigo.AnsiCompare("12484458") == 0)
				// {
				// int a = 0;
				// }
				// //TESTE
				// if(barra->Tag == 1)
				// {
				// return(barra);
				// }
				if (barra->Extern_id.AnsiCompare(bar_ref->Extern_id) == 0)
				{
					// protecao
					if (chave != NULL)
					{
						if ((barra != chave->pbarra1) && (barra != chave->pbarra2))
						{
							return (barra);
						}
					}
					else
					{
						return (barra);
					}

				}
				// return(barra);
			}
		}
	}
	return (NULL);
}

// ---------------------------------------------------------------------------
void __fastcall TConecta::TransfereEqptosDeAparaB(VTBarra *barraA, VTBarra *barraB)
{
	// variaveis locais
	VTEdita *edita = (VTEdita*)apl->GetObject(__classid(VTEdita));
	VTRedes *redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
	TList *lisLIGACAO_BA, *lisEqbar;
	VTEqbar *eqbar;
	VTLigacao *ligacao;

	// protecao
	if ((barraA == NULL) || (barraB == NULL))
	{
		return;
	}
	// protecao
	if ((barraA == barraB))
	{
		return;
	}
	// rede
	lisLIGACAO_BA = new TList;
	// pega tudo ligado à barra 1
	lisEqbar = barraA->LisEqbar();
	redes->LisLigacao(lisLIGACAO_BA, barraA);
	// move todos eqbar
	for (int neq = 0; neq < lisEqbar->Count; neq++)
	{
		eqbar = (VTEqbar*) lisEqbar->Items[neq];
		// if(edita)
		// {
		// edita->TrocaBarraEqbar(eqbar,barraB);
		// }
		eqbar->pbarra->RemoveEqbar(eqbar);
		barraB->InsereEqbar(eqbar);
	}
	// move todas ligacoes
	for (int nl = 0; nl < lisLIGACAO_BA->Count; nl++)
	{
		ligacao = (VTLigacao*) lisLIGACAO_BA->Items[nl];
		// verifica qual barra trocar
		// edita->TrocaBarraLigacao(ligacao,barraA,barraB);
		// verifica qual barra trocar
		if (ligacao->pbarra1 == barraA)
		{
			// caso seja essa, move essa e mantem as outras
			ligacao->DefineObjBarra(barraB, ligacao->pbarra2, ligacao->pbarra3);
			ligacao->Status[sttALTERADO] = true;
		}
		else if (ligacao->pbarra2 == barraA)
		{
			// caso seja essa, move essa e mantem as outras
			ligacao->DefineObjBarra(ligacao->pbarra1, barraB, ligacao->pbarra3);
			ligacao->Status[sttALTERADO] = true;
		}
		else if (ligacao->pbarra3 == barraA)
		{
			// caso seja essa, move essa e mantem as outras
			ligacao->DefineObjBarra(ligacao->pbarra1, ligacao->pbarra2, barraB);
			ligacao->Status[sttALTERADO] = true;
		}
		else
		{
			continue;
		}
		// corrige trafo se necessario
		if (ligacao->Tipo() == eqptoTRAFO)
		{
			((VTTrafo*)ligacao)->RedefineVnom(barraB);
			ligacao->Status[sttALTERADO] = true;
		}
	}
	// if(edita)
	// {
	// edita->Retira(barraA);
	// }
	// destroi lista
	if (lisLIGACAO_BA)
	{
		delete lisLIGACAO_BA;
		lisLIGACAO_BA = NULL;
	}
}
// ---------------------------------------------------------------------------
// eof
