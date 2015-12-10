 /////////////////////////////////////////////////////////
//                                                      //
// Program: weatherstation.ino                          //
//                                                      //
// Purpose: Pulls weather current/forcast conditions    //
//          from two webhooks.  Weather from forcast.io //
//          and tides from wunderground api             //
//                                                      //
// Credit: Inspired by Grady Hillhouse 2015             //
//         modified by Tom Feroli                       //
//                                                      //
/////////////////////////////////////////////////////////

//
// Debug Flag (no output = 0, serial output = 1, cloud output = 2)
//
int debug = 2;
char buffer [80];

//
// Define Pins
// 
int pTemperature    = D0;
int pHumidity       = D1;
int pPressure       = D2;
int pPrecipProb     = D3;
int pWindSpeed      = WKP;
int pWindDir        = TX;
int pTide           = RX;
int pAlert          = D4;
int pPressUp        = D5;
int pPressDown      = D6;
int pDayOne         = A1;
int pDayTwo         = A2;
int pDayThree       = A3;

//
// Refresh Vars
//
int refreshWeather         = 90;   // Refresh time in seconds for weather
int refreshTide            = 300;  // Refresh time in seconds for tide
double lastRefreshWeather  = 0;    // Unix time of last refresh weather
double lastRefreshTide     = 0;    // Unix time of last refresh tide
double lastGotWeather      = 0;    // Unix time of last time the webhook data was returned
double lastGotTide         = 0;    // Unix time of last time the webhook data was returned

//
// Pressue Up/Down Vars
//
double pressureDelta       = 1.0;  // Pressure change requred to chage pressure trend
double previousPressure    = 0.0;  // Store the previous pressure from pressureDelta threshold

//
// Raw Data
//
float currentTemperature;           
float currentHumidity;
float currentPressure;
float currentPrecipProb;
float currentWindSpeed;
float currentWindDir;
float d0TempMin;           
float d0TempMax;           
float d0Humidity;
float d0Pressure;
float d0PrecipProb;
float d0WindSpeed;
float d0WindDir;
float d1TempMin;           
float d1TempMax;           
float d1Humidity;
float d1Pressure;
float d1PrecipProb;
float d1WindSpeed;
float d1WindDir;
float d2TempMin;           
float d2TempMax;           
float d2Humidity;
float d2Pressure;
float d2PrecipProb;
float d2WindSpeed;
float d2WindDir;

float tide;
float minTide;
float maxTide;
float ifAlert;
float ifPressUp;
float ifPressDown;

//
// 0-255 Range Data initilaized to middle value
//
int mTemperature = 128;
int mHumidity = 128;
int mPressure = 128;
int mPrecipProb = 128;
int mWindSpeed = 128;
int mWindDir = 128;
int mTide = 128;
int mMinTide = 128;
int mMaxTide = 128;

////////////////////////
// Initialize Program //
////////////////////////
void setup() {

//
// Serial Initialize
//
  if (debug == 1) {
    Serial.begin(9600);
    ltoa(Time.now(),buffer,10);
    Serial.print(strcat(buffer,": Initialized\r\n"));
  }

//
// Subscribe to Webhooks
//
  Particle.subscribe("hook-response/forecastio_webhook", gotWeatherData, MY_DEVICES);
  Particle.subscribe("hook-response/wundergroundtide_webhook", gotTideData, MY_DEVICES);
  delay(5000);

//
// Define Pinmodes
//
  pinMode(pTemperature,   OUTPUT);
  pinMode(pHumidity,      OUTPUT);
  pinMode(pPressure,      OUTPUT);
  pinMode(pPrecipProb,    OUTPUT);
  pinMode(pWindSpeed,     OUTPUT);
  pinMode(pWindDir,       OUTPUT);
  pinMode(pTide,          OUTPUT);
  pinMode(pAlert,         OUTPUT);
  pinMode(pPressUp,       OUTPUT);
  pinMode(pPressDown,     OUTPUT);
  pinMode(pDayOne,        INPUT);
  pinMode(pDayTwo,        INPUT);
  pinMode(pDayThree,      INPUT);
  delay(5000);

//
// Define Timezone
//    
  Time.zone(-5); //Set time zone as eastern
  delay(5000);

//
// Publish Webhook to Cloud
//  
  Particle.publish("forecastio_webhook");
  Particle.publish("wundergroundtide_webhook");
  lastRefreshWeather = Time.now();
  lastRefreshTide = Time.now();
  delay(5000);
}

