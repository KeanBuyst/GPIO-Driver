sudo rmmod gpio_driver

sudo dmesg -C

sudo insmod gpio-driver.ko

sleep 1
clear
dmesg
