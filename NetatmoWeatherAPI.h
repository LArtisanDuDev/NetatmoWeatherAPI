#ifndef NetatmoWeatherAPI_h
#define NetatmoWeatherAPI_h
#endif

#include <WString.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define EXPIRED_ACCESS_TOKEN 3
#define INVALID_ACCESS_TOKEN 2
#define SOMETHING_ELSE 1
#define VALID_ACCESS_TOKEN 0

struct module_struct
{
  String name = "";
  String temperature = "";
  String min = "";
  String max = "";
  String trend = "";
  int battery_percent = 0;
  String co2 = "";
  String humidity = "";
  String rain = "";
  String sum_rain_1h = "";
  String sum_rain_24h = "";
  String reachable = "";

  unsigned long timemin = 0;
  unsigned long timemax = 0;
  unsigned long timeupdate = 0;
};

class NetatmoWeatherAPI
{
public:
  NetatmoWeatherAPI();
  ~NetatmoWeatherAPI();
  void setDebug(bool debug);
  int getStationsData(char (&access_token)[58], String device_id, unsigned long delay_timezone);
  bool getRefreshToken(char (&access_token)[58], char (&refresh_token)[58], String client_secret, String client_id);
  
  module_struct
      // station
      NAMain,
      // module extérieur
      NAModule1,
      // modules intérieurs
      NAModule4[3],
      // pluviomètre
      NAModule3;

protected:
  void sendDataToDebugServer(String message);
  void dumpModule(module_struct module);
  
  bool _debug;
  
};