//---------------------------------------------------------------------------
#ifndef TSorteioH
#define TSorteioH
//---------------------------------------------------------------------------
class TSorteio
{
private:

public:
	// Construtor
	__fastcall TSorteio();

	// Operadores
	double __fastcall Uniforme(double a, double b);
	double __fastcall Normal(double mu, double sigma);

	// Métodos auxiliares
	double __fastcall Truncar(double x, int numDecimais);
};
//---------------------------------------------------------------------------
#endif
