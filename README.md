# ppg-instrument
This is an instrument meant for assisting in the monitoring of combustion engines in powered paragliding.

The main features are an RPM gauge, temperature sensors, and hour meter.

There are plans for including and logging other sensors, such as accelerometers, barometer, temperature/humidity.

There are also plans for including a customizable maintenance schedule.

## Electronics
This is based on the [Adalogger](https://www.adafruit.com/?q=adalogger&sort=BestMatch) (trying to support both 32u4 and M0), because of the nice format and space savings from having battery management and SD on the board.

The display is a 0.91" OLED, the choice is mainly due to what I had lying around. A nice drop-in alternative can be a 0.96" OLED, this will probably come in the future.

RPM readings are performed by capacitive coupling a pigtail wire to the spark plug cable. There is a board I designed with input protection for the uController pins.

I wanted to avoid hard switches due to vibrations, in case the instrument gets mounted on the swing arms, so I ended up using all software-based power management and steel tactile domes.

Temperature readings are performed with K-type thermocouples and an AD8495 based amplifier, such as the relatively cheap CJMCU 8495 boards

### Note on thermocouple sensor
The thermocouple CHT (cylinder head temperature) sensor is kind of tricky. This is an insulated thermocouple connected to a spark plug gasket.

The sensors you can find online are really expensive or do not work well. Since electrical insulation is needed due to large voltage oscillations of the engine block, commercial cheaper sensors tend to use ceramics just dropped in a steel tube, and have a huge air gap between the thermocouple junction and the spark plug gasket. Furthermore, the gaskets are usually made of steel. This makes the response extremely slow: dipped in boiling water, it takes 10 minutes to get 95°C.

A CHT sensor can easily be built using an M12 eye cable lug, which needs to be adapted to remove the tinning, smooth out the surface, and make the hole 14.2mm.

After adapting the gasket, the thermocouple junction can be insulated using a wrap of kapton tape (max T>300C), then tightly wrapping in aluminum foil, and then carefully crimping the thermocouple junction in the eye terminal.

## Case
For the case, there are 2 parts plus the buttons that can be 3d printed with an FDM printer.

I printed all parts in PLA with 0.2mm layers, the lid and box with supports. For printing the buttons, make sure the first layer height is just right.

Inspiration for the buttons came from [here](https://www.instagram.com/p/BypbXyjohMr/)

I also use a 1" dual hook & loop strap (like velcro omni-tape) to keep it fixed to my throttle.

## Assembly Instructions
Check [here](hardware/Assembly%20Instructions/Instructions.md) for assembly instructions.


### Parts List 
|Part|Source|Price|                        
| ------------- |:-------------:| -----:|
| Adalogger board | [Adafruit](https://www.adafruit.com/product/2796), [Amazon](https://www.amazon.de/Adafruit-Feather-M0-Adalogger-ADA2796/dp/B01BMRDBXW/ref=sr_1_1?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&keywords=adalogger+m0&qid=1636467653&qsid=261-5169470-0170866&sr=8-1&sres=B01BMRDBXW%2CB00D45ZOFE%2CB01GHY0ZH0%2CB00KREL5C6%2CB01BZRN8B4%2CB01MUFRHQ2%2CB079R8QZR6%2CB08FYDDQVV%2CB00556QVIY%2CB07CZ7HMHF%2CB007EJ37W8%2CB07MMTRSHV%2CB083R3RQLK%2CB092LCR66T%2CB071K692QJ%2CB0928G89SM%2CB07MLWKK6V%2CB08FY4ZXZ9%2CB07RD2JCFW%2CB07MCGYWFW) | €27.90 |
| Battery | [1300mAh 503759](https://www.ebay.com/itm/171260143191)      |   €6.60 |
| SD card | [ebay, 4Gb](https://www.ebay.it/itm/303426113204?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2060353.m2749.l2649)      |    €7.16 |
| Thermocouple amp | [Aliexpress](https://www.aliexpress.com/i/33014907258.html)      |    $5.38 |
| Thermocouple wire | [ebay](https://www.ebay.it/itm/274905710453?hash=item4001a83f75:g:jTAAAOSwh59hFzC6)   |    €2.48 per meter |
| Spark plug gasket | [Amazon](https://www.ebay.it/itm/272789432218?hash=item3f8384679a:g:DPAAAOSwQcJaDBZB)      |    ~ €2.00 per piece |
| 3D printed parts | filament      |   ~ €1.00 |
| Spark plug input board |       |    ~ €3.00 |
| Screen | [0.91" white oled, Amazon](https://www.amazon.de/-/en/0-91-I2-°C-Serial-128x32-White-Display-Arduino/dp/B06XFHGT9P/ref=sr_1_3?keywords=0%2C91+weiß+oled&qid=1636468399&qsid=262-1393502-5513624&sr=8-3&sres=B07V8B3LSR%2CB07TKPXFLV%2CB06XFHGT9P%2CB0979YQWGT%2CB08K8XVJBT%2CB07TWJR7L9%2CB07JN2NHT4%2CB079KRC9X3%2CB01IROOJ48%2CB08216LXVQ%2CB01N97V0KH%2CB07FYG8MZN%2CB074N9VLZX%2CB014I8SIJY%2CB07PGZ9B51%2CB08MZV1TT9)      |    €3.00 per piece |
| hook&loop strap | [treadlite](https://www.treadlitegear.co.uk/ripfast-solo-hook-and-loop-in-20mm-and-25mm-hiking-bike-packing-choose-length-211-p.asp)  |    £5.99 |
| Tactile dome switches | [Aliexpress](https://www.aliexpress.com/item/32668884101.html)    |    $0.24 for 2 pieces |