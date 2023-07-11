// Including the libraries
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// Select Camera Model
#define CAMERA_MODEL_AI_THINKER

// GPIO of Camera Model

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Invalid Camera Model"
#endif

// Gear Motors GPIO
#define Motor_R_Dir_Pin   2
#define Motor_R_PWM_Pin   15
#define Motor_L_Dir_Pin   13
#define Motor_L_PWM_Pin   12

// Servo Motors GPIO
#define servo_LeftRight_Pin   14

// ESP32 CAM LED GPIO
#define LED_OnBoard 4

// Setting PWM Properties For LED
const int PWM_LED_Freq = 5000;
const int PWM_LED_Channel = 6;
const int PWM_LED_resolution = 8;

// Setting PWM Properties For Gear Motors
const int PWM_Mtr_Freq = 5000;
const int PWM_Mtr_Resolution = 8;
const int PWM_Channel_Mtr_R = 4;
const int PWM_Channel_Mtr_L = 5;

// Variable For Gear Motor / Motor Driver PWM Value
int PWM_Motor_DC = 0;

// Variable For Servo Position
int servo_LeftRight_Pos = 75;

// Initialize Servos
Servo servo_temp_1; // Timer 0 or Channel 0
Servo servo_temp_2; // Timer 1 or Channel 1
Servo servo_LeftRight; // Timer 2 or Channel 2

// Access Point Declaration & Configuration
const char* ssid = "SentinalResQ";  // AP Name
const char* password = "rover123"; // AP Password

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Empty handle to esp_http_server
httpd_handle_t index_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

// HTML Code For the Web Page
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>

<head>

    <title>SentinalResQ</title>

    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">

    <style>
        * {
            box-sizing: border-box;
        }

        body {
            background-color: #212427;
            color: white;
            font-family: sans-serif;
            text-align: center;
            padding-top: 20px;
            max-width: 400px;
            margin: 0 auto;
        }

        .title {
            font-size: 30px;
        }

        .slidecontainer {
            width: 100%;
        }

        .slider {
            -webkit-appearance: none;
            width: 100%;
            height: 10px;
            border-radius: 5px;
            background: #333333;
            outline: none;
            opacity: 0.6;
            -webkit-transition: .2s;
            transition: opacity .2s;
        }

        .slider:hover {
            opacity: 1;
        }

        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 14px;
            height: 14px;
            border-radius: 50%;
            background: #04AA6D;
            cursor: pointer;
        }

        .slider::-moz-range-thumb {
            width: 14px;
            height: 14px;
            border-radius: 50%;
            background: #04AA6D;
            cursor: pointer;
        }

        .button {
            display: inline-block;
            padding: 5px;
            font-size: 25px;
            cursor: pointer;
            text-align: center;
            text-decoration: none;
            outline: none;
            color: white;
            background-color: #4CAF50;
            border: none;
            border-radius: 15px;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            -khtml-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
            width: 20%;
            height: 40px;
        }

        .button:hover {
            background-color: #3E8E41;
        }

        .button:active {
            background-color: #3E8E41;
            box-shadow: 0 0px #666666;
            transform: translateY(2px);
        }

        .space {
            width: 15%;
            height: auto;
            display: inline-block;
        }

        img {
            width: 320px;
            height: 240px;
            border-radius: 1rem;
            border: 1px solid white;
        }
    </style>

</head>

