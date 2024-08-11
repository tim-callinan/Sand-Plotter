// WORKING SAND SIZE X580 Y520
// We added code to back off from the 0,0 once it homes.
// Im after wiring in two extra limit switches at each axis maxes.
// ADDED CREATING A LOG FILE FOR ANY ERRORS. 
// ADDED FASTLED CODE TO MAIN SAND PROGRAM 

#include <FastLED.h>
#include <SD.h>
#include <SPI.h>
#include <AccelStepper.h>

// Define step and direction pins for X and Y stepper motors
#define X_STEP_PIN 2
#define X_DIR_PIN 4
#define Y_STEP_PIN 3
#define Y_DIR_PIN 5
#define X_MIN_LIMIT_PIN 6
#define Y_MIN_LIMIT_PIN 7
#define X_MAX_LIMIT_PIN 16
#define Y_MAX_LIMIT_PIN 15
#define ENABLE_PIN 8
#define CHIP_SELECT 4

// LED STUFF
#define LED_PIN_BOTTOM     17  //PIN OUT FOR BOTTOM LEDS
#define LED_PIN_TOP        18   //PIN OUT FOR TOP LEDS
#define NUM_LEDS_BOTTOM    30  // Corrected number of LEDs for the bottom
#define NUM_LEDS_TOP       176 // Number of LEDs for the top
#define BRIGHTNESS_BOTTOM  100 // Adjust brightness from 0 to 255 for bottom LEDs
#define BRIGHTNESS_TOP     100 // Adjust brightness from 0 to 255 for top LEDs
#define LED_TYPE           WS2812B
#define COLOR_ORDER        GRB

CRGB ledsTop[NUM_LEDS_TOP];
CRGB ledsBottom[NUM_LEDS_BOTTOM];

// Initialize AccelStepper for X and Y stepper motors
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

// Adjust stepsPerMm based on your calibration
const float stepsPerMm = 26.80; //was 53.70 before
const int BACKOFF_DIST = 50; // Distance to back off from limit switches in steps
int totalXTravelInMm;
int totalYTravelInMm;
String fileName;


void setup() {
  Serial.begin(115200);
  while (!Serial) ; // Wait for Serial to initialize
  delay(1000);

  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  stepperX.setPinsInverted(true, false, false);
  pinMode(X_MIN_LIMIT_PIN, INPUT);
  pinMode(Y_MIN_LIMIT_PIN, INPUT);
  pinMode(X_MAX_LIMIT_PIN, INPUT);
  pinMode(Y_MAX_LIMIT_PIN, INPUT);

  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // Enable steppers


  FastLED.addLeds<LED_TYPE, LED_PIN_TOP, COLOR_ORDER>(ledsTop, NUM_LEDS_TOP).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE, LED_PIN_BOTTOM, COLOR_ORDER>(ledsBottom, NUM_LEDS_BOTTOM).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS_TOP); // This will set the global brightness, consider setting individual brightness if needed

  fill_solid(ledsBottom, NUM_LEDS_BOTTOM, CRGB(255, 240, 220)); // Soft White
  fill_solid(ledsTop, NUM_LEDS_TOP, CRGB(255, 240, 220)); // Soft White
  FastLED.show();


}

void loop() 
{
  //chooseAndExecuteLightPattern();
  homeGantry(); // Home the gantry
  doSandPattern(); // Open and execute a random file from the SD card
}

