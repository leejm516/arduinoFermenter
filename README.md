# arduinoFermenter

This is the source code of a custom Arduino-based bioreactor for cultivating microbes. 
It was first motivated by unaffordable prices of bioreactors from BIG companies. In addition, many of those bioreactors are difficult 
to customize depending on the purpose of experiments.

Currently, the code is working on Arduino Mega, and modifications will be necessary if you want to use in other Arduino boards.
Because of me being a researcher in microbiology (and biochemical engineering), note that my code is not clean and development will be slow.

## Primary Goals

- Implement control functions for basic parameters (temperature, pH, agitation, DO, etc.)


## TODOs

- Implement communication between an ESP32 board (for transmitting data wirelessly to a server) and Arduino Mega