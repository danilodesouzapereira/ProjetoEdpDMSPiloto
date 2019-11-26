// ---------------------------------------------------------------------------

#pragma hdrstop

#include "TConectaMaeFilha.h"
#include "VTConectaMaeFilha.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
// ---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <math.h>
#include <Math.hpp>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
// #include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Edita\VTEdita.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTComentario.h>
#include <PlataformaSinap\Fontes\Rede\VTEqbar.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo.h>
#include <PlataformaSinap\Fontes\Rede\VTTrafo3E.h>
// #include <PlataformaSinap\DLL_Inc\Cartografia.h>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include <PlataformaSinap\DLL_Inc\Rede.h>

// #include "..\Empresa\VTPrimario.h"
// #include "..\Empresa\VTSubestacao.h"
// #include "..\Eqpto\TSe.h"
// #include "..\Eqpto\TPri.h"
// #include "..\Importa\VTConversor.h"
// #include "..\PreMonta\VTPreMonta.h"
// #ifdef _DEBUG
// #include "TLogThread.h"
// #include "TLogManager.h"
// #endif
// ---------------------------------------------------------------------------
VTConectaMaeFilha* __fastcall NewObjConectaMaeFilha(VTApl *apl_owner)
{
	try
	{
		return (new TConectaMaeFilha(apl_owner));
	}
	catch (Exception &e)
	{
	}
	// erro na criação do objeto
	return (NULL);
}

// ---------------------------------------------------------------------------
__fastcall TConectaMaeFilha::TConectaMaeFilha(VTApl *apl)
{
	// salva ponteiros p/ objetos
	this->apl = apl;
	// cria listas
	lisBAR_ISO = new TList;
	lisEQPTO_MOD = new TList;
}

// ---------------------------------------------------------------------------
__fastcall TConectaMaeFilha::~TConectaMaeFilha(void)
{
	// limpa a lista
	 if (lisBAR_ISO)
	 {
		 delete lisBAR_ISO;
		 lisBAR_ISO = NULL;
	 }
	 if (lisEQPTO_MOD)
	 {
		 delete lisEQPTO_MOD;
		 lisEQPTO_MOD = NULL;
	 }
}

// ---------------------------------------------------------------------------
bool __fastcall TConectaMaeFilha::Executa(void)
{
	return (TrataSEMaeFilha());
}

// ---------------------------------------------------------------------------
double __fastcall TConectaMaeFilha::CalculaDistancia_m(double flat1, double flon1, double flat2,
	double flon2)
{ // DVK 2017.06.01
	// variáveis locais
	double delta_lat, delta_lon;
	double comp_m, a, c;
	double lon1, lat1, lon2, lat2;

	// calcula comprimento entre barras (está num falso utm, ainda é lat/long)
	// fonte: http://www.movable-type.co.uk/scripts/latlong.html
	lon1 = DegToRad((flon1 * 1e-7) - 100.);
	lat1 = DegToRad((flat1 * 1e-7) - 100.);
	lon2 = DegToRad((flon2 * 1e-7) - 100.);
	lat2 = DegToRad((flat2 * 1e-7) - 100.);
	delta_lat = lat2 - lat1;
	delta_lon = lon2 - lon1;
	a = (sin(delta_lat / 2.) * sin(delta_lat / 2.)) +
		((cos(lat1) * cos(lat2)) * (sin(delta_lon / 2.) * sin(delta_lon / 2.)));
	c = 2. * atan2(sqrt(a), sqrt(1. - a));
	// 6371e3 = raio médio da terra em m
	comp_m = 6371e3 * c;

	return (comp_m);
}

// ---------------------------------------------------------------------------
VTRede* __fastcall TConectaMaeFilha::ExisteAlimentador(int extern_id)
{ // VARIAVEIS LOCAIS

	VTRedes *redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
	VTRede *rede = NULL;
	VTRede *alimentador = NULL;
	TList *lisAlimentador;
	int id_externo_alimentador;

	lisAlimentador = new TList;

	redes->LisRede(lisAlimentador, redePRI);
	// percorre a lista de alimentadores
	for (int nr = 0; nr < lisAlimentador->Count; nr++)
	{
		rede = (VTRede*) lisAlimentador->Items[nr];
		if (rede->Carregada == false)
		{
			continue;
		}
		// verfifica se o id externo é igual
		try
		{
			id_externo_alimentador = StrToInt(rede->Extern_id);
		}
		catch (Exception &e)
		{
			// erro de conversao
			id_externo_alimentador = -1;
		}
		// compara
		if (id_externo_alimentador == extern_id)
		{
			alimentador = rede;
			break;
		}
	}
	// destroi lista
	if (lisAlimentador)
	{
		delete lisAlimentador;
		lisAlimentador = NULL;
	}
	return (alimentador);
}