void homeGantry() {
  
  // Assuming stepsPerMm is defined globally or within this function's scope
  stepperX.setMaxSpeed(500); // steps per second
  stepperX.setAcceleration(1500); // steps per second squared

  stepperY.setMaxSpeed(500); // steps per second
  stepperY.setAcceleration(1500); // steps per second squared

  Serial.println("HOMING X AXIS");
  // Home X axis to X_MIN
  while (digitalRead(X_MIN_LIMIT_PIN) == LOW) {
      stepperX.move(-100000); // Move towards X_MIN
      stepperX.run();
    }
    stepperX.stop(); // Stop if X_MIN is triggered
  

  // Back off from X_MIN
  Serial.println("Backing off from X_MIN");
  stepperX.move(BACKOFF_DIST); // Back off from X_MIN
  while(stepperX.distanceToGo() != 0) {
    stepperX.run();
  }
  stepperX.setCurrentPosition(0); // Reset position after backing off from X_MIN

  // Move to X_MAX
  Serial.println("Moving to X_MAX");
  while (digitalRead(X_MAX_LIMIT_PIN) == LOW) {

      stepperX.move(100000); // Move towards X_MAX
      stepperX.run();
    }
    stepperX.stop(); // Stop if X_MAX is triggered
  

  // Back off from X_MAX
  Serial.println("Backing off from X_MAX");
  stepperX.move(-BACKOFF_DIST); // Slightly back off from X_MAX
  while(stepperX.distanceToGo() != 0) {
    stepperX.run();
  }
  // Calculate and store the total travel distance in mm for X axis
  long totalXTravel = stepperX.currentPosition();
  totalXTravelInMm = totalXTravel /stepsPerMm;
  Serial.print("Total X axis travel in mm: ");
  Serial.println(totalXTravelInMm);

  // Homing Y Axis
  Serial.println("HOMING Y AXIS");
  // Home Y axis to Y_MIN
  while (digitalRead(Y_MIN_LIMIT_PIN) == LOW) {
      stepperY.move(-100000); // Move towards Y_MIN
      stepperY.run();
    }
    stepperY.stop(); // Stop if Y_MIN is triggered
  

  // Back off from Y_MIN
  Serial.println("Backing off from Y_MIN");
  stepperY.move(BACKOFF_DIST); // Back off from Y_MIN
  while(stepperY.distanceToGo() != 0) {
    stepperY.run();
  }
  stepperY.setCurrentPosition(0); // Reset position after backing off from Y_MIN

  // Move to Y_MAX
  Serial.println("Moving to Y_MAX");
  while (digitalRead(Y_MAX_LIMIT_PIN) == LOW) {
      stepperY.move(100000); // Move towards Y_MAX
      stepperY.run();
    }
    stepperY.stop(); // Stop if Y_MAX is triggered
  

  // Back off from Y_MAX
  Serial.println("Backing off from Y_MAX");
  stepperY.move(-BACKOFF_DIST); // Slightly back off from Y_MAX
  while(stepperY.distanceToGo() != 0) {
    stepperY.run();
  }
  // Calculate and store the total travel distance in mm for Y axis
  long totalYTravel = stepperY.currentPosition();
  totalYTravelInMm = totalYTravel /stepsPerMm;
  Serial.print("Total Y axis travel in mm: ");
  Serial.println(totalYTravelInMm);


  delay(1000);
}


void doSandPattern() {
  stepperX.setMaxSpeed(750); // steps per second
  stepperX.setAcceleration(1500); // steps per second squared
  stepperY.setMaxSpeed(750); // steps per second
  stepperY.setAcceleration(1500); // steps per second squared

  // Initialize random seed
  randomSeed(analogRead(0));

  // Counting SAND files
  int fileCount = countSandFiles();
  if (fileCount == 0) {
    Serial.println("No SAND files found.");
    return;
  }

  // Generate a random file number between 1 and the total number of SAND files
  int fileIndex = random(1, fileCount + 1);
  fileName = "SAND" + String(fileIndex) + ".txt";

  Serial.println("Executing file: " + fileName);
  File file = SD.open(fileName);
  if (!file) {
    Serial.println("Failed to open file: " + fileName);
    return;
  }

  processFile(file);
  file.close();
}

int countSandFiles() {
  File root = SD.open("/");
  int count = 0;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    String fileName = entry.name();
    if (fileName.startsWith("SAND")) {
      count++;
    }
    entry.close();
  }
  root.close();
  return count;
}


void processFile(File &file) {
    while (file.available()) {
        String line = file.readStringUntil('\n');
        processLine(line);
    }
    
}


