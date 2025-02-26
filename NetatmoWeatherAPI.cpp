#include "NetatmoWeatherAPI.h"

NetatmoWeatherAPI::NetatmoWeatherAPI()
{
  _debug = false;
  lastBody = "";
  allHttpCodes = "";
}

NetatmoWeatherAPI::~NetatmoWeatherAPI()
{
}

void NetatmoWeatherAPI::setDebug(bool debug)
{
  _debug = debug;
}

bool NetatmoWeatherAPI::getRefreshToken(char (&access_token)[58], char (&refresh_token)[58], String client_secret, String client_id)
{
  String tmp_refresh_token = String(refresh_token);
  tmp_refresh_token.replace("|", "%7C");

  bool retour = false;
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String netatmoRefreshTokenPayload = "client_secret=" + client_secret + "&grant_type=refresh_token&client_id=" + client_id + "&refresh_token=" + tmp_refresh_token;
    if (_debug)
    {
      Serial.println(netatmoRefreshTokenPayload);
    }

    http.begin("https://api.netatmo.com/oauth2/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(netatmoRefreshTokenPayload);
    allHttpCodes += String(httpCode) + "|";
    if (_debug)
    {
      Serial.println("Server Response :");
      Serial.println(httpCode);
    }

    if (httpCode > 0)
    {
      DynamicJsonDocument doc(1024);
      String body = http.getString();
      lastBody = body;
      
      if (_debug)
      {
        Serial.println("getRefreshToken body :");
        Serial.println(body);
      }

      deserializeJson(doc, body);
      if (doc.containsKey("access_token"))
      {
        const char *tmp = doc["access_token"].as<String>().c_str();

        memcpy(access_token, tmp, 58);
        if (_debug)
        {
          Serial.println(String("Received access token :") + String(access_token));
        }
        retour = true;
      }
      else
      {
        Serial.println("No access_token");
      }

      if (doc.containsKey("refresh_token"))
      {
        const char *tmp = doc["refresh_token"].as<String>().c_str();
        memcpy(refresh_token, tmp, 58);

        if (_debug)
        {
          Serial.println("Received refresh token :" + String(refresh_token));
        }
      }
      else
      {
        retour = false;
      }
    }
    else
    {
      Serial.println("refreshToken Error : " + http.errorToString(httpCode));
    }
    http.end();
  }
  return retour;
}

