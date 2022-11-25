// Viktig informasjon
//  Cant use pin 1 and 2
//  vip kan ikke være under 10grader (det er flatt)

#include <Servo.h>

Servo LeftMotor;
Servo RightMotor;
Servo ClawServo;
Servo VipServo;

//--------------------------------------------Variables--------------------------------------------------

//-------------------------Motor-------------------------

// Motor_stop +- degree of speed will give desired speed in desired direction
//  (0-89:backwards, 90:stand still, 91-180:forward)

int motor_stop = 90;
int fast_speed = 40;
int normal_speed = 20;
int slow_speed = 15;
int ultra_slow_speed = 10;

//-------------------------Claw--------------------------
int claw_max = 105; // Claw closed
int claw_min = 10;  // Claw opened
int claw_pos = 10;  // Claw Start

//-------------------------Vip---------------------------
int vip_max = 80; // Vip position straight up
int vip_min = 10; // Vip position down
int vip_mid = 35; // Vip position lift
int vip_pos = 10; // Vip position start

//-------------------------IR----------------------------
int left_sensor_state;
int right_sensor_state;

//-------------------------UL----------------------------
long duration;
int distance = 100;

//-------------------------Navigation--------------------
bool intersection = false;

//-------------------------Searching---------------------
int rotation_90n = 1200;
int rotation_45n = 600;
int rotation_30n = 400;

// LV1
int cup_side_right = 2;  // Inititated as 2, changed to 1 if cup is on right or 0 if cup is on left. Stay as 2 if no cup is found.
int cup_limit = 35;      // How far a cup will be on LV1
int pickup_distance = 4; // How close a cup must be to start picking it up

// LV4
int cup_found = 46;          // How close a cup will we if there exits a cup on LV4
int LV4_pickup_distance = 5; // How close a cup must be to start picking it up

// LV1 scanning rotation variables
int rotation_amount = 8;
int rotation_time = 90;

// LV4 scanning rotation variables
int LV4_initial_rotation_time = 480;
int LV4_rotation_amount = 19;
int LV4_extra_rotations = 5;
int LV4_rotation_time = 100;
int cup_rotations = 0; // How many times the car has rotated
int cup_found_rotations = 0;
int LV4_exit_search_area_rotation = 200;

//-----------------------------------------Pin Connections-----------------------------------------------

const int motor_right_pin = 12;
const int motor_left_pin = 11;

const int claw_servo_pin = 9;
const int vip_servo_pin = 8;

const int left_ir_pin = 5;
const int right_ir_pin = 4;

const int distance_trig_pin = 7;
const int distance_echo_pin = 6;

//-----------------------------------------Driving functions---------------------------------------------

//----------Basic_Functions----------------
void Right(int motor_speed)
{
  RightMotor.write(motor_speed);
}

void Left(int motor_speed)
{
  LeftMotor.write(motor_speed);
}

void RightForward(int motor_speed = normal_speed)
{
  Right(motor_stop - motor_speed);
}

void LeftForward(int motor_speed = normal_speed)
{
  Left(motor_stop + motor_speed);
}

void RightBackward(int motor_speed = normal_speed)
{
  Right(motor_stop + motor_speed + 5);
}

void LeftBackward(int motor_speed = normal_speed)
{
  Left(motor_stop - motor_speed);
}

void RightStop()
{
  Right(motor_stop);
}

void LeftStop()
{
  Left(motor_stop);
}

//------------Basic_Driving----------------
void Forward(int motor_speed = normal_speed)
{
  RightForward(motor_speed);
  LeftForward(motor_speed);
}

void Backward(int motor_speed = normal_speed)
{
  RightBackward(motor_speed);
  LeftBackward(motor_speed);
}

void Brake()
{
  RightStop();
  LeftStop();
}

//---------------Turning-------------------
// Used for Following line only
void LeftTurn(int motor_speed = normal_speed)
{
  RightForward(motor_speed);
  LeftBackward(slow_speed);
}

void RightTurn(int motor_speed = normal_speed)
{
  RightBackward(slow_speed);
  LeftForward(motor_speed);
}

void LeftTurnBackward(int motor_speed = normal_speed)
{
  RightForward(slow_speed);
  LeftBackward(motor_speed);
}

void RightTurnBackward(int motor_speed = normal_speed)
{
  RightBackward(motor_speed);
  LeftForward(slow_speed);
}

