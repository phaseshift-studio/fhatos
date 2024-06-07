## Introduction

**FhatOS** (pronounced _fat_) is a [distributed operating system](https://en.wikipedia.org/wiki/Distributed_operating_system) for [ESP8266](https://en.wikipedia.org/wiki/ESP8266) and [ESP32](https://en.wikipedia.org/wiki/ESP32) processors with support for sandboxed deployments on Linux and MacOSX systems. All FhatOS resources, from individual datum, files, processes exist within a single [URI](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier) address space called **furi** (pronounced "fury" or "fhat uri"). Processes communicate via a publish-subscribe message passing protocol that rides atop [MQTT](https://en.wikipedia.org/wiki/MQTT) with various levels of access from thread local, to machine local and ultimately, globally via cluster remote.

### Software and Device Drivers

FhatOS software can be written in C/C++ or mm-ADT (multi-model abstract data type). mm-ADT is a cluster-oriented programming language and virtual machine founded on 5 _mono-types_ (**bool**, **int**, **real**, **uri**, and **str**) and 2 _poly-types_ (**lst** and **rec**). mm-ADT programs are sent via message to the FhatOS scheduler and then spawned as a process accordingly. FhatOS provides a collection of device drivers and new device drivers can be written (typically in C/C++). Provided drivers include pulse wave modulation, thermometer, gas, H-bridge, etc. sensors and actuators. 

A simple mm-ADT program is defined below and published `<=` to the `kernel@scheduler` query `?spawn`. The program will run on an individual `thread` with a **furi** id of `@logger` and a reactive `setup` function that logs any messages published to the program's id. 

```xpath2
publish(kernel@scheduler?spawn,
        [process -> thread,
         id      -> @logger,
         setup   -> subscribe(self.by(id),log(__))])
```