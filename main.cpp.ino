#include <Arduino.h>
#include <RunningMedian.h>


//##############################################################################
//                              C O N S T A N T S
//##############################################################################
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
#define PIN_PWM         4
#define PIN_THM         A0



//##############################################################################
//                            F U N C T I O N S
//##############################################################################
double adc2temp(int adc, float Vin, float sr);
double adc2temp(int adc, float Vin, float sr, boolean vccTherm);



//##############################################################################
//                    G L O B A L   V A R I A B L E S
//##############################################################################
RunningMedian temp (100);
unsigned long serialTime, tempTime;
double tempCelsius;



//### Reset function
//void(* resetFunc) (void) = 0; //declare reset function @ address 0




//##############################################################################
//                                   S E T U P
//##############################################################################
void setup() {
  Serial.begin(115200);
  Serial.println("init...");

  // set PIN modes
  pinMode(PIN_THM, INPUT);
  pinMode(PIN_PWM, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  tempTime     = 0;
  serialTime   = 0;
}





//##############################################################################
//                                   L O O P
//##############################################################################
void loop() {
  //lee el termistor y activa/desactiva resistencia según sea necesario
  if(millis() > tempTime){
    temp.add(analogRead(PIN_THM));
    tempTime = millis() + TIME_UPDATE_TEMP;

    tempCelsius = adc2temp(temp.getMedian(), VCC, SERIESRESISTOR, true);

    if (tempCelsius < SET_TEMP){
      digitalWrite(PIN_PWM, HIGH);  

    } else {
      digitalWrite(PIN_PWM, LOW); 
    }
  }

  // Imprime la temperatura por serial
  if(millis() > serialTime){
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.println(tempCelsius);

    serialTime =  millis() + TIME_UPDATE_SERIAL;

    digitalWrite(LED_BUILTIN, LOW);
  }

}



// Función pensada para transformar valores leídos con entrada analógica
// a ºC.
//    - adc: valor obtenido con analogRead
//    - Vin: tensión de alimentación del divisor de tensión
//    - sr: resistencia serie del divisor de tensión
//    - vccTherm: thermistor a VCC (true) o a GND (false)
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

