//vice versa like normal
#define leftMotorOutput1 2
#define leftMotorOutput2 4
#define leftMotorPWM 3
#define rightMotorOutput1 5
#define rightMotorOutput2 7
#define rightMotorPWM 6

int sensorsNumber = 0; //how many sensors are on the lane
float sensorsValues[4] = {0, 0, 0, 0}; //analog read
float convertedSensorsValues[4] = {0, 0, 0, 0}; //analog read converted to digital
float motorSpeed = 255; 
float kp = 64;
float kd = 32;
float error = 0; //different between wanted robot position (0) and current position
float currentPosition = 0;
float change = 0; //change motor speed 
float derivative = 0; //error - previous error
float prevError= 0; //previous error
float leftMotorSpeed = 0; 
float rightMotorSpeed = 0;
int helper[2] = {0, 0};

String readData;
String state = "on";
String dcSpeed;
String kpStr;
String kdStr;

void createMotors();
void setValues();
void readSensorsValues(); //sensors AO[4] = {0,1,2,3}
void convertSensorsValues();
void readCurrentPosition();
void calculateChange();
void setMotorsSpeed();
void showValues();

void setup()
{
  Serial.begin(9600);
  createMotors();
}

void loop() 
{ 
  if (Serial.available() > 0) readData = Serial.readStringUntil('\n');
  {
    if(readData == "on" || readData == "off") state = readData;
    else if(readData[0] == '&') setValues();
  }
  
  readSensorsValues(); //get analog values
  convertSensorsValues(); //convert analog values to digital
  readCurrentPosition(); //get current robot position in realtion to line
  calculateChange(); //calculate motors change values

  if(sensorsNumber == 0)
  { 
    if(helper[0] == 1)
    {
       rightMotorSpeed = motorSpeed;
       leftMotorSpeed = 0;
    }
    else if(helper[1] == 1)
    {
       leftMotorSpeed = motorSpeed;
       rightMotorSpeed = 0;
    }
  }
  else setMotorsSpeed();

  if(state == "on")
  {
    analogWrite(leftMotorPWM, rightMotorSpeed);
    analogWrite(rightMotorPWM, leftMotorSpeed);
  }
  else if(state == "off")
  {
    analogWrite(leftMotorPWM, 0);
    analogWrite(rightMotorPWM, 0);
  }
}

void createMotors()
{
  pinMode(leftMotorOutput1, OUTPUT);
  pinMode(leftMotorOutput2, OUTPUT);
  pinMode(leftMotorPWM, OUTPUT);
  pinMode(rightMotorOutput1, OUTPUT);
  pinMode(rightMotorOutput2, OUTPUT);
  pinMode(rightMotorPWM, OUTPUT);
  digitalWrite(leftMotorOutput1, LOW); 
  digitalWrite(leftMotorOutput2, HIGH); 
  analogWrite(leftMotorPWM, 0);
  digitalWrite(rightMotorOutput1, LOW); 
  digitalWrite(rightMotorOutput2, HIGH); 
  analogWrite(rightMotorPWM, 0);
}

void setValues()
{
  int i=1;
  dcSpeed = "";
  kpStr = "";
  kdStr = "";
  
  while(readData[i] != '&')
  {
    dcSpeed += readData[i];
    i++;
  }
    
  i++;
    
  while(readData[i] != '&')
  {
    kpStr += readData[i];
    i++;
  }
  
  i++;
    
  while(readData[i] != '&')
  {
    kdStr += readData[i];
    i++;
  }

  motorSpeed = dcSpeed.toInt();
  kp = kpStr.toInt();
  kd = kdStr.toInt(); 
}

void readSensorsValues()
{
  for(int i=0; i < 4; i++)
    sensorsValues[i] = analogRead(i);
}

void convertSensorsValues()
{
  for(int i=0; i < 4; i++)
  {
    if(sensorsValues[i] < 400) 
      convertedSensorsValues[i] = 0; 
    else convertedSensorsValues[i] = 1;
  }
}

void readCurrentPosition()
{
  sensorsNumber = 0; //how many sensors are on the lane
  
  for(int i = 0; i < 4; i++)
    if(convertedSensorsValues[i] == 1) sensorsNumber++;
    
  currentPosition = (-2 * convertedSensorsValues[0] - convertedSensorsValues[1] + convertedSensorsValues[2] + 2 * convertedSensorsValues[3]); //sensors weight (-2, -1, 1, 2)

  if(sensorsNumber > 0)
    currentPosition /= sensorsNumber;

  if(currentPosition == -2)
  {
    helper[0] = 1;
    helper[1] = 0;
  }
  else if(currentPosition == 2) 
  {
    helper[0] = 0;
    helper[1] = 1; 
  }
  else if(sensorsNumber != 0)
  {
    helper[0] = 0;
    helper[1] = 0; 
  }
}

void calculateChange()
{
  error = 0 - currentPosition; // 0 is my wanted position
  derivative = error - prevError;
  prevError = error;
  change = kp * error + derivative * kd;  
}

void setMotorsSpeed()
{
  leftMotorSpeed = motorSpeed - change;
  rightMotorSpeed = motorSpeed + change;

  if(leftMotorSpeed > motorSpeed) 
    leftMotorSpeed = motorSpeed;
  else if(leftMotorSpeed < 0) 
    leftMotorSpeed = 0;
    
  if(rightMotorSpeed >= motorSpeed) 
    rightMotorSpeed = motorSpeed;
  else if(rightMotorSpeed <= 0)
    rightMotorSpeed = 0;
}

void showValues()
{
  Serial.print("Sensors values: ");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(sensorsValues[i]);
    Serial.print(' ');
  }
  Serial.println();
  
  Serial.print("Converted sensors values: ");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(convertedSensorsValues[i]);
    Serial.print(' ');
  }
  Serial.println();
  
  Serial.print("Current position: ");
  Serial.println(currentPosition);
  
  Serial.print("Error: ");
  Serial.println(error);
  
  Serial.print("Change: ");
  Serial.println(change);

  Serial.print("Sensors number: ");
  Serial.println(sensorsNumber);
  
  Serial.print("Helper: ");
  Serial.print(helper[0]);
  Serial.print(' ');
  Serial.println(helper[1]);
  
  Serial.print("Left: ");
  Serial.println(leftMotorSpeed);
  
  Serial.print("Right: ");
  Serial.println(rightMotorSpeed);
}