int NetatmoWeatherAPI::getStationsData(char (&access_token)[58], String device_id, unsigned long delay_timezone)
{

  String tmp_device_id = device_id;
  tmp_device_id.replace(":", "%3A");
  String tmp_access_token = String(access_token);

  int result = SOMETHING_ELSE;
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String netatmoGetStationsData = "https://api.netatmo.com/api/getstationsdata?device_id=" + tmp_device_id + "&get_favorites=false";
    if (_debug)
    {
      Serial.println(netatmoGetStationsData);
    }

    http.begin(netatmoGetStationsData);

    if (_debug)
    {
      Serial.println("Bearer " + tmp_access_token);
    }

    http.addHeader("Authorization", "Bearer " + tmp_access_token);
    int httpCode = http.GET();
    allHttpCodes += String(httpCode) + "|";
    
    if (_debug)
    {
      Serial.println("Server Response :");
      Serial.println(httpCode);
    }

    if (httpCode == HTTP_CODE_FORBIDDEN || httpCode == HTTP_CODE_OK)
    {
      DynamicJsonDocument doc(12288);
      String body = http.getString();
      lastBody = body;
        
      if (_debug)
      {
        Serial.println("getStationsData body :");
        Serial.println(body);
      }

      deserializeJson(doc, body);
      if (httpCode == HTTP_CODE_FORBIDDEN)
      {
        errorMessage = doc["error"]["message"].as<String>();
        // deal with error
        unsigned long code = doc["error"]["code"].as<unsigned long>();
        switch (code)
        {
        case 3:
          result = EXPIRED_ACCESS_TOKEN;
          break;
        case 2:
          result = INVALID_ACCESS_TOKEN;
          break;
        default:
          result = SOMETHING_ELSE;
        }
      }

      if (httpCode == HTTP_CODE_OK)
      {
        JsonArray stations = doc["body"]["devices"].as<JsonArray>();
        for (JsonObject station : stations)
        {
          NAMain.name = station["module_name"].as<String>();
          NAMain.min = station["dashboard_data"]["min_temp"].as<String>();
          NAMain.max = station["dashboard_data"]["max_temp"].as<String>();
          NAMain.temperature = station["dashboard_data"]["Temperature"].as<String>();
          NAMain.trend = station["dashboard_data"]["temp_trend"].as<String>();
          NAMain.timemin = delay_timezone + station["dashboard_data"]["date_min_temp"].as<unsigned long>();
          NAMain.timemax = delay_timezone + station["dashboard_data"]["date_max_temp"].as<unsigned long>();
          NAMain.timeupdate = delay_timezone + station["dashboard_data"]["time_utc"].as<unsigned long>();
          NAMain.humidity = station["dashboard_data"]["Humidity"].as<String>();
          JsonArray modules = station["modules"].as<JsonArray>();
          int module4counter = 0;
          for (JsonObject module : modules)
          {
            if (module["type"].as<String>() == "NAModule1")
            {
              NAModule1.name = module["module_name"].as<String>();
              NAModule1.battery_percent = module["battery_percent"].as<int>();
              NAModule1.min = module["dashboard_data"]["min_temp"].as<String>();
              NAModule1.max = module["dashboard_data"]["max_temp"].as<String>();
              NAModule1.temperature = module["dashboard_data"]["Temperature"].as<String>();
              NAModule1.trend = module["dashboard_data"]["temp_trend"].as<String>();
              NAModule1.timemin = delay_timezone + module["dashboard_data"]["date_min_temp"].as<unsigned long>();
              NAModule1.timemax = delay_timezone + module["dashboard_data"]["date_max_temp"].as<unsigned long>();
              NAModule1.timeupdate = delay_timezone + station["dashboard_data"]["time_utc"].as<unsigned long>();
              NAModule1.humidity = module["dashboard_data"]["Humidity"].as<String>();
              NAModule1.reachable = module["reachable"].as<String>();
            }
            if (module["type"].as<String>() == "NAModule4")
            {
              NAModule4[module4counter].name = module["module_name"].as<String>();
              NAModule4[module4counter].battery_percent = module["battery_percent"].as<int>();
              NAModule4[module4counter].min = module["dashboard_data"]["min_temp"].as<String>();
              NAModule4[module4counter].max = module["dashboard_data"]["max_temp"].as<String>();
              NAModule4[module4counter].temperature = module["dashboard_data"]["Temperature"].as<String>();
              NAModule4[module4counter].trend = module["dashboard_data"]["temp_trend"].as<String>();
              NAModule4[module4counter].timemin = delay_timezone + module["dashboard_data"]["date_min_temp"].as<unsigned long>();
              NAModule4[module4counter].timemax = delay_timezone + module["dashboard_data"]["date_max_temp"].as<unsigned long>();
              NAModule4[module4counter].timeupdate = delay_timezone + station["dashboard_data"]["time_utc"].as<unsigned long>();
              NAModule4[module4counter].humidity = module["dashboard_data"]["Humidity"].as<String>();
              NAModule4[module4counter].co2 = module["dashboard_data"]["temp_trend"].as<String>();
              NAModule4[module4counter].reachable = module["reachable"].as<String>();
              module4counter++;
            }
            if (module["type"].as<String>() == "NAModule3")
            {
              NAModule3.name = module["module_name"].as<String>();
              NAModule3.rain = module["dashboard_data"]["Rain"].as<String>();
              NAModule3.sum_rain_1h = module["dashboard_data"]["sum_rain_1"].as<String>();
              NAModule3.sum_rain_24h = module["dashboard_data"]["sum_rain_24"].as<String>();
              NAModule3.battery_percent = module["battery_percent"].as<int>();
              NAModule3.reachable = module["reachable"].as<String>();
            }
          }
        }

        if (_debug)
        {
          dumpModule(NAMain);
          dumpModule(NAModule1);
          dumpModule(NAModule4[0]);
          dumpModule(NAModule4[1]);
          dumpModule(NAModule4[2]);
          dumpModule(NAModule3);
        }
        result = VALID_ACCESS_TOKEN;
      }
    }
    http.end();
    if (_debug)
    {
      Serial.print("getStationsData Result : ");
      Serial.println(result);
    }
  }
  return result;
}

void NetatmoWeatherAPI::dumpModule(module_struct module)
{
  Serial.println("Name :" + module.name);
  Serial.println("Temperature :" + module.temperature);
  Serial.println("Min :" + module.min);
  Serial.println("Max :" + module.max);
  Serial.println("Trend :" + module.trend);
  Serial.print("Battery :");
  Serial.println(module.battery_percent);
  Serial.println("CO2 :" + module.co2);
  Serial.println("Humidity :" + module.humidity);
  Serial.print("Time Min :");
  Serial.println(module.timemin);
  Serial.print("Time Max :");
  Serial.println(module.timemax);
  Serial.print("Time Update :");
  Serial.println(module.timeupdate);

  Serial.print("Rain :" + module.rain);
  Serial.print(" Sum Rain 1h :" + module.sum_rain_1h);
  Serial.print(" Sum Rain 24h :" + module.sum_rain_24h);
  Serial.println(" Reachable :" + module.reachable);
}