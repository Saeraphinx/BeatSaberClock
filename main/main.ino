#include <ArduinoJson.h>

#include <LedControl.h>

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

#include <ESP8266HTTPClient.h>

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "Password123"
#define ScoreSaber 0
#define BeatLeader 1

//       LedControl(DATA,CLOCK,CS,NUM);
LedControl la = LedControl(D0, D2, D1, 1);
LedControl lb = LedControl(D5, D3, D4, 1);
LedControl lc = LedControl(D8, D6, D7, 1);


//
// API
//
ESP8266WebServer server(80);
const String ScoreSaberPlayerAPI = "https://scoresaber.com/api/player/_ID_/full";
const String BeatLeaderPlayerAPI = "https://api.beatleader.xyz/player/_ID_";const String TimeAPIAPI = "https://timeapi.io/api/Time/current/ip?ipAddress=_IP_";
DynamicJsonDocument SSAPI(2048);
DynamicJsonDocument BLAPI(4096);
DynamicJsonDocument TIME(512);

//
// TIME
//
String ExternalIP = "";
byte hour = 0;
byte minute = 0;
byte seconds = 120;
String DisplayTime = "----";

//
// WEBSITE VARIABLES
//
int displayBrightness = 8;
String playerID = "76561198323656813";
bool debug = true;
int display1ID = -1;
int display2ID = -1;
int display3ID = -1;


#pragma region Gloab Functions or smthn idk
void showIP(String IP);
void showString(LedControl display, String arr);
void setupLED();
void setupServer();
void sendHtml();
void spin(unsigned long frame, LedControl display);
int sendRequest(String URL, int apijson);
int getTime();
void showStringRight(LedControl display, String arr);
void setBrightness(int b);
String GetExternalIP();
void updateTime(bool newdata);
#pragma endregion

void setup()
{
  Serial.begin(115200);
  setupLED();
  delay(500);
  /* #region Setup WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.hostname("BeatSaberClock");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);
  // Wait for connection
  unsigned long frames = 0;
  while (WiFi.status() != WL_CONNECTED) {
    spin(frames, la);
    delay(100);
    frames++;
  }
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  showIP(WiFi.localIP().toString());
  ExternalIP = GetExternalIP();
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  /* set up the server */
  server.on("/", sendHtml);
  setupServer();
  // Start server and listen for connections
  server.begin();
  Serial.println("HTTP server started");
}





const unsigned long APIRefreshRate = 15 * 60 * 1000;
const unsigned long SwapRefreshRate = 10 * 1000;
unsigned long APIRefreshTime = millis() + (5 * 1000);
unsigned long TIMErefreshTime = millis() + (5 * 1000);
bool NewTimeData = false;
void loop()
{
  unsigned long currTime = millis();
  server.handleClient();

  if (currTime > APIRefreshTime) {
    // API
    if(WiFi.status() == WL_CONNECTED) {
      delay(1000);
      int ssrescode = sendRequest(ScoreSaberPlayerAPI, ScoreSaber);
      delay(250);
      int blrescode = sendRequest(BeatLeaderPlayerAPI, BeatLeader);
      int timerescode = getTime();
      Serial.println("ScoreSaber rescode: " + (String)ssrescode);
      Serial.println("ScoreSaber name: " + (String)SSAPI["name"]);
      Serial.println("BeatLeader rescode: " + (String)blrescode);
      Serial.println("BeatLeader name: " + (String)BLAPI["name"]);
      Serial.println("Time rescode: " + (String)timerescode);
      Serial.println("ScoreSaber name: " + (String)TIME["dateTime"]);
      setCurrCodeTime();
    }
    APIRefreshTime = millis() + APIRefreshRate;
  }

  // Time
  if (currTime > TIMErefreshTime && !(seconds > 70)) {
    if (!NewTimeData) {  
      minute++;
      if (minute >= 60) {
        hour++;
        minute = 0;
      }
      if (hour >= 24) {
        hour = 0;
      }
    }

    String _minute;
    if (minute < 10) {
      _minute = "0" + (String)minute;
    } else {
      _minute = (String)minute;
    }

    DisplayTime = (String)hour + _minute;  

    if(NewTimeData) {
      byte _rTime = abs(seconds - 60);
      TIMErefreshTime = millis() + _rTime;
    } else {
      TIMErefreshTime = millis() + (60*1000);
    }
  }

  Serial.println("currTime:    " + (String)currTime);
  Serial.println("APIRefreshTime: " + (String)APIRefreshTime);
  Serial.println("WifiConnect: " + (String)(WiFi.status() == WL_CONNECTED));
  Serial.println();
  delay(200);
}
#pragma region Time
void setCurrCodeTime() {
 hour = TIME["hour"];
 minute = TIME["minute"];
 seconds = TIME["seconds"];
 NewTimeData = true;
}

