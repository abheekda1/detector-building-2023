// default calibration mass count
#define N 20

#define MAX_ZONES 10

// ranges to be given at competition
#define RANGE1_MAX 300
#define RANGE2_MAX 600

int numZones = 0;

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

// zones type
struct zone {
  bool red;
  bool green;
  bool blue;
  int lowerBound;
  int upperBound;
};

zone zones[MAX_ZONES];

/* -----------------------*
    Helpers
  ------------------------*/
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
    Curve Fitting
  ------------------------*/
// generates the curve using least squares
// https://mathworld.wolfram.com/LeastSquaresFittingLogarithmic.html
void genCurve(double x[], double y[], int n) {
  double sx = 0, syx = 0, sy = 0, sx2 = 0;
  for (int i = 0; i < n; i++) {
    sx += ln(x[i]);
    syx += y[i] * ln(x[i]);
    sy += y[i];
    sx2 += ln(x[i]) * ln(x[i]);
  }
  b = (n * syx - sx * sy) / (n * sx2 - sx * sx);
  a = (sy - b * sx) / n;
}

// maps a voltage value to a mass
double fitToCurve(int voltage) {
  // inverse of the logarithmic function
  // since we're getting x in terms of y
  return exp((voltage - a) / b);
}
// end curve fitting


/* -----------------------*
    LEDS
  ------------------------*/
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
    Main functions
  ------------------------*/
void setup(void) {
  // serial monitor
  Serial.begin(9600);

  Serial.println("Please input all zones with color specifiers like 'RB' for Red-Blue then the bounds and finally send 'c' to continue:");

  // loop through zones
  for (int i = 0; true; i++) {
    // read current line
    while (!Serial.available()) {}
    String s = Serial.readStringUntil('\n');

    if (s == "c") {
      // set numVals to values read so far instead
      // of total values
      numZones = i;
      break;
    }

    bool red = false;
    bool green = false;
    bool blue = false;

    for (int j = 0; j < s.length(); j++) {
      if (s.charAt(j) == 'R') red = true;
      else if (s.charAt(j) == 'G') green = true;
      else if (s.charAt(j) == 'B') blue = true;
    }

    int lowerBound = 0;
    int upperBound = 1000;

    Serial.print("Lower bound: ");
    while (!Serial.available()) {}
    lowerBound = Serial.readStringUntil('\n').toInt();
    Serial.println(lowerBound);
    Serial.print("Upper bound: ");
    while (!Serial.available()) {}
    upperBound = Serial.readStringUntil('\n').toInt();
    Serial.println(upperBound);

    Serial.println(String(red) + String(green) + String(blue));

    // create a new zone
    zone newZone = {red, green, blue, lowerBound, upperBound};

    // add to list of zones
    zones[i] = newZone;

    // print the values for clarity
    Serial.println("New zone:\n\tLower bound: " + String(lowerBound) + "\n\tUpper bound: " + String(upperBound) + "\n\tColors: " + (red ? "R" : "") + (green ? "G" : "") + (red ? "B" : ""));
  }

  // decrease numVals if loop is
  // exited early
  int numVals = N;

  // set base values of platform
  x[0] = 1;
  fsrReading = analogRead(fsrPin);
  int voltage = analogToVoltage(fsrReading);
  y[0] = voltage;

  Serial.println("Please calibrate the masses then send 'c' to continue:");

  // loop through calibration masses
  for (int i = 1; i <= N; i++) {
    // read current line
    while (!Serial.available()) {}
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

  bool readyToGenCurve = false;

  while (!readyToGenCurve) {
    Serial.println("===================================");
    for (int i = 0; i < numVals; i++) {
      Serial.println(String(i + 1) + ")\n\tMass: " + String(x[i]) + "\n\tFSR Reading: " + String(y[i]));
    }
    Serial.println("===================================");
    Serial.println();

    String response = "";

    while (response != "a" && response != "b") {
      Serial.println("What would you like to do?\n\ta) Generate the curve!\n\tb) Replace a value");
      while (!Serial.available()) {}
      response = Serial.readStringUntil('\n');
    }

    if (response == "a") {
      readyToGenCurve = true;
    } else if (response == "b") {
      Serial.print("Please input the index of the mass you would like to recalibrate: ");
      while (!Serial.available()) {}
      int idx = Serial.readStringUntil('\n').toInt() - 1;
      Serial.println();

      Serial.println("Please place the mass on the FSR and then input the weight in grams");

      // get the mass in g as a double
      // from input
      while (!Serial.available()) {}
      x[idx] = (double)Serial.readStringUntil('\n').toInt();

      // get the corresponding voltage read
      // from the FSR
      fsrReading = analogRead(fsrPin);
      int voltage = analogToVoltage(fsrReading);

      y[idx] = voltage;
    }
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
  Serial.println("Current mass: " + String(mass, 1) + " (" + voltage + " mV)");

  // turn of all LEDs so multiple aren't on at
  // the same time
  turnOffLeds();

  // turn on the LED corresponding to the current
  // mass range
  //if (mass < RANGE1_MAX) {
  //  turnOnBlue();
  //} else if (mass < RANGE2_MAX) {
  //  turnOnRed();
  //} else {
  //  turnOnGreen();
  //}

  for (int i = 0; i < numZones; i++) {
    zone currentZone = zones[i];
    if (mass >= currentZone.lowerBound && mass <= currentZone.upperBound) {
      if (currentZone.red) turnOnRed();
      if (currentZone.green) turnOnGreen();
      if (currentZone.blue) turnOnBlue();
    }
  }

  delay(100);
}
// end main functions