// ---------------------------------------------------------------------------
VTBarra* __fastcall TConectaMaeFilha::ExisteBarra(VTBarra *barra_ref, double lat, double lon,
	double vnom_kv, VTRede *rede, int nivel, double range_percent)
{
	// variáveis locais
	int coord_x, coord_y;
	int delta_x = 0;
	int delta_y;
	VTBarra *barra, *barra_escolhida;
	TList *lisBarra;
	TList *lisCandidatas = new TList;
	double menor_dist, dist;

	barra_escolhida = NULL;
	// VTPreMonta *preMonta = (VTPreMonta*)apl->GetObject(__classid(VTPreMonta));
	VTRedes *redes = (VTRedes*)apl->GetObject(__classid(VTRedes));

	// verifica se foi indicada a Rede para busca da Barra
	if (rede != NULL)
	{
		lisBarra = rede->LisBarra();
	}
	else
	{
		lisBarra = redes->LisBarra();
	}

	if (IsDoubleZero(range_percent, 0.00001))
	{
		delta_x = 0.001 * lon;
		delta_y = 0.001 * lat;
	}
	else
	{
		delta_x = range_percent * lon;
		delta_y = range_percent * lat;
	}

	coord_x = lon;
	coord_y = lat;
	// determina a lista de candidatas
	for (int nb = 0; nb < lisBarra->Count; nb++)
	{
		barra = (VTBarra*) lisBarra->Items[nb];
		// verifica se esta dentro do quadrado
		if ((barra->utm.x > (coord_x - delta_x)) && (barra->utm.x < (coord_x + delta_x)))
		{
			if ((barra->utm.y > (coord_y - delta_y)) && (barra->utm.y < (coord_y + delta_y)))
			{
				if (barra_ref != barra)
				{
//					if (barra->Id > 0)
//					{
						lisCandidatas->Add(barra);
//					}
				}
			}
		}
	}
	// verifica menor distancia
	for (int nb = 0; nb < lisCandidatas->Count; nb++)
	{
		barra = (VTBarra*) lisCandidatas->Items[nb];
		if (nb == 0)
		{
			menor_dist = CalculaDistancia_m(coord_x, coord_y, barra->utm.x, barra->utm.y);
			barra_escolhida = barra;
			continue;
		}
		dist = CalculaDistancia_m(coord_x, coord_y, barra->utm.x, barra->utm.y);
		if (dist < menor_dist)
		{
			barra_escolhida = barra;
			menor_dist = dist;
		}
	}
	// delete
	if (lisCandidatas)
	{
		delete lisCandidatas;
		lisCandidatas = NULL;
	}
	return (barra_escolhida);
}

// ---------------------------------------------------------------------------
void __fastcall TConectaMaeFilha::TransfereLigacoesParaBarra2(VTRede *etd, VTBarra *barra2)
{
	// variaveis locais
	VTEdita *edita = (VTEdita*)apl->GetObject(__classid(VTEdita));
	VTRedes *redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
	TList *lisLig, *lisEqbar;
	VTEqbar *eqbar;
	VTLigacao *ligacao;
	VTBarra *barra1 = etd->BarraInicial();
	lisLig = etd->LisLigacao();

	// protecao
	if ((barra1 == NULL) || (barra2 == NULL))
	{
		return;
	}
	// protecao
	if ((barra1 == barra2))
	{
		return;
	}
	// pega tudo ligado à barra 1
	lisEqbar = barra1->LisEqbar();

	// move todos eqbar
	for (int neq = 0; neq < lisEqbar->Count; neq++)
	{
		eqbar = (VTEqbar*) lisEqbar->Items[neq];
		// verifica se é suprimento
		if (eqbar->Tipo() == eqptoSUPRIMENTO)
			continue;
		// eqbar->pbarra->RemoveEqbar(eqbar);
		// barra2->InsereEqbar(eqbar);
		if (edita)
		{
			edita->TrocaBarraEqbar(eqbar, barra2);
		}
	}
	// move todas ligacoes
	for (int nl = 0; nl < lisLig->Count; nl++)
	{
		ligacao = (VTLigacao*) lisLig->Items[nl];
		// verifica qual barra trocar
		if (ligacao->pbarra1 == barra1)
		{
			// caso seja essa, move essa e mantem as outras
			// ligacao->DefineObjBarra(barra2, ligacao->pbarra2, ligacao->pbarra3);
			if (edita)
			{
				edita->TrocaBarraLigacao(ligacao, barra1, barra2);
			}
		}
		else if (ligacao->pbarra2 == barra1)
		{
			// caso seja essa, move essa e mantem as outras
			// ligacao->DefineObjBarra(ligacao->pbarra1, barra2, ligacao->pbarra3);
			if (edita)
			{
				edita->TrocaBarraLigacao(ligacao, barra1, barra2);
			}
		}
		else if (ligacao->pbarra3 == barra1)
		{
			// caso seja essa, move essa e mantem as outras
			// ligacao->DefineObjBarra(ligacao->pbarra1, ligacao->pbarra2, barra2);
			if (edita)
			{
				edita->TrocaBarraLigacao(ligacao, barra1, barra2);
			}
		}
		else
		{
			continue;
		}
		// corrige trafo se necessario
		if (ligacao->Tipo() == eqptoTRAFO)
		{
			((VTTrafo*)ligacao)->RedefineVnom(barra2);
		}
		// else if(ligacao->Tipo() == eqptoTRAFO3E)
		// {
		// ((VTTrafo3E*)ligacao)->RedefineVnom(barra2);
		// }
	}
	// troca barra inical
	etd->DefineBarraInicial(barra2);

	lisBAR_ISO->Add(barra1);
}

