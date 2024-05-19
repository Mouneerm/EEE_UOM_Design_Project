// Design project - main microcontroller
// Code written by Mouneer Mahomed, Jhevish Ramphul and Hasan Soorefan

// Set up a new SoftwareSerial object
#include <SoftwareSerial.h>
const byte rxPin = 2;
const byte txPin = 3;

SoftwareSerial mySerial(rxPin, txPin);

#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
/////////real time clock///////////

#include "RTClib.h"
RTC_DS1307 rtc;
File myFile;

// ACS current pk pk measurement
#define SAMPLES 300
int ct1 = A1;  // current transformer
int ct0 = A0;
int v_pin = A2;
int sw = 12;
float High_peak, Low_peak;
float High_peak1, Low_peak1;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
int sensor_value = 0;
int sensor = 0;
int i = 0;
int x = 0;

unsigned long previousMillis = 0;

// Variables to measure or calculate current

float Amps_Peak_Peak, I;
float volts_Peak_Peak;
float volts_rms;
// voltage variable declaration

const int number_of_sensor = 2;
// power
float p;
float current_rms[number_of_sensor];
float power[number_of_sensor];  // p and energy were initially double
float energy[number_of_sensor];

int cntt;

String date_rtc;
String time_rtc;

long pMillis = 0;

byte rateSpot = 0;

int data_length;  // sending with AT commands
String sensorsdata;
String sensorsdata_encoded;
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600);
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    pinMode(sw, INPUT);

    if (!rtc.begin()) {
        // Serial.println("Couldn't find RTC");
    }

    lcd.begin();
    lcd.clear();
    lcd.setCursor(1, 0);
}

void loop() {
    if (digitalRead(sw) == 1)
        sensor = 1;
    else
        sensor = 0;

    DateTime now = rtc.now();  // rtc get time
    date_rtc = String(now.day()) + '/' + String(now.month()) + '/' +
               String(now.year());
    time_rtc = String(now.hour()) + ':' + String(now.minute()) + ':' +
               String(now.second());

    volts_rms = read_volt(v_pin);

    current_rms[0] = read_Amps(ct0);
    current_rms[1] = read_Amps(ct1);
    power[0] = volts_rms * current_rms[0];
    power[1] = volts_rms * current_rms[1];
    energy[0] = power[0] + energy[0];
    energy[1] = power[1] + energy[1];

    wrtie_to_sd();
    preprocess();
    to_display(sensor);

    delay(2100);
    sensorsdata = "";
}

void wrtie_to_sd(void) {
    myFile = SD.open(((date_rtc + " Sensor 1"), FILE_WRITE));
    if (myFile) {
        myFile.println(time_rtc + ", " + date_rtc + "," + (String)volts_rms +
                       "V" + "," + (String)current_rms[0] + "A" + "," +
                       (String)power[0] + "W" + "," + (String)energy[0] + "Ws");
        myFile.close();
        Serial.println("done :D");
    } else {
        Serial.println("error opening test.txt");
    }
    myFile = SD.open(((date_rtc + " Sensor 2"), FILE_WRITE));
    if (myFile) {
        myFile.println(time_rtc + ", " + date_rtc + "," + (String)volts_rms +
                       "V" + "," + (String)current_rms[1] + "A" + "," +
                       (String)power[1] + "W" + "," + (String)energy[1] + "Ws");
        myFile.close();
        Serial.println("done :D");
    } else {
        Serial.println("error opening test.txt");
    }
}

void send_data(String sensorvalue, int valuelength) {
    String mymessage;
    mymessage = mymessage + "Send by zigbee" + "," + valuelength + "," +
                sensorvalue + "\r";
    mySerial.println(mymessage);
    mymessage = mymessage + "Send by esp8266 " + "," + valuelength + "," +
                sensorvalue + "\r";
    Serial.println(mymessage);
}