<body>

    <h3 class="title">SentinalResQ</h3>

    <img src="" id="video_stream">

    <br><br>

    <div>
        <!-- "T" and "M" suffixes are used to differentiate between touch screen input and mouse pointer input on buttons -->
        <button class="button" ontouchstart="button_ontouchstart_handle('FT')" ontouchend="button_ontouchend_handle()"
            onmousedown="button_onmousedown_handle('FM')" onmouseup="button_onmouseup_handle()"><b>&#9650;</b></button>
        <br><br>
        <button class="button" ontouchstart="button_ontouchstart_handle('LT')" ontouchend="button_ontouchend_handle()"
            onmousedown="button_onmousedown_handle('LM')" onmouseup="button_onmouseup_handle()"><b>&#9664;</b></button>
        <div class="space"></div>
        <button class="button" ontouchstart="button_ontouchstart_handle('RT')" ontouchend="button_ontouchend_handle()"
            onmousedown="button_onmousedown_handle('RM')" onmouseup="button_onmouseup_handle()"><b>&#9654;</b></button>
        <br><br>
        <button class="button" ontouchstart="button_ontouchstart_handle('BT')" ontouchend="button_ontouchend_handle()"
            onmousedown="button_onmousedown_handle('BM')" onmouseup="button_onmouseup_handle()"><b>&#9660;</b></button>
    </div>

    <br>

    <table style="width:320px;margin:auto;table-layout:fixed" CELLSPACING=10>
        <tr>
            <td style="text-align:left;width:50px;">Speed</td>
            <td style="width:200px;">
                <div class="slidecontainer">
                    <input type="range" min="0" max="10" value="5" class="slider" id="mySlider_pwm_Speed">
                </div>
            </td>
            <td style="text-align:right;font-weight:normal;width:20px;" id="slider_pwm_Speed_id">NN</td>
        </tr>

        <tr>
            <td>Light</td>
            <td>
                <div class="slidecontainer">
                    <input type="range" min="0" max="10" value="0" class="slider" id="mySlider_pwm_LED">
                </div>
            </td>
            <td style="text-align:right;" id="slider_pwm_LED_id">NN</td>
        </tr>

        <tr>
            <td>Pan</td>
            <td>
                <div class="slidecontainer">
                    <input type="range" min="0" max="180" value="75" class="slider" id="mySlider_Pan">
                </div>
            </td>
            <td style="text-align:right;" id="slider_Pan_id">NN</td>
        </tr>
    </table>

    <br>

  <!-- Display Sensor Values -->

    <div>
        <table style="width:320px;margin:auto;table-layout:fixed;background-color: #3f3a957a;border-radius: 2%;" CELLSPACING=10>
            <tr>
                <td>Temperature</td>
                <td style="text-align: right;">
                    <div id="temperature">Warming Up...</div>
                </td>
            </tr>
            <tr>
                <td>Humidity</td>
                <td style="text-align: right;">
                    <div id="humidity">Warming Up...</div>
                </td>
            </tr>
            <tr>
                <td>CO2</td>
                <td style="text-align: right;">
                    <div id="co2">Warming Up...</div>
                </td>
            </tr>
            <tr>
                <td>LPG</td>
                <td style="text-align: right;">
                    <div id="lpg">Warming Up...</div>
                </td>
            </tr>
            <tr>
                <td>Pressure</td>
                <td style="text-align: right;">
                    <div id="pressure">Warming Up...</div>
                </td>
            </tr>
        </table>
        <br>
    </div>

    <script>
        window.onload = document.getElementById("video_stream").src = window.location.href.slice(0, -1) + ":81/stream";

        var slider_pwm_Speed = document.getElementById("mySlider_pwm_Speed");
        var show_slider_pwm_Speed = document.getElementById("slider_pwm_Speed_id")
        show_slider_pwm_Speed.innerHTML = slider_pwm_Speed.value;

        var slider_pwm_LED = document.getElementById("mySlider_pwm_LED");
        var show_slider_pwm_LED = document.getElementById("slider_pwm_LED_id")
        show_slider_pwm_LED.innerHTML = slider_pwm_LED.value;

        var slider_Pan = document.getElementById("mySlider_Pan");
        var show_slider_Pan = document.getElementById("slider_Pan_id")
        show_slider_Pan.innerHTML = slider_Pan.value;

        let slider_Pan_send_rslt = "";

        var myTmr;
        var myTmr_Interval = 500;

        var myTmr_PanCtr;
        var myTmr_PanCtr_Interval = 200;
        var myTmr_PanCtr_Start = 1;
        var myTmr_PanCtr_Send = 0;

        var onmousedown_stat = 0;

        let btn_commands_rslt = "";

        var join_TM = 0;

        slider_pwm_Speed.oninput = function () {
            show_slider_pwm_Speed.innerHTML = slider_pwm_Speed.value;
            let slider_pwm_Speed_send = "SS," + slider_pwm_Speed.value;
            send_cmd(slider_pwm_Speed_send);
        }

        slider_pwm_LED.oninput = function () {
            show_slider_pwm_LED.innerHTML = slider_pwm_LED.value;
            let slider_pwm_LED_send = "SL," + slider_pwm_LED.value;
            send_cmd(slider_pwm_LED_send);
        }

        slider_Pan.oninput = function () {
            show_slider_Pan.innerHTML = slider_Pan.value;
            let slider_Pan_send = "SP," + slider_Pan.value;
            slider_Pan_send_rslt = slider_Pan_send;
            myTmr_PanCtr_Send = 1;
            if (myTmr_PanCtr_Start == 1) {
                myTmr_PanCtr = setInterval(myTimer_PanCtr, myTmr_PanCtr_Interval);
                myTmr_PanCtr_Start = 0;
            }
        }

        slider_Pan.onchange = function () {
            myTmr_PanCtr_Start = 1;
        }

        function button_onmousedown_handle(cmds) {
            clearInterval(myTmr);
            btn_commands_rslt = cmds.substring(0, 1);
            if (join_TM == 0) {
                if (onmousedown_stat == 0) {
                    send_btn_cmd(btn_commands_rslt);
                    onmousedown_stat = 1;
                }
            }
        }

        function button_ontouchstart_handle(cmds) {
            clearInterval(myTmr);
            join_TM = 1;
            btn_commands_rslt = cmds.substring(0, 1);
            if (onmousedown_stat == 0) {
                send_btn_cmd(btn_commands_rslt);
                onmousedown_stat = 1;
            }
        }

        function button_onmouseup_handle() {
            if (join_TM == 0) {
                onmousedown_stat = 0;
                send_btn_cmd("S");
                myTmr = setInterval(myTimer, myTmr_Interval);
            }
        }

        function button_ontouchend_handle() {
            join_TM = 1;
            onmousedown_stat = 0;
            send_btn_cmd("S");
            myTmr = setInterval(myTimer, myTmr_Interval);
        }

        function myTimer() {
            send_btn_cmd("S");
            clearInterval(myTmr);
        }

        function myTimer_PanCtr() {
            if (myTmr_PanCtr_Send == 1) {
                send_cmd(slider_Pan_send_rslt);
                if (myTmr_PanCtr_Start == 1) clearInterval(myTmr_PanCtr);
                myTmr_PanCtr_Send = 0;
            }
        }

        function send_btn_cmd(cmds) {
            let btn_cmd = "B," + cmds;
            send_cmd(btn_cmd);
        }

        function send_cmd(cmds) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=" + cmds, true);
            xhr.send();
        }

        // Function to Send Sensor Data to the Web Server
        function dataCollector() {
          const url = 'http://192.168.1.1:80/data';

          setInterval(() => {
              fetch(url)
                  .then(response => response.json())
                  .then(data => {
                      document.getElementById("temperature").innerHTML = data.sensor1;
                      document.getElementById("humidity").innerHTML = data.sensor2;
                      document.getElementById("co2").innerHTML = data.sensor3;
                      document.getElementById("lpg").innerHTML = data.sensor4;
                      document.getElementById("pressure").innerHTML = data.sensor5;
                  })
                  .catch(error => {
                      console.error('Error:', error);
                  });
          }, 1000);
      }

      dataCollector();

    </script>

