USB_VID = 0x1209
USB_PID = 0x9000
USB_PRODUCT = "MicroBoy"
USB_MANUFACTURER = "Pepe"

CHIP_VARIANT = RP2040
CHIP_FAMILY = rp2

EXTERNAL_FLASH_DEVICES = "W25Q16JVxQ"

CIRCUITPY__EVE = 1

# Include these Python libraries in firmware.
FROZEN_MPY_DIRS += $(TOP)/frozen/frozen/circuitpython-stage-mocha-pi-go/microboy