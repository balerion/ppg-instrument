# ppg-instrument
This is an instrument meant for assisting in the monitoring of thermic engines in powered paragliding.

The main features are an RPM gauge, temperature sensors, and hour meter.

There are plans for including and logging other sensors, such as accelerometers, barometer, temperature/humidity.

There are also plans for including a customizable maintenance schedule.

## Electronics
This is based on the [Adalogger](https://www.adafruit.com/?q=adalogger&sort=BestMatch) (trying to support both 32u4 and M0), because of the nice format and space savings from having battery management and SD on the board.

The display is a 0.91" OLED, the choice is mainly due to what I had lying around. A nice drop-in alternative can be a 0.96" OLED, this will probably come in the future.

Temperature readings are performed with K-type thermocouples and an AD8495 based amplifier, such as the CJMCU 8495 boards

RPM readings are performed by capacitive coupling a pigtail wire to the spark plug cable. There is a board I designed with input protection for the uController pins.

I wanted to avoid hard switches due to vibrations, in case the instrument ended up in the swing arms, so I ended up using all software-based power management and steel tactile domes.

## Case
For the case, there are 2 parts plus the buttons that can be 3d printed with an FDM printer.

I printed all parts in PLA with 0.2mm layers, the lid and box with supports. For printing the buttons, make sure the first layer height is just right.

Inspiration for the buttons came from [here](https://www.instagram.com/p/BypbXyjohMr/)

I also use a 1" dual hook & loop strap (like velcro omni-tape) to keep it fixed to my throttle.