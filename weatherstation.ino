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
int debug = 0;
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
int     refreshWeather      = 90;   // Refresh time in seconds for weather
int     refreshTide         = 300;  // Refresh time in seconds for tide
double  lastRefreshWeather  = 0;    // Unix time of last refresh weather
double  lastRefreshTide     = 0;    // Unix time of last refresh tide
int     lastForecastDay     = -1;   // Calendar Day of last forecast webhook refresh    
double  lastGotWeather      = 0;    // Unix time of last time the webhook data was returned
double  lastGotTide         = 0;    // Unix time of last time the webhook data was returned

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
float d0HumidMin;
float d0HumidMax;
float d0Pressure;
float d0PressMin;
float d0PressMax;
float d0PrecipProb;
float d0WindSpeed;
float d0WindMin;
float d0WindMax;
float d0WindDir;
float d1TempMin;           
float d1TempMax;           
float d1Humidity;
float d1HumidMin;
float d1HumidMax;
float d1Pressure;
float d1PressMin;
float d1PressMax;
float d1PrecipProb;
float d1WindSpeed;
float d1WindMin;
float d1WindMax;
float d1WindDir;
float d2TempMin;           
float d2TempMax;           
float d2Humidity;
float d2HumidMin;
float d2HumidMax;
float d2Pressure;
float d2PressMin;
float d2PressMax;
float d2PrecipProb;
float d2WindSpeed;
float d2WindMin;
float d2WindMax;
float d2WindDir;

float tide;
float minTide;
float maxTide;
float ifAlert;
float ifPressUp;
float ifPressDown;


