object FormProgressBar: TFormProgressBar
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Execu'#231#227'o do Algoritmo Evolutivo'
  ClientHeight = 43
  ClientWidth = 262
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
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 262
    Height = 43
    Align = alClient
    Caption = 'Progresso das Gera'#231#245'es:'
    TabOrder = 0
    ExplicitTop = 40
    ExplicitHeight = 50
    DesignSize = (
      262
      43)
    object ProgressBarGeracoes: TProgressBar
      Left = 11
      Top = 17
      Width = 245
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Step = 1
      TabOrder = 0
    end
  end
end
