//---------------------------------------------------------------------------

#ifndef TConectaMaeFilhaH
#define TConectaMaeFilhaH
#include "VTConectaMaeFilha.h"
#include <Classes.hpp>
//---------------------------------------------------------------------------
class VTApl;
class VTBarra;
class VTRede;
//class TSe;
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
class TConectaMaeFilha : public VTConectaMaeFilha
{
public:
			__fastcall TConectaMaeFilha(VTApl *apl);
			__fastcall ~TConectaMaeFilha(void);
	bool  	__fastcall Executa(void);

private:
	double     __fastcall CalculaDistancia_m(double flat1, double flon1,
													  double flat2, double flon2);
	VTRede*	   __fastcall ExisteAlimentador(int extern_id);
	VTBarra*   __fastcall ExisteBarra(VTBarra *barra_ref, double lat, double lon, double vnom_kv,
						VTRede *rede=NULL, int nivel=-1, double range_percent=0.001);
	void       __fastcall TransfereLigacoesParaBarra2(VTRede *etd, VTBarra *barra2);
	bool       __fastcall TrataSEMaeFilha(void);


	private:
	//dados externos
	//VTPreMonta *premonta;
	VTApl      *apl;
	TList *lisEXT_Subestacao;
	TList *lisBAR_ISO;
	TList *lisEQPTO_MOD;
	//dados internos
	public:

	// #ifdef _DEBUG
//	TLogThread *log_thread;
//	TLogManager *log_manager;
//	AnsiString msglog;
//	char *function;
	// #endif


};

// ---------------------------------------------------------------------------
#endif
// eof
