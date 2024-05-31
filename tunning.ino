/*****************************************************************************
  sTune Get All Tunings Example (MAX31856, PTC Heater / SSR / Software PWM)
  This runs a fast inflection point test to determine tuning parameters.
  Open serial printer to view test progress and results.
  Reference: https://github.com/Dlloydev/sTune/wiki/Examples_MAX31856_PTC_SSR
  ****************************************************************************/
#include <sTune.h>




// default settings
#define SET_TEMP              37.5    //Recomendado entre 37 y 38ºC
#define TIME_UPDATE_TEMP      100
#define TIME_UPDATE_SERIAL    5000

// thermistor constants
#define THERMISTORNOMINAL   100000.0  // resistance at 25 degrees C
#define TEMPERATURENOMINAL  25.0      // temp. for nominal resistance (almost always 25 C)
#define BCOEFFICIENT        3950.0    // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR      98700.0   // the value of the 'other' resistor
#define VCC                 5.0

//pin out
#define PIN_PWM         3
#define PIN_THM         A0








double adc2temp(int adc, float Vin, float sr){
  return adc2temp(adc, Vin, sr, true);
}
double adc2temp(int adc, float Vin, float sr, boolean vccTherm){
  float steinhart, vadc, radc, rinf;

  //convert ADC value to resistance
  vadc = Vin * adc/1023.0;
  if (vccTherm){
      radc = sr*vadc/(Vin-vadc);  //sr*(Vin-vadc)/vadc
  } else {
      radc = sr*(Vin-vadc)/vadc;
  }

  // convert resistance to temperature
  steinhart = radc / THERMISTORNOMINAL;        // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  return steinhart;
}





// pins
const uint8_t relayPin = PIN_PWM;
const uint8_t drdyPin = PIN_THM;

// user settings
uint32_t settleTimeSec = 120;
uint32_t testTimeSec = 5000;  // sample interval = testTimeSec / samples
const uint16_t samples = 400;
const float inputSpan = 20;
const float outputSpan = 255;
float outputStart = 0;
float outputStep = 50;
float tempLimit = 45;

// variables
float Input, Output;

sTune tuner = sTune(&Input, &Output, tuner.ZN_PID, tuner.direct5T, tuner.printALL);





void setup() {
  pinMode(drdyPin, INPUT);
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
  delay(3000);
  Output = 0;

  tuner.Configure(inputSpan, outputSpan, outputStart, outputStep, testTimeSec, settleTimeSec, samples);
  tuner.SetEmergencyStop(tempLimit);
}

void loop() {
  tuner.softPwm(relayPin, Input, Output, 0, outputSpan, 1);

  switch (tuner.Run()) {
    case tuner.sample: // active once per sample during test
      Input = adc2temp(analogRead(PIN_THM), VCC, SERIESRESISTOR, true);
      break;

    case tuner.tunings: // active just once when sTune is done
      Output = 0;
      tuner.SetTuningMethod(tuner.TuningMethod::DampedOsc_PID);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::NoOvershoot_PID);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::CohenCoon_PID);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::Mixed_PID);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::ZN_PI);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::DampedOsc_PI);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::NoOvershoot_PI);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::CohenCoon_PI);
      tuner.printTunings();
      tuner.SetTuningMethod(tuner.TuningMethod::Mixed_PI);
      tuner.printTunings();
      break;
  }
}
