unit visa;

{$mode objfpc}{$H+}

interface

uses visatype;

const
   DllName = 'visa32.dll';

(*- VISA Types -------------------------------------------------------------- *)

type
   ViEvent = ViObject;

   ViPEvent = ^ViEvent;

   ViFindList = ViObject;

   ViPFindList = ^ViFindList;

   ViBusAddress = PtrUInt;

   ViBusSize = PtrUInt;

   ViAttrState = PtrUInt;


   ViBusAddress64 = ViUInt64;

   ViPBusAddress64 = ^ViBusAddress64;


   ViEventType = ViUInt32;

   ViPEventType = ^ViEventType;

   ViAEventType = ^ViEventType;

   ViPAttrState = Pointer;

   ViPAttr = ^ViAttr;

   ViAAttr = ^ViAttr;

   ViKeyId = ViString;

   ViPKeyId = ViPString;

   ViJobId = ViUInt32;

   ViPJobId = ^ViJobId;

   ViAccessMode = ViUInt32;

   ViPAccessMode = ^ViAccessMode;

   ViPBusAddress = ^ViBusAddress;

   ViEventFilter = ViUInt32;

   ViHndlr = function(vi: ViSession; eventType: ViEventType; event: ViEvent; userHandle: ViAddr): ViStatus; stdcall;

(*- Resource Manager Functions and Operations ------------------------------- *)

function viOpenDefaultRM(vi: ViPSession): ViStatus; stdcall; external DllName;

function viFindRsrc(sesn: ViSession; expr: ViString; vi: ViPFindList; retCnt: ViPUInt32; desc: PViChar): ViStatus; stdcall; external DllName;

function viFindNext(vi: ViFindList; desc: PViChar): ViStatus; stdcall; external DllName;

function viParseRsrc(rmSesn: ViSession; rsrcName: ViRsrc; intfType, intfNum: ViPUInt16): ViStatus; stdcall; external DllName;

function viParseRsrcEx(rmSesn: ViSession; rsrcName: ViRsrc; intfType, intfNum: ViPUInt16; rsrcClass, expandedUnaliasedName, aliasIfExists: PViChar): ViStatus; stdcall; external DllName;

function viOpen(sesn: ViSession; name_: ViRsrc; mode: ViAccessMode; timeout: ViUInt32; vi: ViPSession): ViStatus; stdcall; external DllName;

(*- Resource Template Operations -------------------------------------------- *)

function viClose(vi: ViObject): ViStatus; stdcall; external DllName;

function viSetAttribute(vi: ViObject; attrName: ViAttr; attrValue: ViAttrState): ViStatus; stdcall; external DllName;

function viGetAttribute(vi: ViObject; attrName: ViAttr; attrValue: Pointer): ViStatus; stdcall; external DllName;

function viStatusDesc(vi: ViObject; status: ViStatus; desc: PViChar): ViStatus; stdcall; external DllName;

function viTerminate(vi: ViObject; degree: ViUInt16; jobId: ViJobId): ViStatus; stdcall; external DllName;

function viLock(vi: ViSession; lockType: ViAccessMode; timeout: ViUInt32; requestedKey: ViKeyId; accessKey: PViChar): ViStatus; stdcall; external DllName;

function viUnlock(vi: ViSession): ViStatus; stdcall; external DllName;

function viEnableEvent(vi: ViSession; eventType: ViEventType; mechanism: ViUInt16; context: ViEventFilter): ViStatus; stdcall; external DllName;

function viDisableEvent(vi: ViSession; eventType: ViEventType; mechanism: ViUInt16): ViStatus; stdcall; external DllName;

function viDiscardEvents(vi: ViSession; eventType: ViEventType; mechanism: ViUInt16): ViStatus; stdcall; external DllName;

function viWaitOnEvent(vi: ViSession; inEventType: ViEventType; timeout: ViUInt32; outEventType: ViPEventType; outContext: ViPEvent): ViStatus; stdcall; external DllName;

function viInstallHandler(vi: ViSession; eventType: ViEventType; handler: ViHndlr; userHandle: ViAddr): ViStatus; stdcall; external DllName;

function viUninstallHandler(vi: ViSession; eventType: ViEventType; handler: ViHndlr; userHandle: ViAddr): ViStatus; stdcall; external DllName;

(*- Basic I/O Operations ---------------------------------------------------- *)

function viRead(vi: ViSession; buf: ViPBuf; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;

function viReadAsync(vi: ViSession; buf: ViPBuf; cnt: ViUInt32; jobId: ViPJobId): ViStatus; stdcall; external DllName;

function viReadToFile(vi: ViSession; const filename: ViString; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;

function viWrite(vi: ViSession; buf: ViBuf; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;

function viWriteAsync(vi: ViSession; buf: ViBuf; cnt: ViUInt32; jobId: ViPJobId): ViStatus; stdcall; external DllName;

function viWriteFromFile(vi: ViSession; const filename: ViString; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;

function viAssertTrigger(vi: ViSession; protocol: ViUInt16): ViStatus; stdcall; external DllName;

function viReadSTB(vi: ViSession; status: ViPUInt16): ViStatus; stdcall; external DllName;

function viClear(vi: ViSession): ViStatus; stdcall; external DllName;

(*- Formatted and Buffered I/O Operations ----------------------------------- *)

function viSetBuf(vi: ViSession; mask: ViUInt16; size: ViUInt32): ViStatus; stdcall; external DllName;

function viFlush(vi: ViSession; mask: ViUInt16): ViStatus; stdcall; external DllName;

function viBufWrite(vi: ViSession; buf: ViBuf; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;

function viBufRead(vi: ViSession; buf: ViPBuf; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;


function viPrintf(vi: ViSession; writeFmt: ViString): ViStatus; varargs; stdcall; external DllName;

function viSPrintf(vi: ViSession; buf: ViPBuf; writeFmt: ViString): ViStatus; varargs; stdcall; external DllName;

function viScanf(vi: ViSession; readFmt: ViString): ViStatus; varargs; stdcall; external DllName;

function viSScanf(vi: ViSession; buf: ViBuf; readFmt: ViString): ViStatus; varargs; stdcall; external DllName;

function viQueryf(vi: ViSession; writeFmt, readFmt: ViString): ViStatus; varargs; stdcall; external DllName;

(*- Memory I/O Operations --------------------------------------------------- *)

function viIn8(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val8: ViPUInt8): ViStatus; stdcall; external DllName;

function viOut8(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val8: ViUInt8): ViStatus; stdcall; external DllName;

function viIn16(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val16: ViPUInt16): ViStatus; stdcall; external DllName;

function viOut16(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val16: ViUInt16): ViStatus; stdcall; external DllName;

function viIn32(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val32: ViPUInt32): ViStatus; stdcall; external DllName;

function viOut32(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val32: ViUInt32): ViStatus; stdcall; external DllName;

function viIn64(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val64: ViPUInt64): ViStatus; stdcall; external DllName;

function viOut64(vi: ViSession; space: ViUInt16; offset: ViBusAddress; val64: ViUInt64): ViStatus; stdcall; external DllName;

function viIn8Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val8: ViPUInt8): ViStatus; stdcall; external DllName;

function viOut8Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val8: ViUInt8): ViStatus; stdcall; external DllName;

function viIn16Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val16: ViPUInt16): ViStatus; stdcall; external DllName;

function viOut16Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val16: ViUInt16): ViStatus; stdcall; external DllName;

function viIn32Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val32: ViPUInt32): ViStatus; stdcall; external DllName;

function viOut32Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val32: ViUInt32): ViStatus; stdcall; external DllName;