</body>

</html>
)rawliteral";

// Index Handler Function to be Called During GET or URI Request
static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

// Fetching Sensor Data
float  sensorData1, sensorData2, sensorData3, sensorData4, sensorData5;

// Data Handler Function to be Called During GET or URI Request
static esp_err_t data_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  char responseOut[1024];
  sprintf(responseOut, "{\r\n\"sensor1\": \"%.2f\",\r\n\"sensor2\": \"%.2f\",\r\n\"sensor3\": \"%.2f\",\r\n\"sensor4\": \"%.2f\",\r\n\"sensor5\": \"%.2f\"\r\n}", sensorData1, sensorData2, sensorData3, sensorData4, sensorData5);
  return httpd_resp_send(req, responseOut, strlen(responseOut));
}

// Stream Handler Function to be Called During GET or URI Request
static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  // Create a Loop to View Live Footage From ESP32 CAM
  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      // Serial.println("Camera capture failed! (stream_handler)");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            // Serial.println("JPEG compression failed!");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
  }
  return res;
}

// CMD Handler Function to be Called During GET or URI Request
static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
   
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }
 
  int res = 0;

  // Serial.println();
  // Serial.print("Incoming command: ");
  // Serial.println(variable);
  String getData = String(variable);
  String resultData = getValue(getData, ',', 0);

  // Controlling the LED on the ESP32-CAM Board With PWM

  /* 
    Example :
        Let, Incoming command = S,10
        Here, S = Slider & 10 = Slider Value
        
        Slider Value Range Was Set From 0 to 10
        Then the Slider Value will be Changed from 0 to 10 or vice versa to 0 to 255 or vice versa
  */

  if (resultData == "SL") {
    resultData = getValue(getData, ',', 1);
    int pwm = map(resultData.toInt(), 0, 20, 0, 255);
    ledcWrite(PWM_LED_Channel, pwm);
    // Serial.print("PWM LED On Board: ");
    // Serial.println(pwm);
  }

  // Controlling the Servo Motors with the Pan Mount
  if (resultData == "SP") {
    resultData = getValue(getData, ',', 1);
    servo_LeftRight_Pos = resultData.toInt();
    servo_LeftRight.write(servo_LeftRight_Pos);
    // Serial.print("Pan Servo Value: ");
    // Serial.println(servo_LeftRight_Pos);
  }

  // Controlling the Speed of Gear Motors with PWM
  if (resultData == "SS") {
    resultData = getValue(getData, ',', 1);
    int pwm = map(resultData.toInt(), 0, 10, 0, 255);
    PWM_Motor_DC = pwm;
    // Serial.print("PWM Motor DC Value: ");
    // Serial.println(PWM_Motor_DC);
  }

  // Processes & Executes Commands Received From the Web Server

  /* 
    Example :
    Incoming Command = B,F
    B = Button
    F = Command for the car to move forward.
  */
  
  if (resultData == "B") {
    resultData = getValue(getData, ',', 1);

    if (resultData == "F") {
      Move_Forward(PWM_Motor_DC);
    }

    if (resultData == "B") {
      Move_Backward(PWM_Motor_DC);
    }

    if (resultData == "R") {
      Move_Right(PWM_Motor_DC);
    }

    if (resultData == "L") {
      Move_Left(PWM_Motor_DC);
    }

    if (resultData == "S") {
      Move_Stop();
    }

    // Serial.print("Button: ");
    // Serial.println(resultData);
  }
  
  if(res){
    return httpd_resp_send_500(req);
  }
 
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

