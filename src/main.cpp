#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <DHT.h>

#define BAUD_RATE 9600
String my_username = "admin", my_pass = "admin";

// WiFi
#define AP_SSID "ESP"
#define AP_PASS "12345678"
void initAP();

// WebServer
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Login Form</title>
</head>
<body>
    <h1>Login Form</h1>
    <form id="loginForm">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" required><br><br>
        <label for="password">Password:</label>
        <input type="password" id="password" name="password" required><br><br>
        <input type="submit" value="Login">
    </form>
    <script>
        const loginForm = document.getElementById('loginForm');
        loginForm.addEventListener('submit', function (event) {
            event.preventDefault();
            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            var formData = new FormData();
            formData.append('key', 'autha');
            formData.append('username', username);
            formData.append('password', password);
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/check', true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    if (xhr.status === 200) {
                        window.location.href = '/adminForm';
                    } else {
                        window.location.href = '/error';
                    }
                }
            };
            xhr.send(formData);
        });
    </script>
</body>
</html>
)rawliteral";
const char admin_login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Administrator Login</title>
</head>
<body>
    <h1>Welcome to the System</h1>
    <p>In order to be introduced to the system as an administrator and be able to use the system's features, please
        enter your personal username and password.</p>
    <form id="loginForm">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" required><br><br>
        <label for="password">Password:</label>
        <input type="password" id="password" name="password" required><br><br>
        <input type="submit" value="Login">
    </form>
    <script>
        const loginForm = document.getElementById('loginForm');
        loginForm.addEventListener('submit', function (event) {
            event.preventDefault();
            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            var formData = new FormData();
            formData.append('key', 'authb');
            formData.append('username', username);
            formData.append('password', password);
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/check', true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    if (xhr.status === 200) {
                        window.location.href = '/confirm';
                    } else {
                        window.location.href = '/error';
                    }
                }
            };
            xhr.send(formData);
        });
    </script>
</body>
</html>
)rawliteral";
const char confirm_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
      <title>Confirmation</title>
  </head>
  <body>
      <h1>Confirmation</h1>
      <p>Your details have been entered and saved.</p>
      <p>To continue the process, the device will restart after 10 seconds.</p>
  </body>
  </html>
)rawliteral";
const char reset_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Reset</title>
</head>
<body>
    <h1>Reset Factory</h1>
    <p>To continue the process, the device will restart after 10 seconds.</p>
</body>
</html>
)rawliteral";
const char error_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
      <title>Error</title>
  </head>
  <body>
      <h1>Error</h1>
      <p>The information entered is not correct, please try again.</p>
      <p><a href="/">Return to Home Page</a></p>
  </body>
  </html>
)rawliteral";
const char pir_form_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Burglar Alarm</title>
</head>
<body>
    <h1>Enter 1 to turn ON the burglar alarm and 0 to turn it OFF</h1>
    <form id="burglarAlarmForm">
        <label for="number">0 or 1:</label>
        <input type="number" id="number" name="number" required><br><br>
        <input type="submit" value="Submit">
    </form>
    <script>
        const burglarAlarmForm = document.getElementById('burglarAlarmForm');
        burglarAlarmForm.addEventListener('submit', function (event) {
            event.preventDefault();
            const number = document.getElementById('number').value;
            var formData = new FormData();
            formData.append('key', 'url6');
            formData.append('number', number);
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/check', true);
            xhr.onreadystatechange = function () {
                if (xhr.readyState === XMLHttpRequest.DONE) {
                    if (xhr.status === 200) {
                        window.location.href = '/';
                    } else {
                        window.location.href = '/error';
                    }
                }
            };
            xhr.send(formData);
        });
    </script>
</body>
</html>
)rawliteral";
const String url_time_elapsed = "/time_elapsed",
             url_egg_incubator = "/egg_incubator",
             url_light_status = "/light_status",
             url_watering_time = "/watering_time",
             url_feeder_movement = "/feeder_movement",
             url_pir = "/alarm_status";
const String check_url1 = "url1",
             check_url2 = "url2",
             check_url3 = "url3",
             check_url4 = "url4",
             check_url5 = "url5",
             check_url6 = "url6";
