#pragma once

#ifdef __cplusplus
extern "C" {

#ifdef ARDUINO

void loop();
void setup();

#elif defined(ESP_PLATFORM)

void app_main();

#else

int main(int argc, char **argv);

#endif

}
#endif

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#ifdef ARDUINO

#define MAIN() \
void loop(){ \
    Serial.println("loop"); \
    while(1){  } \
}\
int call_setup();\
void setup(){\
    Serial.begin(FOS_SERIAL_BAUDRATE); \
    Serial.println("setup"); \
    call_setup();\
}\
int call_setup()

#elif defined(ESP_PLATFORM)

#define MAIN() \
int call_app_main();\
void app_main(){\
    Serial.begin(FOS_SERIAL_BAUDRATE); \
    Serial.println("entering app_main"); \
    call_app_main();\
}\
int call_app_main()

#else

#define MAIN() \
int main(int argc, char **argv)

#endif