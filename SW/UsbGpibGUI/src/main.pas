unit main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, ExtCtrls,
  ComCtrls;

type

  { TForm1 }

  TForm1 = class(TForm)
    Button1: TButton;
    ComboBox1: TComboBox;
    ComboBox2: TComboBox;
    ComboBox3: TComboBox;
    GroupBox1: TGroupBox;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    ListBox1: TListBox;
    Splitter1: TSplitter;
    StatusBar1: TStatusBar;
    Timer1: TTimer;

    procedure Button1Click(Sender: TObject);
    procedure ComboBox1Change(Sender: TObject);
    procedure ComboBox2Change(Sender: TObject);
    procedure ComboBox3Change(Sender: TObject);
    procedure FormClose(Sender: TObject; var CloseAction: TCloseAction);
    procedure FormCreate(Sender: TObject);
    procedure ListBox1SelectionChange(Sender: TObject; User: boolean);
    procedure Timer1Timer(Sender: TObject);
  private
    procedure NewDeviceSelected;
    function  visaRead : string;
    function  visaQuery(str : string) : string;
    procedure visaWrite(str : string);
    function  FwQuery(str : String) : String;
    procedure FwWrite(str : String);
  public

  end;

var
  Form1: TForm1;

implementation

uses visa, visatype;

{$R *.lfm}

{ TForm1 }
var
  defaultRM, activedevice : ViSession;
  activedevicename : String;

procedure TForm1.visaWrite(str : string);
var
  status : viStatus;
  buflen, retlen : viUint32;
  buf : array[0..255] of Byte;
  I : Integer;

begin
  status := VI_SUCCESS;
  if activedevice <> 0 then
    begin
      buflen := 0;
      for I := 1 to length(str) do
        buf[I-1] := Byte(str[I]); // yeah, not elegant, but I am lazy (better use Move)
      buflen := length(str);
      status := viWrite(activedevice, @buf, buflen, @retlen);
    end;
  if status <> VI_SUCCESS then
    begin
      ListBox1.ItemIndex := -1;
      NewDeviceSelected;
    end;
end;

function TForm1.visaRead : string;
var
  status : viStatus;
  buf : array[0..255] of Byte;
begin
  status := VI_SUCCESS;
  if activedevice <> 0 then
    begin
      status := viscanf(activedevice, pchar('%t'), @buf);
      result := pchar(@buf);
    end;
  if status <> VI_SUCCESS then
    begin
      ListBox1.ItemIndex := -1;
      NewDeviceSelected;
      result := '';
    end;
end;

function TForm1.visaQuery(str : string) : string;
begin
  visaWrite(str);
  result := visaRead;
end;

function TForm1.FwQuery(str : String) : String;
var
  retcnt : ViUint16;
  buf : array[0..15] of Byte;
  status : viStatus;
