/***
 * Classe para Indiv�duos de algoritmos de Estrat�gias Evolutivas
 **/
//---------------------------------------------------------------------------
#pragma hdrstop
#include "TIndividuoEE.h"
#include "TEstrategiaEvolutiva.h"
#include "TSorteio.h"
#include "TConfiguracoes.h"
#include "..\TAreaBusca.h"
#include "..\Auxiliares\FuncoesFL.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
/***
 * Construtor 1
 **/
__fastcall TIndividuoEE::TIndividuoEE()
{
	// Par�metros b�sicos
 	ID = -1;
   IDpaisCruzamento[0] = -1;
   IDpaisCruzamento[1] = -1;
   IDpaiMutacao = -1;
	sorteio = NULL;
	config = NULL;
   IDbarra1 = -1;
   IDbarra2 = -1;
   xPorc = -1;
   codigoTrecho = "";

	// Seta par�metros do indiv�duo
	this->X = -1;
	this->Rf = -1;
	this->sigmaX = -1;
	this->sigmaRf = -1;
   indiceErro = -1.;
   idade = 0;
   IDbarra1 = -1;
   IDbarra2 = -1;
   xPorc = -1;
   codigoTrecho = "";
}
//---------------------------------------------------------------------------
/***
 * Construtor 2
 **/
__fastcall TIndividuoEE::TIndividuoEE(TEstrategiaEvolutiva* estrEvol, double X, double Rf, double sigmaX, double sigmaRf)
{
	String strLocal, valor;

	// Par�metros b�sicos e refer�ncias
	ID = -1;
   IDpaisCruzamento[0] = -1;
   IDpaisCruzamento[1] = -1;
   IDpaiMutacao = -1;
	sorteio = NULL;
	config = NULL;
	this->estrEvol = estrEvol;

	// Seta par�metros do indiv�duo
	this->X = X;
	this->Rf = Rf;
	this->sigmaX = sigmaX;
	this->sigmaRf = sigmaRf;
   indiceErro = -1.;
   idade = 0;
   IDbarra1 = -1;
   IDbarra2 = -1;
   xPorc = -1;
   codigoTrecho = "";

   // A partir do X (%), determina barra1, barra2 e xporc entre elas
   AjustaLocalizacaoIndividuo();
}
//---------------------------------------------------------------------------
/***
 * Seta a rela��o de-para entre X e barra1, barra2 e xporc
 */
void __fastcall TIndividuoEE::AjustaLocalizacaoIndividuo()
{
	String infoLocal, valor;
   TAreaBusca* areaBusca;

   areaBusca = estrEvol->GetAreaBusca();
   // Informa��es do local da falta
   infoLocal = areaBusca->GetTrechoBarra1Barra2LocalDiscret(X);
   // c�digo do trecho
   codigoTrecho = GetCampoCSV(infoLocal, 0, ";");
   // Local da falta: ID da barra 1
   valor = GetCampoCSV(infoLocal, 1, ";");
   IDbarra1 = valor.ToInt();
   // Local da falta: ID da barra 2
   valor = GetCampoCSV(infoLocal, 2, ";");
   IDbarra2 = valor.ToInt();
   // Local da falta: x(%) entre barra 1 e barra 2
   valor = GetCampoCSV(infoLocal, 3, ";");
   xPorc = valor.ToDouble();
}
//---------------------------------------------------------------------------
/***
 * Operador Cruzamento, operando sobre este indiv�duo e um segundo, gerando 1 filho
 **/
