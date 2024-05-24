#include <fhatos.hpp>
#include FOS_PROCESS(scheduler.hpp)
#ifdef NATIVE
#include #include <structure/f_simple.hpp>
using namespace fhatos;
int main(int arg, char **argsv) {
  try {
    Scheduler::singleton()->spawn(new fSimple<Thread>("s1@127.0.0.1"));
    Scheduler::singleton()->spawn(new fSimple<Thread>("s2@127.0.0.1"));
    Scheduler::singleton()->spawn(new fSimple<Thread>("s3@127.0.0.1"));
    Scheduler::singleton()->join();
  } catch (fError* e) {
    LOG(ERROR,"main() error: %s\n",e->what());
   // LOG_EXCEPTION(e);
  }
};
#else
#include FOS_MODULE(kernel/f_kernel.hpp)
#include FOS_MODULE(io/net/f_wifi.hpp)
#include <structure/f_soc.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include FOS_ROUTER(local_router.hpp)
#include FOS_ROUTER(mqtt_router.hpp)
#include FOS_MODULE(io/f_log.hpp)
#include <structure/f_soc.hpp>
#include FOS_MODULE(io/f_serial.hpp)
#include FOS_MODULE(io/net/f_ping.hpp)
#include FOS_MODULE(io/net/f_telnet.hpp)
#include FOS_MODULE(io/fs/f_fs.hpp)
#include <language/fluent.hpp>
#include <language/instructions.hpp>
using namespace fhatos;
void setup() {
  fKernel<>::bootloader({
      fWIFI::singleton(),
      fKernel<>::singleton(),
      FOS_DEFAULT_ROUTER::singleton(),
      fScheduler<>::singleton(),
      fMemory<>::singleton(),
      fFS<>::singleton(),
      fOTA<>::singleton()
  });
  fScheduler<>::singleton()->spawn(fSoC<>::singleton());
  fScheduler<>::singleton()->spawn(new fLog());
  fScheduler<>::singleton()->spawn(fSerial<>::singleton());
  fScheduler<>::singleton()->spawn(new fPing<>());
  fScheduler<>::singleton()->spawn(fTelnet<>::singleton());
}


void loop() {

}
#endif