function viIn64Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val64: ViPUInt64): ViStatus; stdcall; external DllName;

function viOut64Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; val64: ViUInt64): ViStatus; stdcall; external DllName;


function viMoveIn8(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf8: ViAUInt8): ViStatus; stdcall; external DllName;

function viMoveOut8(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf8: ViAUInt8): ViStatus; stdcall; external DllName;

function viMoveIn16(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf16: ViAUInt16): ViStatus; stdcall; external DllName;

function viMoveOut16(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf16: ViAUInt16): ViStatus; stdcall; external DllName;

function viMoveIn32(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf32: ViAUInt32): ViStatus; stdcall; external DllName;

function viMoveOut32(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf32: ViAUInt32): ViStatus; stdcall; external DllName;

function viMoveIn64(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf64: ViAUInt64): ViStatus; stdcall; external DllName;

function viMoveOut64(vi: ViSession; space: ViUInt16; offset: ViBusAddress; length: ViBusSize; buf64: ViAUInt64): ViStatus; stdcall; external DllName;

function viMoveIn8Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf8: ViAUInt8): ViStatus; stdcall; external DllName;

function viMoveOut8Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf8: ViAUInt8): ViStatus; stdcall; external DllName;

function viMoveIn16Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf16: ViAUInt16): ViStatus; stdcall; external DllName;

function viMoveOut16Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf16: ViAUInt16): ViStatus; stdcall; external DllName;

function viMoveIn32Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf32: ViAUInt32): ViStatus; stdcall; external DllName;

function viMoveOut32Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf32: ViAUInt32): ViStatus; stdcall; external DllName;

function viMoveIn64Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf64: ViAUInt64): ViStatus; stdcall; external DllName;

function viMoveOut64Ex(vi: ViSession; space: ViUInt16; offset: ViBusAddress64; length: ViBusSize; buf64: ViAUInt64): ViStatus; stdcall; external DllName;


function viMove(vi: ViSession; srcSpace: ViUInt16; srcOffset: ViBusAddress; srcWidth, destSpace: ViUInt16; destOffset: ViBusAddress; destWidth: ViUInt16; srcLength: ViBusSize): ViStatus; stdcall; external DllName;

function viMoveAsync(vi: ViSession; srcSpace: ViUInt16; srcOffset: ViBusAddress; srcWidth, destSpace: ViUInt16; destOffset: ViBusAddress; destWidth: ViUInt16; srcLength: ViBusSize; jobId: ViPJobId): ViStatus; stdcall; external DllName;

function viMoveEx(vi: ViSession; srcSpace: ViUInt16; srcOffset: ViBusAddress64; srcWidth, destSpace: ViUInt16; destOffset: ViBusAddress64; destWidth: ViUInt16; srcLength: ViBusSize): ViStatus; stdcall; external DllName;

function viMoveAsyncEx(vi: ViSession; srcSpace: ViUInt16; srcOffset: ViBusAddress64; srcWidth, destSpace: ViUInt16; destOffset: ViBusAddress64; destWidth: ViUInt16; srcLength: ViBusSize; jobId: ViPJobId): ViStatus; stdcall; external DllName;


function viMapAddress(vi: ViSession; mapSpace: ViUInt16; mapOffset: ViBusAddress; mapSize: ViBusSize; access: ViBoolean; suggested: ViAddr; address: ViPAddr): ViStatus; stdcall; external DllName;

function viUnmapAddress(vi: ViSession): ViStatus; stdcall; external DllName;

function viMapAddressEx(vi: ViSession; mapSpace: ViUInt16; mapOffset: ViBusAddress64; mapSize: ViBusSize; access: ViBoolean; suggested: ViAddr; address: ViPAddr): ViStatus; stdcall; external DllName;


procedure viPeek8(vi: ViSession; address: ViAddr; val8: ViPUInt8); stdcall; external DllName;

procedure viPoke8(vi: ViSession; address: ViAddr; val8: ViUInt8); stdcall; external DllName;

procedure viPeek16(vi: ViSession; address: ViAddr; val16: ViPUInt16); stdcall; external DllName;

procedure viPoke16(vi: ViSession; address: ViAddr; val16: ViUInt16); stdcall; external DllName;

procedure viPeek32(vi: ViSession; address: ViAddr; val32: ViPUInt32); stdcall; external DllName;

procedure viPoke32(vi: ViSession; address: ViAddr; val32: ViUInt32); stdcall; external DllName;

procedure viPeek64(vi: ViSession; address: ViAddr; val64: ViPUInt64); stdcall; external DllName;

procedure viPoke64(vi: ViSession; address: ViAddr; val64: ViUInt64); stdcall; external DllName;


(*- Shared Memory Operations ------------------------------------------------ *)

function viMemAlloc(vi: ViSession; size: ViBusSize; offset: ViPBusAddress): ViStatus; stdcall; external DllName;

function viMemFree(vi: ViSession; offset: ViBusAddress): ViStatus; stdcall; external DllName;

function viMemAllocEx(vi: ViSession; size: ViBusSize; offset: ViPBusAddress64): ViStatus; stdcall; external DllName;

function viMemFreeEx(vi: ViSession; offset: ViBusAddress64): ViStatus; stdcall; external DllName;


(*- Interface Specific Operations ------------------------------------------- *)

function viGpibControlREN(vi: ViSession; mode: ViUInt16): ViStatus; stdcall; external DllName;

function viGpibControlATN(vi: ViSession; mode: ViUInt16): ViStatus; stdcall; external DllName;

function viGpibSendIFC(vi: ViSession): ViStatus; stdcall; external DllName;

function viGpibCommand(vi: ViSession; cmd: ViBuf; cnt: ViUInt32; retCnt: ViPUInt32): ViStatus; stdcall; external DllName;

function viGpibPassControl(vi: ViSession; primAddr, secAddr: ViUInt16): ViStatus; stdcall; external DllName;

