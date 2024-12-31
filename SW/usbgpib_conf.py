# nicegui interface to configure (read/write) the xyphro UsbGpib settings
# TODO: This needs work to support more than one UsbGpib interface.
# Written by Jim Houston  
# published 17 Jan 2024 here:
# https://github.com/xyphro/UsbGpib/discussions/55

# Usage: usbgpib_conf command
# !autoid? off
# !term? eoi
# !string? normal
# !ver? V1.6
#
# An example of running the script:
# $ python3 usbgpib_conf.py autoid on
#
from nicegui import ui
import usbtmc

cmd_list = [
    {'desc':'String length:', 'cmd':'!string',
    'values':['normal', 'short'] },
    {'desc':'Auto ID:', 'cmd':'!autoid',
    'values': ['off', 'on', 'slow', 'slower', 'slowest'] },
    {'desc':'Read terminator:', 'cmd':'!term',
    'values':['lf', 'cr', 'eoi', 'store'] }
]

def get_value(c):
    inst = usbtmc.Instrument(0x3eb, 0x2065)
    inst.pulse()
    v = inst.ask(c + '?')
    inst.close()
    return(v)

def set_value(cmd, v):
    inst = usbtmc.Instrument(0x3eb, 0x2065)
    inst.pulse()
    inst.write(f'{cmd} {v}')
    inst.close()
    usbgpib_conf.refresh()

@ui.refreshable
def usbgpib_conf() -> None:
    ui.label('Usbgpib configuration GUI')
    with ui.grid(columns=2):
        ui.label('Firmware version:')
        ui.label(get_value('!ver?'))
        for c in cmd_list:
            v = get_value(c['cmd'])
            ui.label(c['desc'])
            sel = ui.select(options=c['values'], value=v,
                on_change= lambda e: set_value(c['cmd'], e.value))
            sel.c = c

usbgpib_conf()

ui.run()
