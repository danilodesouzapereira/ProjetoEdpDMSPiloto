String __fastcall DoubleToString(double val, int casasDec)
{
	double mult, parteInt;
   String strFinal = "", strAux = "";

   if(casasDec <= 0) return "";

   // Ajusta casas decimais
   mult = 1;
   for(int i=0; i<casasDec; i++) mult *= 10.;
   parteInt = val * mult;
   // Verifica arredondamento
   if((parteInt - (int)parteInt) > 0.5)
		val = ((int)parteInt + 1) / mult;
   else
	   val = (int)parteInt / mult;

   // Monta string de saída
   strAux = String(val);

   // Verifica se tem ","
   for(int i=1; i<strAux.Length()+1; i++)
   {
		if(strAux.SubString(i,1) == ",")
      {
         strFinal += ".";
      }
      else
      {
         strFinal += strAux.SubString(i,1);
      }
   }

   return strFinal;
}
//---------------------------------------------------------------------------
double __fastcall Round(double valIni, int ndigitos)
{
	double resp;
	double base = 1.;
   for(int i=0; i<ndigitos; i++) base *= 10.;

   resp = (int)(valIni * base)/base;
   return resp;
}
//---------------------------------------------------------------------------
