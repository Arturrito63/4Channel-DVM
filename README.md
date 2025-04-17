## BluePill STM32F103C8T6 - Voltimetro de 4 canales
***(BluePill STM32F103C8T6 - 4 channel voltmeter)***
#### Voltimetro de 4 canales con registro y trazado de lecturas/muestras en tiempo real
***(4-channel voltmeter with real-time recording and plotting of readings/samples)***

Ver video  
[![Ver video](https://img.youtube.com/vi/DioO7w2Ja70/0.jpg)](https://www.youtube.com/watch?v=DioO7w2Ja70)

------------

### Montaje

En este proyecto utilizo un display típico de Arduino, un Shield ILI9341 de 2.8". Estos funcionan en 5V, tanto la alimentación (+VCC) como las señales digitales, pero el display que trae el shield funciona en 3.3V por lo que consta de un regulador de voltaje (AMS1117) más dos chips (74HC245) que se encargan de convertir las señales de 5V a 3.3V y viceversa.

Para poder conectar este shield con el modulo BluPill hay que hacer que funcione a 3.3V, para ello quitamos los chips y el regulador, además de otras modificaiones.

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/Display%20ILI9341%20ante%20y%20despues.jpg)

Es necesario hacer un puente como muestra la siguiente imagen, y encima del mismo se recomienda aplicar algún aislante (en mi caso cinta kapton).

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/ILI9341%203V3%20reset%20jumper.jpg)

Al shield le quite los pines que originalmente trajo soldados y solde nuevos en los contactos libres, de modo que el pinout queda según se muestra la imágen a continuación.

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/ILI934%203V3%20pinout.jpg) 


La conexión del display con el modulo BluePill para este proyecto queda como se muestra en la siguiente tabla.
 
| ILI9341 | - | Shield | BluePill |
| ------------ | ------------ | ------------ | ------------ |
|  DB10 |   | D0   | PB0  |
|  DB11 |   | D1   | PB1  |
|  DB12 |   | D2   | PB2  |
|  DB13 |   | D3   | PB3  |
|  DB14 |   | D4   | PB4  |
|  DB15 |   | D5   | PB5  |
|  DB16 |   | D6   | PB6  |
|  DB17 |   | D7   | PB7  |
|   |   |   |   |
|  RSTn |   | RST  | PB8  |
|  CSn |   | CS   | PB12  |
|  D/Cn |   | DC   | PB13  |
|  WRn |   | WR   | PB14  |
|  RDn |   | RD   | PB15  |

------------
