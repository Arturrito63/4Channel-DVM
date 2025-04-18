## BluePill STM32F103C8T6 - Voltimetro de 4 canales
***(BluePill STM32F103C8T6 - 4 channel voltmeter)***
#### Voltimetro de 4 canales con registro y trazado de lecturas/muestras en tiempo real
***(4-channel voltmeter with real-time recording and plotting of readings/samples)***

Ver video  
[![Ver video](https://img.youtube.com/vi/DioO7w2Ja70/0.jpg)](https://www.youtube.com/watch?v=DioO7w2Ja70)

------------

### Preparación...

En este proyecto utilizo un display típico de Arduino, un Shield ILI9341 de 2.8". Estos funcionan en 5V, tanto la alimentación (+VCC) como las señales digitales, pero el display que trae el shield funciona en 3.3V por lo que consta de un regulador de voltaje (AMS1117) más dos chips (74HC245) que se encargan de convertir las señales de 5V a 3.3V y viceversa.

Para poder conectar este shield con el modulo BluPill hay que hacer que funcione a 3.3V, para ello quitamos los chips y el regulador, además de otras modificaiones.

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/Display%20ILI9341%20ante%20y%20despues.jpg)

Es necesario hacer un puente como muestra la siguiente imagen, y encima del mismo se recomienda aplicar algún aislante (en mi caso cinta kapton).

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/ILI9341%203V3%20reset%20jumper.jpg)

Al shield le quite los pines que originalmente trajo soldados y solde nuevos en los contactos libres, de modo que el pinout queda según se muestra la imágen a continuación.

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/ILI934%203V3%20pinout.jpg) 


Para este proyecto utilizo un modulo BluePill con un MCU STM32F103C8T6. La conexión del display con el modulo BluePill queda entonces como se muestra en la siguiente tabla.
 
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

#### Importante

En el modulo BluePill no existe un pin conectado directamente a PB2 del chip STM32F103C8T6, PB2 esta conectado a travez de un resistencia (R4) de 100KΩ al pin BOOT1. Durante el arranque normal del modulo es necesario que este pin este conectado a GND (la posición por defecto del jumper en BOOT1). En el diagrama a continuación puede verse a R4 conctado de un extremo al pin 4 del header (BOOT1) y de su otro extremo al pin 20 del MCU.

[![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/BluePill%20diagram.png)](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/STM32F103-Blue-Pill.pdf)


Para poder contar con PB2 hay que remover R4, hacer un puente en su lugar (puede usarse una resistencia de 0Ω) y luego soldar R4 entre los pines 4 y 6 del header BOOT0/BOOT1. Esto elimina la necesidad del jumper selector en BOOT1 por lo que ahora ya se podrá utilizar el pin 4 del header como PB2. Hay que asegurarse que durante el inicio del MCU no haya en este pin un valor lógico diferente de 0. 

![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/BluePill_front_%26_back_pcb.jpg)

------------

### Montaje...

Para el montaje además del modulo BluePill ya modificado, al igual que el shield ILI9341 tambien modificado para tal fin, se suma un led a elección, una resistencia de 330Ω 1/4W, un switch tipo pulsador (push button), un resonador piezoelectrico (buzzer) y un encoder rotativo con pulsador.

[![](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/4-Channel-DVM.png)](https://github.com/Arturrito63/4Channel-DVM/blob/main/Documents/4-Channel-DVM.pdf)

------------

### Programa...
