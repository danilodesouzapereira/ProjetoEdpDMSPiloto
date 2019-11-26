object FormExportaLigacoes: TFormExportaLigacoes
  Left = 0
  Top = 0
  Caption = 'Exporta Liga'#231#245'es'
  ClientHeight = 403
  ClientWidth = 598
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Button1: TButton
    Left = 64
    Top = 80
    Width = 121
    Height = 73
    Caption = 'Exporta'
    TabOrder = 0
    OnClick = Button1Click
  end
  object edtCont: TEdit
    Left = 56
    Top = 208
    Width = 121
    Height = 21
    TabOrder = 1
    Text = '0'
  end
end