//
// Time Offset Vars
//
int   timeOffset;
int   currentOffset = 0;

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
  Particle.subscribe("hook-response/wundergroundforecast_webhook", gotForecastData, MY_DEVICES);
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
// Get Data from webhooks
//  
  Particle.publish("forecastio_webhook");
  lastRefreshWeather = Time.now();
  delay(10000);
  Particle.publish("wundergroundtide_webhook");
  lastRefreshTide = Time.now();
  delay(10000);
  Particle.publish("wundergroundforecast_webhook");
  lastForecastDay = Time.weekday();
  delay(10000);
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
//  Check for forecast refresh
//
  if (Time.weekday() != lastForecastDay) {
    Particle.publish("wundergroundforecast_webhook");
    lastForecastDay = Time.weekday();
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
  timeOffset               = atoi(strtok(strBuffer, "~"));
  currentTemperature       = atof(strtok(NULL, "~"));
  currentHumidity          = atof(strtok(NULL, "~"));
  currentPressure          = atof(strtok(NULL, "~"));
  currentPrecipProb        = atof(strtok(NULL, "~"));
  currentWindSpeed         = atof(strtok(NULL, "~"));
  currentWindDir           = atof(strtok(NULL, "~"));
  d0TempMax                = atof(strtok(NULL, "~"));
  d0TempMin                = atof(strtok(NULL, "~"));
  d0PrecipProb             = atof(strtok(NULL, "~"));
  d0WindDir                = atof(strtok(NULL, "~"));
  d1TempMax                = atof(strtok(NULL, "~"));
  d1TempMin                = atof(strtok(NULL, "~"));
  d1PrecipProb             = atof(strtok(NULL, "~"));
  d1WindDir                = atof(strtok(NULL, "~"));
  d2TempMax                = atof(strtok(NULL, "~"));
  d2TempMin                = atof(strtok(NULL, "~"));
  d2PrecipProb             = atof(strtok(NULL, "~"));
  d2WindDir                = atof(strtok(NULL, "~"));
  ifAlert                  = atof(strtok(NULL, "~"));

//
// Check and Set Time Offset
//
  if (timeOffset != currentOffset) {
    Time.zone(timeOffset);
    currentOffset = timeOffset;
  }
  
//
// Debug
//
  if (debug == 2) {
    Particle.publish("TimeOffset", String(timeOffset));
    delay(1000);
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
    Particle.publish("Day0 PrecipProb", String(d0PrecipProb));
    delay(1000);
    Particle.publish("Day0 Wind Dir", String(d0WindDir));
    delay(1000);
    Particle.publish("Day1 Temp Max", String(d1TempMax));
    delay(1000);
    Particle.publish("Day1 Temp Min", String(d1TempMin));
    delay(1000);
    Particle.publish("Day1 PrecipProb", String(d1PrecipProb));
    delay(1000);
    Particle.publish("Day1 Wind Dir", String(d1WindDir));
    delay(1000);
    Particle.publish("Day2 Temp Max", String(d2TempMax));
    delay(1000);
    Particle.publish("Day2 Temp Min", String(d2TempMin));
    delay(1000);
    Particle.publish("Day2 PrecipProb", String(d2PrecipProb));
    delay(1000);
    Particle.publish("Day2 Wind Dir", String(d2WindDir));
    delay(1000);
  }

//
// Convert From Sandard Units to 0-255 range for output
//                                                                      min          max        min
  mTemperature        = (int) constrain( (255.0) * (currentTemperature       - 0.0)      / (100.0    - 0.0),     0, 255 );
  mHumidity           = (int) constrain( (255.0) * (currentHumidity          - 0.0)      / (1.0      - 0.0),     0, 255 );
  mPressure           = (int) constrain( (255.0) * (currentPressure          - 980.0)    / (1030.0   - 980.0),   0, 255 );
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

////////////////////////////////////////////
// Forecast Data Webhook Respnse Function //
////////////////////////////////////////////
void gotForecastData(const char *name, const char *data) {
//
// Initialize
//
  int i;
  float humid;
  float windSp;
  float press;
  String str = String(data);
  char strBuffer[850] = "";
  str.toCharArray(strBuffer, 850);
//
// Get Time
//
  lastForecastDay = Time.weekday();

//
// Parse String
//
  d0HumidMin = atof(strtok(strBuffer, "~"))/100.0;
  d0HumidMax = d0HumidMin;
  d0WindMin  = atof(strtok(NULL, "~"));
  d0WindMax  = d0WindMin;
  d0PressMin = atof(strtok(NULL, "~"));
  d0PressMax = d0PressMin;

  for ( i=1; i<12; i++) {
    humid = atof(strtok(NULL, "~"))/100.0;
    windSp = atof(strtok(NULL, "~"));
    press = atof(strtok(NULL, "~"));
    if (humid < d0HumidMin) d0HumidMin = humid;
    if (humid > d0HumidMax) d0HumidMax = humid;
    if (windSp < d0WindMin)  d0WindMin = windSp;
    if (windSp > d0WindMax)  d0WindMax = windSp;
    if (press < d0PressMin) d0PressMin = press;
    if (press > d0PressMax) d0PressMax = press;
  }

  d1HumidMin = atof(strtok(NULL, "~"))/100.0;
  d1HumidMax = d1HumidMin;
  d1WindMin  = atof(strtok(NULL, "~"));
  d1WindMax  = d1WindMin;
  d1PressMin = atof(strtok(NULL, "~"));
  d1PressMax = d1PressMin;

  for ( i=1; i<12; i++) {
    humid = atof(strtok(NULL, "~"))/100.0;
    windSp = atof(strtok(NULL, "~"));
    press = atof(strtok(NULL, "~"));
    if (humid < d1HumidMin) d1HumidMin = humid;
    if (humid > d1HumidMax) d1HumidMax = humid;
    if (windSp < d1WindMin)  d1WindMin = windSp;
    if (windSp > d1WindMax)  d1WindMax = windSp;
    if (press < d1PressMin) d1PressMin = press;
    if (press > d1PressMax) d1PressMax = press;
  }

  d2HumidMin = atof(strtok(NULL, "~"))/100.0;
  d2HumidMax = d2HumidMin;
  d2WindMin  = atof(strtok(NULL, "~"));
  d2WindMax  = d2WindMin;
  d2PressMin = atof(strtok(NULL, "~"));
  d2PressMax = d2PressMin;

  for ( i=1; i<12; i++) {
    humid = atof(strtok(NULL, "~"))/100.0;
    windSp = atof(strtok(NULL, "~"));
    press = atof(strtok(NULL, "~"));
    if (humid < d2HumidMin) d2HumidMin = humid;
    if (humid > d2HumidMax) d2HumidMax = humid;
    if (windSp < d2WindMin)  d2WindMin = windSp;
    if (windSp > d2WindMax)  d2WindMax = windSp;
    if (press < d2PressMin) d2PressMin = press;
    if (press > d2PressMax) d2PressMax = press;
  }

//
// Debug
//  
  if ( debug == 2 ) {
    Particle.publish("Humidity Min Day0", String(d0HumidMin));
    Particle.publish("Humidity Max Day0", String(d0HumidMax));
    delay(1000);
    Particle.publish("Wind Min Day0", String(d0WindMin));
    Particle.publish("Wind Max Day0", String(d0WindMax));
    delay(1000);
    Particle.publish("Pressure Min Day0", String(d0PressMin));
    Particle.publish("Pressure Max Day0", String(d0PressMax));
    delay(1000);
    Particle.publish("Humidity Min Day1", String(d1HumidMin));
    Particle.publish("Humidity Max Day1", String(d1HumidMax));
    delay(1000);
    Particle.publish("Wind Min Day1", String(d1WindMin));
    Particle.publish("Wind Max Day1", String(d1WindMax));
    delay(1000);
    Particle.publish("Pressure Min Day1", String(d1PressMin));
    Particle.publish("Pressure Max Day1", String(d1PressMax));
    delay(1000);
    Particle.publish("Humidity Min Day2", String(d2HumidMin));
    Particle.publish("Humidity Max Day2", String(d2HumidMax));
    delay(1000);
    Particle.publish("Wind Min Day2", String(d2WindMin));
    Particle.publish("Wind Max Day2", String(d2WindMax));
    delay(1000);
    Particle.publish("Pressure Min Day2", String(d2PressMin));
    Particle.publish("Pressure Max Day2", String(d2PressMax));
    delay(1000);
  }

//
// Convert From Sandard Units to 0-255 range for output
//
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