// ---------------------------------------------------------------------------
bool __fastcall TConectaMaeFilha::TrataSEMaeFilha(void)
{
	// VARIAVEIS LOCAIS
	VTBarra *barra, *barra_prox;
	VTEdita *edita = (VTEdita*)apl->GetObject(__classid(VTEdita));
	VTRedes *redes = (VTRedes*)apl->GetObject(__classid(VTRedes));
	VTRede *rede;
	VTRede *etd;
	VTRede *alimentador;
	int multiplicador = 1;
	TList *lisREDES;
	// VTComentario *comentario;
	int id_externo_pai, id_externo_alimentador;
	int pos = -1;
	int len = 0;
	// bool sucesso = false;

	// lisREDES = redes->LisRede();
	lisREDES = new TList;
	redes->LisRedeCarregada(lisREDES);
	//clear nas listas
	lisBAR_ISO->Clear();
	lisEQPTO_MOD->Clear();
	// percorre a lista de redes
	for (int nr = 0; nr < lisREDES->Count; nr++)
	{
		alimentador = NULL;
		// comentario = NULL;
		rede = NULL;
		barra_prox = NULL;
		rede = (VTRede*) lisREDES->Items[nr];
		// comentario = redes->ExisteComentario(rede);
		len = 0;
		pos = -1;
		pos = rede->Extern_id.Pos("_");
		// se existir comentario
		// if(comentario)
		if (pos > 0)
		{ // converte comentario
			try
			{
				// id_externo_pai = StrToInt(comentario->Texto);
				len = rede->Extern_id.Length();
				id_externo_pai = StrToInt(rede->Extern_id.SubString((pos + 1), len));
			}
			catch (Exception &e)
			{
				id_externo_pai = -1;
			}
			// caso tenha e seja numero
			if (id_externo_pai >= 0)
			{
				alimentador = ExisteAlimentador(id_externo_pai);
			}
			// caso exista o alimentador
			if (alimentador)
			{
				barra = rede->BarraInicial();
				// caso possua barra inicial
				if (barra != NULL)
				{
					// encontra a barra proxima
					// 0.0056506043007377
					for (int retry = 0; retry < 3; retry++)
					{
						barra_prox = NULL;
						// por quantas vezes o range sera multiplicado
						multiplicador = 10 ^ retry;
						barra_prox = ExisteBarra(barra, barra->utm.y, barra->utm.x, barra->vnom,
							alimentador, -1, multiplicador * 0.00001);
						if (barra_prox != NULL)
						{
							break;
						}
					}
					// move ligacoes da barra AT para a barra aprox
					if (barra_prox)
					{
						TransfereLigacoesParaBarra2(rede, barra_prox);
						// limpa o extern id
						rede->Extern_id = rede->Extern_id.SubString(0, (pos - 1));
					}
				}
			}
			else
			{
				continue;
			}
		}
	}
	if (edita)
	{
		edita->RetiraLisEqpto(lisBAR_ISO);
	}
	if (lisREDES)
	{
		delete lisREDES;
		lisREDES = NULL;
	}
	return true; // FKM
}
// -----------------------------------------------------------------------------
