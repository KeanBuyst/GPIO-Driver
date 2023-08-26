# GPIO-Driver

Simple linux gpio driver for the raspberrypi.

Interact with the driver using `/proc/gpio` once installed.

## For writing (input)

- g <pin> <0/1> - set pin input or output
- o <pin> <1/0> - output high or low
- l <pin> <1/0> - read pin for high or low (binary ingnored)

## For reading (output)

returns the 0 or 1 based on the read instruction `l <pin> <1/0>`