#pragma endregion

#pragma region Load
void updateDisplays() {
  setDisplayData(la, display1ID);
  setDisplayData(lb, display2ID);
  setDisplayData(lc, display3ID);
}
/* Data Key
1 - PP
2 - Formatted PP



*/
void setDisplayData(LedControl display, int selData){
  String temp;
  switch (selData)
{
  case 1:
    showStringRight(display, SSAPI["pp"]);
    break;
  case 2:
    temp = (String)SSAPI["pp"];
    if (temp.length() > 4) {
      temp = temp.substring(0,5);
    }
    showStringRight(display, temp);
    //                  .ABCDEFG
    display.setRow(0,7,B11100111);
    display.setRow(0,6,B11100111);
    break;
  case 5:
    showStringRight(display, SSAPI["scoreStats"]["totalPlayCount"]);
    break;
  case 6:
    showStringRight(display, SSAPI["scoreStats"]["rankedPlayCount"]);
    break;
  case 7:
    showStringRight(display, SSAPI["rank"]);
    break;
  case 8:
    showStringRight(display, SSAPI["countryRank"]);
    break;

  case 101:
    showStringRight(display, BLAPI["pp"]);
    break;
  case 102:
    temp = (String)SSAPI["pp"];
    if (temp.length() > 4) {
      temp = temp.substring(0,5);
    }
    showStringRight(display, temp);
    //                  .ABCDEFG
    display.setRow(0,7,B11100111);
    display.setRow(0,6,B11100111);
    break;
  case 105:
    showStringRight(display, BLAPI["scoreStats"]["totalPlayCount"]);
    break;
  case 106:
    showStringRight(display, BLAPI["scoreStats"]["rankedPlayCount"]);
    break;
  case 107:
    showStringRight(display, BLAPI["rank"]);
    break;
  case 108:
    showStringRight(display, BLAPI["countryRank"]);
    break;
    
  default:
  display.clearDisplay(0);
    break;
  }
}
#pragma endregion

#pragma region Website
void setupServer()
{
  server.on(UriBraces("/update/brightness/{}"), []() {
    displayBrightness = server.pathArg(0).toInt();
    setBrightness(displayBrightness);
    server.send(302, "text/pain", "/");
  });

  server.on(UriBraces("/update/refresh"), []() {
    APIRefreshTime = 0;
    server.send(302, "text/pain", "/");
  });

  server.on(UriBraces("/update/playerid/{}"), []() {
    playerID = server.pathArg(0);
    server.send(302, "text/pain", "/");
  });
}

