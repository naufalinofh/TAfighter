int Distance = 0; // Record the number of steps we've taken 
int stepPin=3;  // connect CLK+ to D3
int dirPin=4; //// connect CW+ to D4. Connect CW- and CLK - to GND Arduino
bool dir = false;
void setup() {

  pinMode(dirPin, OUTPUT);
  
  pinMode(stepPin, OUTPUT);
  
  digitalWrite(dirPin, dir);
  
  digitalWrite(stepPin, LOW);
  Serial.begin(9600); 

}

void loop() {

  digitalWrite(stepPin, HIGH);

  delay(2);

  digitalWrite(stepPin, LOW);

  delay(2);

  Distance = Distance + 1; // record this step // Check to see if we are at the end of our move

// two rotation for 1/dirPin bridge and 1 rotation for 1/6 bridge (for this code)

  if (Distance == 200) { // We are! Reverse direction (invert DIR signal)
    Serial.println("reverse");
    if (digitalRead(dirPin) == LOW) {
    
      digitalWrite(dirPin, HIGH); }
    
    else {
    
      digitalWrite(dirPin, LOW);
    
    } // Reset our distance back to zero since we're // starting a new move
  
    Distance = 0; // Now pause for half a second delay(500);
  }

}
