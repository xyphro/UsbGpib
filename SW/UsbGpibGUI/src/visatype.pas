unit visatype;

{$mode objfpc}{$H+}

interface

type
   ViInt32 = longint;
   ViUInt32 = longword;

   ViUInt64 = qword;

   ViInt64 = int64;

   PViUInt64 = ^ViUInt64;
   ViPUInt64 = PViUInt64;

   ViAUInt64 = PViUInt64;

   PViInt64 = ^ViInt64;
   ViPInt64 = PViInt64;

   ViAInt64 = PViInt64;

   PViUInt32 = ^ViUInt32;
   ViPUInt32 = PViUInt32;

   ViAUInt32 = PViUInt32;

   PViInt32 = ^ViInt32;
   ViPInt32 = PViInt32;

   ViAInt32 = PViInt32;

   ViUInt16 = word;

   PViUInt16 = ^ViUInt16;
   ViPUInt16 = PViUInt16;

   ViAUInt16 = PViUInt16;

   ViInt16 = smallint;

   PViInt16 = ^ViInt16;
   ViPInt16 = PViInt16;

   ViAInt16 = PViInt16;

   ViUInt8 = byte;

   PViUInt8 = ^ViUInt8;
   ViPUInt8 = PViUInt8;

   ViAUInt8 = PViUInt8;

   ViInt8 = shortint;

   PViInt8 = ^ViInt8;
   ViPInt8 = PViInt8;

   ViAInt8 = PViInt8;

   ViChar = char;

   PViChar = ^ViChar;
   ViPChar = PViChar;

   ViAChar = PViChar;

   ViByte = byte;

   PViByte = ^ViByte;
   ViPByte = PViByte;

   ViAByte = PViByte;

   ViAddr = pointer;

   PViAddr = ^ViAddr;
   ViPAddr = PViAddr;

   ViAAddr = PViAddr;

   ViReal32 = single;

   PViReal32 = ^ViReal32;
   ViPReal32 = PViReal32;

   ViAReal32 = PViReal32;

   ViReal64 = double;

   PViReal64 = ^ViReal64;
   ViPReal64 = PViReal64;

   ViAReal64 = PViReal64;

   ViBuf = ViPByte;

   ViPBuf = ViPByte;

   PViPByte = ^ViPByte;
   ViABuf = PViPByte;

   ViString = ViPChar;

   ViPString = ViPChar;

   PViPChar = ^ViPChar;
   ViAString = PViPChar;

   ViRsrc = ViString;

   ViPRsrc = ViString;

   PViString = ^ViString;
   ViARsrc = PViString;

   ViBoolean = ViUInt16;

   PViBoolean = ^ViBoolean;
   ViPBoolean = PViBoolean;

   ViABoolean = PViBoolean;

   ViStatus = ViInt32;

   PViStatus = ^ViStatus;
   ViPStatus = PViStatus;

   ViAStatus = PViStatus;

   ViVersion = ViUInt32;

   PViVersion = ^ViVersion;
   ViPVersion = PViVersion;

   ViAVersion = PViVersion;

   ViObject = ViUInt32;

   PViObject = ^ViObject;
   ViPObject = PViObject;

   ViAObject = PViObject;

   ViSession = ViObject;

    PViSession = ^ViSession;
   ViPSession = PViSession;

   ViASession = PViSession;

   ViAttr = ViUInt32;

const
  _VI_ERROR = ( - 2147483647 - 1 );
  VI_SUCCESS = 0;

implementation

end.

