//Joystick Advance

#include <math.h>
#include <Unistep2.h>
#include <SoftwareSerial.h>

#define buzzer  5

#define L298N_IN1 6
#define L298N_IN2 7
#define L298N_IN3 8
#define L298N_IN4 9

#define L298N_ENA A2
#define L298N_ENB A3

#define step_IN1  10
#define step_IN2  11
#define step_IN3  12
#define step_IN4  13

SoftwareSerial btSerial(2, 3); // RX, TX
Unistep2 stepper(step_IN1, step_IN2, step_IN3, step_IN4, 4096, 900);// IN1, IN2, IN3, IN4, total step, delay of every step(in micros)

class Value {
  public:
    int angle;          //0~359
    float strength;     //0~100
    int button;         //1~4

    float radian;       //Radians = Degrees x (π/180)
    float left_rate;    //-1~+1
    float right_rate;   //-1~+1

    //constructor for setup
    Value() {
      Serial.println("Class Value setup...");
    }
    //seperate string cipher to each value
    void parting(String str) {
      //      angle = (str.substring(0, 3).toInt() + 450) % 360;  //rotate 90 degree counterclockwise
      angle = str.substring(0, 3).toInt();
      strength = str.substring(3, 6).toFloat() / 100;
      button = str.substring(6, 8).toInt();
      radian = angle * M_PI / 180;
      left_rate = strength * (sin(radian) + cos(radian) / 2) / 1.119;   //Max value of y=sin(x)+cos(x)/2 is 1.118034
      right_rate = strength * (sin(radian) - cos(radian) / 2) / 1.119;  //In order to make rate in range -1~+1
    }
    //print all value
    void show() {
      Serial.print("angle: ");
      Serial.print(angle);
      Serial.print('\t');
      Serial.print("strength: ");
      Serial.print(strength);
      Serial.print('\t');
      Serial.print("button: ");
      Serial.print(button);

      Serial.print('\t');
      Serial.print("radians: ");
      Serial.print(radian);
      Serial.print('\t');
      Serial.print("left_rate: ");
      Serial.print(left_rate);
      Serial.print('\t');
      Serial.print("right_rate: ");
      Serial.print(right_rate);

      Serial.println("");
      //      Serial.flush(); //Waits for the transmission of outgoing serial data to complete
    }

}; Value val;

bool BT_cmd(); //bluetooth command
void Go_Wheel(); //control 2 dc motor
void turn_table();
void shoot_sound();  //buzzer shoot sound

void setup() {

  pinMode(buzzer, OUTPUT);

  pinMode(L298N_IN1, OUTPUT);         // Initialize the L298N Input pins as an output
  pinMode(L298N_IN2, OUTPUT);
  pinMode(L298N_IN3, OUTPUT);
  pinMode(L298N_IN4, OUTPUT);
  pinMode(L298N_ENA, OUTPUT);
  pinMode(L298N_ENB, OUTPUT);

  digitalWrite(L298N_IN1, LOW);       // Set the L298N Input pins to LOW //strange IN_1's HIGH is LOW vice versa
  digitalWrite(L298N_IN2, LOW);
  digitalWrite(L298N_IN3, LOW);
  digitalWrite(L298N_IN4, LOW);
  analogWrite(L298N_ENA, 0);
  analogWrite(L298N_ENB, 0);

  Serial.begin(9600);
  btSerial.begin(9600);

  Serial.setTimeout(100); //reduce the delay of bluetooth

  while (!Serial) {
    ; // wait for serial port to connect
  }
  Serial.println("L298N Module setup...");

}
void loop() {
  stepper.run();// We need to call run() frequently during loop()

  if (BT_cmd()) {
    Go_Wheel();
    turn_table();
    shoot_sound();
  }

}

bool BT_cmd() {
  if (btSerial.available() > 0) {
    String str = btSerial.readStringUntil('#');
    //    Serial.print("str = ");
    //    Serial.println(str);
    if (str.length() == 7) {
      val.parting(str);
      val.show();
      return true;
    }
  }
  return false;
}

void Go_Wheel() {

  float r_rate = abs(val.right_rate) * 255;
  float l_rate = abs(val.left_rate) * 255;

  r_rate = map(r_rate, 0, 255, 127, 255); //motor won't go if rate is too low
  l_rate = map(l_rate, 0, 255, 127, 255);

  r_rate = r_rate > 143 ? r_rate : 0;  //rate below 143 set 0
  l_rate = l_rate > 143 ? l_rate : 0;  //press middle and motor set 0 motor stop

  Serial.print("r_rate_map = ");
  Serial.print(r_rate);
  Serial.print("\t");
  Serial.print("l_rate_map = ");
  Serial.print(l_rate);
  Serial.println("");

  analogWrite(L298N_ENA, r_rate);
  analogWrite(L298N_ENB, l_rate);

  if (val.right_rate >= 0) {
    //    Serial.println("r > 0 ");
    digitalWrite(L298N_IN1, HIGH);
    digitalWrite(L298N_IN2, LOW);
  }
  else {
    //    Serial.println("r < 0 ");
    digitalWrite(L298N_IN1, LOW);
    digitalWrite(L298N_IN2, HIGH);
  }

  if (val.left_rate >= 0) {
    //    Serial.println("l > 0 ");
    digitalWrite(L298N_IN3, HIGH);
    digitalWrite(L298N_IN4, LOW);
  }
  else {
    //    Serial.println("l < 0 ");
    digitalWrite(L298N_IN3, LOW);
    digitalWrite(L298N_IN4, HIGH);
  }
}

void turn_table() {

  if (val.button == 2 && stepper.stepsToGo() == 0) {   //turn table turn left
    stepper.move(4096 / 16);
  }
  if (val.button == 4 && stepper.stepsToGo() == 0) {   //turn table turn right
    stepper.move(-4096 / 16);
  }
}

void shoot_sound() {
  
  if (val.button == 3 ) {
    Serial.println("3 3 3");
    for (int i = 0; i < 600; i++) {
      digitalWrite(buzzer, HIGH);
      delayMicroseconds(i);
      digitalWrite(buzzer, LOW);
      delayMicroseconds(i * 2);
    }
  }

}