// Subroutine For Starting the Web Server / startCameraServer
void startCameraWebServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t data_uri = {
    .uri  = "/data",
    .method   = HTTP_GET,
    .handler  = data_handler,
    .user_ctx  = NULL
  };
    
  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
    
  if (httpd_start(&index_httpd, &config) == ESP_OK) {
      httpd_register_uri_handler(index_httpd, &index_uri);
      httpd_register_uri_handler(index_httpd, &cmd_uri);
      httpd_register_uri_handler(index_httpd, &data_uri);
  }

  config.server_port += 1;
  config.ctrl_port += 1;  
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
      httpd_register_uri_handler(stream_httpd, &stream_uri);
  }

  // Serial.println();
  // Serial.println("Camera Server started successfully!");
  // Serial.print("http://");
  // Serial.println(local_ip);
  // Serial.println();
}

// String Function to Process the Data Command
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Gear Motors GPIO

// Subroutine For the Rover Moves Forward (F)
void Move_Forward(int pwm_val) {
  int pwm_result = 255 - pwm_val;
  digitalWrite(Motor_R_Dir_Pin, HIGH);
  ledcWrite(PWM_Channel_Mtr_R, pwm_result);
  digitalWrite(Motor_L_Dir_Pin, HIGH);
  ledcWrite(PWM_Channel_Mtr_L, pwm_result);
}

// Subroutine For the Rover Moves Backward (B)
void Move_Backward(int pwm_val) {
  digitalWrite(Motor_R_Dir_Pin, LOW);
  ledcWrite(PWM_Channel_Mtr_R, pwm_val);
  digitalWrite(Motor_L_Dir_Pin, LOW);
  ledcWrite(PWM_Channel_Mtr_L, pwm_val);
}

// Subroutine For the Rover Turns Right (R)
void Move_Right(int pwm_val) {
  int pwm_result = 255 - pwm_val;
  digitalWrite(Motor_R_Dir_Pin, LOW);
  ledcWrite(PWM_Channel_Mtr_R, pwm_val);
  digitalWrite(Motor_L_Dir_Pin, HIGH);
  ledcWrite(PWM_Channel_Mtr_L, pwm_result);
}

