#include <MFRC522.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <RTClib.h> // Include RTC library
#include <SD.h> // for the SD card

#define ledPin 5
#define buzzerPin 3

#define SS_PIN 10 // RFID
#define RST_PIN 9 // RFID

#define TFT_CS 8
#define TFT_RST 7
#define TFT_DC 6

#define CS_SD 4 


#define BLACK   0x0000
#define GREEN   0x07E0
#define CYAN    0x07FF
#define YELLOW  0xFFE0  
#define RED     0xF800
#define BLUE    0x0000FF
#define WHITE   0xFFFF



File myFile;

// Initialize the TFT display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Initialize the RFID reader
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialize the RTC module
RTC_DS3231 rtc;

// Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
MFRC522::MIFARE_Key key;

// Store the tag ID from the RFID card
String tagID = "";

//*****************************************************************************************//
void setup() {
  SPI.begin();
  Serial.begin(9600);

  // Initialize the TFT display
  tft.initR(INITR_BLACKTAB);
  
  tft.fillScreen(BLACK);  // Set the background color to BLACK

  // Initialize the RFID reader
  mfrc522.PCD_Init();

  // Setup for the SD card
  Serial.print("Initializing SD card...");
  if(!SD.begin(CS_SD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  // Initialize the RTC module
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set RTC to current time if it lost power
  }

  // Set all key bytes to 0xFF (default key)
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  

  // Display welcome message
  printMessage(">>Scan your ID<<");
  pinMode(ledPin,OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  
}

//*****************************************************************************************//
void loop() {
  digitalWrite(buzzerPin, HIGH); // !!Iitializing of buzzer to e turned off!!
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println(F("**Card Detected:**"));
    
     

    //------------------------------------------- GET FIRST NAME

    // Get the first name from block 4
    String firstName = getFirstName(mfrc522, key, 4);
    if (firstName != "") {
      // Proceed if the first name is successfully read
      Serial.println(firstName);
      // Continue with your logic (e.g., displaying the first name on TFT, logging, etc.)
    } else {
      Serial.println("Failed to retrieve first name.");
    }

    Serial.println("!!Led turned on!!");
    // Turn on the buzzer and LED when a card is successfully read
    digitalWrite(buzzerPin, LOW);  // Turn on the buzzer
    digitalWrite(ledPin, HIGH); 
    delay(100);                     // Wait for 300 milliseconds
    digitalWrite(buzzerPin, HIGH);   // Turn off the buzzer
    
    
    digitalWrite(ledPin, HIGH);     // Turn on the LED
    // Display the first name on the TFT as "CUID"
    tft.fillScreen(BLACK);  // Set the background color to BLACK
    tft.setTextSize(1.8);
    tft.setTextColor(RED);
    tft.setCursor(15, 60);
    tft.println("CUID:");
    tft.setTextSize(1.7);
    tft.setTextColor(RED);
    tft.setCursor(50, 60);
    tft.println(firstName);  // Display the first name in the CUID field
    Serial.println(firstName);

    // Get the current date and time from the RTC module
    DateTime now = rtc.now();

    // Display the date and time
    tft.setTextSize(1.7);
    tft.setTextColor(YELLOW);
    tft.setCursor(15, 70);
    tft.print("DATE: ");
    tft.print(now.day(), DEC);
    tft.print('/');
    tft.print(now.month(), DEC);
    tft.print('/');
    tft.print(now.year(), DEC);
    tft.setTextColor(WHITE);
    tft.setCursor(15, 80);
    tft.print("TIME: ");
    tft.print(now.hour(), DEC);
    tft.print(':');
    tft.print(now.minute(), DEC);
    tft.print(':');
    tft.print(now.second(), DEC);

    // Convert DateTime to string
    String dateTimeStr = String(now.year()) + "/" +
                        String(now.month()) + "/" +
                        String(now.day()) + " " +
                        String(now.hour()) + ":" +
                        String(now.minute()) + ":" +
                        String(now.second());

// Concatenate the firstName with the dateTime string
    String combinedStr = firstName + ", " + dateTimeStr;
    logcard(firstName, dateTimeStr);

    delay(2000);
    tft.fillScreen(BLACK);  // Set the background color to BLUE
    tft.setTextSize(1);
    tft.setTextColor(RED);
    tft.setCursor(25, 50);
    tft.println("Developed By");
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.setCursor(15, 70);
    tft.println("Dr SOMNATH SINHA");
    tft.setCursor(15, 80);
    tft.println("SANTHOSH 2347148");
    tft.setCursor(15, 90);
    tft.println("HITESH 2347118");
    delay(2000);

    // Prepare to read another card
    Serial.println("!!Led turned off!!");
    digitalWrite(ledPin,LOW);
    printMessage(">>Scan your ID<<");
  }
}

//*****************************************************************************************//
void printMessage(String message) {
  tft.fillScreen(BLACK);  // Set the background color to BLACK
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.setCursor(30, 5);
  tft.println("CHRIST");
  tft.setCursor(5, 25);
  tft.println("UNIVERSITY");  
  tft.setTextSize(1.7);
  tft.setTextColor(YELLOW);
  tft.setCursor(14, 85);
  tft.println(message);
}


void logcard(String firstname, String time){
  
  digitalWrite(CS_SD, LOW);

  myFile = SD.open("DATA.txt", FILE_WRITE);

  if (myFile){
    Serial.println("File opened ok");
    myFile.print(firstname);
    myFile.print(", ");  // Add a separator (comma or space) between name and time
    myFile.println(time);

    Serial.println("Done writing reg no. and time as");
    Serial.println(firstname);
    Serial.println(time);
  

    //closing
    myFile.close();

  }
  else {
    Serial.println("error opening data.txt");  
  }
  // Disables SD card chip select pin  
  digitalWrite(CS_SD,HIGH);

}

String getFirstName(MFRC522 &mfrc522, MFRC522::MIFARE_Key &key, byte block) {
  byte len = 18;
  byte buffer[18];
  MFRC522::StatusCode status;

  // Authenticate to access the block
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return "";  // Return an empty string on authentication failure
  }

  // Read the first name from the specified block
  status = mfrc522.MIFARE_Read(block, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return "";  // Return an empty string on read failure
  }

  // Extract only numeric characters from the buffer
  String firstName = "";
  for (uint8_t i = 0; i < 16; i++) {
    if (isDigit(buffer[i])) {  // Check if the character is a digit
      firstName += (char)buffer[i];
    }
  }

  mfrc522.PICC_HaltA(); // Halt the current card
  mfrc522.PCD_StopCrypto1(); // Stop encryption

  return firstName;  // Return the numeric string
}

