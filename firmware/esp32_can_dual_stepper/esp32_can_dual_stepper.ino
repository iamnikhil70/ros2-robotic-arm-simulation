#include "driver/twai.h"
#include <math.h>

// ======================================================
// CAN / SN65HVD230
// ======================================================

#define CAN_TX GPIO_NUM_18
#define CAN_RX GPIO_NUM_21

// ======================================================
// TB6600 MOTOR DRIVER PINS
// ======================================================

#define M1_STEP 25
#define M1_DIR  26

#define M2_STEP 32
#define M2_DIR  33

// ======================================================
// MOTOR / MOTION SETTINGS
// ======================================================

// 1.8 degree stepper = 200 full steps/revolution
// TB6600 set to 1/16 microstepping:
// 200 x 16 = 3200 pulses/revolution
#define STEPS_PER_REV 3200

// Half-period delay values for STEP pulses.
// Starts slowly and accelerates to about 62.5 RPM.
#define START_DELAY_US 1200
#define MIN_DELAY_US    150
#define RAMP_STEPS      1000

// ======================================================
// SMOOTH RAMP
// ======================================================

int smoothDelay(float progress)
{
  // Cosine interpolation from 0 -> 1.
  // This softens the acceleration/deceleration transition.
  float smooth = 0.5f - 0.5f * cosf(PI * progress);

  float delayValue =
    START_DELAY_US -
    (START_DELAY_US - MIN_DELAY_US) * smooth;

  return (int)delayValue;
}

// ======================================================
// SINGLE MOTOR MOVE
// ======================================================

void moveSingleMotor(
  int stepPin,
  int dirPin,
  bool direction,
  int totalSteps)
{
  digitalWrite(dirPin, direction);
  delayMicroseconds(100);

  int rampSteps = RAMP_STEPS;

  if (rampSteps * 2 > totalSteps)
  {
    rampSteps = totalSteps / 2;
  }

  Serial.println("Smooth acceleration...");

  for (int i = 0; i < rampSteps; i++)
  {
    float progress = (float)i / (float)rampSteps;
    int d = smoothDelay(progress);

    digitalWrite(stepPin, HIGH);
    delayMicroseconds(d);

    digitalWrite(stepPin, LOW);
    delayMicroseconds(d);
  }

  Serial.println("Constant speed...");

  int constantSteps = totalSteps - (2 * rampSteps);

  for (int i = 0; i < constantSteps; i++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(MIN_DELAY_US);

    digitalWrite(stepPin, LOW);
    delayMicroseconds(MIN_DELAY_US);
  }

  Serial.println("Smooth deceleration...");

  for (int i = rampSteps - 1; i >= 0; i--)
  {
    float progress = (float)i / (float)rampSteps;
    int d = smoothDelay(progress);

    digitalWrite(stepPin, HIGH);
    delayMicroseconds(d);

    digitalWrite(stepPin, LOW);
    delayMicroseconds(d);
  }

  Serial.println("Motor stopped.");
}

// ======================================================
// SYNCHRONIZED TWO-MOTOR MOVE
// ======================================================

void moveBothMotors(
  bool motor1Direction,
  bool motor2Direction,
  int totalSteps)
{
  Serial.println();
  Serial.println("==============================");
  Serial.println(" SYNCHRONIZED DUAL MOTOR MOVE");
  Serial.println("==============================");

  digitalWrite(M1_DIR, motor1Direction);
  digitalWrite(M2_DIR, motor2Direction);

  delayMicroseconds(100);

  int rampSteps = RAMP_STEPS;

  if (rampSteps * 2 > totalSteps)
  {
    rampSteps = totalSteps / 2;
  }

  Serial.println("Smooth acceleration...");

  for (int i = 0; i < rampSteps; i++)
  {
    float progress = (float)i / (float)rampSteps;
    int d = smoothDelay(progress);

    digitalWrite(M1_STEP, HIGH);
    digitalWrite(M2_STEP, HIGH);
    delayMicroseconds(d);

    digitalWrite(M1_STEP, LOW);
    digitalWrite(M2_STEP, LOW);
    delayMicroseconds(d);
  }

  Serial.println("Constant synchronized speed...");

  int constantSteps = totalSteps - (2 * rampSteps);

  for (int i = 0; i < constantSteps; i++)
  {
    digitalWrite(M1_STEP, HIGH);
    digitalWrite(M2_STEP, HIGH);
    delayMicroseconds(MIN_DELAY_US);

    digitalWrite(M1_STEP, LOW);
    digitalWrite(M2_STEP, LOW);
    delayMicroseconds(MIN_DELAY_US);
  }

  Serial.println("Smooth deceleration...");

  for (int i = rampSteps - 1; i >= 0; i--)
  {
    float progress = (float)i / (float)rampSteps;
    int d = smoothDelay(progress);

    digitalWrite(M1_STEP, HIGH);
    digitalWrite(M2_STEP, HIGH);
    delayMicroseconds(d);

    digitalWrite(M1_STEP, LOW);
    digitalWrite(M2_STEP, LOW);
    delayMicroseconds(d);
  }

  Serial.println("Both motors stopped.");
}