TIndividuoEE* __fastcall TIndividuoEE::Cruzamento(TIndividuoEE* indiv2, double pc)
{
	double X, Rf, sigmaX, sigmaRf, a;
   double X1, X2, Rf1, Rf2;
   double sigmaX1, sigmaX2, sigmaRf1, sigmaRf2;
   double p;

	// Verifica o tipo de cruzamento
	switch(this->config->GetTipoCruzamento())
	{
		case mediaLocal:
			a = sorteio->Uniforme(0.0, 1.0);
			break;

		case mediaGlobal:
			a = 0.5;
			break;
	}

   // Verifica se haver� cruzamento
   p = sorteio->Uniforme(0.0, 1.0);
   if(p > pc)
   {
		return NULL;
   }

   // Caracter�sticas indiv 1
   X1       = this->X;
   Rf1      = this->Rf;
   sigmaX1  = this->sigmaX;
   sigmaRf1 = this->sigmaRf;
   // Caracter�sticas indiv 2
   X2       = indiv2->X;
   Rf2      = indiv2->Rf;
   sigmaX2  = indiv2->sigmaX;
   sigmaRf2 = indiv2->sigmaRf;
	// Calcula os par�metros do indiv�duo filho
	X = a * X1 + (1. - a) * X2;
	Rf = a * Rf1 + (1. - a) * Rf2;
	sigmaX = a * sigmaX1 + (1. - a) * sigmaX2;
	sigmaRf = a * sigmaRf1 + (1. - a) * sigmaRf2;

	// Cria indiv�duo filho e seta alguns par�metros
	TIndividuoEE* filho = new TIndividuoEE(this->estrEvol, X, Rf, sigmaX, sigmaRf);
   filho->IDpaisCruzamento[0] = this->ID;
   filho->IDpaisCruzamento[1] = indiv2->ID;
	filho->SetSorteio(sorteio);
	filho->SetConfig(config);
	
	return filho;
}
//---------------------------------------------------------------------------
/***
 * Operador Muta��o. Opera sobre um �nico indiv�duo, gerando v�rios filhos
 * Sa�da: TList*
 **/
TList* __fastcall TIndividuoEE::Mutacao(double pm)
{
	// Defini��o de par�metros
	double p;
	double deltaX, deltaRf;
	double X, Rf, sigmaX, sigmaRf;
   double Xini, Rfini, sigmaXini, sigmaRfini;
	int numFilhosMut = config->GetNumFilhosMutacao();
	bool HouveMutacao = false;
   double N01, N1_01, N2_01;
   double tau, tau_linha, beta, lambida;

	// Verifica��o
   if(pm < 0. || pm > 1.) return NULL;

  	TList* listaSaida = new TList();

   // Valores num�ricos empregados
   beta = 2.;
   lambida = 0.0873;
   tau_linha = 1. / sqrt(2. * beta);
   tau = 1. / sqrt(2. * sqrt(beta));

   // Guarda os par�metros do indiv�duo pai
   Xini = this->X;
   Rfini = this->Rf;
   sigmaXini = this->sigmaX;
   sigmaRfini = this->sigmaRf;

	// Verifica, por algumas vezes, se algumas caracter�sticas v�o sofrer muta��o
	for(int i=0; i<numFilhosMut; i++)
	{
   	// N�mero aleat�rio para o indiv�duo
      N01 = sorteio->Normal(0.0, 1.0);

		// :::::::::::::::::::::::::::::::::::::::::::
      // Verifica muta��o de X
		// :::::::::::::::::::::::::::::::::::::::::::

      // N�mero aleat�rio para varia��o de X
      N1_01 = sorteio->Normal(0.0, 1.0);
      sigmaX = sigmaXini * exp(tau_linha*N01 + tau*N1_01);  //auto-adapta��o
      // Sorteia muta��o de X
		p = sorteio->Uniforme(0.0, 1.0);
		if(p <= pm)
		{
			deltaX = this->sigmaX * N1_01;
			HouveMutacao = true;
		}
		else
		{
      	deltaX = 0.;
		}

 		// :::::::::::::::::::::::::::::::::::::::::::
		// Verifica muta��o de Rf
		// :::::::::::::::::::::::::::::::::::::::::::

      // N�mero aleat�rio para varia��o de Rf
      N2_01 = sorteio->Normal(0.0, 1.0);
      sigmaRf = sigmaRfini * exp(tau_linha*N01 + tau*N2_01); //auto-adapta��o
      // Sorteia muta��o de Rf
		p = sorteio->Uniforme(0.0, 1.0);
		if(p <= pm)
		{
      	deltaRf = sigmaRf * N2_01;
			HouveMutacao = true;
		}
		else
		{
      	deltaRf = 0.;
      }
		
		// Se n�o houve muta��o de pelo menos um par�metro, ignora indiv�duo novo
		if(!HouveMutacao) continue;

      // :::: OBS: SE FOREM CONSIDERADAS ROTA��ES, IMPLEMENTAR AQUI:

      // :::::::::::::::::::::::::::::::::::::::::::
		// Cria objeto de indiv�duo para o filho gerado
		// :::::::::::::::::::::::::::::::::::::::::::

      // Aplica as corre��es de X e Rf
      X = Xini + deltaX;
      Rf = Rfini + deltaRf;

      // Verifica se os par�metros X e Rf s�o consistentes
      if(X < 0. || X > 100.) continue;

      // Cria indiv�duo com os par�metros sorteados
      TIndividuoEE* indiv = new TIndividuoEE(this->estrEvol, X, Rf, sigmaX, sigmaRf);
      indiv->SetSorteio(sorteio);
      indiv->SetConfig(config);
      indiv->IDpaiMutacao = this->ID;
      // Adiciona indiv�duo � lista externa
      listaSaida->Add(indiv);
    }

    return listaSaida;
}
//---------------------------------------------------------------------------
/***
 * Retorna o valor da fun��o de avalia��o
 */
