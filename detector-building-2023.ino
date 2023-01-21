// default calibration mass count
#define N 10

// ranges to be given at competition
#define RANGE1_MAX 200
#define RANGE2_MAX 400

const int fsrPin = 0;     // the FSR and 10K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider

// values for LED pins
const int redPin   = 2;
const int greenPin = 4;
const int bluePin  = 7;

// x-values (mass)
double x[N];
// y-values (voltage)
double y[N];

// needed for ln func
double a, b;

/* -----------------------*
 *  Helpers               *
 *------------------------*/
// maps analog data values to voltage
int analogToVoltage(int data) {
  return map(data, 0, 1023, 0, 5000);
}

// function to take ln easily
double ln(double x) {
    return log(x) / log(exp(1) /* e */);
}
// end helpers

/* -----------------------*
 *  Curve Fitting         *
 *------------------------*/
// generates the curve using least squares
// https://mathworld.wolfram.com/LeastSquaresFittingLogarithmic.html
void genCurve(double x[], double y[], int n) {
  double sx = 0, sy = 0, sxy = 0, sx2 = 0;
  for (int i = 0; i < n; i++) {
    sx += ln(x[i]);
    sy += y[i] * ln(x[i]);
    sxy += y[i];
    sx2 += ln(x[i]) * ln(x[i]);
  }
  b = (n * sy - sx * sxy) / (n * sx2 - sx * sx);
  a = (sxy - b * sx) / n;
}

// maps a voltage value to a mass
double fitToCurve(int voltage) {
  // inverse of the logarithmic function
  // since we're getting x in terms of y
  return exp((voltage - a) / b);
}
// end curve fitting


/* -----------------------*
 *  LEDS                  *
 *------------------------*/
void turnOffLeds(void) {
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void turnOnRed(void) {
  analogWrite(redPin, 255);
}

void turnOnGreen(void) {
  analogWrite(greenPin, 255);
}

void turnOnBlue(void) {
  analogWrite(bluePin, 255);
}
// end LEDs

/* -----------------------*
 *  Main functions        *
 *------------------------*/
void setup(void) {
  // serial monitor
  Serial.begin(9600);

  // decrease numVals if loop is
  // exited early
  int numVals = N;

  // loop through calibration masses
  for (int i = 0; i < N; i++) {
    // wait for input
    while(Serial.available() == 0) { }
    
    // read current line
    String s = Serial.readStringUntil('\n');
    
    if (s == "c") {
      // set numVals to values read so far instead
      // of total values
      numVals = i;
      break;
    }

    // print the input to prevent confusion
    // Serial.println("input: " + s);

    // get the mass in g as a double
    // from input
    x[i] = (double)s.toInt();

    // get the corresponding voltage read
    // from the FSR
    fsrReading = analogRead(fsrPin);
    int voltage = analogToVoltage(fsrReading);

    y[i] = voltage;

    // print the values for clarity
    Serial.println("x[" + String(i) + "] = " + String(x[i]) + "\ny[" + String(i) + "] = " + String(y[i]) + "\n\n");
  }

  // generate a and b vals
  genCurve(x, y, numVals);

  // print the equation
  Serial.println("y = " + String(a) + " + " + String(b) + "ln(x)");
}

void loop(void) {
  // get analog data from pin
  fsrReading = analogRead(fsrPin);

  // Serial.print("Analog reading = ");
  // Serial.println(fsrReading);     // the raw analog reading

  // map the analog values from 0 - 5000 mV
  int voltage = analogToVoltage(fsrReading);

  // grab the mass from the created logarithmic curve
  // based on the read voltage
  double mass = fitToCurve(voltage);

  // display the current mass
  Serial.println("Current mass: " + String(mass, 1));

  // turn of all LEDs so multiple aren't on at
  // the same time
  turnOffLeds();

  // turn on the LED corresponding to the current
  // mass range
  if (mass < RANGE1_MAX) {
    turnOnRed();
  } else if (mass < RANGE2_MAX) {
    turnOnGreen();
  } else {
    turnOnBlue();
  }
  
  delay(100);
}
// end main functions