///////////////
// Main Loop //
///////////////
void loop() {

//
// Blink Alert if no weather is pulled down 
//    
  if( (Time.now() - lastGotWeather) > 1200 ||
      (Time.now() - lastGotTide) > 900) {
    if( (Time.now() - lastGotWeather) > 1200) {
      Particle.publish("Failed Weather Update");
    }
    if( (Time.now() - lastGotTide) > 900) {
      Particle.publish("Failed Tide Update");
    }
    digitalWrite(pAlert, LOW);
    delay(500);
    digitalWrite(pAlert, HIGH);
    delay(500);
    digitalWrite(pAlert, LOW);
    delay(500);
    digitalWrite(pAlert, HIGH);
    delay(500);
    digitalWrite(pAlert, LOW);
    delay(500);
    digitalWrite(pAlert, HIGH);
    delay(500);
    digitalWrite(pAlert, LOW);
  }

//
// Check for weather refresh
//
  if ((Time.now() - lastRefreshWeather) > refreshWeather) {
    if (debug == 1) {
      ltoa(Time.now(),buffer,10);
      Serial.print(strcat(buffer,": Request Weather Refresh\r\n"));
    }
    Particle.publish("forecastio_webhook");
    lastRefreshWeather = Time.now();
    delay(10000);
  }

//
// Check for Tide refresh
//
  if ((Time.now() - lastRefreshTide) > refreshTide) {
    if (debug == 1) {
      ltoa(Time.now(),buffer,10);
      Serial.print(strcat(buffer,": Request Tide Refresh\r\n"));
    }
    Particle.publish("wundergroundtide_webhook");
    lastRefreshTide = Time.now();
    delay(10000);
  }

//
// Write out all pins
//
  analogWrite(pTemperature,   mTemperature);
  analogWrite(pHumidity,      mHumidity   );
  analogWrite(pPressure,      mPressure   );
  analogWrite(pPrecipProb,    mPrecipProb );
  analogWrite(pWindSpeed,     mWindSpeed  );
  analogWrite(pWindDir,       mWindDir    );
  analogWrite(pTide,          mTide);

//
// Wait one second, End Loop
//
  delay(1000);
}


