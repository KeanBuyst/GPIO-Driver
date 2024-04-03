cmd_/home/pi/drivers/gpio-driver/gpio-driver.mod := printf '%s\n'   gpio-driver.o | awk '!x[$$0]++ { print("/home/pi/drivers/gpio-driver/"$$0) }' > /home/pi/drivers/gpio-driver/gpio-driver.mod