//----------------Rotation-----------------
void RightRotation(int motor_speed = normal_speed)
{
  RightBackward(motor_speed);
  LeftForward(motor_speed);
}

void LeftRotation(int motor_speed = normal_speed)
{
  RightForward(motor_speed);
  LeftBackward(motor_speed);
}

void RightRotation90n(int turns = 1)
{
  RightRotation(slow_speed);
  delay(turns * rotation_90n);
  Brake();
}

void RightRotation45n(int turns = 1)
{
  RightRotation(slow_speed);
  delay(turns * rotation_45n);
  Brake();
}

void RightRotation30n(int turns = 1)
{
  RightRotation(slow_speed);
  delay(turns * rotation_30n);
  Brake();
}
void LeftRotation90n(int turns = 1)
{
  LeftRotation(slow_speed);
  delay(turns * (rotation_90n - 90));
  Brake();
}

void LeftRotation45n(int turns = 1)
{
  LeftRotation(slow_speed);
  delay(turns * (rotation_45n - 45));
  Brake();
}

void LeftRotation30n(int turns = 1)
{
  LeftRotation(slow_speed);
  delay(turns * (rotation_30n - 30));
  Brake();
}

void LeftRotationForTime(int t, int motor_speed = normal_speed)
{
  LeftRotation(motor_speed); // Rotate for rotation_time amount of time
  delay(t);
  Brake();
}

void RightRotationForTime(int t, int motor_speed = normal_speed)
{
  RightRotation(motor_speed); // Rotate for rotation_time amount of time
  delay(t);
  Brake();
}

//------------Intersections----------------

void CrossIntersection(int t = 200)
{
  Forward();
  delay(t);
  Brake();
  intersection = false;
}

void CrossIntersectionBackward(int t = 200)
{
  Backward();
  delay(t);
  Brake();
  intersection = false;
}

void LeftTurnInIntersection()
{
  CrossIntersection();
  LeftRotation90n();
  CrossIntersection();
}

void RightTurnInIntersection()
{
  CrossIntersection();
  RightRotation90n();
  CrossIntersection();
}

//-----------------------------------------Claw Functions------------------------------------------------

//------------------Claw-------------------

void Grip()
{
  for (claw_pos; claw_pos <= claw_max; claw_pos += 1)
  {                            // goes to claw_max degrees in steps of 1 degree
    ClawServo.write(claw_pos); // tell servo to go to position in variable 'pos'
    delay(15);                 // waits 15 ms for the servo to reach the position
  }
}

void Release()
{
  for (claw_pos; claw_pos >= claw_min; claw_pos -= 1)
  {                            // goes to claw_min degrees in steps of 1 degree
    ClawServo.write(claw_pos); // tell servo to go to position in variable 'pos'
    delay(15);                 // waits 15 ms for the servo to reach the position
  }
}

//------------------Vip--------------------

void VipUp()
{
  for (vip_pos; vip_pos < vip_max; vip_pos += 1)
  {                          // goes to claw_max degrees in steps of 1 degree
    VipServo.write(vip_pos); // tell servo to go to position in variable 'pos'
    delay(15);               // waits 15 ms for the servo to reach the position
  }
}

void VipDown()
{
  for (vip_pos; vip_pos >= vip_min; vip_pos -= 1)
  {                          // goes to claw_min degrees in steps of 1 degree
    VipServo.write(vip_pos); // tell servo to go to position in variable 'pos'
    delay(15);               // waits 15 ms for the servo to reach the position
  }
}

void VipLift()
{
  for (vip_pos; vip_pos <= vip_mid; vip_pos += 1)
  {                          // goes to claw_max degrees in steps of 1 degree
    VipServo.write(vip_pos); // tell servo to go to position in variable 'pos'
    delay(15);               // waits 15 ms for the servo to reach the position
  }
}

void LiftCup()
{

  Grip();
  delay(1000);
  VipLift();
  delay(1000);
}

void LowerCup()
{
  VipDown();
  delay(1000);
  Release();
  delay(1000);
}

//-----------------------------------------IRSensor Functions--------------------------------------------

void DigitalRead()
{
  left_sensor_state = digitalRead(left_ir_pin);
  right_sensor_state = digitalRead(right_ir_pin);
}

