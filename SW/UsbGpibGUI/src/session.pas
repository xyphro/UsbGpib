unit session;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, syncobjs, visa, visatype;

type

 TOnCloseNotify = procedure(ASender: TObject) of object;

 { TVisaSession }

 TVisaSession = class(TComponent)
 private
  fThrowException,
  fActive: boolean;
  fAddress: string;
  fConnectTimeout: longint;

  fLock: TCriticalSection;

  fOnClose: TOnCloseNotify;

  fHandle: ViSession;
  function Send(const Str: string): longint;
  function Recv(ALength: longint; out res: ViStatus): string;
  procedure SetActive(AValue: boolean);

  procedure CheckError(EC: ViStatus);

  procedure Connect;
  procedure Disconnect;
 protected
  procedure Loaded; override;
 public
  procedure Write(const Str: string);
  function Read: string;

  procedure Lock(ATimeout: longint = 2000);
  procedure Unlock;

  function Query(const AQuery: string): string;
  procedure Command(const ACmd: string);
  procedure Command(const ACmd: string; const Args: array of const);

  procedure DeviceClear;
  function ReadSTB: byte;

  function QueryBlock(const AQuery: string; var Data; ALength: longint): longint;
  function CommandBlock(const ACmd: string; const Data; ALength: longint): longint;

  constructor Create(AOwner: TComponent); override;
  destructor Destroy; override;
 published
  property Address: string read fAddress write fAddress;
  property ConnectTimeout: longint read fConnectTimeout write fConnectTimeout;
  property Active: boolean read fActive write SetActive;

  property OnClose: TOnCloseNotify read fOnClose write fOnClose;
 end;

 EVisaException = class(Exception);

procedure Register;

implementation

{ TVisaSession }

function TVisaSession.Send(const Str: string): longint;
var res: ViUInt32;
begin
   CheckError(viWrite(fHandle, @str[1], length(str), @res));
   result := res;
end;

function TVisaSession.Recv(ALength: longint; out res: ViStatus): string;
var buf: array of char;
    res2: ViUInt32;
begin
   setlength(buf, alength);
   res := viRead(fHandle, @buf[0], Alength, @res2);
   CheckError(res);
   setlength(result, res2);
   move(buf[0], result[1], res2);
end;

procedure TVisaSession.SetActive(AValue: boolean);
begin
   if fActive=AValue then Exit;
   fActive := AValue;

   //if (ComponentState*[csDesigning,csLoading])<>[] then exit;

   if AValue then
   begin
      try
         connect;
      except
         fActive := false;
         raise exception.create('Failed to connect');
      end;
   end
   else
      disconnect;
end;

procedure TVisaSession.CheckError(EC: ViStatus);
var buffer: array[0..4095] of char;
begin
   if (ec < VI_SUCCESS) then
   begin
      if EC = VI_ERROR_CONN_LOST then
      begin
         if assigned(fOnClose) then
            fOnClose(self);
         fActive := false;
      end;

      if fThrowException then
      begin
         if viStatusDesc(fHandle, EC, @buffer[0]) >= VI_SUCCESS then
            raise EVisaException.CreateFmt('VISA Error(%d) "%s"', [ec, pchar(buffer)]) at get_caller_addr(get_frame)
         else
            raise EVisaException.CreateFmt('VISA Unknown error %d', [ec]);
      end;
   end;
end;

procedure TVisaSession.Connect;
var p: ViSession;
begin
   CheckError(viGetDefaultRM(@p));
   CheckError(viOpen(p, pchar(fAddress), VI_NO_LOCK, fConnectTimeout, @fHandle));

   viSetAttribute(fHandle, VI_ATTR_TMO_VALUE, fConnectTimeout);
end;

procedure TVisaSession.Disconnect;
begin
   CheckError(viClose(fHandle));
end;

procedure TVisaSession.Loaded;
begin
   //if fActive then Connect;
end;

procedure TVisaSession.Write(const Str: string);
var i: longint;
begin
   i := 0;

   while i < length(str) do
      i := i+Send(copy(str, i+1, length(str)-i));
end;

function TVisaSession.Read: string;
var res: ViStatus;
begin
   result := '';

   repeat
      result := result + recv(1024, res);
   until (res <> VI_SUCCESS_MAX_CNT);
end;

procedure TVisaSession.Lock(ATimeout: longint);
begin
   CheckError(viLock(fHandle, VI_EXCLUSIVE_LOCK, atimeout, nil, nil));
end;

procedure TVisaSession.Unlock;
begin
   CheckError(viUnlock(fHandle));
end;

function TVisaSession.Query(const AQuery: string): string;
begin
   fLock.Enter;
   try
      Write(AQuery);
      result := Read;
   finally
      fLock.Leave;
   end;
end;

procedure TVisaSession.Command(const ACmd: string);
begin
   fLock.Enter;
   try
      Write(ACmd);
   finally
      fLock.Leave;
   end;
end;

procedure TVisaSession.Command(const ACmd: string; const Args: array of const);
begin
   Command(format(ACmd, args));
end;

procedure TVisaSession.DeviceClear;
begin
   CheckError(viClear(fHandle));
end;

function TVisaSession.ReadSTB: byte;
var res: ViUInt16;
begin
   CheckError(viReadSTB(fHandle, @res));
   result := res;
end;

function TVisaSession.QueryBlock(const AQuery: string; var Data; ALength: longint): longint;
var st: string;
    digits,size,ofs: longint;
begin
   fLock.Enter;
   try
      Write(AQuery);
      st := Read;

      if length(st) < 2 then raise exception.Create('Invalid IEEE488.2 data block');
      if st[1] <> '#' then raise exception.Create('Invalid IEEE488.2 data block');

      digits := strtoint(st[2]);
      if digits = 0 then
      begin
         ofs := 2;
         size := length(st)-2;
      end
      else
      begin
         ofs := 3+digits;
         size := strtoint(copy(st,3,digits));
      end;

      if size > ALength then
         size := ALength;

      move(st[ofs], data, size);
      result := size;
   finally
      fLock.Leave;
   end;
end;

function TVisaSession.CommandBlock(const ACmd: string; const Data; ALength: longint): longint;
var msg, lenStr: string;
begin
   result := ALength;

   fLock.Enter;
   try
      lenStr:=inttostr(ALength);

      setlength(msg, length(ACmd)+1+1+length(lenStr)+alength);
      move(ACmd[1], msg[1], length(ACmd));
      msg[1+length(ACmd)] := '#';
      msg[1+length(ACmd)+1] := char(byte('0')+length(lenStr));
      move(lenstr[1], msg[1+length(ACmd)+2], length(lenStr));
      move(data, msg[1+length(ACmd)+2+length(lenstr)], ALength);

      Write(msg);
   finally
      fLock.Leave;
   end;
end;

constructor TVisaSession.Create(AOwner: TComponent);
begin
   inherited Create(AOwner);
   fThrowException := true;
   fLock := TCriticalSection.Create;
   fActive := false;
   fAddress := '';
   fHandle := 0;
   fConnectTimeout := 2000;
end;

destructor TVisaSession.Destroy;
begin
   fThrowException := false;
   Active := false;
   fLock.Free;
   inherited Destroy;
end;

procedure Register;
begin
   RegisterComponents('FP-Visa', [TVisaSession]);
end;

end.