///////////////////////////////////////
// Weather Webhook Response Function //
///////////////////////////////////////
void gotWeatherData(const char *name, const char *data) {

//
// Initialize
//    
  String str = String(data);
  char strBuffer[200] = "";
  str.toCharArray(strBuffer, 200);
//
// Get time
//
  lastGotWeather = Time.now();

//  
//  DEBUG
//
  if (debug == 1) {
    ltoa(Time.now(),buffer,10);
    Serial.println(strcat(buffer,": Raw Weather: " + str));
  }

//
// Parse String
//
  currentTemperature       = atof(strtok(strBuffer, "~"));
  currentHumidity          = atof(strtok(NULL, "~"));
  currentPressure          = atof(strtok(NULL, "~"));
  currentPrecipProb        = atof(strtok(NULL, "~"));
  currentWindSpeed         = atof(strtok(NULL, "~"));
  currentWindDir           = atof(strtok(NULL, "~"));
  d0TempMax                = atof(strtok(NULL, "~"));
  d0TempMin                = atof(strtok(NULL, "~"));
  d0Humidity               = atof(strtok(NULL, "~"));
  d0Pressure               = atof(strtok(NULL, "~"));
  d0PrecipProb             = atof(strtok(NULL, "~"));
  d0WindSpeed              = atof(strtok(NULL, "~"));
  d0WindDir                = atof(strtok(NULL, "~"));
  d1TempMax                = atof(strtok(NULL, "~"));
  d1TempMin                = atof(strtok(NULL, "~"));
  d1Humidity               = atof(strtok(NULL, "~"));
  d1Pressure               = atof(strtok(NULL, "~"));
  d1PrecipProb             = atof(strtok(NULL, "~"));
  d1WindSpeed              = atof(strtok(NULL, "~"));
  d1WindDir                = atof(strtok(NULL, "~"));
  d2TempMax                = atof(strtok(NULL, "~"));
  d2TempMin                = atof(strtok(NULL, "~"));
  d2Humidity               = atof(strtok(NULL, "~"));
  d2Pressure               = atof(strtok(NULL, "~"));
  d2PrecipProb             = atof(strtok(NULL, "~"));
  d2WindSpeed              = atof(strtok(NULL, "~"));
  d2WindDir                = atof(strtok(NULL, "~"));
  ifAlert                  = atof(strtok(NULL, "~"));
  
//
// Debug
//
  if (debug == 2) {
    Particle.publish("Temp", String(currentTemperature));
    delay(1000);
    Particle.publish("Humidity", String(currentHumidity));
    delay(1000);
    Particle.publish("Pressure", String(currentPressure));
    delay(1000);
    Particle.publish("PrecipProb", String(currentPrecipProb));
    delay(1000);
    Particle.publish("WindSpeed", String(currentWindSpeed));
    delay(1000);
    Particle.publish("Wind Dir", String(currentWindDir));
    delay(1000);
    Particle.publish("Day0 Temp Max", String(d0TempMax));
    delay(1000);
    Particle.publish("Day0 Temp Min", String(d0TempMin));
    delay(1000);
    Particle.publish("Day0 Humidity", String(d0Humidity));
    delay(1000);
    Particle.publish("Day0 Pressure", String(d0Pressure));
    delay(1000);
    Particle.publish("Day0 PrecipProb", String(d0PrecipProb));
    delay(1000);
    Particle.publish("Day0 WindSpeed", String(d0WindSpeed));
    delay(1000);
    Particle.publish("Day0 Wind Dir", String(d0WindDir));
    delay(1000);
    Particle.publish("Day1 Temp Max", String(d1TempMax));
    delay(1000);
    Particle.publish("Day1 Temp Min", String(d1TempMin));
    delay(1000);
    Particle.publish("Day1 Humidity", String(d1Humidity));
    delay(1000);
    Particle.publish("Day1 Pressure", String(d1Pressure));
    delay(1000);
    Particle.publish("Day1 PrecipProb", String(d1PrecipProb));
    delay(1000);
    Particle.publish("Day1 WindSpeed", String(d1WindSpeed));
    delay(1000);
    Particle.publish("Day1 Wind Dir", String(d1WindDir));
    delay(1000);
    Particle.publish("Day2 Temp Max", String(d2TempMax));
    delay(1000);
    Particle.publish("Day2 Temp Min", String(d2TempMin));
    delay(1000);
    Particle.publish("Day2 Humidity", String(d2Humidity));
    delay(1000);
    Particle.publish("Day2 Pressure", String(d2Pressure));
    delay(1000);
    Particle.publish("Day2 PrecipProb", String(d2PrecipProb));
    delay(1000);
    Particle.publish("Day2 WindSpeed", String(d2WindSpeed));
    delay(1000);
    Particle.publish("Day2 Wind Dir", String(d2WindDir));
    delay(1000);
  }

//
// Convert From Sandard Units to 0-255 range for output
//                                                                      min          max        min
  mTemperature        = (int) constrain( (255.0) * (currentTemperature       - 0.0)      / (100.0    - 0.0),     0, 255 );
  mHumidity           = (int) constrain( (255.0) * (currentHumidity          - 0.0)      / (1.0      - 0.0),     0, 255 );
  mPressure           = (int) constrain( (255.0) * (currentPressure          - 960.0)    / (1060.0   - 960.0),   0, 255 );
  mPrecipProb         = (int) constrain( (255.0) * (currentPrecipProb - 0.0)      / (1.0      - 0.0),     0, 255 );
  mWindSpeed          = (int) constrain( (255.0) * (currentWindSpeed         - 0.0)      / (30.0     - 0.0),     0, 255 );
  mWindDir             = (int) constrain( (255.0) * (currentWindDir - 17)  / (322.0), 0, 255 );

//
// Write if alert
//    
  if (ifAlert > 0.0) {
    digitalWrite(pAlert, HIGH);
  }
  else {
    digitalWrite(pAlert, LOW);
  }

//
// Check pressure for rise/fall write to pins
//
  if ( currentPressure >= previousPressure + pressureDelta ) {
    previousPressure = currentPressure;
    digitalWrite(pPressUp, HIGH);
    digitalWrite(pPressDown, LOW);
    if (debug == 2) {
      Particle.publish("Pressure", "Up");
    }
  }
  else if ( currentPressure <= previousPressure - pressureDelta ) {
    previousPressure = currentPressure;
    digitalWrite(pPressUp, LOW);
    digitalWrite(pPressDown, HIGH);
    if (debug == 2) {
      Particle.publish("Pressure", "Down");
    }
  }
}

/////////////////////////////////////////
// Tide Data Webhook Respnse Function  //
/////////////////////////////////////////
void gotTideData(const char *name, const char *data) {
//
// Initialize
//    
  String str = String(data);
  char strBuffer[125] = "";
  str.toCharArray(strBuffer, 125);
//
// Get Time
//
  lastGotTide = Time.now();

//  
//  DEBUG
//
  if (debug == 1) {
    ltoa(Time.now(),buffer,10);
    Serial.println(strcat(buffer,": Raw Tide: " + str));
  }

//
// Parse String
//
  tide         = atof(strtok(strBuffer, "~"));
  minTide      = atof(strtok(NULL, "~"));
  maxTide      = atof(strtok(NULL, "~"));

//
// Convert From Sandard Units to 0-255 range for output
//
  mTide    = (int) constrain( (255.0) * (tide    + 0.86) /  2.71,  0, 255 );
  mMaxTide = (int) constrain( (255.0) * (minTide + 0.86) /  2.71,  0, 255 );
  mMaxTide = (int) constrain( (255.0) * (maxTide + 0.86) /  2.71,  0, 255 );
}
