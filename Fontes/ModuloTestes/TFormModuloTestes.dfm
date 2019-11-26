object FormModuloTestes: TFormModuloTestes
  Left = 0
  Top = 0
  Caption = 'M'#243'dulo de testes'
  ClientHeight = 141
  ClientWidth = 517
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 40
    Top = 48
    Width = 111
    Height = 13
    Caption = 'Visualiza'#231#227'o de trechos'
  end
  object btnSelecionaArquivoTrechos: TButton
    Left = 176
    Top = 43
    Width = 105
    Height = 25
    Caption = 'Selecionar arquivo'
    TabOrder = 0
    OnClick = btnSelecionaArquivoTrechosClick
  end
  object Button1: TButton
    Left = 304
    Top = 43
    Width = 105
    Height = 25
    Caption = 'Limpa'
    TabOrder = 1
    OnClick = Button1Click
  end
  object OpenDialog1: TOpenDialog
    Left = 472
  end
end