void sendHtml()
{
  String response
      = R"(<!DOCTYPE html><html><head> <title>ESP-8266 - 60 LED Ring</title> <meta name="viewport" content="width=device-width, initial-scale=1"> <link href="http://fonts.googleapis.com/css?family=Lato:400,700" rel="stylesheet" type="text/css"> <link href="style.css" rel="stylesheet" type="text/css"> <link rel="icon" href="https://saeraphinx.dev/images/favicon.ico" type="image/x-icon"/></head><body> <style>body{background-color: black; font-family: 'Lato', Tahoma, Geneva, Verdana, sans-serif;}div.topbar{background-color: #252525; width: 100%; border-radius: 10px; margin: 0; margin-bottom: 15px;}p.topbar{display: inline-block; color: white; font-size: 18px; padding-top: 10px; padding-bottom: 10px; padding-right: 15px; padding-left: 15px; margin: 0;}p.topbar.right{float: right; text-align: right;}rainbow{background-image: linear-gradient(to left, violet, rgb(137, 18, 223), rgb(0, 162, 255), green, yellow, orange, red); -webkit-background-clip: text; -webkit-text-fill-color: transparent;}.panels{display: flex; flex-direction: row; flex-wrap: wrap; justify-content: space-around; align-items: center; margin: 0; margin-top: 15px; margin-bottom: 15px;}.panel{background-image: linear-gradient(#01a0a0, rgb(0, 0, 0)); border-radius: 10px; width: 250px; height: 250px; text-align: center; align-items: center; margin-top: 0px; margin-bottom: 15px;}p.bPanelText{color: white; font-size: 24px; text-align: center; padding-top: 5px; margin-top: 5px;}p.tPanelText{color: white; font-size: 16px; text-align: center; padding-top: 0px; margin-top: 5px;}.panel.ssStatus{background-image: linear-gradient(#ffee00cb, rgb(0, 0, 0));}.panel.blStatus{background-image: linear-gradient(#c206a9, rgb(0, 0, 0));}button{margin-top: 8px;}</style> <script>function brightness(){var brightness=document.getElementById("in_bright").value; if(brightness > 16){brightness=16;}else if(brightness < 0){brightness=0;}location.href='update/brightness/' + brightness;}function refresh(){location.href='update/refresh';}function id(){var brightness=document.getElementById("in_id").value; location.href='update/playerid/' + id;}</script> <div class="topbar"> <p class="topbar">ESP-8266 Web Interface - <rainbow>BeatSaberClock</rainbow> </p><p class="topbar right">Local IP: _LOCALIP_</p></div><div class="panels"> <div class="panel ssStatus"> <p class="bPanelText">ScoreSaber Status</p><img src="_SSAPIpfp_", width="36px" height="36px" style="border-radius: 14px;"> <p class="tPanelText">Name: _SSAPIname_<br>Rank: _SSAPIrank_<br>PP: _SSAPIpp_</p></div><div class="panel blStatus"> <p class="bPanelText">BeatLeader Status</p><img src="_BLAPIpfp_", width="36px" height="36px" style="border-radius: 14px;"> <p class="tPanelText">Name: _BLAPIname_<br>Rank: _BLAPIrank_<br>PP: _BLAPIpp_</p></div><div class="panel brightness"> <p class="bPanelText">Brightness</p><p class="tPanelText">_BRIGHTNESS_</p><input class="brightness" id="in_bright"><br><button onclick='brightness()'>Update</button> </div><div class="panel brightness"> <p class="bPanelText">Refresh</p><p class="tPanelText">Next refresh in:<br>_REFRESH_</p><button onclick='refresh()'>Refresh</button> </div><div class="panel brightness"> <p class="bPanelText">Player ID</p><p class="tPanelText">Current ID:<br>_PLAYERID_</p><input class="brightness" id="in_id"><br><button onclick='id()'>Update</button> </div></div></body>
  )";
  response.replace("_LOCALIP_", WiFi.localIP().toString());
  response.replace("_SSAPIname_", (String)SSAPI["name"]);
  response.replace("_SSAPIrank_", (String)SSAPI["rank"]);
  response.replace("_SSAPIpp_", (String)SSAPI["pp"]);
  response.replace("_SSAPIpfp_", (String)SSAPI["profilePicture"]);
  response.replace("_BLAPIname_", (String)BLAPI["name"]);
  response.replace("_BLAPIrank_", (String)BLAPI["rank"]);
  response.replace("_BLAPIpp_", (String)BLAPI["pp"]);
  response.replace("_BLAPIpfp_", (String)BLAPI["avatar"]);
  response.replace("_BRIGHTNESS_", (String)displayBrightness);
  response.replace("_PLAYERID_", (String)playerID);

  unsigned long tempTimeMils = APIRefreshTime - millis();
  int tempTimeSec = tempTimeMils / 1000;
  int tempTimeMins = tempTimeSec / 60;
  tempTimeSec = tempTimeSec % 60;
  response.replace("_REFRESH_", (String)tempTimeMins+":"+(String)tempTimeSec);

  server.send(200, "text/html", response);
}
#pragma endregion