void processLine(String line) {
    // Look for the start indexes of X and Y coordinates
    int xIndex = line.indexOf('X') + 1; // Start reading right after 'X'
    int yIndex = line.indexOf('Y') + 1; // Start reading right after 'Y'

    if (xIndex > 0 && yIndex > 0) {
        // Extract the coordinate substrings
        String xStr = line.substring(xIndex, line.indexOf(' ', xIndex));
        String yStr = line.substring(yIndex, line.length());

        // Convert the coordinate strings to integers
        int xCoord = xStr.toInt();
        int yCoord = yStr.toInt();

        // Move the gantry to the extracted coordinates
        moveToPosition(xCoord, yCoord);
    }
}

void moveToPosition(int x, int y) {
    int targetX = constrain(x, 0, totalXTravelInMm); // Maximum X bound BASED ON HOMING CALC
    int targetY = constrain(y, 0, totalYTravelInMm); // Maximum Y bound BASED ON HOMING CALC

    Serial.println("Moving to X" + String(targetX) + " Y" + String(targetY));
    stepperX.moveTo(targetX * stepsPerMm);
    stepperY.moveTo(targetY * stepsPerMm);
   
    while (stepperX.distanceToGo() != 0 || stepperY.distanceToGo() != 0) {
        stepperX.run();
        stepperY.run();

        // Check if X limit switch is triggered during operation
        if (digitalRead(X_MIN_LIMIT_PIN) == HIGH) {
            Serial.println("Error: X_MIN limit switch activated during operation");
            logError("Error: X_MIN limit switch activated during operation");
            stepperX.stop();
            stepperY.stop();
            // Engage an indefinite loop for the alert
            while (true) {}
        }
        if (digitalRead(X_MAX_LIMIT_PIN) == HIGH) {
            Serial.println("Error: X_MAX limit switch activated during operation");
            logError("Error: X_MAX limit switch activated during operation");
            stepperX.stop();
            stepperY.stop();
            // Engage an indefinite loop for the alert
            while (true) {}
        }
       if (digitalRead(Y_MIN_LIMIT_PIN) == HIGH) {
            Serial.println("Error: Y_MIN limit switch activated during operation");
            logError("Error: Y_MIN limit switch activated during operation");
            stepperX.stop();
            stepperY.stop();
            // Engage an indefinite loop for the alert
            while (true) {}
        }
        if (digitalRead(Y_MAX_LIMIT_PIN) == HIGH) {
            Serial.println("Error: Y_MAX limit switch activated during operation");
            logError("Error: Y_MAX limit switch activated during operation");
            stepperX.stop();
            stepperY.stop();
            // Engage an indefinite loop for the alert
            while (true) {}
        }

    }
}

/*
void chooseAndExecuteLightPattern() {
  int patternIndex = random(1, 5); // Generates a random number between 1 and 4

  switch (patternIndex) {
    case 1:
      lightPattern1();
      break;
    case 2:
      lightPattern2();
      break;
    case 3:
      lightPattern3();
      break;
    case 4:
      lightPattern4();
      break;
  }
}

void lightPattern1() {
  // Example: Fade in and out with blue
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
  }
  FastLED.show();
  fadeToBlackBy(leds, NUM_LEDS, 10); // Adjust fade amount as needed
}

void lightPattern2() {
  // Example: Green lights running
  static int pos = 0;
  fill_solid(leds, NUM_LEDS, CRGB::Black); // Turn all LEDs off
  leds[pos] = CRGB::Green;
  FastLED.show();
  if (++pos >= NUM_LEDS) pos = 0;
}

void lightPattern3() {
  // Example: Random color burst
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random8(), 255, 255); // Random hue, full saturation and value
  }
  FastLED.show();
}

void lightPattern4() {
  // Example: Ocean colors
  fill_gradient_RGB(leds, 0, CRGB::Blue, NUM_LEDS / 2, CRGB::Green);
  fill_gradient_RGB(leds, NUM_LEDS / 2, CRGB::Green, NUM_LEDS - 1, CRGB::Blue);
  FastLED.show();
}
*/

void logError(const String& message) {
  // Open the log file in append mode
  File logFile = SD.open("error.txt", FILE_WRITE);
  if (logFile) {
    logFile.println(message); // Write the error message
    logFile.close(); // Close the file to save the content
  } else {
    // If the file didn't open, print an error to Serial (optional)
    Serial.println("Failed to open log file for writing");
  }
}