void AnalogRead()
{
  left_sensor_state = analogRead(left_ir_pin);
  right_sensor_state = analogRead(right_ir_pin);
}

void PrintState()
{
  Serial.println(right_sensor_state);
  Serial.println(left_sensor_state);
  Serial.println("\n");
}

//-----------------------------------------US-Sensor Functions-------------------------------------------

int ReadDistance()
{
  // Clears the trigPin
  digitalWrite(distance_trig_pin, LOW);
  delayMicroseconds(2);

  // Sets the distance_trig_pin on HIGH state for 10 micro seconds
  digitalWrite(distance_trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distance_trig_pin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(distance_echo_pin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;

  // Serial.println(distance);

  return distance;
}

//-----------------------------------------Smart Functions-----------------------------------------------

//----------Follow Line Forward------------
void FollowLine(int motor_speed = normal_speed)
{
  DigitalRead();
  // PrintState();

  if (left_sensor_state == 0 & right_sensor_state == 0)
  {
    Forward(motor_speed);
  }
  else if (left_sensor_state == 1 & right_sensor_state == 1)
  {
    Brake();
    intersection = true;
  }
  else if (right_sensor_state == 1)
  {
    RightTurn(motor_speed);
  }
  else if (left_sensor_state == 1)
  {
    LeftTurn(motor_speed);
  }
}

void FollowLineUntilIntersection(int motor_speed = normal_speed)
{
  while (intersection == false)
  {
    FollowLine(motor_speed);
  }
}

void FollowLineToCup(int motor_speed = slow_speed)
{
  while (ReadDistance() > pickup_distance)
  {
    FollowLine(motor_speed);
  }
  Brake();
}

void FollowLineIgnoreIntersection(int n)
{
  for (int i = 0; i < n; i++)
  {
    FollowLine();
    delay(10);
    if (intersection == true)
    {
      CrossIntersection(100);
    }
  }
  Brake();
  delay(200);
}

//----------Follow Line Backward-----------
void FollowLineBackward(int motor_speed = normal_speed)
{

  DigitalRead();
  // PrintState();

  if (left_sensor_state == 0 & right_sensor_state == 0)
  {
    Backward(motor_speed);
  }
  else if (left_sensor_state == 1 & right_sensor_state == 1)
  {
    Brake();
    intersection = true;
  }
  else if (left_sensor_state == 1)
  {
    RightTurnBackward(motor_speed);
  }
  else if (right_sensor_state == 1)
  {
    LeftTurnBackward(motor_speed);
  }
}

void FollowLineBackwardsUntilIntersection(int motor_speed = normal_speed)
{
  while (intersection == false)
  {
    FollowLineBackward(motor_speed);
  }
}

//--------------LV1 Functions--------------

void LV1_Scan()
{
  RightRotation45n();
  if (ReadDistance() < cup_limit)
  {                     // Checks if cup is found
    cup_side_right = 1; // Saves that cup was found on right
  }
  delay(1000);
  LeftRotation90n();
  if (ReadDistance() < cup_limit)
  {                     // Checks if cup is found
    cup_side_right = 0; // Saves that cup was found on right
  }
  delay(1000);
  RightRotation45n();
  delay(1000);
}

void MoveCup(int cup_side_right)
{

  // Takes right or left turn in intersection
  if (cup_side_right == 1)
  {
    RightTurnInIntersection();
  }
  else if (cup_side_right == 0)
  {
    LeftTurnInIntersection();
  }

  // Follows line until 5 cm from cup
  FollowLineToCup(ultra_slow_speed);

  // Picks up cup
  LiftCup();

  // Rotates to leave line at angle towards lv 2
  if (cup_side_right == 1)
  {
    LeftRotationForTime(450, slow_speed);
  }
  else if (cup_side_right == 0)
  {
    RightRotationForTime(450, slow_speed);
  }

  // Back up a to leave line
  for (int i = 0; i < 70; i++)
  { // Følger linjen en liten stund
    Backward();
    delay(10);
  }
  Brake();

  // Continues backward until it hits a line and then straightens up when it hits the line
  intersection = false;
  FollowLineBackwardsUntilIntersection(ultra_slow_speed);
  CrossIntersection(150);

  // Rotates to face intersection
  if (cup_side_right == 1)
  {
    LeftRotation90n();
  }
  else if (cup_side_right == 0)
  {
    RightRotation90n();
  }

  // Follows line until intersection
  //FollowLineIgnoreIntersection(100);
  intersection = false;
  FollowLineUntilIntersection();

  if (cup_side_right == 1)
  {
    LeftTurnInIntersection();
  }
  else if (cup_side_right == 0)
  {
    RightTurnInIntersection();
  }

  // Follows line to cup placement
  for (int i = 0; i < 80; i++)
  { // Følger linjen en liten stund
    FollowLine();
    delay(10);
  }
  Brake();

  // Places cup
  LowerCup();
}

void LeaveLV1()
{
  // Lifts claw max, but doesnt close
  VipUp();
  delay(1000);

  // Rotates to leave line at angle towards lv 2
  if (cup_side_right == 1)
  {
    LeftRotationForTime(450, slow_speed);
  }
  else if (cup_side_right == 0)
  {
    RightRotationForTime(450, slow_speed);
  }

  // Back up a to leave line
  for (int i = 0; i < 70; i++)
  { // Følger linjen en liten stund
    Backward();
    delay(10);
  }
  Brake();

  // Continues backward until it hits a line and then straightens up when it hits the line
  FollowLineBackwardsUntilIntersection(slow_speed);
  CrossIntersection(150);

  // Rotates to face LV 2
  if (cup_side_right == 1)
  {
    RightRotation90n();
  }
  else if (cup_side_right == 0)
  {
    LeftRotation90n();
  }

  // Corrects the rotation by following the line a bit
  for (int i = 0; i < 150; i++)
  { // Følger linjen en liten stund
    FollowLine();
    delay(10);
  }
  Brake();

  // Closes claw before entering the gate
  Grip();
  delay(1000);
}

//--------------LV4 Functions--------------

void LV4GoToIntersection()
{
  // Follows line from LV3 to first storage intersection
  FollowLineUntilIntersection(slow_speed);
  delay(500);

  // Crosses first intersection straight forward
  CrossIntersection(150);

  // Follows line to second storage intersection
  FollowLineUntilIntersection(slow_speed);
  delay(500);

  // Crosses first intersection at an angle
  LeftRotationForTime(250);
  CrossIntersection(150);

  // Follows line to searching intersection
  FollowLineUntilIntersection(slow_speed);
  delay(1000);
}

void ForwardToCup(int motor_speed = slow_speed)
{
  while (ReadDistance() > LV4_pickup_distance)
  {
    Forward(motor_speed);
  }
  Brake();
}

void LV4Scan(int n = LV4_rotation_amount, int t = LV4_rotation_time)
{
  // Initial scan until a cup is found
  for (int i = 0; i < n; i++)
  {
    cup_rotations = i;

    RightRotationForTime(t, ultra_slow_speed);
    delay(500);
    distance = ReadDistance(); // Reads distance from sensor
    Serial.println(distance);
    delay(500);

    if (distance < cup_found)
    {
      break;
    }
  }
  delay(2000);

  // Second rotation to aim for the centre of the cup
  for (int j = 0; j < LV4_extra_rotations; j++)
  {
    cup_rotations += 1;
    RightRotationForTime(t, ultra_slow_speed);
    delay(500);
  }
}

void LV4FindCup()
{
  // Rotates to start scanning and sets intersection to false as it leaves the intersection
  LeftRotationForTime(LV4_initial_rotation_time);
  intersection = false;
  delay(1000);

  // Lifts the vip so it doesnt collide with the ultrasound readings
  VipServo.write(15);

  // Scans for cups
  LV4Scan();

  // Goes forward until it can grab the cup
  ForwardToCup();

  // Lifts the cup
  LiftCup();
}

void LV4ReturnToLine()
{
  // Goes backward to either side of the intersection based on how far it initially rotated
  // When it reaches the line it stops, rotates, finds the searching intersection and leaves the searching area
  if (cup_rotations <= ((LV4_rotation_amount + LV4_extra_rotations) / 2))
  {
    RightRotationForTime(LV4_exit_search_area_rotation);

    FollowLineBackwardsUntilIntersection(slow_speed);

    // Tester en cross her
    CrossIntersectionBackward(200);
    RightRotation90n();
    intersection = false;
    

    FollowLineUntilIntersection();
    
    RightTurnInIntersection();
  }
  else
  {
    LeftRotationForTime(LV4_exit_search_area_rotation);

    FollowLineBackwardsUntilIntersection(slow_speed);

    CrossIntersectionBackward(200);
    LeftRotation90n();
    intersection = false;
    

    FollowLineUntilIntersection();
    

    LeftTurnInIntersection();

  }

  // Follows Line to first intersection
  FollowLineIgnoreIntersection(35);
  FollowLineUntilIntersection(ultra_slow_speed);
}

void LV4FindCupAndReturn() // The above functions combined
{
  LV4GoToIntersection();

  LV4FindCup();

  LV4ReturnToLine();
}

void LV4PlaceCup(int n)
{
  delay(1000);

  // If it is to store the cup at the second storage it goes to the next intersection, if not it stays
  if (n == 2)
  {
    RightRotationForTime(300);
    CrossIntersection(150);
    // FollowLineIntersection(100);
    FollowLineIgnoreIntersection(55);
    FollowLineUntilIntersection(ultra_slow_speed);
  }
  delay(1000);

  // Takes a left turn into the storage path
  CrossIntersection();
  LeftRotation45n();
  LeftRotation30n();

  // Follows line until at storage position
  for (int i = 0; i < 80; i++)
  { // Følger linjen en liten stund
    FollowLine();
    delay(10);
  }
  Brake();

  // Place the cup in storage
  LowerCup();
  delay(500);
}

void LV4FindStart()
{
  // Backward until past line
  Backward();
  delay(1500);
  Brake();

  // Rotate to face the storage area
  LeftRotation90n();

  // Go backwards until it hits the line
  intersection = false;
  FollowLineBackwardsUntilIntersection();

  // Rotates to end up facing LV4
  CrossIntersection(150);
  RightRotation90n();
}

//--------------Miscellaneous--------------

void VictoryDance()
{
  Backward();
  delay(500);
  Brake();

  while (true)
  {
    LeftRotation();
    LiftCup();
    LowerCup();
    LiftCup();
    LowerCup();
    RightRotation();
    LiftCup();
    LowerCup();
    LiftCup();
    LowerCup();
  }
}

//-------------------------------------------Complete Levels---------------------------------------------

void LV1()
{
  // Scans For Cup
  // cup_side_right = ScanForCup();
  LV1_Scan();

  // Singal in case it doesnt find a cup
  if (cup_side_right == 2)
  {
    LiftCup();
    LowerCup();
  }

  // Follows line until intersection
  FollowLineUntilIntersection();

  // Moves cup from one side to another and gets ready to keep driving
  MoveCup(cup_side_right);

  // Leaves LV1 to start of LV2
  LeaveLV1();
}

void LV2()
{
  FollowLineUntilIntersection();
}

void LV3()
{
  // Takes a Left Turn in the intersection
  LeftTurnInIntersection();

  FollowLineIgnoreIntersection(1300);

  FollowLineUntilIntersection();

  LeftTurnInIntersection();

  delay(100);
  LowerCup();
}

void LV4()
{
  // Find a cup
  LV4FindCupAndReturn();

  // Place it in the second storage
  LV4PlaceCup(2);

  // Go back to LV4 start
  LV4FindStart();

  // Find next cup
  LV4FindCupAndReturn();

  // Place it in the first storage
  LV4PlaceCup(1);
}

void ImpressAmund()
{
  LV1();
  LV2();
  LV3();
  LV4();
  VictoryDance();
}

//--------------------------------------------Code Start-------------------------------------------------
//----------------------------------------------Setup----------------------------------------------------

void setup()
{

  ClawServo.write(10); // Sets start-position to zero (Åpen)
  VipServo.write(10);  // Sets start-position to zero (Nede)

  LeftMotor.attach(motor_left_pin);
  RightMotor.attach(motor_right_pin);
  ClawServo.attach(claw_servo_pin);
  VipServo.attach(vip_servo_pin);

  pinMode(distance_trig_pin, OUTPUT); // Sets the trigPin as an Output
  pinMode(distance_echo_pin, INPUT);  // Sets the echoPin as an Input

  Serial.begin(9600);

  //-------------------------------------------Program Start-----------------------------------------------

  delay(2000);

  //LV1();
  // LV2();
  // LV3();
  // LV4();
  //VictoryDance();
  
  ImpressAmund();
}

//-----------------------------------------------Loop----------------------------------------------------
void loop()
{
}
