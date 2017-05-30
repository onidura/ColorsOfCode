  #include <MFRC522.h>
  #include <SPI.h>

  #define SS_PIN 10
  #define RST_PIN 9
 
  MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
  MFRC522::MIFARE_Key key; 

  /*** SOUND ***/
  int sound = 6;
  
  // Arrays containing audio frequencies, used to make sound with the piezo.
  double startSound[]= {164.81, 196.00, 329.63, 261.63, 293.66, 392.00};
  double registeredCardSound[] = {1975.53, 2637.02};
  boolean startSoundHasBeenPlayed = false;

  /*** RFID ***/
  // The UID's of the known RFID tags, index 0 = task 1 etc.
  String knownUIDs[] = {"174 191 49 181", "94 191 49 181", "14 191 49 181", "190 190 49 181"};
  int numberOfKnownUIDs = 4;
  
  // Array that holds the right answers, will be iterated through at the same time as the array of knownUIDs to register the right answer.
  String correctAnswers[] = {"yellow", "green", "yellow", "red"};

  /*** CHECK VARIABLES ***/
  String uidString;
  boolean known = false;
  String lastKnownUID = "";
  boolean cardRegistered = false;
  String correctAnswer;

  /*** LED ***/
  #define RED_LED 7
  #define GREEN_LED 8
  #define BLUE_LED A0

  /*** BUTTONS ***/
  const int redButton = 4;
  const int greenButton = 2;
  const int blueButton = 3;
  const int yellowButton = 5;

  const String red = "red";
  const String green = "green";
  const String blue = "blue";
  const String yellow = "yellow";

  int buttonStateRed = 0;
  int buttonStateGreen = 0;
  int buttonStateBlue = 0;
  int buttonStateYellow = 0;
 
  int pressed = false;
  const int off = 0;
  const int on = 255;
  
  /*** MILLIS ***/
  int feetTime = 0;
  int checkingTime = (feetTime + 3000) ;
  unsigned long previousMillis = 0;
  unsigned long currentMillis = 0;
  int lengthOfFeedback = 5000;
  int pressedDelay = 150;
  int shortFlash = 100;
  int platetime = 125;

  void setup() {
    Serial.begin(9600);
    // Init SPI bus
    SPI.begin();
    
    // Init MFRC522  
    rfid.PCD_Init();
    
    Serial.println("Setup completed!");

    pinMode(redButton, INPUT);
    pinMode(greenButton,INPUT);
    pinMode(yellowButton,INPUT);
    pinMode(blueButton,INPUT);

    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
  }


  /*** MAINLOOP ***/
  void loop() {
    
    if(!startSoundHasBeenPlayed) {
      playStartSound();
    }
    
    if(!cardRegistered){
      // If card is scanned, read card.
      if (rfid.PICC_IsNewCardPresent()){
        readRFID();
        playCardRegisteredSound();
        cardRegistered = true;
        ligthFlash();
        
      } 
      delay(600);
      known = false;
    }

    // Is card registered?
    if(cardRegistered){
      // Checking button status
      buttonStateRed = digitalRead(redButton);
      buttonStateGreen = digitalRead(greenButton);
      buttonStateYellow = digitalRead(yellowButton);
      buttonStateBlue = digitalRead(blueButton);
        
      if(buttonStateRed == HIGH) {   
        Serial.println("Red is pressed");
        checkPlate(red);    
      } 
      if(buttonStateGreen == HIGH) {
        //Serial.println("Green is pressed");
        checkPlate(green);
      }
      if(buttonStateBlue == HIGH) {
        //Serial.println("Blue is pressed");
        checkPlate(blue);
      }
      if(buttonStateYellow == HIGH) {
        //Serial.println("Yellow is pressed");
        checkPlate(yellow);
      }
      if(buttonStateRed == LOW && buttonStateGreen== LOW && buttonStateYellow == LOW && buttonStateBlue == LOW ){
        //Serial.println("Nothing is pressed");
        currentMillis = 0;
        pressed = false;
      }
    }
  } 

  /****RFID FUNCTIONS****/
  void readRFID(){
    // Reads the tags UID and saves this in the RFID object. 
    // The funksjon from the MFRC522 library stores this in an array calles uidByte[].
     rfid.PICC_ReadCardSerial();

   // Serial.println("\nScanned tag's UID:"); 

   // Gets the UID of scanned card from the rfid object. 
    uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " + String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);
    
   // Serial.print(uidString); 
    Serial.println("Scanning tag");

    // If scanned card has already been scanned, return to mainloop. If not, carry on.
    if(hasCardBeenRead(uidString)) {
        delay(500);
        return;
    }
    
    lastKnownUID = uidString;
  
    // Is the card known?
    known = isCardKnown(uidString);
    if(known){
      Serial.println("\nCard is known!");
      //playCardRegisteredSound();
      
    } else {
      Serial.println("\nCard is unknown");
    }
  }

  // Returns status of incoming UID, to find out if the card already has been scanned.
   boolean hasCardBeenRead(String uidString) {
   return lastKnownUID.equals(uidString);
  }

  // Function for checking if incoming UID is known.
  boolean isCardKnown(String uidString) {
    String knownUID = "";
    int oppgaveNr = 0;
    for(int i = 0; i < numberOfKnownUIDs; i++) {
       knownUID = String(knownUIDs[i]);
       if(knownUID.equals(uidString)) {
        correctAnswer = correctAnswers[i];
        return true;
       }
    }
    return false;  
  }

  /*** FUNCTIONS FOR SOUND ***/
  void playCardRegisteredSound() {
    for(int i = 0; i < (sizeof(registeredCardSound) / sizeof(double)); i++){
      tone(sound, registeredCardSound[i]);
      delay(200);
    }
    noTone(sound);
  }

  void playStartSound() {
    for(int i = 0; i < (sizeof(startSound) / sizeof(double)); i++){
      tone(sound, startSound[i]);
      delay(300);
    }
    noTone(sound);
    startSoundHasBeenPlayed = true;
}  
  
  /*** FUNCTIONS FOR BUTTONS ***/
  // Checking if plate is pressed
  void checkPlate(String plate){
    if(!pressed){
      currentMillis = millis();
      pressed = true;
    }
    
    if ((millis()- currentMillis)>= feetTime) {
    ligthPlateIsPressed();
    }
    
    if ((millis()- currentMillis)>= checkingTime) {
      pressed = false;
      Serial.println("Checking answer");
      checkAnswer(plate, correctAnswer);
      previousMillis = currentMillis;
    }
  }
   
  // Function for checking incoming answer againt the correct answer.
  void checkAnswer(String answer, String correctAnswer){
    if(correctAnswer.equals(answer)){
      //Serial.println("Correct answer");
      cardRegistered = false;
      correct();
    } else{
      //Serial.println("Wrong answer");
      wrongAnswer();
      delay(300);
    }
  }

  // Functions for activating different the lights in LED-strip
  void correct(){
    ligthAnswerFeedback(off, on);
  }
  void wrongAnswer(){
    ligthAnswerFeedback(on ,off);
  }


   /*** FUNCTIONS FOR LEDS ***/
  void setLEDStripColor(int red, int green, int blue){
    analogWrite(RED_LED, red);
    analogWrite(GREEN_LED, green);
    analogWrite(BLUE_LED, blue); 
  } 

  void ligthPlateIsPressed(){    
    setLEDStripColor(off, off, on);
    delay(platetime);
    setLEDStripColor(on, off, off);
    delay(platetime);    
    setLEDStripColor(off, on, off);  
    delay(platetime);
    setLEDStripColor(off, off, off);
  }

  void ligthFlash(){
    int i = 0;
    while(i < 3) {
      setLEDStripColor(on, on, on);
      delay(shortFlash);
      setLEDStripColor(off, off, off);
      delay(shortFlash);
      i++;
    }  
  }  
 
  void ligthAnswerFeedback(int red, int green){
    delay(250);
    ligthFlash();
    delay(250);
    setLEDStripColor(red, green, off);
    delay(lengthOfFeedback);
    setLEDStripColor(off, off, off);
   }