float read_Amps(int ct)  // read_Amps function calculate the difference between
                         // the high peak and low peak
{                   // get peak to peak value
    int cnt;        // Counter
    High_peak = 0;  // We first assume that our high peak is equal to 0 and low
                    // peak is 1024, yes inverted
    Low_peak = 1024;

    for (cnt = 0; cnt < SAMPLES; cnt++)  // everytime a sample (module value) is
                                         // taken it will go through test
    {
        sensor_value = analogRead(ct);  // We read a single value from the
                                        // module

        if (sensor_value > High_peak)  // If that value is higher than the high
                                       // peak (at first is 0)
        {
            High_peak = sensor_value;  // The high peak will change from 0 to
                                       // that value found
        }

        if (sensor_value < Low_peak)  // If that value is lower than the low
                                      // peak (at first is 1024)
        {
            Low_peak = sensor_value;  // The low peak will change from 1024 to
                                      // that value found
        }
    }  // We keep looping until we take all samples and at the end we will have
       // the high/low peaks values

    Amps_Peak_Peak = High_peak - Low_peak;  // Calculate the difference
    return Amps_Peak_Peak * 0.7 * 0.0125;   // convert to rms before
}
float read_volt(int v_pin) {  // get peak to peak value
    int cnt;                  // Counter
    float Value;
    High_peak1 = 0;  // We first assume that our high peak is equal to 0 and low
                     // peak is 1024, yes inverted
    Low_peak1 = 1024;

    for (cntt = 0; cntt < SAMPLES; cntt++)  // everytime a sample (module value)
                                            // is taken it will go through test
    {
        Value = analogRead(v_pin);  // We read a single value from the module

        if (Value > High_peak1)  // If that value is higher than the high peak
                                 // (at first is 0)
        {
            High_peak1 =
                Value;  // The high peak will change from 0 to that value found
        }

        if (Value < Low_peak1)  // If that value is lower than the low peak (at
                                // first is 1024)
        {
            Low_peak1 = Value;  // The low peak will change from 1024 to that
                                // value found
        }
    }  // We keep looping until we take all samples and at the end we will have
       // the high/low peaks values

    volts_Peak_Peak = High_peak1 - Low_peak1;  // Calculate the difference
    volts_rms = volts_Peak_Peak / 1.414;
    return volts_rms;
}

void to_display(int sensor_to_display) {
    lcd.setCursor(0, 0);
    lcd.print("I=");
    lcd.setCursor(2, 0);
    lcd.print(String(current_rms[sensor_to_display]));

    lcd.setCursor(8, 0);
    lcd.print("V=");
    lcd.setCursor(10, 0);
    lcd.print(String(volts_rms));

    lcd.setCursor(0, 1);
    lcd.print("P=");
    lcd.setCursor(2, 1);
    lcd.print(String(power[sensor_to_display]));

    lcd.setCursor(8, 1);
    lcd.print("E=");
    lcd.setCursor(10, 1);
    lcd.print(String(energy[sensor_to_display]));
}

void preprocess(void) {
    sensorsdata_encoded = "";
    sensorsdata = "";

    sensorsdata = "Sensor 1 :  %" + (String(volts_rms)) + "%" +
                  (String(current_rms[0])) + "%" + (String(power[0])) + "%" +
                  (String(energy[0]));
    sensorsdata_encoded = encode("Sensor 1 :  %") + encode(String(volts_rms)) +
                          "%" + encode(String(current_rms[0])) + "%" +
                          encode(String(power[0])) + "%" +
                          encode(String(energy[0]));

    data_length = sensorsdata.length();
    send_data(sensorsdata, data_length);
    data_length = sensorsdata_encoded.length();
    send_data(sensorsdata_encoded, data_length);

    sensorsdata_encoded = "";
    sensorsdata = "";

    sensorsdata = "Sensor 2 :  %" + (String(volts_rms)) + "%" +
                  (String(current_rms[1])) + "%" + (String(power[1])) + "%" +
                  (String(energy[1]));
    sensorsdata_encoded = encode("Sensor 2 :  %") + encode(String(volts_rms)) +
                          "%" + encode(String(current_rms[1])) + "%" +
                          encode(String(power[1])) + "%" +
                          encode(String(energy[1]));

    data_length = sensorsdata.length();
    send_data(sensorsdata, data_length);
    data_length = sensorsdata_encoded.length();
    send_data(sensorsdata_encoded, data_length);
}

String encode(String to_encode) {
    char en_array[10] = {'g', 't', 'u', 'h', 's', 'n', 'p', 'd', 'j', 'a'};

    int to_encode_length = to_encode.length();
    for (int i = 0; i < to_encode_length; i++) {
        if (to_encode[i] == '.') {
            to_encode[i] = 'm';
        } else {
            to_encode[i] = en_array[int(to_encode[i]) - 48];
        }
    }
    return to_encode;
}
