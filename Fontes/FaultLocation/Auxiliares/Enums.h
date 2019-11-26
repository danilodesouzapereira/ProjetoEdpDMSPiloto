// ::::::::::::::::::::::::::::::::::::::::
// ENUMERADORES
// ::::::::::::::::::::::::::::::::::::::::
//---------------------------------------------------------------------------
enum TipoFalta
{
	faltaINDEF = -1, // Falta de tipo indefinido
   falta3F,         // Falta trif�sica
   faltaAB,         // Falta dupla-fase
   faltaBC,
   faltaCA,
   faltaABG,        // Falta dupla-fase-terra
   faltaBCG,
   faltaCAG,
   faltaAG,         // Falta fase-terra
   faltaBG,
   faltaCG,
   falta2F_3F       // Bif�sica ou trif�sica
};
//---------------------------------------------------------------------------
enum TipoRompCabo
{
	rompINDEF = -1, // Falta de tipo indefinido
	rompABC,         // Falta trif�sica
	rompAB,         // Falta dupla-fase
	rompBC,
	rompCA,
	rompA,
	rompB,
	rompC
};
//---------------------------------------------------------------------------
enum FonteClassificacao
{
	fonteINDEF = -1,  // indefinido
	fontePROTECAO,    // falta classificada a partir de DJ/RE
   fonteSENSOR,      // falta classificada a partir de Sensor
   fonteQUALIMETRO   // falta classificada a partir do Qual�metro na sa�da do alimentador
};
//---------------------------------------------------------------------------
enum TipoLocalizacaoFaltas
{
	faultlocationINDEF = -1,    // indefinido
	faultlocationFS,            // Fault Study (FL 1)        ==> necess�rio pelo menos 1 corrente de defeito
	faultlocationALGOFASORIAL,  // Algoritmo fasorial (FL 2) ==> necess�rios fasores
	faultlocationROMPCABO,      // Rompimento de cabo (FL 3) ==> necess�rios afundamentos de tens�o
	faultlocationDMS1,      // Localizador LF1 do DMS da EDP ==> Localiza��o de faltas que n�o envolvem a terra. Similar ao FL1.
	faultlocationDMS2,      // Localizador LF1 do DMS da EDP ==> Localiza��o de faltas que envolvem a terra.
	faultlocationDMS3    // Localizador LF2 do DMS da EDP ==> Localiza��o de rompimento de cabo. Similar ao FL3.
};
//---------------------------------------------------------------------------
