#include <Arduino.h>
#include <Ticker.h>
#include <DHT.h>

#define BAUD_RATE 9600

// MQ5 LPG Gas Sensor
#define MQ5Sensor_A0_pin 36

// DHT11 Temperature and Humidity Sensor
#define DHTPIN 21
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
void calculateTemperatureHumidity()
{
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C | Heat index: "));
    Serial.print(hic);
    Serial.println(F("°C "));
}

// HC-SR04 Ultrasonic Sensor
#define SOUND_SPEED 0.034
#define ultrasonicSensor_trigger_pin 23
#define ultrasonicSensor_echo_pin 22
long ultrasonicSensor_duration;
int ultrasonicSensor_distance;
void calculateDistance()
{
    digitalWrite(ultrasonicSensor_trigger_pin, LOW); // set trigger signal low for 2us
    delayMicroseconds(2);
    digitalWrite(ultrasonicSensor_trigger_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultrasonicSensor_trigger_pin, LOW);

    ultrasonicSensor_duration = pulseIn(ultrasonicSensor_echo_pin, HIGH);
    ultrasonicSensor_distance = ultrasonicSensor_duration * SOUND_SPEED / 2;

    if (ultrasonicSensor_distance > 10)
        digitalWrite(LED_BUILTIN, HIGH);
    else
        digitalWrite(LED_BUILTIN, LOW);

    Serial.print("Distance: ");
    Serial.print(ultrasonicSensor_distance);
    Serial.println(" cm");
}

// PIR Motion Sensor
#define pirSensor_pin 19
#define numberMotionChecks 20
int sumMovement = 0, cntMovement = 0;
bool motionDetect()
{
    return digitalRead(pirSensor_pin);
}

Ticker timer;
void timerHandler()
{
    sumMovement += motionDetect();
    if (cntMovement > numberMotionChecks)
    {
        if (sumMovement >= 0.7 * numberMotionChecks)
        {
            Serial.println("Dozd!");
        }
        Serial.println(sumMovement);
        sumMovement = 0;
        cntMovement = 0;
    }
    cntMovement++;
    // calculateDistance();
    // calculateTemperatureHumidity();
}

byte MQ6_Pin = A0;          /* Define A0 for MQ Sensor Pin */
float Referance_V = 3300.0; /* ESP32 Referance Voltage in mV */
float RL = 1.0;             /* In Module RL value is 1k Ohm */
float Ro = 10.0;            /* The Ro value is 10k Ohm */
float mVolt = 0.0;
const float Ro_clean_air_factor = 10.0;
float Calculate_Rs(float Vo)
{
    /*
     *  Calculate the Rs value
     *  The equation Rs = (Vc - Vo)*(RL/Vo)
     */
    float Rs = (Referance_V - Vo) * (RL / Vo);
    return Rs;
}

unsigned int LPG_PPM(float RsRo_ratio)
{
    /*
     * Calculate the PPM using below equation
     * LPG ppm = [(Rs/Ro)/18.446]^(1/-0.421)
     */
    float ppm;
    ppm = pow((RsRo_ratio / 18.446), (1 / -0.421));
    return (unsigned int)ppm;
}

float Get_mVolt(byte AnalogPin)
{
    /* Calculate the ADC Voltage using below equation
     *  mVolt = ADC_Count * (ADC_Referance_Voltage / ADC_Resolution)
     */
    int ADC_Value = analogRead(AnalogPin);
    delay(1);
    float mVolt = ADC_Value * (Referance_V / 4096.0);
    return mVolt;
}
/* configure D9 and D11 as digital input and output respectively */
void setup()
{
    pinMode(pirSensor_pin, INPUT);
    Serial.begin(BAUD_RATE);
    // pinMode(ultrasonicSensor_trigger_pin, OUTPUT); // configure the trigger_pin(D9) as an Output
    // pinMode(LED_BUILTIN, OUTPUT);                  // Set the LED (D13) pin as a digital output
    // pinMode(ultrasonicSensor_echo_pin, INPUT);     // configure the Echo_pin(D11) as an Input

    timer.attach_ms(100, timerHandler);

    // dht.begin();

    // pinMode(MQ6_Pin, INPUT); /* Define A0 as a INPUT Pin */
    // delay(500);
    // Serial.println("Wait for 30 sec warmup");
    // delay(30000); /* Set the warmup delay wich is 30 Sec */
    // Serial.println("Warmup Complete");

    // for (int i = 0; i < 30; i++)
    // {
    //     mVolt += Get_mVolt(MQ6_Pin);
    // }
    // mVolt = mVolt / 30.0; /* Get the volatage in mV for 30 Samples */
    // Serial.print("Voltage at A0 Pin = ");
    // Serial.print(mVolt);
    // Serial.println("mVolt");
    // Serial.print("Rs = ");
    // Serial.println(Calculate_Rs(mVolt));
    // Ro = Calculate_Rs(mVolt) / Ro_clean_air_factor;
    // Serial.print("Ro = ");
    // Serial.println(Ro);
    // Serial.println(" ");
    // mVolt = 0.0;
}

void loop()
{
    // for (int i = 0; i < 500; i++)
    // {
    //     mVolt += Get_mVolt(MQ6_Pin);
    // }
    // mVolt = mVolt / 500.0; /* Get the volatage in mV for 500 Samples */
    // Serial.print("Voltage at A0 Pin = ");
    // Serial.print(mVolt); /* Print the mV in Serial Monitor */
    // Serial.println(" mV");

    // float Rs = Calculate_Rs(mVolt);
    // Serial.print("Rs = ");
    // Serial.println(Rs); /* Print the Rs value in Serial Monitor */
    // float Ratio_RsRo = Rs / Ro;

    // Serial.print("RsRo = ");
    // Serial.println(Ratio_RsRo);

    // Serial.print("LPG ppm = ");
    // unsigned int LPG_ppm = LPG_PPM(Ratio_RsRo);
    // Serial.println(LPG_ppm); /* Print the Gas PPM value in Serial Monitor */

    // Serial.println("");
    // mVolt = 0.0; /* Set the mVolt variable to 0 */

    // int sensor_Aout = analogRead(MQ5Sensor_A0_pin); /*Analog value read function*/
    // Serial.print("Gas Sensor: ");
    // Serial.print(sensor_Aout); /*Read value printed*/
    // Serial.print("\t");
    // Serial.print("\t");
    // if (sensor_Aout > 1800)
    // { /*if condition with threshold 1800*/
    //     Serial.println("Gas");
    // }
    // else
    // {
    //     Serial.println("No Gas");
    // }
    // delay(2000);
}