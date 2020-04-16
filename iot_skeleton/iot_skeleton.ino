/* **************************************************************** *
 * sketch: POST skeleton
 * author: kristof aldenderfer
 * desc: creates basic functionality for http POST requests
 * req hardware: Teensy 3.x+ (uC)
 *               Adafruit HUZZAH ESP8266 (WiFi funcionality)
 * **************************************************************** */

#define USBComms Serial
#define WiFiComms Serial1
#define STA_GOTIP 5                                // connected to AP (currently unused)

// GLOBALS YOU CAN/SHOULD CHANGE!
bool verbose_mode = false;                         // for extra information (currently underused)
String wifi_name = "";
String wifi_password = "";
unsigned int post_interval_seconds = 0;            // how frequently should the device POST?
String http_host = "";
String http_url = "";
String POST_data_to_send = "";                     // THIS IS THE IMPORTANT ONE!
// ADD YOUR OWN GLOBALS HERE





// globals you should leave alone!
IntervalTimer delay_interval;                      // timer for short delays (replaces delay())
IntervalTimer post_interval;                       // timer for the POST interval
volatile int msg_state = 1;                        // state-machine logic for POSTing
unsigned long post_interval_time = post_interval_seconds*1000000;

/* ******************************** *
 * desc: one-time functionality
 * params: none
 * returns: none
 * ******************************** */
void setup() {
    USBComms.begin(115200);                        // teensy <--> USB
    WiFiComms.begin(9600);                         // ESP8266 <--> teensy
    while(!USBComms);                              // wait for Serial Monitor to open
    USBComms.print("> ");
    wifi_setup();
    delay_interval.begin(wifi_check, 3000000);     // wait for a bit before checking for wifi connectivity
}

/* ******************************** *
 * desc: repeated functionality
 * params: none
 * returns: none
 * ******************************** */
void loop() {
    if (WiFiComms.available()) {
        readFromESP();
    }
    if (USBComms.available()) {
        readFromTeensy();
    }
    // WHATEVER ELSE YOU WANT TO PUT IN HERE




}

/* ******************************** *
 * desc: sets up wifi
 * params: none
 * returns: none
 * ******************************** */
void wifi_setup() {
    if (verbose_mode) USBComms.println("====setting up wifi====");
    String msg = "wifi.sta.config('"+wifi_name+"', '"+wifi_password+"')";
    WiFiComms.println(msg);
}

/* ******************************** *
 * desc: checks to make sure wifi is working, then starts POST timer.
 * params: none
 * returns: none
 * ******************************** */
void wifi_check() {
    delay_interval.end();
    if (verbose_mode) USBComms.println("====checking wifi====");
    WiFiComms.println("print(wifi.sta.status())");
    http_POST();
    post_interval.begin(http_POST, post_interval_time);
}

/* ******************************** *
 * desc: POSTs data to the server
 * params: none
 * returns: none
 * ******************************** */
void http_POST() {
    noInterrupts();
    String request_type = "POST";
    String msg1 = "sk=net.createConnection(net.TCP, 0)  sk:on('receive', function(sck, c) print(c) end )  sk:connect(80, '" + http_host + "')";
    String msg2 = "sk:send('" + request_type + " " + http_url + " HTTP/1.1\\r\\nHost: " + http_host + "\\r\\nContent-Type: application/x-www-form-urlencoded\\r\\nConnection: keep-alive\\r\\nAccept: */*\\r\\nContent-Length: " + String(POST_data_to_send.length()) + "\\r\\n\\r\\n" + POST_data_to_send + "\\r\\n\\r\\n')";
    if (msg_state == 1) {
        WiFiComms.println(msg1);
        delay_interval.begin(http_POST, 2000000);
        msg_state++;
    } else {
        delay_interval.end();
        WiFiComms.println(msg2);
        msg_state = 1;
    }
    interrupts();
}

/* ******************************** *
 * desc: prints messages coming from the ESP8266
 * params: none
 * returns: none
 * ******************************** */
String readFromESP() {
    while (WiFiComms.available()) {
        int c = WiFiComms.read();                  // read the next character
        if (verbose_mode) USBComms.write((char)c); // writes data to the serial monitor
    }
}

/* ******************************** *
 * desc: sends messages to the ESP8266
 * params: none
 * returns: none
 * ******************************** */
String readFromTeensy() {
    delay(10);
    String cmd = "";                               // read the input command in a string
    while (USBComms.available()) {                 // ''
        cmd += (char)USBComms.read();              // ''
    }
    WiFiComms.print(cmd);                          // send the read character to the ESP8266
}

/* ******************************** *
 * desc: parses return messages (ideally from ESP) and checks for supplied pattern (currently unused)
 * params: none
 * returns: none
 * ******************************** */
bool check_return (char *str, char *pattern) {
    char * token;
    char msg[256];
    sprintf (msg, "Splitting string \"%s\" into tokens:\n", str);
    Serial.print(msg);
    
    // get the first token
    token = strtok (str, ".- \t");
    while (token != NULL) {
        sprintf (msg, "checking %s against %s\n", token, pattern);
        Serial.print(msg);
        if ((String)token == (String)pattern) {
            Serial.println("hooray!");
            return true;
        } else {
            Serial.println("not yet");
        }
        // go through other tokens
        token = strtok (NULL, ".- \t");
    }
    return false;
}