// Subroutine For the Rover Turns Left (L)
void Move_Left(int pwm_val) {
  int pwm_result = 255 - pwm_val;
  digitalWrite(Motor_R_Dir_Pin, HIGH);
  ledcWrite(PWM_Channel_Mtr_R, pwm_result);
  digitalWrite(Motor_L_Dir_Pin, LOW);
  ledcWrite(PWM_Channel_Mtr_L, pwm_val);
}

// Subroutine for the Rover Stops (S)
void Move_Stop() {
  digitalWrite(Motor_R_Dir_Pin, LOW);
  ledcWrite(PWM_Channel_Mtr_R, 0);
  digitalWrite(Motor_L_Dir_Pin, LOW);
  ledcWrite(PWM_Channel_Mtr_L, 0);
}

// Setup Function
void setup() {

  // Disable Brownout Detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  pinMode(LED_OnBoard, OUTPUT);

  // Setting Servos Motors
  // Serial.println();
  // Serial.println("Start Setting Servos Motors...");
  servo_LeftRight.setPeriodHertz(50); // Standard 50hz Servo 
  servo_temp_1.attach(-1, 1000, 2000);
  servo_temp_2.attach(-2, 1000, 2000);
  servo_LeftRight.attach(servo_LeftRight_Pin, 1000, 2000);
  
  servo_LeftRight.write(servo_LeftRight_Pos);
  // Serial.println("Successfully Set Up Servo Motors!");

  delay(500);

  // The Pin Which is Used to Set the Direction of Rotation of the Motor is Set as OUTPUT
  pinMode(Motor_R_Dir_Pin, OUTPUT);
  pinMode(Motor_L_Dir_Pin, OUTPUT);

  // Setting PWM

  // Set the PWM for the On Board LED
  // Serial.println();
  // Serial.println("Start setting PWM for On Board LED...");
  ledcAttachPin(LED_OnBoard, PWM_LED_Channel);
  ledcSetup(PWM_LED_Channel, PWM_LED_Freq, PWM_LED_resolution);
  // Serial.println("Successfully Set Up PWM for On Board LED!");
  // Serial.println();

  // Set the PWM for DC Motor / Motor Drive.
  // Serial.println("Start setting PWM for Gear Motors...");
  ledcAttachPin(Motor_R_PWM_Pin, PWM_Channel_Mtr_R);
  ledcAttachPin(Motor_L_PWM_Pin, PWM_Channel_Mtr_L);
  ledcSetup(PWM_Channel_Mtr_R, PWM_Mtr_Freq, PWM_Mtr_Resolution);
  ledcSetup(PWM_Channel_Mtr_L, PWM_Mtr_Freq, PWM_Mtr_Resolution);
  // Serial.println("Successfully Set Up PWM for Gear Motors!");

  delay(500);

  // Camera Configuration

  // Serial.println();
  // Serial.println("Start Configuring & Initializing the Camera...");
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    // Serial.printf("Camera Init Failed With Error 0x%x", err);
    ESP.restart();
  }
  
  // Serial.println("Configured and Initialized the Camera Successfully!");
  // Serial.println();

  // Start Access Point Mode

  // Serial.println();
  // Serial.println("Starting Access Point Mode...");
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  // Serial.println("Access Point Mode Started Successfully!");
  // Serial.println();

  // Start Camera Web Server
  startCameraWebServer(); 

  // Initialize Serial Communication Speed (Baud Rate)
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  // Serial.println();

  while(!Serial) {
    ;
  }

  // Establishing Contact With the Atmega Chip
  establishContact();
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("Waiting for Atmega to Send Sensor Data...");
    delay(250);
  }
}

void readSensorData() {
  if (Serial.available()>0) {
    String jsonString = Serial.readStringUntil('\n');

    // Create a JSON document
    StaticJsonDocument<1024> jsonDoc;

    // Deserialize the received JSON string
    deserializeJson(jsonDoc, jsonString);

    // Extract the sensor values from the JSON object
    sensorData1 = jsonDoc["temperature"];
    sensorData2 = jsonDoc["humidity"];
    sensorData3 = jsonDoc["co2"];
    sensorData4 = jsonDoc["lpg"];
    sensorData5 = jsonDoc["pressure"];
  }
}

void loop() {
  delay(1000);
  readSensorData();
}