WebServer server(80);
void initWebServer();
String dashboard_html();
String timer_form_html(const String &check_url);
void handleRoot();
void handleAdminForm();
void handleConfirm();
void handleTimeElapsed();
void handleEggIncubator();
void handleLightStatus();
void handleWateringTime();
void handleFeederMovement();
void handlePir();
void handleCheck();
void handleReset();
void handleError();
void handleNotFound();

// MQ5 LPG Gas Sensor
#define MQ5Sensor_A0_pin 36
float Referance_V = 3300.0;
float RL = 1.0;
float Ro = 10.0;
float mVolt = 0.0;
const float Ro_clean_air_factor = 10.0;
unsigned int LPG_ppm = 0;
float calculateRS(float Vo);
unsigned int LPG_PPM(float RsRo_ratio);
float get_mVolt(byte AnalogPin);

// DHT11 Temperature and Humidity Sensor
#define DHTPIN 21
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float humidity, temperature, heatIndex;
// Reading temperature or humidity takes about 250 milliseconds!
void calculateTemperatureHumidity();

// HC-SR04 Ultrasonic Sensor
#define SOUND_SPEED 0.034
#define ultrasonicSensor_trigger_pin 23
#define ultrasonicSensor_echo_pin 22
long ultrasonicSensor_duration;
int ultrasonicSensor_distance;
// return Distance (cm)
void calculateDistance();

// EEPROM
#define EEPROM_SIZE 150
#define magicVal 13
#define magic_idx 0
#define currentState_idx 1
#define eeprom_data_idx 2
#define username_idx 25
#define password_idx 75
#define pair 0x13
#define notPair 0x12
byte currentState = notPair;
struct eeprom_data
{
    int FCD = 0;        // timer kod jam kon
    int CE = 0;         // timer tokhm morq jam kon
    int OFLED = 0;      // timer on/off LED
    int OFW = 0;        // timer on/off Water
    int OFR = 0;        // timer on/off Reil
    byte PIR = notPair; // on/off PIR Motion
} e_data;
void initEEPROM();
void resetFactory();
String readEEPROM(const byte &_startIdx);
void writeEEPROM(const byte &_startIdx, const String &_str);

// PIR Motion Sensor
#define pirSensor_pin 19
#define numberMotionChecks 20
#define pirSensor_ms 100
int sumMovement = 0, cntMovement = 0;
bool motionDetect()
{
    return digitalRead(pirSensor_pin);
}

// Timers
#define samplingRate_ms 5
#define samplingRate_thershold 500
#define DHT11_thershold 300 / samplingRate_ms
int counterMQ5 = 0, counterDHT11 = 0;
Ticker timerTimeElapsed,
    timerEggIncubator,
    timerLightStatus,
    timerWateringTime,
    timerFeederMovement,
    timerPir,
    timer;
void timerTimeElapsedHandler();
void timerEggIncubatorHandler();
void timerLightStatusHandler();
void timerWateringTimeHandler();
void timerFeederMovementHandler();
void timerPirHandler();
void timerHandler();

//----------------------------------------------------------------------------------------------------------------------------------

void setup()
{
    pinMode(ultrasonicSensor_trigger_pin, OUTPUT); // configure the trigger_pin(D9) as an Output
    pinMode(ultrasonicSensor_echo_pin, INPUT);     // configure the Echo_pin(D11) as an Input
    pinMode(pirSensor_pin, INPUT);
    pinMode(MQ5Sensor_A0_pin, INPUT);
    Serial.begin(BAUD_RATE);

    initEEPROM();
    // resetFactory();
    initAP();
    initWebServer();
    dht.begin();

    if (currentState == pair)
    {
        delay(500);
        Serial.println("Wait for 30 sec warmup");
        delay(30000); /* Set the warmup delay wich is 30 Sec */
        Serial.println("Warmup Complete");
        for (int i = 0; i < 30; i++)
        {
            mVolt += get_mVolt(MQ5Sensor_A0_pin);
        }
        mVolt = mVolt / 30.0; /* Get the volatage in mV for 30 Samples */
        Serial.print("Voltage at A0 Pin = ");
        Serial.print(mVolt);
        Serial.println("mVolt");
        Serial.print("Rs = ");
        Serial.println(calculateRS(mVolt));
        Ro = calculateRS(mVolt) / Ro_clean_air_factor;
        Serial.print("Ro = ");
        Serial.println(Ro);
        Serial.println(" ");
        mVolt = 0.0;

        timer.attach_ms(samplingRate_ms, timerHandler);
    }

    // pinMode(32, OUTPUT);
    // pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    server.handleClient();

    // digitalWrite(32, LOW);
    // digitalWrite(LED_BUILTIN, LOW);
    // delay(1000);
    // digitalWrite(32, HIGH);
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(1000);

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
    delay(2000);
}

void initAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
}
//----------------------------------------------------------------------------------------------------------------------------------
void initWebServer()
{
    server.on("/", handleRoot);
    server.on("/adminForm", handleAdminForm);
    server.on("/confirm", handleConfirm);
    server.on("/check", handleCheck);
    server.on(url_time_elapsed, handleTimeElapsed);
    server.on(url_egg_incubator, handleEggIncubator);
    server.on(url_light_status, handleLightStatus);
    server.on(url_watering_time, handleWateringTime);
    server.on(url_feeder_movement, handleFeederMovement);
    server.on(url_pir, handlePir);
    server.on("/reset", handleReset);
    server.on("/error", handleError);
    server.onNotFound(handleNotFound);
    server.begin();
}
String dashboard_html()
{
    String html = "<!DOCTYPE html><html><head><title>Dashboard</title></head><body>";
    html += "<h1>System Dashboard</h1>";
    html += "<h2>Time Elapsed:</h2><p>" + String(e_data.FCD) + "s <a href=\"" + url_time_elapsed + "\">Change Status</a></p>";
    html += "<h2>Egg Incubator Time:</h2><p>" + String(e_data.CE) + "s <a href=\"" + url_egg_incubator + "\">Change Status</a></p>";
    html += "<h2>Light Status (On/Off):</h2><p>" + String(e_data.OFLED) + "s <a href=\"" + url_light_status + "\">Change Status</a></p>";
    html += "<h2>Watering Time:</h2><p>" + String(e_data.OFW) + "s <a href=\"" + url_watering_time + "\">Change Status</a></p>";
    html += "<h2>Feeder Movement Time:</h2><p>" + String(e_data.OFR) + "s <a href=\"" + url_feeder_movement + "\">Change Status</a></p>";
    String OnOff = "Off";
    if (e_data.PIR == pair)
    {
        OnOff = "On";
    }
    html += "<h2>Alarm Status:</h2><p>" + OnOff + " <a href=\"" + url_pir + "\">Change Status</a></p>";
    html += "<h2>Temperature and Humidity:</h2><p>Temperature: " + String(temperature) + " °C<br>Humidity: " + String(humidity) + "%<br>Heat index: " + String(heatIndex) + " °C</p>";
    html += "<h2>LPG ppm:</h2><p>" + String(LPG_ppm) + " PPM</p>";
    html += "<h2>Food Container Level:</h2><p>" + String(ultrasonicSensor_distance) + "cm</p><br><br><a href=\"reset\">Reset Factory</a></body></html>";
    return html;
    ///
}
String timer_form_html(const String &check_url)
{
    String html = "<!DOCTYPE html><html><head><title>Timer Input</title></head><body>";
    html += "<h1>Enter the time you need in seconds</h1>";
    html += "<form id=\"timerForm\">";
    html += "<label for=\"number\">Timer:</label>";
    html += "<input type=\"number\" id=\"number\" name=\"number\" required><br><br>";
    html += "<input type=\"submit\" value=\"Submit\">";
    html += "</form><script>const timerForm = document.getElementById('timerForm');";
    html += "timerForm.addEventListener('submit', function (event) {";
    html += "event.preventDefault();const number = document.getElementById('number').value;var formData = new FormData();";
    html += "formData.append('key', '" + check_url + "');";
    html += "formData.append('number', number);";
    html += "const xhr = new XMLHttpRequest();";
    html += "xhr.open('POST', '/check', true);";
    html += "xhr.onreadystatechange = function () {if (xhr.readyState === XMLHttpRequest.DONE) {";
    html += "if (xhr.status === 200) {window.location.href = '/';} else {window.location.href = '/error';}}};";
    html += "xhr.send(formData);});</script></body></html>";
    return html;
}
void handleRoot()
{
    if (currentState == notPair)
    {
        server.send(200, "text/html", login_html);
    }
    else if (currentState == pair)
    {
        if (!server.authenticate(my_username.c_str(), my_pass.c_str()))
        {
            server.requestAuthentication();
            return;
        }
        server.send(200, "text/html", dashboard_html());
    }
}
void handleAdminForm()
{
    if (currentState == notPair)
    {
        server.send(200, "text/html", admin_login_html);
    }
    else if (currentState == pair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleConfirm()
{
    if (currentState == notPair)
    {
        server.send(200, "text/html", confirm_html);
        delay(10000);
        ESP.restart();
    }
    else if (currentState == pair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
///
void handleTimeElapsed()
{
    if (currentState == pair)
    {
        String timer_form_html2 = "";
        // formData.append('key', 'authb');
        // timer_form_html2 += "formData.append('key', '" + check_url1 + "');";
        // timer_form_html2 += "var params = 'key=" + check_url1 + "' + 'number=' + encodeURIComponent(number);";
        server.send(200, "text/html", timer_form_html(check_url1));
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleEggIncubator()
{
    if (currentState == pair)
    {
        String timer_form_html2 = "";
        // formData.append('key', 'authb');
        // timer_form_html2 += "formData.append('key', '" + check_url2 + "');";
        // timer_form_html2 += "var params = 'key=" + check_url2 + "' + 'number=' + encodeURIComponent(number);";
        // server.send(200, "text/html", timer_form_html1 + timer_form_html2 + timer_form_html3);
        server.send(200, "text/html", timer_form_html(check_url2));
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleLightStatus()
{
    if (currentState == pair)
    {
        String timer_form_html2 = "";
        // formData.append('key', 'authb');
        // timer_form_html2 += "formData.append('key', '" + check_url3 + "');";
        // timer_form_html2 += "var params = 'key=" + check_url3 + "' + 'number=' + encodeURIComponent(number);";
        // server.send(200, "text/html", timer_form_html1 + timer_form_html2 + timer_form_html3);
        server.send(200, "text/html", timer_form_html(check_url3));
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleWateringTime()
{
    if (currentState == pair)
    {
        String timer_form_html2 = "";
        // formData.append('key', 'authb');
        // timer_form_html2 += "formData.append('key', '" + check_url4 + "');";
        // timer_form_html2 += "var params = 'key=" + check_url4 + "' + 'number=' + encodeURIComponent(number);";
        // server.send(200, "text/html", timer_form_html1 + timer_form_html2 + timer_form_html3);
        server.send(200, "text/html", timer_form_html(check_url4));
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleFeederMovement()
{
    if (currentState == pair)
    {
        String timer_form_html2 = "";
        // formData.append('key', 'authb');
        // timer_form_html2 += "formData.append('key', '" + check_url5 + "');";
        // timer_form_html2 += "var params = 'key=" + check_url5 + "' + 'number=' + encodeURIComponent(number);";
        // server.send(200, "text/html", timer_form_html1 + timer_form_html2 + timer_form_html3);
        server.send(200, "text/html", timer_form_html(check_url5));
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
///
void handlePir()
{
    if (currentState == pair)
    {
        server.send(200, "text/html", pir_form_html);
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleCheck()
{
    String key = server.arg("key"),
           temp_username = server.arg("username"),
           temp_pass = server.arg("password"),
           temp_number = server.arg("number");

    ///
    Serial.print("key: ");
    Serial.println(key);
    Serial.print("temp_username: ");
    Serial.println(temp_username);
    Serial.print("temp_pass: ");
    Serial.println(temp_pass);
    Serial.print("temp_number: ");
    Serial.println(temp_number);

    if (key == "autha" && temp_username == my_username && temp_pass == my_pass)
    {
        server.send(200);
        return;
    }
    else if (key == "authb" && temp_username != "" && temp_pass != "")
    {
        EEPROM.write(currentState_idx, pair);
        writeEEPROM(username_idx, temp_username);
        writeEEPROM(password_idx, temp_pass);
        server.send(200);
        return;
    }
    else if (key == check_url1 && temp_number != "")
    {
        e_data.FCD = temp_number.toInt();
        EEPROM.put(eeprom_data_idx, e_data);
        EEPROM.commit();
        timerTimeElapsed.detach();
        timerTimeElapsed.attach(e_data.FCD, timerTimeElapsedHandler);
        server.send(200);
        return;
    }
    else if (key == check_url2 && temp_number != "")
    {
        e_data.CE = temp_number.toInt();
        EEPROM.put(eeprom_data_idx, e_data);
        EEPROM.commit();
        timerEggIncubator.detach();
        timerEggIncubator.attach(e_data.CE, timerEggIncubatorHandler);
        server.send(200);
        return;
    }
    else if (key == check_url3 && temp_number != "")
    {
        e_data.OFLED = temp_number.toInt();
        EEPROM.put(eeprom_data_idx, e_data);
        EEPROM.commit();
        timerLightStatus.detach();
        timerLightStatus.attach(e_data.OFLED, timerLightStatusHandler);
        server.send(200);
        return;
    }
    else if (key == check_url4 && temp_number != "")
    {
        e_data.OFW = temp_number.toInt();
        EEPROM.put(eeprom_data_idx, e_data);
        EEPROM.commit();
        timerWateringTime.detach();
        timerWateringTime.attach(e_data.OFW, timerWateringTimeHandler);
        server.send(200);
        return;
    }
    else if (key == check_url5 && temp_number != "")
    {
        e_data.OFR = temp_number.toInt();
        EEPROM.put(eeprom_data_idx, e_data);
        EEPROM.commit();
        timerFeederMovement.detach();
        timerFeederMovement.attach(e_data.OFR, timerFeederMovementHandler);
        server.send(200);
        return;
    }
    ///
    else if (key == check_url6 && temp_number != "")
    {
        if (temp_number == "1")
        {
            e_data.PIR = pair;
            EEPROM.put(eeprom_data_idx, e_data);
            EEPROM.commit();
            timerPir.detach();
            timerPir.attach_ms(pirSensor_ms, timerPirHandler);
            server.send(200);
            return;
        }
        else if (temp_number == "0")
        {
            e_data.PIR = notPair;
            EEPROM.put(eeprom_data_idx, e_data);
            EEPROM.commit();
            timerPir.detach();
            server.send(200);
            return;
        }
    }
    server.send(401);
}
void handleReset()
{
    if (currentState == pair)
    {
        server.send(200, "text/html", reset_html);
        delay(10000);
        resetFactory();
    }
    else if (currentState == notPair)
    {
        server.send(404, "text/plain", "Error 404 - Not Found!");
    }
}
void handleError()
{
    server.send(200, "text/html", error_html);
}
void handleNotFound()
{
    server.send(404, "text/plain", "Error 404 - Not Found!");
}
//----------------------------------------------------------------------------------------------------------------------------------
float calculateRS(float Vo)
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
float get_mVolt(byte AnalogPin)
{
    /* Calculate the ADC Voltage using below equation
     *  mVolt = ADC_Count * (ADC_Referance_Voltage / ADC_Resolution)
     */
    int ADC_Value = analogRead(AnalogPin);
    delay(1);
    float mVolt = ADC_Value * (Referance_V / 4096.0);
    return mVolt;
}
//----------------------------------------------------------------------------------------------------------------------------------
void calculateTemperatureHumidity()
{
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
}
//----------------------------------------------------------------------------------------------------------------------------------
void calculateDistance()
{
    digitalWrite(ultrasonicSensor_trigger_pin, LOW); // set trigger signal low for 2us
    delayMicroseconds(2);
    digitalWrite(ultrasonicSensor_trigger_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultrasonicSensor_trigger_pin, LOW);
    ultrasonicSensor_duration = pulseIn(ultrasonicSensor_echo_pin, HIGH);
    ultrasonicSensor_distance = ultrasonicSensor_duration * SOUND_SPEED / 2;
}
//----------------------------------------------------------------------------------------------------------------------------------
void initEEPROM()
{
    EEPROM.begin(EEPROM_SIZE);

    // EEPROM.write(magic_idx, 0x03);
    // EEPROM.commit();

    if (EEPROM.read(magic_idx) == magicVal)
    {
        currentState = EEPROM.read(currentState_idx);
        if (currentState == pair)
        {
            // if (e_data.OFLED != 0) set Timer
            my_username = readEEPROM(username_idx);
            my_pass = readEEPROM(password_idx);
            EEPROM.get(eeprom_data_idx, e_data);

            ///
            Serial.print("FCD: ");
            Serial.println(e_data.FCD);
            Serial.print("CE: ");
            Serial.println(e_data.CE);
            Serial.print("OFLED: ");
            Serial.println(e_data.OFLED);
            Serial.print("OFW: ");
            Serial.println(e_data.OFW);
            Serial.print("OFR: ");
            Serial.println(e_data.OFR);
            Serial.print("PIR: ");
            Serial.println(e_data.PIR);
            if (e_data.FCD != 0)
            {
            }
            if (e_data.CE != 0)
            {
            }
            if (e_data.OFLED != 0)
            {
            }
            if (e_data.OFR != 0)
            {
            }
            if (e_data.PIR == pair)
            {
                timerPir.attach_ms(pirSensor_ms, timerPirHandler);
            }
        }

        ///
        Serial.print("currentState: ");
        Serial.println(currentState);
        Serial.print("my_username: ");
        Serial.println(my_username);
        Serial.print("my_pass: ");
        Serial.println(my_pass);
    }
    else
    {
        currentState = notPair;
        EEPROM.write(magic_idx, magicVal);
        EEPROM.write(currentState_idx, currentState);
        EEPROM.put(eeprom_data_idx, e_data);
        EEPROM.commit();
    }
    Serial.println("initEEPROM");
}
void resetFactory()
{
    EEPROM.write(currentState_idx, notPair);
    e_data.FCD = 0;
    e_data.CE = 0;
    e_data.OFLED = 0;
    e_data.OFW = 0;
    e_data.OFR = 0;
    e_data.PIR = notPair;
    EEPROM.put(eeprom_data_idx, e_data);
    EEPROM.commit();
    ESP.restart();
}
String readEEPROM(const byte &_startIdx)
{
    String res = "";
    byte _len = EEPROM.read(_startIdx);
    for (byte i = _startIdx + 1; i < _startIdx + _len + 1; i++)
        res += (char)EEPROM.read(i);
    return res;
}
void writeEEPROM(const byte &_startIdx, const String &_str)
{
    byte j = _startIdx;
    EEPROM.write(j++, _str.length());
    for (byte i = 0; i < _str.length(); i++)
        EEPROM.write(j++, _str[i]);
    EEPROM.commit();
}
//----------------------------------------------------------------------------------------------------------------------------------
void timerTimeElapsedHandler()
{
}
void timerEggIncubatorHandler()
{
}
void timerLightStatusHandler()
{
}
void timerWateringTimeHandler()
{
}
void timerFeederMovementHandler()
{
}
void timerPirHandler()
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
}
void timerHandler()
{
    // 10ms * 500 = 5000
    // for (int i = 0; i < 500; i++) // 500
    // {
    mVolt += get_mVolt(MQ5Sensor_A0_pin);
    // }
    if (counterMQ5 >= samplingRate_thershold - 1)
    {
        mVolt = mVolt / 500.0; /* Get the volatage in mV for 500 Samples */
        float Rs = calculateRS(mVolt);
        float Ratio_RsRo = Rs / Ro;
        Serial.print("LPG ppm = ");
        LPG_ppm = LPG_PPM(Ratio_RsRo);
        Serial.println(LPG_ppm); /* Print the Gas PPM value in Serial Monitor */
        Serial.println();
        mVolt = 0.0; /* Set the mVolt variable to 0 */
        counterMQ5 = 0;
    }
    if (counterDHT11 >= DHT11_thershold - 1)
    {
        calculateTemperatureHumidity();
        calculateDistance();
        counterDHT11 = 0;
    }
    counterMQ5++;
    counterDHT11++;
}
//----------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------