#pragma region Animation
void spin(unsigned long frame, LedControl display)
{
  int f = frame % 20;
  int f_p = f - 17;
  int f_inv = abs(f_p);
  display.clearDisplay(0);
  switch (f) {
  case 0:
    Serial.print(".");
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
    //                    .ABCDEFG
    display.setRow(0, f, B01000000); 
    break;
  case 8:
    display.setRow(0, 7, B00000010); 
    break;
  case 9:
    display.setRow(0, 7, B00000100); 
    break;
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
    //                        .ABCDEFG
    display.setRow(0, f_inv, B00001000);
    break;
  case 18:
    display.setRow(0, 0, B00010000);
    break;
  case 19:
    display.setRow(0, 0, B00100000);
    break;
  default:
    break;
  }
}
//           .ABCDEFG
#define L_S B01011011
#define L_U B00111110
#define L_C B01001110
void showIP(String IP)
{
  la.clearDisplay(0);
  lb.clearDisplay(0);
  la.setRow(0, 7, L_S);
  la.setRow(0, 6, L_U);
  la.setRow(0, 5, L_C);
  la.setRow(0, 4, L_C);
  la.setChar(0, 3, 'E', false);
  la.setRow(0, 2, L_S);
  la.setRow(0, 1, L_S);

  showStringRight(lb, IP);
}
#pragma endregion

#pragma region LedControl Stuffs
void showString(LedControl display, String arr)
{
  Serial.println(arr);
  int currDigit = 7;
  bool nextdisdp = false; 
  for (int i = 0; i < arr.length(); i++) {
    char t = arr.charAt(i);
    if (t == '.') {
      continue;
    }

    if (currDigit >= 8) {
      return;
    }

    display.setChar(0, currDigit, t, nextdisdp);
    nextdisdp = false;
    if (!(i + 1 >= arr.length())) {
      if (arr.charAt(i + 1) == '.') {
        nextdisdp = true;
      }
    }
    currDigit--;
  }
}

void showStringRight(LedControl display, String arr)
{
  Serial.println(arr);
  int currDigit = 0;
  bool dp = false;
  for (int i = arr.length() - 1; i >= 0; i--) {
    char t = arr.charAt(i);
    if (t == '.') {
      continue;
    }

    
    if (currDigit >= 8 || currDigit <= -1) {
      return;
    }

    display.setChar(0, currDigit, t, dp);
    dp = false;
    if (!(i - 1 <= -1)) {
      if (arr.charAt(i - 1) == '.') {
        dp = true;
      }
    }
    currDigit++;
  }
}

void setupLED()
{
  // 0-16 brightness
  int b = 3;
  la.shutdown(0, false);
  lb.shutdown(0, false);
  lc.shutdown(0, false);
  la.setIntensity(0, b);
  lb.setIntensity(0, b);
  lc.setIntensity(0, b);
  la.clearDisplay(0);
  lb.clearDisplay(0);
  lc.clearDisplay(0);
}

void setBrightness(int b){
  la.setIntensity(0, b);
  lb.setIntensity(0, b);
  lc.setIntensity(0, b);
}
#pragma endregion

#pragma region API
// https://arduinojson.org/v6/assistant/#


// assure you have wifi before calling this
int sendRequest(String URL, int apijson)
{
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  client.setTimeout(10000);

  String p = URL;
  p.replace("_ID_", playerID);

  http.begin(client, p);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    delay(1000);
    http.end();
    Serial.println(payload);
    DeserializationError error;
    switch (apijson)
    {
      case ScoreSaber:
        error = deserializeJson(SSAPI, payload);
        break;
      case BeatLeader:
        error = deserializeJson(BLAPI, payload);
        break;
      default:
        error = DeserializationError::InvalidInput;
        break;
    }
    Serial.print("deserializeJson() returned ");
    Serial.println(error.c_str());
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();
  }
  return httpResponseCode;
}

int getTime() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  client.setTimeout(10000);

  String p = TimeAPIAPI;
  p.replace("_IP_", ExternalIP);

  http.begin(client, p);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    delay(1000);
    http.end();
    Serial.println(payload);
    DeserializationError error;
    error = deserializeJson(TIME, payload);
    Serial.print("deserializeJson() returned ");
    Serial.println(error.c_str());
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();
  }
  return httpResponseCode;
}

String GetExternalIP()
{
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  client.setTimeout(10000);
  http.begin(client, "api.ipify.org");
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    delay(1000);
    http.end();
    Serial.println(payload);
    return payload;
  }
  http.end();
  return "";
}
#pragma endregion