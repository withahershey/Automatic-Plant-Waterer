// open the webserver
// IP ADRESSSSSSSSSSSSSSSSSSSSS
//     Possible serial noise or corruption error
// Load Wi-Fi library
#include <WiFi.h>
#define pumpPin 25
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define motorEnable 33
//LCD librays
//#include <hd44780.h>
//#include <hd44780ioClass/hd44780_I2Cexp.h>

// Network credentials Here
const char* ssid     = "ESP32-Network";
const char* password = "Esp32-Password";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//Output variable to GPIO pins
const int moistPin = 35;
bool watering = false;
unsigned long wateringStart = 0;
unsigned long wateringDuration = 0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 2000;
//const int pumpPin = 14;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
int count = 0;
void setup() {
  
  // WIFI THINGS
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.softAP(ssid,password);
  // Print IP address and start web server
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
  // PUMP THINGS
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(motorEnable, OUTPUT);
  digitalWrite(motorEnable, HIGH);
  
  lcd.init();// initialize the lcd 
  lcd.backlight(); // Turns on the LCD backlight.
  lcd.print("Automatic plant waterer.");   // Print a message to the LCD.
  delay(3000);
}

void loop() {
  WiFiClient client = server.available();
  int analogValue = analogRead(moistPin);

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();

      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            client.println("<!DOCTYPE html><html>");
            
            // Water buttons
            if (!watering && header.indexOf("GET /water100") >= 0) {
              watering = true;
              wateringStart = millis();
              wateringDuration = 4000;
              Serial.println("Watering 100ml");
            }
            else if (!watering && header.indexOf("GET /water50") >= 0) {
              watering = true;
              wateringStart = millis();
              wateringDuration = 2000;
              Serial.println("Watering 50ml");
            }
            else if (!watering && header.indexOf("GET /water10") >= 0) {
              watering = true;
              wateringStart = millis();
              wateringDuration = 500;
              Serial.println("Watering 10ml");
            }
            client.println("<head>");
            //client.println("<meta http-equiv=\"refresh\" content=\"1\">");
            client.println("<style>");
            client.println("body {");
            client.println("  background-color: rgb(142, 222, 162);");
            client.println("  text-align: center;");
            client.println("  font-family: monospace;");
            client.println("  font-size: 30px;");
            client.println("}");
            client.println("button {");
            client.println("  background-color: rgb(142, 187, 222);");
            client.println("  font-family: monospace;");
            client.println("  font-size: 24px;");
            client.println("  padding: 10px 20px;");
            client.println("  margin: 5px;");
            client.println("}");
            client.println("</style>");
            client.println("</head>");

            client.println("<body>");
            client.println("<h1>ESP32 Plant Monitor</h1>");
            client.println("<h4>Air (Dry): ~2000 to 3200</h4>");
            client.println("<h4>Ideal Moist: ~1200 to 2000</h4>");
            client.println("<h4>Saturated: &lt; 1000</h4>");

            client.println("<p>Moisture Value: " + String(analogRead(moistPin)) + "</p>");

            client.println("<a href=\"/water10\"><button>10ml water</button></a>");
            client.println("<a href=\"/water50\"><button>50ml water</button></a>");
            client.println("<a href=\"/water100\"><button>100ml water</button></a>");

            client.println("</body>");
            client.println("</html>");
            
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    header = "";
    Serial.println("Client disconnected.");
  }

  if (watering) {
    digitalWrite(pumpPin, HIGH);

    if (millis() - wateringStart >= wateringDuration) {
      digitalWrite(pumpPin, LOW);
      watering = false;
    }
  }

  if (!watering) {
  if (analogValue > 2000 && analogValue <= 3200) {
    watering = true;
    wateringStart = millis();
    wateringDuration = 4000;
    Serial.println("AUTO: Watering 100ml");
  }
  else if (analogValue > 1200 && analogValue <= 2000) {
    watering = true;
    wateringStart = millis();
    wateringDuration = 2000;
    Serial.println("AUTO: Watering 50ml");
  }
}

  Serial.printf("Analog value = %d\n", analogValue);
  Serial.printf("Moisture: %d | watering=%d\n", analogValue, watering);
  delay(500);

  lcd.setCursor(0,1);
  lcd.print("Moist: ");
  lcd.print(analogRead(moistPin));
  lcd.print("    ");
}

