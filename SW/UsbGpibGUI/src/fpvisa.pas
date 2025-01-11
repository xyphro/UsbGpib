{ This file was automatically created by Lazarus. Do not edit!
  This source is only used to compile and install the package.
 }

unit fpvisa;

interface

uses
   session, visatype, visa, LazarusPackageIntf;

implementation

procedure Register;
begin
  RegisterUnit('session', @session.Register);
end;

initialization
  RegisterPackage('fpvisa', @Register);
end.
