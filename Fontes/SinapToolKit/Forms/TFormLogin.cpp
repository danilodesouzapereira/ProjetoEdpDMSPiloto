// ---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <Oracle.hpp>
#include <TypInfo.hpp>
#include <DateUtils.hpp>
#include <PlataformaSinap\Fontes\Apl\VTApl.h>
#include <PlataformaSinap\Fontes\Geral\VTGeral.h>
#include <PlataformaSinap\Fontes\Geral\VTInfoset.h>
#include <PlataformaSinap\DLL_Inc\Funcao.h>
#include "TFormLogin.h"

// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

// ---------------------------------------------------------------------------
__fastcall TFormLogin::TFormLogin(TComponent* Owner, VTApl *apl) : TForm(Owner)
{
	// salva ponteiro para objetos
	this->apl = apl;
	// define parâmetros de acesso à base Oracle
	empresa.TNSname = "NeoEnergia_SINAP";
	empresa.Username = "SNP107848";
	empresa.Password = "SNP107848";
	// lê dados para conexão
	DadosConexaoRead();
	// inicia componentes
	EditOracleTNSname->Text = empresa.TNSname;
	EditOracleUsername->Text = empresa.Username;
	EditOraclePassword->Text = empresa.Password;
	// desabilita mostrar os caracteres da senha
	EditOraclePassword->PasswordChar = '*';
}

// ---------------------------------------------------------------------------
__fastcall TFormLogin::~TFormLogin(void)
{
	// nada a fazer
}

// ---------------------------------------------------------------------------
void __fastcall TFormLogin::ButCancelaClick(TObject *Sender)
{
	// fecha o Form
	Close();
}

// ---------------------------------------------------------------------------
void __fastcall TFormLogin::ButConfirmaClick(TObject *Sender)
{
	if (!ValidaDadosConexaoOracle())
		return;
	// atualiza Empresa
	empresa.TNSname = EditOracleTNSname->Text.Trim();
	empresa.Username = EditOracleUsername->Text.Trim();
	empresa.Password = EditOraclePassword->Text.Trim();
	// grava parâmetros
	DadosConexaoWrite();
	// fecha o Form
	Close();
}

// ---------------------------------------------------------------------------
void __fastcall TFormLogin::ButTestaClick(TObject *Sender)
{
	try
	{ // valida valores
		if (!ValidaDadosConexaoOracle())
			return;
		if (IniciaConexaoOracle())
			Aviso("A conexão com Oracle é válida.");
	}
	catch (Exception &e)
	{
		Aviso(e.Message);
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFormLogin::DadosConexaoRead(void)
{
	// variáveis locais
	AnsiString txt;
	int cabo_id;
	VTGeral *geral = (VTGeral*)apl->GetObject(__classid(VTGeral));

	if (geral)
	{ // lê parâmetros para conexão
		if (geral->Infoset->LoadFromFile("ConexaoOracle"))
		{
			if (geral->Infoset->GetInfo("TNSname", txt))
				empresa.TNSname = txt;
			if (geral->Infoset->GetInfo("Username", txt))
				empresa.Username = txt;
			if (geral->Infoset->GetInfo("Password", txt))
				empresa.Password = txt;
		}
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFormLogin::DadosConexaoWrite(void)
{
	// variáveis locais
	VTGeral *geral = (VTGeral*)apl->GetObject(__classid(VTGeral));

	if (geral)
	{ // salva parâmetros do Form
		geral->Infoset->Clear();
		geral->Infoset->AddInfo("TNSname", empresa.TNSname);
		geral->Infoset->AddInfo("Username", empresa.Username);
		geral->Infoset->AddInfo("Password", empresa.Password);
		// grava no arquivo
		geral->Infoset->SaveToFile("ConexaoOracle");
	}
}

// ---------------------------------------------------------------------------
bool __fastcall TFormLogin::IniciaConexaoOracle(void)
{
	// variáveis locais
	TOracleSession* ptrbd_session;
	bool conecta = false;

	try
	{
		// If you want to retrieve UTF8 data as WE8MSWIN1252 you can set the
		// global NoUnicodeSupport Boolean variable in the Oracle unit to True.
		// !OOPS! Isso aqui deveria se tornar um parâmetro do Sinap para futuros problemas
		// Precisa ver se funciona corretamente na Sinapsis
		NoUnicodeSupport = true;

		if ((ptrbd_session = new TOracleSession(NULL)) != NULL)
		{
			ptrbd_session->LogonUsername = EditOracleUsername->Text;
			ptrbd_session->LogonPassword = EditOraclePassword->Text;
			ptrbd_session->LogonDatabase = EditOracleTNSname->Text;
			ptrbd_session->LogOn();
			conecta = true;
			ptrbd_session->LogOff();
			delete ptrbd_session;
		}
	}
	catch (EOracleError &e)
	{
		Erro("Falha na conexão com o Oracle: \n" + e.Message);
		if (ptrbd_session)
		{
			ptrbd_session->LogOff();
			delete ptrbd_session;
		}
		return (false);
	}
	catch (Exception &e)
	{
		if (ptrbd_session)
		{
			ptrbd_session->LogOff();
			delete ptrbd_session;
		}
		return (false);
	}
	return (conecta);
}

// ---------------------------------------------------------------------------
bool __fastcall TFormLogin::ValidaDadosConexaoOracle(void)
{
	// verifica dados para conexão
	if (EditOracleTNSname->Text.IsEmpty())
	{
		Aviso("Defina o TNSname da base oracle");
		return (false);
	}
	if (EditOracleUsername->Text.IsEmpty())
	{
		Aviso("Defina o Username para acesso a base oracle");
		return (false);
	}
	if (EditOraclePassword->Text.IsEmpty())
	{
		Aviso("Defina o Password para acesso a base oracle");
		return (false);
	}
	return (true);
}
// ---------------------------------------------------------------------------
// eof
