// ::::::::::::::::::::::::::::::::::::::::
// ENUMERADORES
// ::::::::::::::::::::::::::::::::::::::::
//---------------------------------------------------------------------------
enum TipoFalta
{
	faltaINDEF = -1, // Falta de tipo indefinido
   falta3F,         // Falta trifásica
   faltaAB,         // Falta dupla-fase
   faltaBC,
   faltaCA,
   faltaABG,        // Falta dupla-fase-terra
   faltaBCG,
   faltaCAG,
   faltaAG,         // Falta fase-terra
   faltaBG,
   faltaCG,
   falta2F_3F       // Bifásica ou trifásica
};
//---------------------------------------------------------------------------
enum TipoRompCabo
{
	rompINDEF = -1, // Falta de tipo indefinido
	rompABC,         // Falta trifásica
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
   fonteQUALIMETRO   // falta classificada a partir do Qualímetro na saída do alimentador
};
//---------------------------------------------------------------------------
enum TipoLocalizacaoFaltas
{
	faultlocationINDEF = -1,    // indefinido
	faultlocationFS,            // Fault Study (FL 1)        ==> necessário pelo menos 1 corrente de defeito
	faultlocationALGOFASORIAL,  // Algoritmo fasorial (FL 2) ==> necessários fasores
	faultlocationROMPCABO,      // Rompimento de cabo (FL 3) ==> necessários afundamentos de tensão
	faultlocationDMS1,      // Localizador LF1 do DMS da EDP ==> Localização de faltas que não envolvem a terra. Similar ao FL1.
	faultlocationDMS2,      // Localizador LF1 do DMS da EDP ==> Localização de faltas que envolvem a terra.
	faultlocationDMS3    // Localizador LF2 do DMS da EDP ==> Localização de rompimento de cabo. Similar ao FL3.
};
//---------------------------------------------------------------------------