// ======================================================
// SETUP
// ======================================================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===============================");
  Serial.println(" DUAL MOTOR CAN CONTROLLER");
  Serial.println("===============================");

  pinMode(M1_STEP, OUTPUT);
  pinMode(M1_DIR, OUTPUT);
  pinMode(M2_STEP, OUTPUT);
  pinMode(M2_DIR, OUTPUT);

  digitalWrite(M1_STEP, LOW);
  digitalWrite(M2_STEP, LOW);
  digitalWrite(M1_DIR, LOW);
  digitalWrite(M2_DIR, LOW);

  Serial.println("Motor 1 STEP = GPIO25");
  Serial.println("Motor 1 DIR  = GPIO26");
  Serial.println("Motor 2 STEP = GPIO32");
  Serial.println("Motor 2 DIR  = GPIO33");
  Serial.println("Microstepping = 1/16");
  Serial.println("Pulses/rev    = 3200");

  twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT(
      CAN_TX,
      CAN_RX,
      TWAI_MODE_NORMAL
    );

  twai_timing_config_t t_config =
    TWAI_TIMING_CONFIG_250KBITS();

  twai_filter_config_t f_config =
    TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(
        &g_config,
        &t_config,
        &f_config) != ESP_OK)
  {
    Serial.println("CAN installation FAILED!");

    while (1)
    {
      delay(1000);
    }
  }

  if (twai_start() != ESP_OK)
  {
    Serial.println("CAN start FAILED!");

    while (1)
    {
      delay(1000);
    }
  }

  Serial.println();
  Serial.println("CAN started successfully.");
  Serial.println("CAN speed = 250 kbps");
  Serial.println("CAN ID = 0x100");
  Serial.println();
  Serial.println("COMMANDS:");
  Serial.println("01 = Motor 1");
  Serial.println("02 = Motor 2");
  Serial.println("03 = Both motors, opposite directions");
  Serial.println("04 = Both motors, reverse directions");
  Serial.println();
  Serial.println("Waiting for CAN...");
}

// ======================================================
// MAIN LOOP
// ======================================================

void loop()
{
  twai_message_t message;

  if (twai_receive(
        &message,
        pdMS_TO_TICKS(1000)) == ESP_OK)
  {
    Serial.println();
    Serial.println("==============================");
    Serial.println(" CAN MESSAGE RECEIVED");
    Serial.println("==============================");

    Serial.print("ID   : 0x");
    Serial.println(message.identifier, HEX);

    Serial.print("LEN  : ");
    Serial.println(message.data_length_code);

    Serial.print("DATA : ");

    for (int i = 0; i < message.data_length_code; i++)
    {
      if (message.data[i] < 0x10)
      {
        Serial.print("0");
      }

      Serial.print(message.data[i], HEX);
      Serial.print(" ");
    }

    Serial.println();

    if (message.identifier != 0x100)
    {
      Serial.println("Wrong CAN ID.");
      return;
    }

    if (message.data_length_code < 1)
    {
      Serial.println("No command byte.");
      return;
    }

    uint8_t command = message.data[0];

    if (command == 0x01)
    {
      Serial.println("Motor 1: one revolution");

      moveSingleMotor(
        M1_STEP,
        M1_DIR,
        LOW,
        STEPS_PER_REV
      );
    }
    else if (command == 0x02)
    {
      Serial.println("Motor 2: one revolution");

      moveSingleMotor(
        M2_STEP,
        M2_DIR,
        HIGH,
        STEPS_PER_REV
      );
    }
    else if (command == 0x03)
    {
      Serial.println("Both motors: same step rate, opposite directions");

      moveBothMotors(
        LOW,
        HIGH,
        STEPS_PER_REV
      );
    }
    else if (command == 0x04)
    {
      Serial.println("Both motors: reverse differential direction");

      moveBothMotors(
        HIGH,
        LOW,
        STEPS_PER_REV
      );
    }
    else
    {
      Serial.println("Unknown command.");
    }
  }
}
