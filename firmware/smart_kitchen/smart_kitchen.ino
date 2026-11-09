/* ============================================================
    COCINA INTELIGENTE - CÓDIGO FINAL ESTABLE (PIN 26 + TRANSISTOR 2N2222)
    ============================================================ */
#include <LiquidCrystal.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ---------------- PINES ----------------
#define DHTPIN             4     // Sensor DHT11
#define DHTTYPE            DHT11
#define LED_ESTUFA         27    // LED estufa
#define INTERRUPTOR_ESTUFA 14    // Interruptor ON/OFF estufa
#define SERVO_PIN          18    // Servo nevera
#define LED_NEVERA         19    // LED nevera
#define INTERRUPTOR_NEVERA 5     // Interruptor ON/OFF nevera
#define PIN_VENTILADOR     26    // Ventilador -> Base del 2N2222 (via resistencia 1k)

// Pines LCD 16x2
#define LCD_RS 21
#define LCD_E  22
#define LCD_D4 23
#define LCD_D5 25
#define LCD_D6 32
#define LCD_D7 33

// ---------------- CONFIGURACION ----------------
#define TEMP_UMBRAL        28.0  // Temperatura para activar ventilador
#define INTERVALO_DHT      2500  // Intervalo de lectura DHT11 (ms)

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Servo servoNevera;

unsigned long lastLecturaDHT = 0;

void setup() {
  Serial.begin(115200);

  // Transistor 2N2222 como interruptor directo: LOW = apagado, HIGH = encendido
  digitalWrite(PIN_VENTILADOR, LOW);
  pinMode(PIN_VENTILADOR, OUTPUT);
  digitalWrite(PIN_VENTILADOR, LOW);  // Ventilador APAGADO al iniciar

  // Configuración de otros periféricos
  pinMode(LED_ESTUFA, OUTPUT);
  pinMode(LED_NEVERA, OUTPUT);
  pinMode(INTERRUPTOR_ESTUFA, INPUT_PULLUP);
  pinMode(INTERRUPTOR_NEVERA, INPUT_PULLUP);
  digitalWrite(LED_ESTUFA, LOW);
  digitalWrite(LED_NEVERA, HIGH); // LED de nevera inicia encendido porque la puerta arranca cerrada (0°)

  dht.begin();
  lcd.begin(16, 2);
  lcd.print("Iniciando...");

  servoNevera.setPeriodHertz(50);
  servoNevera.attach(SERVO_PIN, 500, 2500);
  servoNevera.write(0);

  delay(1500);
  lcd.clear();

  Serial.println("Sistema listo. Ventilador debe estar APAGADO.");
}

void loop() {
  // ---- Lógica de INTERRUPTORES ----
  bool estufaEncendida = (digitalRead(INTERRUPTOR_ESTUFA) == LOW);
  digitalWrite(LED_ESTUFA, estufaEncendida ? HIGH : LOW);

  bool neveraAbierta = (digitalRead(INTERRUPTOR_NEVERA) == LOW);
  
  // MODIFICADO: 
  // - Si la nevera está abierta (true), el LED se apaga (LOW).
  // - Si la nevera está cerrada (false), el LED se enciende (HIGH).
  digitalWrite(LED_NEVERA, neveraAbierta ? LOW : HIGH);
  
  // El servo se abre a 120 grados cuando el interruptor se activa
  servoNevera.write(neveraAbierta ? 120 : 0);

  // ---- Control Ventilador (DHT11) ----
  if (millis() - lastLecturaDHT > INTERVALO_DHT) {
    lastLecturaDHT = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" C | Hum: ");
    Serial.print(h);
    Serial.print(" % | ");

    if (!isnan(h) && !isnan(t)) {
      lcd.setCursor(0, 0);
      lcd.print("Temp: "); lcd.print(t, 1); lcd.print((char)223); lcd.print("C   ");
      lcd.setCursor(0, 1);
      lcd.print("Hum:  "); lcd.print(h, 1); lcd.print(" %   ");

      bool ventiladorEncendido = (t >= TEMP_UMBRAL);
      digitalWrite(PIN_VENTILADOR, ventiladorEncendido ? HIGH : LOW);

      Serial.println(ventiladorEncendido ? "VENTILADOR ON" : "Ventilador OFF");
    } else {
      Serial.println("ERROR: Lectura DHT11 fallida (NaN)");
    }
  }
}