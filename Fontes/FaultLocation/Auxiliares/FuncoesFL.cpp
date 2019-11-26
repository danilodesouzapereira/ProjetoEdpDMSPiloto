//---------------------------------------------------------------------------
#pragma hdrstop
#include "FuncoesFL.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
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
int __fastcall CSVCamposCount(String linha, String separador)
{
	int contador, comp;
   String substr;

   // Adiciona ";" ao final da string
   if(linha.SubString(linha.Length(),1) != ";")
   	linha += ";";

   // Percorre os campos da linha
   comp = linha.Length();

   contador = 0;
   for(int i=1; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
      if(substr == separador)
      {
      	contador++;
      }
   }

   return contador;
}
//---------------------------------------------------------------------------
String __fastcall GetSubstr(String strInicial, String strPos1, String strPos2)
{
	int pos1, pos2;
	String resp = "";

	if(strInicial == "") return "";

	pos1 = pos2 = 1;
	for(int i=1; i<=strInicial.Length(); i++)
	{
		if(strInicial.SubString(i,1) == strPos1)
		{
			pos1 = i;
			break;
		}
	}

	for (int i=pos1; i<=strInicial.Length(); i++)
	{
		if(strInicial.SubString(i,1) == strPos2)
		{
			pos2 = i;
			break;
		}
	}

	resp = strInicial.SubString(pos1+1, pos2 - pos1 - 1);
	return(resp);
}
//---------------------------------------------------------------------------
int __fastcall GetPosSubstr(String strTotal, String substr)
{
	int pos1 = -1;
	int len = substr.Length();
	for(int i=1; i<=strTotal.Length(); i++)
	{
		if(strTotal.SubString(i, len) == substr)
		{
			pos1 = i;
			break;
      }
	}
	return (pos1);
}
//---------------------------------------------------------------------------
int __fastcall ContemSubstr(String strTotal, String substr)
{
	if(GetPosSubstr(strTotal, substr) != -1)
		return(true);

	return(false);
}
//---------------------------------------------------------------------------
bool __fastcall ContemSubstring(String strTotal, String substr)
{
	if(GetPosSubstr(strTotal, substr) != -1)
		return(true);

	return(false);
}
//---------------------------------------------------------------------------
String __fastcall GetCampoCSV(String linha, int indiceCampo, String separador)
{
	int comp;
   int pos1, tamanho, iCampo;
   String substr, strFinal;

   // Adiciona ";" ao final da string
   if(linha.SubString(linha.Length(),1) != ";")
   	linha += ";";

   // Inicializa resposta
   strFinal = "";

   // Percorre os campos da linha
   comp = linha.Length();
   pos1 = 1;
   tamanho = 0;
   iCampo = 0;
   for(int i=1; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
      if(substr != separador)
      {
      	tamanho++;
	      continue;
      }

		if(iCampo < indiceCampo)
      {
      	iCampo++;
         pos1 = i+1;
			tamanho = 0;
         continue;
      }

      strFinal = linha.SubString(pos1, tamanho);
      break;
   }

   return strFinal;
}
//---------------------------------------------------------------------------
int __fastcall NumeroCampos(String linha, String separador)
{
	int contador, comp;
   String substr;

   // Adiciona separador ao final da string
   if(linha.SubString(linha.Length(),1) != separador)
   	linha += separador;

   // Percorre os campos da linha
   comp = linha.Length();

   contador = 0;
   for(int i=1; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
      if(substr == separador)
      {
      	contador++;
      }
   }

   return contador;
}
//---------------------------------------------------------------------------
String __fastcall GetCampo(String linha, int indiceCampo, String separador)
{
	int comp;
   int pos1, tamanho, iCampo;
   String substr, strFinal;

   // Adiciona separador ao final da string
   if(linha.SubString(linha.Length(),1) != separador)
   	linha += separador;

   // Inicializa resposta
   strFinal = "";

   // Percorre os campos da linha
   comp = linha.Length();
   pos1 = 1;
   tamanho = 0;
   iCampo = 0;
   for(int i=1; i<=comp; i++)
   {
		substr = linha.SubString(i,1);
      if(substr != separador)
      {
      	tamanho++;
	      continue;
      }

		if(iCampo < indiceCampo)
      {
      	iCampo++;
         pos1 = i+1;
         tamanho = 0;
         continue;
      }

      strFinal = linha.SubString(pos1, tamanho);
      break;
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
String __fastcall StrReplace(String strIni, int indice, String valor)
{
	String strFinal = "";

   strFinal = strIni.SubString(1, indice);
   strFinal += valor;
   strFinal += strIni.SubString(indice+2, strIni.Length()-indice-2);

   return strFinal;
}
//---------------------------------------------------------------------------
String __fastcall StrReplace(String strIni, String valorAnt, String valorNovo)
{
   int cont;
	String strFinal = "", valor;

   cont = strIni.Length();
   for(int i=1; i<cont+1; i++)
   {
   	valor = strIni.SubString(i,1);
		if(valor != valorAnt)
      {
      	strFinal += valor;
      }
      else
      {
         strFinal += valorNovo;
      }
   }

   return strFinal;
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
