# ppg-instrument
This is an instrument meant for assisting in the monitoring of combustion engines in powered paragliding.

The main features are an RPM gauge, temperature sensors, and hour meter.

There are plans for including and logging other sensors, such as accelerometers, barometer, temperature/humidity.

There are also plans for including a customizable maintenance schedule.

## Electronics
This is based on the [Adalogger](https://www.adafruit.com/?q=adalogger&sort=BestMatch) (trying to support both 32u4 and M0), because of the nice format and space savings from having battery management and SD on the board.

The display is a 0.91" OLED, the choice is mainly due to what I had lying around. A nice drop-in alternative can be a 0.96" OLED, this will probably come in the future.

Temperature readings are performed with K-type thermocouples and an AD8495 based amplifier, such as the relatively cheap CJMCU 8495 boards

RPM readings are performed by capacitive coupling a pigtail wire to the spark plug cable. There is a board I designed with input protection for the uController pins.

I wanted to avoid hard switches due to vibrations, in case the instrument gets mounted on the swing arms, so I ended up using all software-based power management and steel tactile domes.

## Case
For the case, there are 2 parts plus the buttons that can be 3d printed with an FDM printer.

I printed all parts in PLA with 0.2mm layers, the lid and box with supports. For printing the buttons, make sure the first layer height is just right.

Inspiration for the buttons came from [here](https://www.instagram.com/p/BypbXyjohMr/)

I also use a 1" dual hook & loop strap (like velcro omni-tape) to keep it fixed to my throttle.

## Assembly Instructions
For the buttons, I chose to go for a cheaper, more reliable, more customizable way. This is however a little more tricky.
You need some of these

The screen is small and quite delicate. Be careful when assembling. Should probably design retaining niche better

the batetry i had lying around was a 1000mAh 60x40mmx5mm battery. This was pretty old, now they make them smaller of larger capacity for same size. probably a smaller one will fit nicely

The SD card quality is quite important. When not in use, larger, faster SD cards will use up a lot of battery. I bought some cheap 4Gb ones, which use 200uA when in standby.

The fastening strap is mixed hook&loop, which means it sticks to itself. I found this pretty convenient, and pretty safe.

The spark plug input circuit works by capacitive coupling a pigtail wire to the spark plug cable. The spark plug cable itself will reach pretty high negative voltages, in the 10kV range. This in turn causes a positive voltage spike in the pigtail wire. The circuit is designed with an input low pass filter, so the spike does not reach dangerous values, and right after this, a high pass filter that is more meant as a protection than a filter. Then, there are two diodes for input protection against overvoltages and undervoltages for a transistor, which generates a ~20us low-logic pulse every time a spark happens.
You can make the circuit on perf board, using smd components.



### Parts List 
|a|s|d|                        
| ------------- |:-------------:| -----:|
| Adalogger board | right-aligned | $1600 |
| Battery | centered      |   $12 |
| SD card | are neat      |    $1 |
| Thermocouple amp | are neat      |    $1 |
| 3D printed parts | are neat      |    $1 |
| Spark plug input board | are neat      |    $1 |
| screen | are neat      |    $1 |
| hook&loop strap | are neat      |    $1 |
| buttons | are neat      |    $1 |