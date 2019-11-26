/***
 * CLASSE PARA LIDAR COM NÚMEROS ALEATÓRIOS E DISTRIBUIÇÕES
 **/
//---------------------------------------------------------------------------
#pragma hdrstop
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "TSorteio.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
/***
 * Construtor da classe de sorteios
 **/
__fastcall TSorteio::TSorteio()
{
	srand((unsigned)time(NULL));
}
//---------------------------------------------------------------------------
/***
 * Gera número aleatório de distribuição uniforme do intervalo [a,b].
 **/
double __fastcall TSorteio::Uniforme(double a, double b)
{
	double lim1, lim2, x;

	if(a < b)
	{
		lim1 = a;
		lim2 = b;
	}
	else if(a == b)
	{
		return a;
	}
	else if(a > b)
	{
		lim1 = b;
		lim2 = a;
	}

	// Sorteia uniforme entre 0 e 1
	x = (rand()%1000+1) / 1000.;
	// Multiplica pela largura do intervalo
	x *= (lim2 - lim1);
	// Translada
	x += lim1;

	// Trunca a resposta
	x = Truncar(x, 2);

	return x;
}
//---------------------------------------------------------------------------
/***
 * Gera número aleatório com distribuição normal, de média mu e d.padr. sigma
 **/
double __fastcall TSorteio::Normal(double mu, double sigma)
{
	double U1, U2, W, mult;
	static double X1, X2;
	static int call = 0;
	double resp;

	if (call == 1)
	{
		call = !call;
		resp = (mu + sigma * (double) X2);
		resp = Truncar(resp, 2);
		return resp;
	}

	do
	{
		U1 = -1 + ((double) rand () / RAND_MAX) * 2;
		U2 = -1 + ((double) rand () / RAND_MAX) * 2;
		W = pow (U1, 2) + pow (U2, 2);
	}
	while (W >= 1 || W == 0);

	mult = sqrt ((-2 * log (W)) / W);
	X1 = U1 * mult;
	X2 = U2 * mult;

	call = !call;

	resp = (mu + sigma * (double) X1);

	resp = Truncar(resp, 2);

	return resp;
}
//---------------------------------------------------------------------------
/***
 * Método para trucar um double com "numDecimais" algarismos após a vírgula
 **/
double __fastcall TSorteio::Truncar(double x, int numDecimais)
{
	double fator = 1.0;
	for(int i=0; i<numDecimais; i++) fator *= 10.;

	double resp = ((int)(x * fator)) / fator;

	return resp;
}
//---------------------------------------------------------------------------
