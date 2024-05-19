// Design project - receiver microcontroller
// Code written by Mouneer Mahomed, Jhevish Ramphul and Hasan Soorefan

#include <ESP8266SSDP.h>
#include <SimpleTimer.h>
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
ESP8266WebServer server(80);
SimpleTimer timer;

String myString;
String garbage;
char data;
int q1, r1, s1, t1;

char username[] = "812e4f50-29a2-11ed-bf0a-bb4ba43bd3f6";
char password[] = "1f508cd396726f5d7b8f34e8c834c1f4c30420fd";
char clientID[] = "b2b98420-2ad0-11ed-baf6-35fab7fd0ac8";
// char auth[] = "o4g2hM_HGE7qQsiOl3yf2Hp-0ambUpRh";
char auth[] = "9u_tHRWEPAvRrOxaz9nhmRrux0gus8jd";
//#define BLYNK_TEMPLATE_ID "TMPL3-NnuEcl"
//#define BLYNK_DEVICE_NAME "Jhev"
//#define BLYNK_AUTH_TOKEN "9u_tHRWEPAvRrOxaz9nhmRrux0gus8jd"
bool wifi_connected = 0;

struct settings {
    char ssid[30];
    char password[30];
} user_wifi = {};

void setup() {
    Serial.begin(115200);
    Serial.print("AT\r\n");

    EEPROM.begin(sizeof(struct settings));
    EEPROM.get(0, user_wifi);

    WiFi.mode(WIFI_STA);
    WiFi.begin(user_wifi.ssid, user_wifi.password);

    byte tries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        if (tries++ > 10) {
            WiFi.mode(WIFI_AP);
            WiFi.softAP("Setup Portal", "12345678");
            break;
        }
    }
    server.on("/", handlePortal);
    server.begin();
}

void loop() {
    if (WiFi.status() == WL_CONNECTED && wifi_connected == 0) {
        Blynk.begin(auth, user_wifi.ssid, user_wifi.password, "blynk.cloud",
                    80);
        Cayenne.begin(username, password, clientID, user_wifi.ssid,
                      user_wifi.password);
        wifi_connected = 1;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Receiveddata();
        // Serial.println(user_wifi.ssid);
        Blynk.run();
        Cayenne.loop();
        timer.run();
    }
    server.handleClient();
}

void handlePortal() {
    if (server.method() == HTTP_POST) {
        strncpy(user_wifi.ssid, server.arg("ssid").c_str(),
                sizeof(user_wifi.ssid));
        strncpy(user_wifi.password, server.arg("password").c_str(),
                sizeof(user_wifi.password));
        user_wifi.ssid[server.arg("ssid").length()] =
            user_wifi.password[server.arg("password").length()] = '\0';
        EEPROM.put(0, user_wifi);
        EEPROM.commit();

        server.send(
            200, "text/html",
            "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta "
            "name='viewport' content='width=device-width, "
            "initial-scale=1'><title>Wifi "
            "Setup</"
            "title><style>*,::after,::before{box-sizing:border-box;}body{"
            "margin:0;font-family:'Segoe UI',Roboto,'Helvetica "
            "Neue',Arial,'Noto Sans','Liberation "
            "Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#"
            "212529;background-color:#f5f5f5;}.form-control{display:block;"
            "width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid "
            "#ced4da;}button{border:1px solid "
            "transparent;color:#fff;background-color:#007bff;border-color:#"
            "007bff;padding:.5rem "
            "1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:"
            "100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:"
            "auto;}h1,p{text-align: center}</style> </head> <body><main "
            "class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings "
            "have been saved successfully!<br />Please restart the "
            "device.</p></main></body></html>");
    } else {
        server.send(
            200, "text/html",
            "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta "
            "name='viewport' content='width=device-width, "
            "initial-scale=1'><title>Wifi Setup</title> "
            "<style>*,::after,::before{box-sizing:border-box;}body{margin:0;"
            "font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto "
            "Sans','Liberation "
            "Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#"
            "212529;background-color:#f5f5f5;}.form-control{display:block;"
            "width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid "
            "#ced4da;}button{cursor: pointer;border:1px solid "
            "transparent;color:#fff;background-color:#007bff;border-color:#"
            "007bff;padding:.5rem "
            "1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:"
            "100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:"
            "auto;}h1{text-align: center}</style> </head> <body><main "
            "class='form-signin'> <form action='/' method='post'> <h1 "
            "class=''>Wifi Setup</h1><br/><div "
            "class='form-floating'><label>SSID</label><input type='text' "
            "class='form-control' name='ssid'> </div><div "
            "class='form-floating'><br/><label>Password</label><input "
            "type='password' class='form-control' "
            "name='password'></div><br/><br/><button "
            "type='submit'>Save</button><p style='text-align: right'><a "
            "href='https://www.mrdiy.ca' style='color: "
            "#32C5FF'>mrdiy.ca</a></p></form></main> </body></html>");
    }
}