double __fastcall TIndividuoEE::GetAvaliacao()
{
	return fAval;
}
//---------------------------------------------------------------------------
int __fastcall TIndividuoEE::GetIDbarra1()
{
	return IDbarra1;
}
//---------------------------------------------------------------------------
int __fastcall TIndividuoEE::GetIDbarra2()
{
	return IDbarra2;
}
//---------------------------------------------------------------------------
double __fastcall TIndividuoEE::GetXporc()
{
	return xPorc;
}
//---------------------------------------------------------------------------
int __fastcall TIndividuoEE::GetIdade()
{
	return idade;
}
//---------------------------------------------------------------------------
double __fastcall TIndividuoEE::GetIndiceErro()
{
	return indiceErro;
}
//---------------------------------------------------------------------------
/***
 * Retorna o valor do par�metro X do indiv�duo
 */
double __fastcall TIndividuoEE::GetX()
{
	return this->X;
}
//---------------------------------------------------------------------------
/***
 * Retorna o valor do par�metro Rf do indiv�duo
 */
double __fastcall TIndividuoEE::GetRf()
{
	return this->Rf;
}
//---------------------------------------------------------------------------	
double __fastcall TIndividuoEE::GetSigmaX()
{
	return this->sigmaX;
}
//---------------------------------------------------------------------------	
double __fastcall TIndividuoEE::GetSigmaRf()
{
	return this->sigmaRf;
}
//---------------------------------------------------------------------------
void __fastcall TIndividuoEE::IncrementaIdade()
{
	idade += 1;
}
//---------------------------------------------------------------------------
/***
 * A partir do �ndice de erro, determina a fun��o de avalia��o.
 * Quanto menor o valor de "indiceErro", maior o valor de "fAval".
 * fAval fica entre 0 e 100%.
 */
void __fastcall TIndividuoEE::SetAvaliacao(double indiceErro)
{
   // Salva o �ndice de erro
   this->indiceErro = indiceErro;

   // Fun��o de avalia��o normalizada (em %)
	fAval = 1. - indiceErro;
   fAval *= 100.;

   // Satura limite inferior em zero
   if(fAval < 0.) fAval = 0.;
}
//---------------------------------------------------------------------------
void __fastcall TIndividuoEE::SetIndiceErro(double indiceErro)
{
	this->indiceErro = indiceErro;
}
//---------------------------------------------------------------------------
void __fastcall TIndividuoEE::SetLocalizacao(int IDbarra1, int IDbarra2, double xPorc)
{
   this->IDbarra1 = IDbarra1;
   this->IDbarra2 = IDbarra2;
   this->xPorc = xPorc;
}
//---------------------------------------------------------------------------
void __fastcall TIndividuoEE::SetX(double X)
{
	this->X = X;
}
//---------------------------------------------------------------------------	
void __fastcall TIndividuoEE::SetRf(double Rf)
{
	this->Rf = Rf;
}
//---------------------------------------------------------------------------	
void __fastcall TIndividuoEE::SetSigmaX(double sigmaX)
{
	this->sigmaX = sigmaX;
}
//---------------------------------------------------------------------------	
void __fastcall TIndividuoEE::SetSigmaRf(double sigmaRf)
{
	this->sigmaRf = sigmaRf;
}
//---------------------------------------------------------------------------
void __fastcall TIndividuoEE::SetSorteio(TSorteio* sorteio)
{
	this->sorteio = sorteio;
}
//---------------------------------------------------------------------------
void __fastcall TIndividuoEE::SetConfig(TConfiguracoes* config)
{
	this->config = config;
}
//---------------------------------------------------------------------------