begin
  status := viUsbControlIn(activedevice, $a1, $40, 0, 0, 1, @buf, @retcnt);
  result := visaQuery(str+#13+#10);
end;

procedure TForm1.FwWrite(str : String);
var
  retcnt : ViUint16;
  buf : array[0..15] of Byte;
  status : viStatus;
begin
  status := viUsbControlIn(activedevice, $a1, $40, 0, 0, 1, @buf, @retcnt);
  visaWrite(str+#13+#10);
end;

procedure  TForm1.NewDeviceSelected;
var
  devname : String;
  status : viStatus;
  str : String;
begin
  if ListBox1.ItemIndex > -1 then
    devname := Listbox1.Items[ListBox1.ItemIndex]
  else
    devname := '';
  if devname <> activedevicename then
    begin
      if activedevice <> 0 then
        viClose(activedevice); // close previous opened device, ignore any error
      activedevicename := ''; // no device active anymore
      activedevice := 0;
      if devname <> '' then
        begin // a device was selected, lets try to open it!
          status := viOpen(defaultRM, pchar(devname), 0, 1000, @activedevice);
          if status = VI_SUCCESS then
            begin
              activedevicename := devname;
              // Read settings and update GUI

              // read FW version
              Label2.Caption := FwQuery('!ver?');

              // read autoid setting
              str := FwQuery('!autoid?');
              if str = 'on' then
                ComboBox1.ItemIndex := 1
              else if str = 'off' then
                ComboBox1.ItemIndex := 0
              else if str = 'auto' then
                ComboBox1.ItemIndex := 2
              else
                ComboBox1.ItemIndex := -1;

              // read termination setting
              str := FwQuery('!term?');
              if str = 'cr' then
                ComboBox2.ItemIndex := 0
              else if str = 'lf' then
                ComboBox2.ItemIndex := 1
              else if str = 'eoi' then
                ComboBox2.ItemIndex := 2
              else
                ComboBox2.ItemIndex := -1;

              // read termination setting
              str := FwQuery('!string?');
              if str = 'short' then
                ComboBox3.ItemIndex := 0
              else if str = 'normal' then
                ComboBox3.ItemIndex := 1
              else
                ComboBox3.ItemIndex := -1;

            end
        end;
    end;
  GroupBox1.Enabled := (activedevicename <> '');
  if activedevicename = '' then
    begin
      ListBox1.ItemIndex := -1;
      Label2.Caption := '-';
      ComboBox1.ItemIndex := -1;
      ComboBox2.ItemIndex := -1;
      ComboBox3.ItemIndex := -1;
    end;
end;

procedure TForm1.FormClose(Sender: TObject; var CloseAction: TCloseAction);
begin
  viClose(defaultRM);
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  FwWrite('!reset');
  ListBox1.ItemIndex := -1; // deselect adapter
end;

procedure TForm1.ComboBox1Change(Sender: TObject);
var
  str : string;
begin
  str := '';
  if ComboBox1.ItemIndex = 0 then
    str := 'off'
  else if ComboBox1.ItemIndex = 1 then
    str := 'on'
  else if ComboBox1.ItemIndex = 2 then
    str := 'slow';
  if str <> '' then
    FwWrite('!autoid '+str);
end;

procedure TForm1.ComboBox2Change(Sender: TObject);
var
  str : string;
begin
  str := '';
  if ComboBox2.ItemIndex = 0 then
    str := 'cr'
  else if ComboBox2.ItemIndex = 1 then
    str := 'lf'
  else if ComboBox2.ItemIndex = 2 then
    str := 'eoi';
  if str <> '' then
    FwWrite('!term '+str);
end;

procedure TForm1.ComboBox3Change(Sender: TObject);
var
  str : string;
begin
  str := '';
  if ComboBox3.ItemIndex = 0 then
    str := 'short'
  else if ComboBox3.ItemIndex = 1 then
    str := 'normal';
  if str <> '' then
    FwWrite('!string '+str);
end;

procedure TForm1.FormCreate(Sender: TObject);
var
  status : viStatus;
begin
  activedevicename := '';
  activedevice := 0;
  status := viGetDefaultRM(@defaultRM);
  if status < VI_SUCCESS then
    begin
      ShowMessage('Unable to get default ressource manager');
      Exit;
    end;

end;

procedure TForm1.ListBox1SelectionChange(Sender: TObject; User: boolean);
begin
  NewDeviceSelected;
end;

procedure TForm1.Timer1Timer(Sender: TObject);
var
  status : viStatus;
  fl : ViFindList;
  nmatches : ViUInt32;
  matches : ViRsrc;
  buffer : array[0..255] of viChar;
  devlist : TStringList;
  prevname : string;
  idx : Integer;
begin
  matches := @buffer;
  status := viFindRsrc(defaultRM, pchar('USB?*::0x03EB::0x2065::?*'), @fl, @nmatches, matches); // "USB?*
  if nmatches = 0 then
    StatusBar1.SimpleText := 'No Xyphro UsbGpib device found.'
  else if nmatches = 1 then
    StatusBar1.SimpleText := IntToStr(nmatches) + ' Xyphro UsbGpib device found.'
  else
    StatusBar1.SimpleText := IntToStr(nmatches) + ' Xyphro UsbGpib devices found.';
  // store previous selected device
  prevname := '';
  if Listbox1.ItemIndex >= 0 then
    prevname := Listbox1.Items[listbox1.Itemindex];
  devlist := TStringList.Create;
  while status = VI_SUCCESS do
    begin
      devlist.add(pchar(matches));
      status := viFindNext(fl, matches);
    end;
  ListBox1.Items.Assign(devlist);
  if prevname <> '' then
    begin
      idx := Listbox1.Items.IndexOf(prevname);
      if idx >= 0 then
        Listbox1.Itemindex := idx
      else
        NewDeviceSelected;
    end;
  devlist.free;
end;

end.