void Receiveddata() {
    if (Serial.available() > 0) {
        garbage = Serial.readString();  // consists of the +ERR=2 ERROR.
        myString = Serial.readString();
        Serial.println(myString);

        String l = getValue(myString, ',', 0);  // address
        String m = getValue(myString, ',', 1);  // data length
        String n = getValue(myString, ',', 2);  // data
        String o = getValue(myString, ',', 3);  // RSSI
        String p = getValue(myString, ',', 4);  // SNR

        String q = getValue(n, '%', 0);  // sensor1
        Serial.print("Voltage:");
        q = deencode(q);
        Serial.println(q);
        q = deencode(q);
        float q1 = q.toFloat();
        Blynk.virtualWrite(V50, q1);
        Cayenne.virtualWrite(V51, q1);

        String r = getValue(n, '%', 1);  // sensor2
        Serial.print("Current:");
        r = deencode(r);
        Serial.println(r);
        float r1 = r.toFloat();
        Blynk.virtualWrite(V52, r1);
        Cayenne.virtualWrite(V53, r1);

        String s = getValue(n, '%', 2);  // sensor3
        Serial.print("Power:");
        s = deencode(s);
        Serial.println(s);
        float s1 = s.toFloat();
        Blynk.virtualWrite(V54, s1);
        Cayenne.virtualWrite(V55, s1);

        String t = getValue(n, '%', 3);  // sensor4
        Serial.print("Energy:");
        t = deencode(t);
        Serial.println(deencode(t));
        float t1 = t.toFloat();
        Blynk.virtualWrite(V56, t1);
        Cayenne.virtualWrite(V57, t1);

        myString = "";
    }
}

String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";

    // 5 datas
}

String deencode(String to_deencode) {
    char en_array[10] = {'g', 't', 'u', 'h', 's', 'n', 'p', 'd', 'j', 'a'};

    int to_deencode_length = to_deencode.length();
    for (int i = 0; i < to_deencode_length; i++) {
        if (to_deencode[i] == 'm') {
            to_deencode[i] = '.';
        }
        for (int j = 0; j < 10; j++) {
            if (to_deencode[i] == en_array[j]) {
                // println(char(j));
                switch (j) {
                    case 0:
                        to_deencode[i] = '0';
                        break;
                    case 1:
                        to_deencode[i] = '1';
                        break;
                    case 2:
                        to_deencode[i] = '2';
                        break;
                    case 3:
                        to_deencode[i] = '3';
                        break;
                    case 4:
                        to_deencode[i] = '4';
                        break;
                    case 5:
                        to_deencode[i] = '5';
                        break;
                    case 6:
                        to_deencode[i] = '6';
                        break;
                    case 7:
                        to_deencode[i] = '7';
                        break;
                    case 8:
                        to_deencode[i] = '8';
                        break;
                    case 9:
                        to_deencode[i] = '9';
                        break;
                }
            }
        }
    }
    return to_deencode;
}
