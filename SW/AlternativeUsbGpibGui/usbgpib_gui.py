# nicegui interface to configure the xyphro UsbGpib settings
# Jim Houston

import multiprocessing
multiprocessing.set_start_method("spawn", force=True)
from multiprocessing import Process, Lock

from nicegui import ui, app, run
import usbtmc
import usb.core
import usb.util

cmd_list = [
    {'desc':'Firmware version:', 'cmd':'!ver' },
    {'desc':'String length:', 'cmd':'!string',
    'values':['normal', 'short'] },
    {'desc':'Auto ID:', 'cmd':'!autoid',
    'values': ['off', 'on', 'slow', 'slower', 'slowest'] },
    {'desc':'Read terminator:', 'cmd':'!term',
    'values':['lf', 'cr', 'eoi', 'store'] }
]

dev_sel = None
sel = {}
lab = {}
lock = Lock()

def open_inst(sel):
    # print(f'open_inst(sel={sel})')
    if not sel or not sel.value:
        print(f'No serial number selected')
        return None
    inst = usbtmc.Instrument(0x3eb, 0x2065, sel.value)
    if not inst:
        print(f'Failed to open instrument {sel.value}')
    return inst

def set_value(cmd, v):
    # print(f'set_value({cmd}, {v})')
    lock.acquire()
    try:
        inst = open_inst(dev_sel)
        if not inst:
            return
        inst.pulse()
        discard = inst.ask(f'{cmd} {v}')
        # read back the value.  When we select store I want to go
        # back to showing the term value.
        inst.pulse()
        v = inst.ask(cmd + '?')
        inst.close()
    finally:
        lock.release()
    return v

async def update_value(e):
    # print(f'update_value({e})')
    sel = e.sender
    v = e.value
    cmd = sel.cmd
    # on_change is called when we set the initial value sigh.
    # Don't update the hardware if nothing has changed.
    # print(f'cmd = {cmd} v = {v} sel.value = {sel.value}')
    # print(f'sel.last_val = {sel.last_val}')
    if  sel.last_val == v:
        return
    v = await run.io_bound(set_value, cmd, v)
    if not v:
        return
    if sel.value != v:
        sel.value = v
        sel.last_val = v

def do_exit():
    ui.run_javascript('window.close()')
    app.shutdown()

def get_values():
    # print(f'get_values')
    lock.acquire()
    try:
        inst = open_inst(dev_sel)
        if not inst:
            return
        v = {}
        for c in cmd_list:
            cmd = c['cmd']
            inst.pulse()
            v[cmd] = inst.ask(cmd + '?')
        inst.close()
    finally:
        lock.release()
    return(v)

async def async_get_values():
    v = await run.io_bound(get_values)
    # print(f'v = {v}')
    if v:
        for cmd in v.keys():
            if cmd in sel.keys():
                sel[cmd].last_val = v[cmd]
                sel[cmd].value = v[cmd]
            if cmd in lab.keys():
                lab[cmd].text = v[cmd]

# I poll to detect USB insert/removal.  There usbtmc python
# library doesn't detect that the usbgpib has been removed.

def list_devs():
#    devices = usbtmc.list_devices()
#    l = []
#    for d in devices:
#        if d.idVendor == 0x3eb and d.idProduct == 0x2065:
#            try:
#                l.append(d.serial_number)
#            except ValueError:
#                print(f'list_devs: got ValueError exception')
# find our device
    lock.acquire()
    try:
        l = []
        for d  in usb.core.find(idVendor=0x03eb, idProduct=0x2065, find_all=True):
            #langids = d.langids
            #print(langids)
            #serial = usb.util.get_string(d, d.iSerialNumber, langids[0])
            serial = usb.util.get_string(d, d.iSerialNumber, 1033)
            l.append(serial)
    finally:
        lock.release()

    return l

async def update_devices(sel):
    #l = await run.cpu_bound(list_devs)
    l = list_devs()
    if set(l) == set(sel.options):
        return
    v = sel.value
    # pick the first serial number if we don't have a useable value
    # if l and not sel.value in l:
    if l != [] and (sel.value == None or (not sel.value in l)):
        sel.set_options(l, value=l[0])
    else:
        sel.set_options(l)

@ui.page("/")
async def usbgpib_conf() -> None:
    global dev_sel
    ui.label('Usbgpib configuration GUI')
    ui.button('Exit', on_click = lambda: do_exit())

    c1 = ui.card()
    c2 = ui.card()
    
    with c2:
        with ui.grid(columns=2):
            for c in cmd_list:
                cmd = c['cmd']
                ui.label(c['desc'])
                if 'values' in c.keys():
                    sel[cmd] = ui.select(options=c['values'],
                        on_change= update_value)
                    sel[cmd].cmd = cmd
                else:
                    lab[cmd] = ui.label()
    
    with c1:
        dev_sel = ui.select([], label='Select device serial number',
            on_change= lambda: async_get_values())
    
    ui.timer(1.0, lambda: update_devices(dev_sel))



ui.run(reload=False)
#ui.run(native=True, window_size=(400, 500), fullscreen=False,reload=False)
