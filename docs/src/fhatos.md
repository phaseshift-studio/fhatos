# Introduction

**FhatOS** (pronounced _fat_) is
a [distributed operating system](https://en.wikipedia.org/wiki/Distributed_operating_system)
for [ESP8266](https://en.wikipedia.org/wiki/ESP8266) and [ESP32](https://en.wikipedia.org/wiki/ESP32) processors with
support for sandboxed deployments on Linux and MacOSX systems. All FhatOS resources, from individual datum, files,
processes exist within a single [URI](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier) address space called
**furi** (pronounced "fury" or "fhat uri"). Processes communicate via a publish-subscribe message passing protocol that
rides atop [MQTT](https://en.wikipedia.org/wiki/MQTT) with various levels of access from thread local, to machine local
and ultimately, globally via cluster remote.

### Booting FhatOS

#### Booting on Linux/Unix/Mac

#### Booting on ESP32

#### Booting on ESP8266

### A FhatOS Language

FhatOS software can be written in C/C++ or mm-ADT (**multi-model abstract data type**). mm-ADT is a cluster-oriented
programming language and virtual machine founded on 5 _mono-types_ (`bool`, `int`, `real`, `uri`, and `str`)
and 2 _poly-types_ (**lst** and **rec**), where `rec` is a `lst` of 2-`obj` `lsts` with particular semantics regarding
key uniqueness. mm-ADT programs are sent via message to the FhatOS scheduler and then spawned
as a process accordingly. FhatOS provides a collection of device drivers and new device drivers can be written (
typically in C/C++). Provided drivers include pulse wave modulation, thermometer, gas, H-bridge, etc. sensors and
actuators.

A simple mm-ADT program is defined below. The program is a specialization of the poly-type `rec` called `thread`,
where `thread` is abstractly defined as

```.cpp
thread[[setup => __]
        loop  => __]]
```

```.cpp
fhatos> define(/rec/person,[name=>as(/str/),age=>is(gt(0))]) 
```

The `thread` object is published to the fURI endpoint `esp32@127.0.0.1/scheduler/threads/logger`. The scheduler spawns
the program on an individual `thread` accessible via the target fURI. Once spawned, the `setup` function prints the
thread's id and halts.

```.cpp
fhatos> thread[[setup => print('setup complete'),
                loop  => stop(/abc/)]].to(/abc/)
```

```.cpp
fhatos> */abc/
==> thread[[setup => print('setup complete'),
            loop  => stop(/abc/)]]
```

### A FhatOS Console

> [!important]
> The FhatOS Console is a composite of 3 other actors:
> 1. The `Terminal` (`/sys/io/terminal/`) provides thread-safe access to hardware I/O.
> 2. The `Parser` (`/sys/lang/parser/`) converts string input to bytecode output.
> 3. The `Processor` (`/sys/lang/processor/`) executes bytecode.

```
terminal/in =[str]=> console 
  =[str]=> parser =bcode<~/abc>=> 
    processor =[objs]=> ~/abc 
      <=[objs]= console 
        =[str]=> terminal/out
```

### fURI and MQTT

[MQTT](https://en.wikipedia.org/wiki/MQTT) is a publish/subscribe message passing protocol that has found extensive
usage in embedded systems. Hierarchically specified _topics_ can be _subscribed_ and _published_ to. In MQTT, there is
no direct communication between actors, though such behavior can be simulated if an actor's mailbox is a unique topic.
FhatOS leverages MQTT, but from the vantage point of URIs instead of topics with message routing being location-aware.
There exist three MQTT routers:

1. `MonadRouter`: An MQTT router scoped to an active monad (**thread**) processing a monoid (**program**).
2. `MonoidRouter`: An MQTT router scoped to a monoid (**program**).
3. `LocalRouter`: An MQTT router scoped to the current host (**machine**).
4. `GlobalRouter`: An MQTT router scoped to the current swarm (**cluster**).
5. `MetaRouter`: An MQTT router dynamically scoped to other routers based on fURI endpoints.

> [!note]
> The following is a list of common FhatOS fURI endpoints, where `fos:` is the namespace prefix
> for `furi://fhatos.org/`.
> * `fos:types/+` (the base types of mm-ADT)
> * `fos:types/+/+` (user defined base-extending types)
> * `furi://+` (the machines in the cluster)
> * `furi://+/+` (the kernel processes of the machines in the cluster)
> * `furi://+/scheduler/thread/` (the machine's threads)
> * `furi://#/scheduler/thread/` (the cluster's threads)

<!-- CODE:BASH:START -->
<!-- ../build/docs/build/main_runner.out -->
<!-- CODE:END -->
<!-- OUTPUT:START -->
...
<!-- OUTPUT:END -->
