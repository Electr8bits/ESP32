#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

const char* ssid = "MAJU 2.4G";
const char* password = "luhanaevaluna";

const int ledPin = 2; // cambia por el pin que uses
AsyncWebServer server(80);

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {Serial.print("."); delay(500);}
  Serial.println("\nConectado!");
  Serial.print("IP: ");Serial.println(WiFi.localIP());

  // Página principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String htmlContent = "<html><body>";
    htmlContent += "<h1>Control de LED</h1>";
    htmlContent += "<button onclick=\"sendRequest('/on');\">Encender</button>";
    htmlContent += "<button onclick=\"sendRequest('/off');\">Apagar</button>";
    htmlContent += "<script>function sendRequest(url){";
    htmlContent += "var xhr=new XMLHttpRequest();xhr.open('GET',url,true);xhr.send();}</script>";
    htmlContent += "</body></html>";
    request->send(200, "text/html", htmlContent);
  });

  // Ruta para encender LED
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);
    request->send(200, "text/plain", "Led Encendido");
  });

  // Ruta para apagar LED
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);
    request->send(200, "text/plain", "Led Apagado");
  });

  server.begin();
}

void loop() {
  // Nada que hacer aquí, AsyncWebServer es asíncrono
}
