//---------------------------------------------------------------------------
#pragma hdrstop

#include "TBlocosRedeRadial.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include <PlataformaSinap\DLL_Inc\Bloco.h>
#include <PlataformaSinap\DLL_Inc\Ordena.h>
#include <PlataformaSinap\DLL_Inc\Rede.h>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Bloco\VTBloco.h>
#include <PlataformaSinap\Fontes\Bloco\VTBlocos.h>
#include <PlataformaSinap\Fontes\Bloco\VTElo.h>
#include <PlataformaSinap\Fontes\Diretorio\VTPath.h>
#include <PlataformaSinap\Fontes\Ordena\VTOrdena.h>
#include <PlataformaSinap\Fontes\Rede\VTBarra.h>
#include <PlataformaSinap\Fontes\Rede\VTChave.h>
#include <PlataformaSinap\Fontes\Rede\VTLigacao.h>
#include <PlataformaSinap\Fontes\Rede\VTRede.h>
#include <PlataformaSinap\Fontes\Rede\VTRedes.h>
#include <PlataformaSinap\Fontes\Rede\VTTipoRede.h>
#include <PlataformaSinap\Fontes\Rede\VTTrecho.h>
//---------------------------------------------------------------------------
__fastcall TBlocosRedeRadial::TBlocosRedeRadial(VTApl* apl, VTRedes* redes, VTRede* rede)
{
	this->apl = apl;
	this->rede = rede;
	this->redes = redes;

	if((ordena = (VTOrdena*) apl->GetObject(__classid(VTOrdena))) == NULL)
		this->ordena = DLL_NewObjOrdena(this->apl);
	this->ordena->Executa(this->redes);
	blocos = (VTBlocos*) apl->GetObject(__classid(VTBlocos));
	if(blocos == NULL)
	{
		blocos = DLL_NewObjBlocos();
		apl->Add(blocos);
	}

	// Inicializa blocos, executando para as redes
	blocos->Executa(redes);

	lisStrBlocos = new TList();

	Inicializa();

	VTLigacao* liga = NULL;
	for(int i=0; i<rede->LisLigacao()->Count; i++)
	{
		liga = (VTLigacao*) rede->LisLigacao()->Items[i];
		if(liga->Codigo == "97385902")
			break;
	}
	TList* lisAux = new TList();
	GetBlocosJusanteLigacao(liga, lisAux);
}
//---------------------------------------------------------------------------
__fastcall TBlocosRedeRadial::~TBlocosRedeRadial()
{
	// Destroi listas
	delete lisStrBlocos; lisStrBlocos = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TBlocosRedeRadial::AtualizaStrBlocos(StrBloco* strBlocoMont, StrBloco* strBlocoJus)
{
//	if(strBlocoMont == NULL || strBlocoJus == NULL) return;
//
//	if()
}
//---------------------------------------------------------------------------
bool __fastcall TBlocosRedeRadial::ExisteStrBloco(StrBloco* strBlocoRef)
{
	for(int i=0; i<lisStrBlocos->Count; i++)
	{
		StrBloco* strBloco = (StrBloco*) lisStrBlocos->Items[i];
		if(strBloco->bloco == strBlocoRef->bloco)
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool __fastcall TBlocosRedeRadial::ExisteStrBlocoJusante(StrBloco* strBlocoMont, StrBloco* strBlocoJus)
{
	for(int i=0; i<strBlocoMont->lisStrBlocosJusante->Count; i++)
	{
		StrBloco* strBloco = (StrBloco*) strBlocoMont->lisStrBlocosJusante->Items[i];
		if(strBloco->bloco == strBlocoJus->bloco)
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void __fastcall TBlocosRedeRadial::Inicializa()
{
	VTBloco *bloco1, *bloco2;
	StrBloco *strBlocoMont, *strBlocoJus;
	TList* listaAuxElos;


	listaAuxElos = new TList();
	for(int i=0; i<blocos->LisElo()->Count; i++)
	{
		VTElo* elo = (VTElo*) blocos->LisElo()->Items[i];
		if(elo->Chave->Aberta) continue;
		if(elo->Chave->rede != rede)	continue;
		listaAuxElos->Add(elo);
	}

	int cont1=0, cont2=0, cont3=0, cont4=0, cont5=0, cont6=0, cont7=0, cont8=0;
	for(int i=0; i<listaAuxElos->Count; i++)
	{
		VTElo* elo = (VTElo*) listaAuxElos->Items[i];

		//debug
		VTChave* chv = elo->Chave;
		if(chv->Codigo == "J50017")
		{
			int a = 0;
      }


		bloco1 = elo->Bloco1;
		bloco2 = elo->Bloco2;

		if(bloco1->ExisteBarra(elo->Chave->ligaPai->Barra(0)) || bloco1->ExisteBarra(elo->Chave->ligaPai->Barra(1)))
		{
			strBlocoMont = GetStrBloco(bloco1);
			strBlocoJus = GetStrBloco(bloco2);

			if(strBlocoMont != NULL && strBlocoJus != NULL)
			{
				cont1 ++;
				if(!ExisteStrBlocoJusante(strBlocoMont, strBlocoJus))
					strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;
			}
			else if(strBlocoMont != NULL && strBlocoJus == NULL)
			{
				cont2++;
				strBlocoJus = new StrBloco();
				strBlocoJus->lisStrBlocosJusante = new TList();
				strBlocoJus->bloco = bloco2;
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;
				strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);

				lisStrBlocos->Add(strBlocoJus);
			}
			else if(strBlocoMont == NULL && strBlocoJus != NULL)
			{
				cont3++;
				strBlocoMont = new StrBloco();
				strBlocoMont->lisStrBlocosJusante = new TList();
				strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoMont->bloco = bloco1;
				strBlocoMont->strBlocoMontante = NULL;
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;

				lisStrBlocos->Add(strBlocoMont);
			}
			else
			{
				cont4++;
				strBlocoJus = new StrBloco();
				strBlocoJus->lisStrBlocosJusante = new TList();
				strBlocoJus->bloco = bloco2;

				strBlocoMont = new StrBloco();
				strBlocoMont->lisStrBlocosJusante = new TList();
				strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoMont->bloco = bloco1;
				strBlocoMont->strBlocoMontante = NULL;
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;

				lisStrBlocos->Add(strBlocoMont);
				lisStrBlocos->Add(strBlocoJus);
			}
		}
		else if(bloco2->ExisteBarra(elo->Chave->ligaPai->Barra(0)) || bloco2->ExisteBarra(elo->Chave->ligaPai->Barra(1)))
		{
			strBlocoMont = GetStrBloco(bloco2);
			strBlocoJus = GetStrBloco(bloco1);

			if(strBlocoMont != NULL && strBlocoJus != NULL)
			{
				cont5++;
				if(!ExisteStrBlocoJusante(strBlocoMont, strBlocoJus))
					strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;
			}
			else if(strBlocoMont != NULL && strBlocoJus == NULL)
			{
				cont6++;
				strBlocoJus = new StrBloco();
				strBlocoJus->lisStrBlocosJusante = new TList();
				strBlocoJus->bloco = bloco1;
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoJus->chaveMont = elo->Chave;
				lisStrBlocos->Add(strBlocoJus);
			}
			else if(strBlocoMont == NULL && strBlocoJus != NULL)
			{
				cont7++;
				strBlocoMont = new StrBloco();
				strBlocoMont->lisStrBlocosJusante = new TList();
				strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoMont->bloco = bloco2;
				strBlocoMont->strBlocoMontante = NULL;
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;

				lisStrBlocos->Add(strBlocoMont);
			}
			else
			{
				cont8++;
				strBlocoJus = new StrBloco();
				strBlocoJus->lisStrBlocosJusante = new TList();
				strBlocoJus->bloco = bloco1;

				strBlocoMont = new StrBloco();
				strBlocoMont->lisStrBlocosJusante = new TList();
				strBlocoMont->lisStrBlocosJusante->Add(strBlocoJus);
				strBlocoMont->bloco = bloco2;
				strBlocoMont->strBlocoMontante = NULL;
				strBlocoJus->strBlocoMontante = strBlocoMont;
				strBlocoJus->chaveMont = elo->Chave;

				lisStrBlocos->Add(strBlocoMont);
				lisStrBlocos->Add(strBlocoJus);
			}
		}
	}

	TList* lisAux = new TList();
	for(int i=0; i<lisStrBlocos->Count; i++)
	{
		StrBloco* strBloco = (StrBloco*) lisStrBlocos->Items[i];
		if(strBloco->lisStrBlocosJusante->Count == 0)
			lisAux->Add(strBloco);
	}

	// Destroi listas
   delete listaAuxElos; listaAuxElos = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TBlocosRedeRadial::GetBlocosJusanteLigacao(VTLigacao* ligacao, TList* lisEXT)
{
	if(ligacao == NULL || lisEXT == NULL) return;

	TList* lisAux = new TList();

	StrBloco* strBlocoRef = GetStrBloco(ligacao);
	for(int i=0; i<strBlocoRef->lisStrBlocosJusante->Count; i++)
	{
		StrBloco* strBlocoJus = (StrBloco*) strBlocoRef->lisStrBlocosJusante->Items[i];
		lisAux->Add(strBlocoJus);
	}

	while(lisAux->Count > 0)
	{
		// Adiciona os blocos na lista externa
		for(int i=lisAux->Count-1; i>=0; i--)
		{
			StrBloco* strBloco = (StrBloco*) lisAux->Items[i];
			lisEXT->Add(strBloco->bloco);

			for(int j=0; j<strBloco->lisStrBlocosJusante->Count; j++)
			{
				StrBloco* strBlocoJus = (StrBloco*) strBloco->lisStrBlocosJusante->Items[j];
				lisAux->Insert(0, strBlocoJus);
			}
			lisAux->Remove(strBloco);
		}
	}

	delete lisAux; lisAux = NULL;
}
//---------------------------------------------------------------------------
StrBloco* __fastcall TBlocosRedeRadial::GetStrBloco(VTBloco* bloco)
{
	if(bloco == NULL) return NULL;

	StrBloco* strBloco = NULL;
	for(int i=0; i<lisStrBlocos->Count; i++)
	{
		strBloco = (StrBloco*) lisStrBlocos->Items[i];
		if(strBloco->bloco == bloco)
			break;
		else
			strBloco = NULL;
	}
	return(strBloco);
}
//---------------------------------------------------------------------------
StrBloco* __fastcall TBlocosRedeRadial::GetStrBloco(VTLigacao* ligacao)
{
	if(ligacao == NULL) return NULL;

	StrBloco* strBloco = NULL;
	if(ligacao->Tipo() == eqptoCHAVE)
	{
		VTChave* chave = (VTChave*) ligacao;
		for(int i=0; i<lisStrBlocos->Count; i++)
		{
			strBloco = (StrBloco*) lisStrBlocos->Items[i];
			if(strBloco->chaveMont == chave)
				break;
			else
				strBloco = NULL;
		}
	}
	else
	{
		for(int i=0; i<lisStrBlocos->Count; i++)
		{
			strBloco = (StrBloco*) lisStrBlocos->Items[i];
			if(strBloco->bloco->ExisteLigacao(ligacao))
				break;
			else
				strBloco = NULL;
		}
	}
	return(strBloco);
}
//---------------------------------------------------------------------------
StrBloco* __fastcall TBlocosRedeRadial::GetStrBlocoMontante(VTBloco* bloco)
{
	if(bloco == NULL)
		return NULL;

	StrBloco* strBloco = NULL;
	for(int i=0; i<lisStrBlocos->Count; i++)
	{
		strBloco = (StrBloco*) lisStrBlocos->Items[i];
		if(strBloco->bloco == bloco)
			break;
		else
			strBloco = NULL;
	}
	return(strBloco);
}
//---------------------------------------------------------------------------
void __fastcall TBlocosRedeRadial::InserirStrBlocoJusante(StrBloco* strBlocoMontante, StrBloco* strBlocoJusante)
{
//	if(strBlocoMontante == NULL || strBlocoJusante == NULL) return;
//
//	strBlocoMontante->lisStrBlocosJusante->Add(strBlocoJusante);
//	InserirStrBlocoJusante(strBlocoMontante->blocoMontante, strBlocoMontante);
}
//---------------------------------------------------------------------------
