import sys
import usbtmc
import sys 
import getopt 
  
def display_conf(serial):
    print(f'UsbGpib config for {serial}')
    inst = usbtmc.Instrument(0x3eb, 0x2065, serial)
    for cmd in [ '!autoid?', '!term?', '!string?', '!ver?' ]:
        inst.pulse()
        print(cmd, inst.ask(cmd))
    inst.close()

def send_cmd(serial, cmd):
    inst = usbtmc.Instrument(0x3eb, 0x2065, serial)
    inst.pulse()
    if cmd[-1] == '?':
        print(inst.ask(cmd))
    else:
        inst.write(cmd)
    inst.close()


def display_all():
    devices = usbtmc.list_devices()
    for d in devices:
        if d.idVendor == 0x3eb and d.idProduct == 0x2065:
            display_conf(d.serial_number)

def usbgpib_conf():
    serial = None
    argv = sys.argv[1:] 
  
    try: 
        opts, args = getopt.getopt(argv, "s:") 
      
    except: 
        print("Error") 
  
    for opt, arg in opts: 
        if opt in ['-s']: 
            serial = arg 

    if len(args) == 0:
        if serial:
            display_conf(serial)
        else:
            display_all()
        return

    cmd = '!' + ' '.join(args)
    print(cmd)
    send_cmd(serial, cmd)
      
usbgpib_conf()     