function viVxiCommandQuery(vi: ViSession; mode: ViUInt16; cmd: ViUInt32; response: ViPUInt32): ViStatus; stdcall; external DllName;

function viAssertUtilSignal(vi: ViSession; line: ViUInt16): ViStatus; stdcall; external DllName;

function viAssertIntrSignal(vi: ViSession; mode: ViInt16; statusID: ViUInt32): ViStatus; stdcall; external DllName;

function viMapTrigger(vi: ViSession; trigSrc, trigDest: ViInt16; mode: ViUInt16): ViStatus; stdcall; external DllName;

function viUnmapTrigger(vi: ViSession; trigSrc, trigDest: ViInt16): ViStatus; stdcall; external DllName;

function viUsbControlOut(vi: ViSession; bmRequestType, bRequest: ViInt16; wValue, wIndex, wLength: ViUInt16; buf: ViBuf): ViStatus; stdcall; external DllName;

function viUsbControlIn(vi: ViSession; bmRequestType, bRequest: ViInt16; wValue, wIndex, wLength: ViUInt16; buf: ViPBuf; retCnt: ViPUInt16): ViStatus; stdcall; external DllName;


(*- Attributes (platform independent size) ---------------------------------- *)

const
   VI_ATTR_RSRC_CLASS = ($BFFF0001);
   VI_ATTR_RSRC_NAME = ($BFFF0002);
   VI_ATTR_RSRC_IMPL_VERSION = ($3FFF0003);
   VI_ATTR_RSRC_LOCK_STATE = ($3FFF0004);
   VI_ATTR_MAX_QUEUE_LENGTH = ($3FFF0005);
   VI_ATTR_USER_DATA_32 = ($3FFF0007);
   VI_ATTR_FDC_CHNL = ($3FFF000D);
   VI_ATTR_FDC_MODE = ($3FFF000F);
   VI_ATTR_FDC_GEN_SIGNAL_EN = ($3FFF0011);
   VI_ATTR_FDC_USE_PAIR = ($3FFF0013);
   VI_ATTR_SEND_END_EN = ($3FFF0016);
   VI_ATTR_TERMCHAR = ($3FFF0018);
   VI_ATTR_TMO_VALUE = ($3FFF001A);
   VI_ATTR_GPIB_READDR_EN = ($3FFF001B);
   VI_ATTR_IO_PROT = ($3FFF001C);
   VI_ATTR_DMA_ALLOW_EN = ($3FFF001E);
   VI_ATTR_ASRL_BAUD = ($3FFF0021);
   VI_ATTR_ASRL_DATA_BITS = ($3FFF0022);
   VI_ATTR_ASRL_PARITY = ($3FFF0023);
   VI_ATTR_ASRL_STOP_BITS = ($3FFF0024);
   VI_ATTR_ASRL_FLOW_CNTRL = ($3FFF0025);
   VI_ATTR_RD_BUF_OPER_MODE = ($3FFF002A);
   VI_ATTR_RD_BUF_SIZE = ($3FFF002B);
   VI_ATTR_WR_BUF_OPER_MODE = ($3FFF002D);
   VI_ATTR_WR_BUF_SIZE = ($3FFF002E);
   VI_ATTR_SUPPRESS_END_EN = ($3FFF0036);
   VI_ATTR_TERMCHAR_EN = ($3FFF0038);
   VI_ATTR_DEST_ACCESS_PRIV = ($3FFF0039);
   VI_ATTR_DEST_BYTE_ORDER = ($3FFF003A);
   VI_ATTR_SRC_ACCESS_PRIV = ($3FFF003C);
   VI_ATTR_SRC_BYTE_ORDER = ($3FFF003D);
   VI_ATTR_SRC_INCREMENT = ($3FFF0040);
   VI_ATTR_DEST_INCREMENT = ($3FFF0041);
   VI_ATTR_WIN_ACCESS_PRIV = ($3FFF0045);
   VI_ATTR_WIN_BYTE_ORDER = ($3FFF0047);
   VI_ATTR_GPIB_ATN_STATE = ($3FFF0057);
   VI_ATTR_GPIB_ADDR_STATE = ($3FFF005C);
   VI_ATTR_GPIB_CIC_STATE = ($3FFF005E);
   VI_ATTR_GPIB_NDAC_STATE = ($3FFF0062);
   VI_ATTR_GPIB_SRQ_STATE = ($3FFF0067);
   VI_ATTR_GPIB_SYS_CNTRL_STATE = ($3FFF0068);
   VI_ATTR_GPIB_HS488_CBL_LEN = ($3FFF0069);
   VI_ATTR_CMDR_LA = ($3FFF006B);
   VI_ATTR_VXI_DEV_CLASS = ($3FFF006C);
   VI_ATTR_MAINFRAME_LA = ($3FFF0070);
   VI_ATTR_MANF_NAME = ($BFFF0072);
   VI_ATTR_MODEL_NAME = ($BFFF0077);
   VI_ATTR_VXI_VME_INTR_STATUS = ($3FFF008B);
   VI_ATTR_VXI_TRIG_STATUS = ($3FFF008D);
   VI_ATTR_VXI_VME_SYSFAIL_STATE = ($3FFF0094);
   VI_ATTR_WIN_BASE_ADDR_32 = ($3FFF0098);
   VI_ATTR_WIN_SIZE_32 = ($3FFF009A);
   VI_ATTR_ASRL_AVAIL_NUM = ($3FFF00AC);
   VI_ATTR_MEM_BASE_32 = ($3FFF00AD);
   VI_ATTR_ASRL_CTS_STATE = ($3FFF00AE);
   VI_ATTR_ASRL_DCD_STATE = ($3FFF00AF);
   VI_ATTR_ASRL_DSR_STATE = ($3FFF00B1);
   VI_ATTR_ASRL_DTR_STATE = ($3FFF00B2);
   VI_ATTR_ASRL_END_IN = ($3FFF00B3);
   VI_ATTR_ASRL_END_OUT = ($3FFF00B4);
   VI_ATTR_ASRL_REPLACE_CHAR = ($3FFF00BE);
   VI_ATTR_ASRL_RI_STATE = ($3FFF00BF);
   VI_ATTR_ASRL_RTS_STATE = ($3FFF00C0);
   VI_ATTR_ASRL_XON_CHAR = ($3FFF00C1);
   VI_ATTR_ASRL_XOFF_CHAR = ($3FFF00C2);
   VI_ATTR_WIN_ACCESS = ($3FFF00C3);
   VI_ATTR_RM_SESSION = ($3FFF00C4);
   VI_ATTR_VXI_LA = ($3FFF00D5);
   VI_ATTR_MANF_ID = ($3FFF00D9);
   VI_ATTR_MEM_SIZE_32 = ($3FFF00DD);
   VI_ATTR_MEM_SPACE = ($3FFF00DE);
   VI_ATTR_MODEL_CODE = ($3FFF00DF);
   VI_ATTR_SLOT = ($3FFF00E8);
   VI_ATTR_INTF_INST_NAME = ($BFFF00E9);
   VI_ATTR_IMMEDIATE_SERV = ($3FFF0100);
   VI_ATTR_INTF_PARENT_NUM = ($3FFF0101);
   VI_ATTR_RSRC_SPEC_VERSION = ($3FFF0170);
   VI_ATTR_INTF_TYPE = ($3FFF0171);
   VI_ATTR_GPIB_PRIMARY_ADDR = ($3FFF0172);
   VI_ATTR_GPIB_SECONDARY_ADDR = ($3FFF0173);
   VI_ATTR_RSRC_MANF_NAME = ($BFFF0174);
   VI_ATTR_RSRC_MANF_ID = ($3FFF0175);
   VI_ATTR_INTF_NUM = ($3FFF0176);
   VI_ATTR_TRIG_ID = ($3FFF0177);
   VI_ATTR_GPIB_REN_STATE = ($3FFF0181);
   VI_ATTR_GPIB_UNADDR_EN = ($3FFF0184);
   VI_ATTR_DEV_STATUS_BYTE = ($3FFF0189);
   VI_ATTR_FILE_APPEND_EN = ($3FFF0192);
   VI_ATTR_VXI_TRIG_SUPPORT = ($3FFF0194);
   VI_ATTR_TCPIP_ADDR = ($BFFF0195);
   VI_ATTR_TCPIP_HOSTNAME = ($BFFF0196);
   VI_ATTR_TCPIP_PORT = ($3FFF0197);
   VI_ATTR_TCPIP_DEVICE_NAME = ($BFFF0199);
   VI_ATTR_TCPIP_NODELAY = ($3FFF019A);
   VI_ATTR_TCPIP_KEEPALIVE = ($3FFF019B);
   VI_ATTR_4882_COMPLIANT = ($3FFF019F);
   VI_ATTR_USB_SERIAL_NUM = ($BFFF01A0);
   VI_ATTR_USB_INTFC_NUM = ($3FFF01A1);
   VI_ATTR_USB_PROTOCOL = ($3FFF01A7);
   VI_ATTR_USB_MAX_INTR_SIZE = ($3FFF01AF);
   VI_ATTR_PXI_DEV_NUM = ($3FFF0201);
   VI_ATTR_PXI_FUNC_NUM = ($3FFF0202);
   VI_ATTR_PXI_BUS_NUM = ($3FFF0205);
   VI_ATTR_PXI_CHASSIS = ($3FFF0206);
   VI_ATTR_PXI_SLOTPATH = ($BFFF0207);
   VI_ATTR_PXI_SLOT_LBUS_LEFT = ($3FFF0208);
   VI_ATTR_PXI_SLOT_LBUS_RIGHT = ($3FFF0209);
   VI_ATTR_PXI_TRIG_BUS = ($3FFF020A);
   VI_ATTR_PXI_STAR_TRIG_BUS = ($3FFF020B);
   VI_ATTR_PXI_STAR_TRIG_LINE = ($3FFF020C);
   VI_ATTR_PXI_MEM_TYPE_BAR0 = ($3FFF0211);
   VI_ATTR_PXI_MEM_TYPE_BAR1 = ($3FFF0212);
   VI_ATTR_PXI_MEM_TYPE_BAR2 = ($3FFF0213);
   VI_ATTR_PXI_MEM_TYPE_BAR3 = ($3FFF0214);
   VI_ATTR_PXI_MEM_TYPE_BAR4 = ($3FFF0215);
   VI_ATTR_PXI_MEM_TYPE_BAR5 = ($3FFF0216);
   VI_ATTR_PXI_MEM_BASE_BAR0 = ($3FFF0221);
   VI_ATTR_PXI_MEM_BASE_BAR1 = ($3FFF0222);
   VI_ATTR_PXI_MEM_BASE_BAR2 = ($3FFF0223);
   VI_ATTR_PXI_MEM_BASE_BAR3 = ($3FFF0224);
   VI_ATTR_PXI_MEM_BASE_BAR4 = ($3FFF0225);
   VI_ATTR_PXI_MEM_BASE_BAR5 = ($3FFF0226);
   VI_ATTR_PXI_MEM_SIZE_BAR0 = ($3FFF0231);
   VI_ATTR_PXI_MEM_SIZE_BAR1 = ($3FFF0232);
   VI_ATTR_PXI_MEM_SIZE_BAR2 = ($3FFF0233);
   VI_ATTR_PXI_MEM_SIZE_BAR3 = ($3FFF0234);
   VI_ATTR_PXI_MEM_SIZE_BAR4 = ($3FFF0235);
   VI_ATTR_PXI_MEM_SIZE_BAR5 = ($3FFF0236);
   VI_ATTR_PXI_IS_EXPRESS = ($3FFF0240);
   VI_ATTR_PXI_SLOT_LWIDTH = ($3FFF0241);
   VI_ATTR_PXI_MAX_LWIDTH = ($3FFF0242);
   VI_ATTR_PXI_ACTUAL_LWIDTH = ($3FFF0243);
   VI_ATTR_PXI_DSTAR_BUS = ($3FFF0244);
   VI_ATTR_PXI_DSTAR_SET = ($3FFF0245);
   VI_ATTR_TCPIP_HISLIP_OVERLAP_EN = ($3FFF0300);
   VI_ATTR_TCPIP_HISLIP_VERSION = ($3FFF0301);
   VI_ATTR_TCPIP_HISLIP_MAX_MESSAGE_KB = ($3FFF0302);
   VI_ATTR_JOB_ID = ($3FFF4006);
   VI_ATTR_EVENT_TYPE = ($3FFF4010);
   VI_ATTR_SIGP_STATUS_ID = ($3FFF4011);
   VI_ATTR_RECV_TRIG_ID = ($3FFF4012);
   VI_ATTR_INTR_STATUS_ID = ($3FFF4023);
   VI_ATTR_STATUS = ($3FFF4025);
   VI_ATTR_RET_COUNT_32 = ($3FFF4026);
   VI_ATTR_BUFFER = ($3FFF4027);
   VI_ATTR_RECV_INTR_LEVEL = ($3FFF4041);
   VI_ATTR_OPER_NAME = ($BFFF4042);
   VI_ATTR_GPIB_RECV_CIC_STATE = ($3FFF4193);
   VI_ATTR_RECV_TCPIP_ADDR = ($BFFF4198);
   VI_ATTR_USB_RECV_INTR_SIZE = ($3FFF41B0);
   VI_ATTR_USB_RECV_INTR_DATA = ($BFFF41B1);
   (*- Attributes (platform dependent size) ------------------------------------ *)

  {$ifdef CPU64}
   VI_ATTR_USER_DATA_64 = ($3FFF000A);
   VI_ATTR_RET_COUNT_64 = ($3FFF4028);
   VI_ATTR_USER_DATA = (VI_ATTR_USER_DATA_64);
   VI_ATTR_RET_COUNT = (VI_ATTR_RET_COUNT_64);
  {$else}
   VI_ATTR_USER_DATA = (VI_ATTR_USER_DATA_32);
   VI_ATTR_RET_COUNT = (VI_ATTR_RET_COUNT_32);
  {$endif}
  {$ifdef CPU64}
   VI_ATTR_WIN_BASE_ADDR_64 = ($3FFF009B);
   VI_ATTR_WIN_SIZE_64 = ($3FFF009C);
   VI_ATTR_MEM_BASE_64 = ($3FFF00D0);
   VI_ATTR_MEM_SIZE_64 = ($3FFF00D1);
   VI_ATTR_WIN_BASE_ADDR = (VI_ATTR_WIN_BASE_ADDR_64);
   VI_ATTR_WIN_SIZE = (VI_ATTR_WIN_SIZE_64);
   VI_ATTR_MEM_BASE = (VI_ATTR_MEM_BASE_64);
   VI_ATTR_MEM_SIZE = (VI_ATTR_MEM_SIZE_64);
  {$else}
   VI_ATTR_WIN_SIZE = (VI_ATTR_WIN_SIZE_32);
   VI_ATTR_MEM_BASE = (VI_ATTR_MEM_BASE_32);
   VI_ATTR_MEM_SIZE = (VI_ATTR_MEM_SIZE_32);
  {$endif}
   (*- Event Types ------------------------------------------------------------- *)

   VI_EVENT_IO_COMPLETION = ($3FFF2009);
   VI_EVENT_TRIG = ($BFFF200A);
   VI_EVENT_SERVICE_REQ = ($3FFF200B);
   VI_EVENT_CLEAR = ($3FFF200D);
   VI_EVENT_EXCEPTION = ($BFFF200E);
   VI_EVENT_GPIB_CIC = ($3FFF2012);
   VI_EVENT_GPIB_TALK = ($3FFF2013);
   VI_EVENT_GPIB_LISTEN = ($3FFF2014);
   VI_EVENT_VXI_VME_SYSFAIL = ($3FFF201D);
   VI_EVENT_VXI_VME_SYSRESET = ($3FFF201E);
   VI_EVENT_VXI_SIGP = ($3FFF2020);
   VI_EVENT_VXI_VME_INTR = ($BFFF2021);
   VI_EVENT_PXI_INTR = ($3FFF2022);
   VI_EVENT_TCPIP_CONNECT = ($3FFF2036);
   VI_EVENT_USB_INTR = ($3FFF2037);
   VI_ALL_ENABLED_EVENTS = ($3FFF7FFF);
   (*- Completion and Error Codes ---------------------------------------------- *)

   VI_SUCCESS_EVENT_EN = ($3FFF0002);  (* 3FFF0002,  1073676290  *)
   VI_SUCCESS_EVENT_DIS = ($3FFF0003);  (* 3FFF0003,  1073676291  *)
   VI_SUCCESS_QUEUE_EMPTY = ($3FFF0004);  (* 3FFF0004,  1073676292  *)
   VI_SUCCESS_TERM_CHAR = ($3FFF0005);  (* 3FFF0005,  1073676293  *)
   VI_SUCCESS_MAX_CNT = ($3FFF0006);  (* 3FFF0006,  1073676294  *)
   VI_SUCCESS_DEV_NPRESENT = ($3FFF007D);  (* 3FFF007D,  1073676413  *)
   VI_SUCCESS_TRIG_MAPPED = ($3FFF007E);  (* 3FFF007E,  1073676414  *)
   VI_SUCCESS_QUEUE_NEMPTY = ($3FFF0080);  (* 3FFF0080,  1073676416  *)
   VI_SUCCESS_NCHAIN = ($3FFF0098);  (* 3FFF0098,  1073676440  *)
   VI_SUCCESS_NESTED_SHARED = ($3FFF0099);  (* 3FFF0099,  1073676441  *)
   VI_SUCCESS_NESTED_EXCLUSIVE = ($3FFF009A);  (* 3FFF009A,  1073676442  *)
   VI_SUCCESS_SYNC = ($3FFF009B);  (* 3FFF009B,  1073676443  *)
   VI_WARN_QUEUE_OVERFLOW = ($3FFF000C);  (* 3FFF000C,  1073676300  *)
   VI_WARN_CONFIG_NLOADED = ($3FFF0077);  (* 3FFF0077,  1073676407  *)
   VI_WARN_NULL_OBJECT = ($3FFF0082);  (* 3FFF0082,  1073676418  *)
   VI_WARN_NSUP_ATTR_STATE = ($3FFF0084);  (* 3FFF0084,  1073676420  *)
   VI_WARN_UNKNOWN_STATUS = ($3FFF0085);  (* 3FFF0085,  1073676421  *)
   VI_WARN_NSUP_BUF = ($3FFF0088);  (* 3FFF0088,  1073676424  *)
   VI_WARN_EXT_FUNC_NIMPL = ($3FFF00A9);  (* 3FFF00A9,  1073676457  *)
   VI_ERROR_SYSTEM_ERROR = (_VI_ERROR + $3FFF0000);  (* BFFF0000, -1073807360  *)
   VI_ERROR_INV_OBJECT = (_VI_ERROR + $3FFF000E);  (* BFFF000E, -1073807346  *)
   VI_ERROR_RSRC_LOCKED = (_VI_ERROR + $3FFF000F);  (* BFFF000F, -1073807345  *)
   VI_ERROR_INV_EXPR = (_VI_ERROR + $3FFF0010);  (* BFFF0010, -1073807344  *)
   VI_ERROR_RSRC_NFOUND = (_VI_ERROR + $3FFF0011);  (* BFFF0011, -1073807343  *)
   VI_ERROR_INV_RSRC_NAME = (_VI_ERROR + $3FFF0012);  (* BFFF0012, -1073807342  *)
   VI_ERROR_INV_ACC_MODE = (_VI_ERROR + $3FFF0013);  (* BFFF0013, -1073807341  *)
   VI_ERROR_TMO = (_VI_ERROR + $3FFF0015);  (* BFFF0015, -1073807339  *)
   VI_ERROR_CLOSING_FAILED = (_VI_ERROR + $3FFF0016);  (* BFFF0016, -1073807338  *)
   VI_ERROR_INV_DEGREE = (_VI_ERROR + $3FFF001B);  (* BFFF001B, -1073807333  *)
   VI_ERROR_INV_JOB_ID = (_VI_ERROR + $3FFF001C);  (* BFFF001C, -1073807332  *)
   VI_ERROR_NSUP_ATTR = (_VI_ERROR + $3FFF001D);  (* BFFF001D, -1073807331  *)
   VI_ERROR_NSUP_ATTR_STATE = (_VI_ERROR + $3FFF001E);  (* BFFF001E, -1073807330  *)
   VI_ERROR_ATTR_READONLY = (_VI_ERROR + $3FFF001F);  (* BFFF001F, -1073807329  *)
   VI_ERROR_INV_LOCK_TYPE = (_VI_ERROR + $3FFF0020);  (* BFFF0020, -1073807328  *)
   VI_ERROR_INV_ACCESS_KEY = (_VI_ERROR + $3FFF0021);  (* BFFF0021, -1073807327  *)
   VI_ERROR_INV_EVENT = (_VI_ERROR + $3FFF0026);  (* BFFF0026, -1073807322  *)
   VI_ERROR_INV_MECH = (_VI_ERROR + $3FFF0027);  (* BFFF0027, -1073807321  *)
   VI_ERROR_HNDLR_NINSTALLED = (_VI_ERROR + $3FFF0028);  (* BFFF0028, -1073807320  *)
   VI_ERROR_INV_HNDLR_REF = (_VI_ERROR + $3FFF0029);  (* BFFF0029, -1073807319  *)
   VI_ERROR_INV_CONTEXT = (_VI_ERROR + $3FFF002A);  (* BFFF002A, -1073807318  *)
   VI_ERROR_NENABLED = (_VI_ERROR + $3FFF002F);  (* BFFF002F, -1073807313  *)
   VI_ERROR_ABORT = (_VI_ERROR + $3FFF0030);  (* BFFF0030, -1073807312  *)
   VI_ERROR_RAW_WR_PROT_VIOL = (_VI_ERROR + $3FFF0034);  (* BFFF0034, -1073807308  *)
   VI_ERROR_RAW_RD_PROT_VIOL = (_VI_ERROR + $3FFF0035);  (* BFFF0035, -1073807307  *)
   VI_ERROR_OUTP_PROT_VIOL = (_VI_ERROR + $3FFF0036);  (* BFFF0036, -1073807306  *)
   VI_ERROR_INP_PROT_VIOL = (_VI_ERROR + $3FFF0037);  (* BFFF0037, -1073807305  *)
   VI_ERROR_BERR = (_VI_ERROR + $3FFF0038);  (* BFFF0038, -1073807304  *)
   VI_ERROR_IN_PROGRESS = (_VI_ERROR + $3FFF0039);  (* BFFF0039, -1073807303  *)
   VI_ERROR_INV_SETUP = (_VI_ERROR + $3FFF003A);  (* BFFF003A, -1073807302  *)
   VI_ERROR_QUEUE_ERROR = (_VI_ERROR + $3FFF003B);  (* BFFF003B, -1073807301  *)
   VI_ERROR_ALLOC = (_VI_ERROR + $3FFF003C);  (* BFFF003C, -1073807300  *)
   VI_ERROR_INV_MASK = (_VI_ERROR + $3FFF003D);  (* BFFF003D, -1073807299  *)
   VI_ERROR_IO = (_VI_ERROR + $3FFF003E);  (* BFFF003E, -1073807298  *)
   VI_ERROR_INV_FMT = (_VI_ERROR + $3FFF003F);  (* BFFF003F, -1073807297  *)
   VI_ERROR_NSUP_FMT = (_VI_ERROR + $3FFF0041);  (* BFFF0041, -1073807295  *)
   VI_ERROR_LINE_IN_USE = (_VI_ERROR + $3FFF0042);  (* BFFF0042, -1073807294  *)
   VI_ERROR_NSUP_MODE = (_VI_ERROR + $3FFF0046);  (* BFFF0046, -1073807290  *)
   VI_ERROR_SRQ_NOCCURRED = (_VI_ERROR + $3FFF004A);  (* BFFF004A, -1073807286  *)
   VI_ERROR_INV_SPACE = (_VI_ERROR + $3FFF004E);  (* BFFF004E, -1073807282  *)
   VI_ERROR_INV_OFFSET = (_VI_ERROR + $3FFF0051);  (* BFFF0051, -1073807279  *)
   VI_ERROR_INV_WIDTH = (_VI_ERROR + $3FFF0052);  (* BFFF0052, -1073807278  *)
   VI_ERROR_NSUP_OFFSET = (_VI_ERROR + $3FFF0054);  (* BFFF0054, -1073807276  *)
   VI_ERROR_NSUP_VAR_WIDTH = (_VI_ERROR + $3FFF0055);  (* BFFF0055, -1073807275  *)
   VI_ERROR_WINDOW_NMAPPED = (_VI_ERROR + $3FFF0057);  (* BFFF0057, -1073807273  *)
   VI_ERROR_RESP_PENDING = (_VI_ERROR + $3FFF0059);  (* BFFF0059, -1073807271  *)
   VI_ERROR_NLISTENERS = (_VI_ERROR + $3FFF005F);  (* BFFF005F, -1073807265  *)
   VI_ERROR_NCIC = (_VI_ERROR + $3FFF0060);  (* BFFF0060, -1073807264  *)
   VI_ERROR_NSYS_CNTLR = (_VI_ERROR + $3FFF0061);  (* BFFF0061, -1073807263  *)
   VI_ERROR_NSUP_OPER = (_VI_ERROR + $3FFF0067);  (* BFFF0067, -1073807257  *)
   VI_ERROR_INTR_PENDING = (_VI_ERROR + $3FFF0068);  (* BFFF0068, -1073807256  *)
   VI_ERROR_ASRL_PARITY = (_VI_ERROR + $3FFF006A);  (* BFFF006A, -1073807254  *)
   VI_ERROR_ASRL_FRAMING = (_VI_ERROR + $3FFF006B);  (* BFFF006B, -1073807253  *)
   VI_ERROR_ASRL_OVERRUN = (_VI_ERROR + $3FFF006C);  (* BFFF006C, -1073807252  *)
   VI_ERROR_TRIG_NMAPPED = (_VI_ERROR + $3FFF006E);  (* BFFF006E, -1073807250  *)
   VI_ERROR_NSUP_ALIGN_OFFSET = (_VI_ERROR + $3FFF0070);  (* BFFF0070, -1073807248  *)
   VI_ERROR_USER_BUF = (_VI_ERROR + $3FFF0071);  (* BFFF0071, -1073807247  *)
   VI_ERROR_RSRC_BUSY = (_VI_ERROR + $3FFF0072);  (* BFFF0072, -1073807246  *)
   VI_ERROR_NSUP_WIDTH = (_VI_ERROR + $3FFF0076);  (* BFFF0076, -1073807242  *)
   VI_ERROR_INV_PARAMETER = (_VI_ERROR + $3FFF0078);  (* BFFF0078, -1073807240  *)
   VI_ERROR_INV_PROT = (_VI_ERROR + $3FFF0079);  (* BFFF0079, -1073807239  *)
   VI_ERROR_INV_SIZE = (_VI_ERROR + $3FFF007B);  (* BFFF007B, -1073807237  *)
   VI_ERROR_WINDOW_MAPPED = (_VI_ERROR + $3FFF0080);  (* BFFF0080, -1073807232  *)
   VI_ERROR_NIMPL_OPER = (_VI_ERROR + $3FFF0081);  (* BFFF0081, -1073807231  *)
   VI_ERROR_INV_LENGTH = (_VI_ERROR + $3FFF0083);  (* BFFF0083, -1073807229  *)
   VI_ERROR_INV_MODE = (_VI_ERROR + $3FFF0091);  (* BFFF0091, -1073807215  *)
   VI_ERROR_SESN_NLOCKED = (_VI_ERROR + $3FFF009C);  (* BFFF009C, -1073807204  *)
   VI_ERROR_MEM_NSHARED = (_VI_ERROR + $3FFF009D);  (* BFFF009D, -1073807203  *)
   VI_ERROR_LIBRARY_NFOUND = (_VI_ERROR + $3FFF009E);  (* BFFF009E, -1073807202  *)
   VI_ERROR_NSUP_INTR = (_VI_ERROR + $3FFF009F);  (* BFFF009F, -1073807201  *)
   VI_ERROR_INV_LINE = (_VI_ERROR + $3FFF00A0);  (* BFFF00A0, -1073807200  *)
   VI_ERROR_FILE_ACCESS = (_VI_ERROR + $3FFF00A1);  (* BFFF00A1, -1073807199  *)
   VI_ERROR_FILE_IO = (_VI_ERROR + $3FFF00A2);  (* BFFF00A2, -1073807198  *)
   VI_ERROR_NSUP_LINE = (_VI_ERROR + $3FFF00A3);  (* BFFF00A3, -1073807197  *)
   VI_ERROR_NSUP_MECH = (_VI_ERROR + $3FFF00A4);  (* BFFF00A4, -1073807196  *)
   VI_ERROR_INTF_NUM_NCONFIG = (_VI_ERROR + $3FFF00A5);  (* BFFF00A5, -1073807195  *)
   VI_ERROR_CONN_LOST = (_VI_ERROR + $3FFF00A6);  (* BFFF00A6, -1073807194  *)
   VI_ERROR_NPERMISSION = (_VI_ERROR + $3FFF00A8);  (* BFFF00A8, -1073807192  *)
   (*- Other VISA Definitions -------------------------------------------------- *)

  {VI_VERSION_MAJOR = ( ( ( ( ViVersion ) ver ) and $FFF00000 ) shr 20 );
  VI_VERSION_MINOR = ( ( ( ( ViVersion ) ver ) and $000FFF00 ) shr 8 );
  VI_VERSION_SUBMINOR = ( ( ( ( ViVersion ) ver ) and $000000FF ) );}

   VI_FIND_BUFLEN = (256);
   VI_INTF_GPIB = (1);
   VI_INTF_VXI = (2);
   VI_INTF_GPIB_VXI = (3);
   VI_INTF_ASRL = (4);
   VI_INTF_PXI = (5);
   VI_INTF_TCPIP = (6);
   VI_INTF_USB = (7);
   VI_PROT_NORMAL = (1);
   VI_PROT_FDC = (2);
   VI_PROT_HS488 = (3);
   VI_PROT_4882_STRS = (4);
   VI_PROT_USBTMC_VENDOR = (5);
   VI_FDC_NORMAL = (1);
   VI_FDC_STREAM = (2);
   VI_LOCAL_SPACE = (0);
   VI_A16_SPACE = (1);
   VI_A24_SPACE = (2);
   VI_A32_SPACE = (3);
   VI_A64_SPACE = (4);
   VI_PXI_ALLOC_SPACE = (9);
   VI_PXI_CFG_SPACE = (10);
   VI_PXI_BAR0_SPACE = (11);
   VI_PXI_BAR1_SPACE = (12);
   VI_PXI_BAR2_SPACE = (13);
   VI_PXI_BAR3_SPACE = (14);
   VI_PXI_BAR4_SPACE = (15);
   VI_PXI_BAR5_SPACE = (16);
   VI_OPAQUE_SPACE = ($FFFF);
   VI_UNKNOWN_LA = (-1);
   VI_UNKNOWN_SLOT = (-1);
   VI_UNKNOWN_LEVEL = (-1);
   VI_UNKNOWN_CHASSIS = (-1);
   VI_QUEUE = (1);
   VI_HNDLR = (2);
   VI_SUSPEND_HNDLR = (4);
   VI_ALL_MECH = ($FFFF);
   VI_ANY_HNDLR = (0);
   VI_TRIG_ALL = (-2);
   VI_TRIG_SW = (-1);
   VI_TRIG_TTL0 = (0);
   VI_TRIG_TTL1 = (1);
   VI_TRIG_TTL2 = (2);
   VI_TRIG_TTL3 = (3);
   VI_TRIG_TTL4 = (4);
   VI_TRIG_TTL5 = (5);
   VI_TRIG_TTL6 = (6);
   VI_TRIG_TTL7 = (7);
   VI_TRIG_ECL0 = (8);
   VI_TRIG_ECL1 = (9);
   VI_TRIG_PANEL_IN = (27);
   VI_TRIG_PANEL_OUT = (28);
   VI_TRIG_PROT_DEFAULT = (0);
   VI_TRIG_PROT_ON = (1);
   VI_TRIG_PROT_OFF = (2);
   VI_TRIG_PROT_SYNC = (5);
   VI_TRIG_PROT_RESERVE = (6);
   VI_TRIG_PROT_UNRESERVE = (7);
   VI_READ_BUF = (1);
   VI_WRITE_BUF = (2);
   VI_READ_BUF_DISCARD = (4);
   VI_WRITE_BUF_DISCARD = (8);
   VI_IO_IN_BUF = (16);
   VI_IO_OUT_BUF = (32);
   VI_IO_IN_BUF_DISCARD = (64);
   VI_IO_OUT_BUF_DISCARD = (128);
   VI_FLUSH_ON_ACCESS = (1);
   VI_FLUSH_WHEN_FULL = (2);
   VI_FLUSH_DISABLE = (3);
   VI_NMAPPED = (1);
   VI_USE_OPERS = (2);
   VI_DEREF_ADDR = (3);
   VI_TMO_IMMEDIATE = (0);
   VI_TMO_INFINITE = ($FFFFFFFF);
   VI_NO_LOCK = (0);
   VI_EXCLUSIVE_LOCK = (1);
   VI_SHARED_LOCK = (2);
   VI_LOAD_CONFIG = (4);
   VI_NO_SEC_ADDR = ($FFFF);
   VI_ASRL_PAR_NONE = (0);
   VI_ASRL_PAR_ODD = (1);
   VI_ASRL_PAR_EVEN = (2);
   VI_ASRL_PAR_MARK = (3);
   VI_ASRL_PAR_SPACE = (4);
   VI_ASRL_STOP_ONE = (10);
   VI_ASRL_STOP_ONE5 = (15);
   VI_ASRL_STOP_TWO = (20);
   VI_ASRL_FLOW_NONE = (0);
   VI_ASRL_FLOW_XON_XOFF = (1);
   VI_ASRL_FLOW_RTS_CTS = (2);
   VI_ASRL_FLOW_DTR_DSR = (4);
   VI_ASRL_END_NONE = (0);
   VI_ASRL_END_LAST_BIT = (1);
   VI_ASRL_END_TERMCHAR = (2);
   VI_ASRL_END_BREAK = (3);
   VI_STATE_ASSERTED = (1);
   VI_STATE_UNASSERTED = (0);
   VI_STATE_UNKNOWN = (-1);
   VI_BIG_ENDIAN = (0);
   VI_LITTLE_ENDIAN = (1);
   VI_DATA_PRIV = (0);
   VI_DATA_NPRIV = (1);
   VI_PROG_PRIV = (2);
   VI_PROG_NPRIV = (3);
   VI_BLCK_PRIV = (4);
   VI_BLCK_NPRIV = (5);
   VI_D64_PRIV = (6);
   VI_D64_NPRIV = (7);
   VI_WIDTH_8 = (1);
   VI_WIDTH_16 = (2);
   VI_WIDTH_32 = (4);
   VI_WIDTH_64 = (8);
   VI_GPIB_REN_DEASSERT = (0);
   VI_GPIB_REN_ASSERT = (1);
   VI_GPIB_REN_DEASSERT_GTL = (2);
   VI_GPIB_REN_ASSERT_ADDRESS = (3);
   VI_GPIB_REN_ASSERT_LLO = (4);
   VI_GPIB_REN_ASSERT_ADDRESS_LLO = (5);
   VI_GPIB_REN_ADDRESS_GTL = (6);
   VI_GPIB_ATN_DEASSERT = (0);
   VI_GPIB_ATN_ASSERT = (1);
   VI_GPIB_ATN_DEASSERT_HANDSHAKE = (2);
   VI_GPIB_ATN_ASSERT_IMMEDIATE = (3);
   VI_GPIB_HS488_DISABLED = (0);
   VI_GPIB_HS488_NIMPL = (-1);
   VI_GPIB_UNADDRESSED = (0);
   VI_GPIB_TALKER = (1);
   VI_GPIB_LISTENER = (2);
   VI_VXI_CMD16 = ($0200);
   VI_VXI_CMD16_RESP16 = ($0202);
   VI_VXI_RESP16 = ($0002);
   VI_VXI_CMD32 = ($0400);
   VI_VXI_CMD32_RESP16 = ($0402);
   VI_VXI_CMD32_RESP32 = ($0404);
   VI_VXI_RESP32 = ($0004);
   VI_ASSERT_SIGNAL = (-1);
   VI_ASSERT_USE_ASSIGNED = (0);
   VI_ASSERT_IRQ1 = (1);
   VI_ASSERT_IRQ2 = (2);
   VI_ASSERT_IRQ3 = (3);
   VI_ASSERT_IRQ4 = (4);
   VI_ASSERT_IRQ5 = (5);
   VI_ASSERT_IRQ6 = (6);
   VI_ASSERT_IRQ7 = (7);
   VI_UTIL_ASSERT_SYSRESET = (1);
   VI_UTIL_ASSERT_SYSFAIL = (2);
   VI_UTIL_DEASSERT_SYSFAIL = (3);
   VI_VXI_CLASS_MEMORY = (0);
   VI_VXI_CLASS_EXTENDED = (1);
   VI_VXI_CLASS_MESSAGE = (2);
   VI_VXI_CLASS_REGISTER = (3);
   VI_VXI_CLASS_OTHER = (4);
   VI_PXI_ADDR_NONE = (0);
   VI_PXI_ADDR_MEM = (1);
   VI_PXI_ADDR_IO = (2);
   VI_PXI_ADDR_CFG = (3);
   VI_TRIG_UNKNOWN = (-1);
   VI_PXI_LBUS_STAR_TRIG_BUS_0 = (1000);
   VI_PXI_LBUS_STAR_TRIG_BUS_1 = (1001);
   VI_PXI_LBUS_STAR_TRIG_BUS_2 = (1002);
   VI_PXI_LBUS_STAR_TRIG_BUS_3 = (1003);
   VI_PXI_LBUS_STAR_TRIG_BUS_4 = (1004);
   VI_PXI_LBUS_STAR_TRIG_BUS_5 = (1005);
   VI_PXI_LBUS_STAR_TRIG_BUS_6 = (1006);
   VI_PXI_LBUS_STAR_TRIG_BUS_7 = (1007);
   VI_PXI_LBUS_STAR_TRIG_BUS_8 = (1008);
   VI_PXI_LBUS_STAR_TRIG_BUS_9 = (1009);
   VI_PXI_STAR_TRIG_CONTROLLER = (1413);
   (*- Backward Compatibility Macros ----------------------------------------- *)

   VI_ERROR_INV_SESSION = (VI_ERROR_INV_OBJECT);
   VI_INFINITE = (VI_TMO_INFINITE);
   VI_NORMAL = (VI_PROT_NORMAL);
   VI_FDC = (VI_PROT_FDC);
   VI_HS488 = (VI_PROT_HS488);
   VI_ASRL488 = (VI_PROT_4882_STRS);
   VI_ASRL_IN_BUF = (VI_IO_IN_BUF);
   VI_ASRL_OUT_BUF = (VI_IO_OUT_BUF);
   VI_ASRL_IN_BUF_DISCARD = (VI_IO_IN_BUF_DISCARD);
   VI_ASRL_OUT_BUF_DISCARD = (VI_IO_OUT_BUF_DISCARD);

function viGetDefaultRM(vi: ViPSession): ViStatus;

implementation

function viGetDefaultRM(vi: ViPSession): ViStatus;
begin
   Result := viOpenDefaultRM(vi);
end;

end.
