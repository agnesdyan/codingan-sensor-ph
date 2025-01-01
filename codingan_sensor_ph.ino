#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <SD.h>
#include <FS.h>

File myFile;

// Konfigurasi OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Konfigurasi RTC
RTC_DS3231 rtc;

// Konfigurasi microSD  
#define SD_CS_PIN 5
#define LDR_PIN 34 // Pin analog untuk LDR

void setup() {
  // Inisialisasi komunikasi serial
  Serial.begin(9600);
  while (!Serial) {
    ; // Tunggu koneksi serial untuk port USB native
  }

  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Pastikan alamat I2C sesuai
    Serial.println(F("OLED tidak terdeteksi!"));
    while (1);
  }
  display.clearDisplay();

  // Inisialisasi RTC
  if (!rtc.begin()) {
    Serial.println(F("RTC tidak terdeteksi!"));
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println(F("RTC kehilangan daya, atur ulang waktu!"));
  }

  // Inisialisasi microSD
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // Membuat atau membuka file untuk menulis
  myFile = SD.open("/ldr_log.txt", FILE_APPEND);
  if (myFile) {
    Serial.println("File opened successfully.");
    myFile.println("Tanggal,Waktu,Kecerahan (%)");
    myFile.close();
  } else {
    Serial.println("Error opening ldr_log.txt");
  }
}

void loop() {
  // Membaca data dari LDR
  int ldrValue = analogRead(LDR_PIN);
  float brightnessPercent = (ldrValue / 4095.0) * 100.0; // Ubah ke persen

  // Membaca waktu dari RTC
  DateTime now = rtc.now();
  char timeString[20];
  sprintf(timeString, "%02d/%02d/%04d %02d:%02d:%02d",
          now.day(), now.month(), now.year(),
          now.hour(), now.minute(), now.second());

  // Menampilkan data ke OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Kecerahan:"));
  display.setTextSize(2);
  display.printf("%.2f%%\n", brightnessPercent);
  display.setTextSize(1);
  display.println(F("Waktu:"));
  display.println(timeString);
  display.display();

  // Menulis data ke microSD
  myFile = SD.open("/ldr_log.txt", FILE_APPEND);
  if (myFile) {
    myFile.printf("%02d/%02d/%04d,%02d:%02d:%02d,%.2f\n",
                  now.day(), now.month(), now.year(),
                  now.hour(), now.minute(), now.second(),
                  brightnessPercent);
    myFile.close();
    Serial.print("Logged value: ");
    Serial.println(brightnessPercent);
  } else {
    Serial.println("Error opening ldr_log.txt");
  }

  // Tunggu 1 detik sebelum membaca lagi
  delay(1000);
}
