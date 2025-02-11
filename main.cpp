# define TRIG_PIN 5
# define ECHO_PIN 18
# define LED_PIN 16  
# define BALL_SWITCH_PIN 14  

float previousDistance = 0;  
const float distanceThreshold = 2.4;  
const int numSamples = 5;  
float distanceSamples[numSamples] = {0};  
int sampleIndex = 0;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 300;  

int stableCounter = 0;
const int stableThreshold = 5; 

void setup() {
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BALL_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);  
}

float getFilteredDistance() {
    long duration;
    float distance;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000);  
    if (duration == 0) {
        Serial.println("Ultrasonic sensor timeout!");
        return previousDistance;
    }

    distance = (duration * 0.0343) / 2;  

    distanceSamples[sampleIndex] = distance;
    sampleIndex = (sampleIndex + 1) % numSamples;  

    float sum = 0;
    for (int i = 0; i < numSamples; i++) {
        sum += distanceSamples[i];
    }
    return sum / numSamples;  
}

void loop() {
    bool ballSwitchActivated = false;
    unsigned long currentTime = millis();

    if (digitalRead(BALL_SWITCH_PIN) == LOW && (currentTime - lastDebounceTime) > debounceDelay) {
        stableCounter++;  
        lastDebounceTime = currentTime;
    } else if (digitalRead(BALL_SWITCH_PIN) == HIGH && (currentTime - lastDebounceTime) > debounceDelay) {
        stableCounter = 0;  
        lastDebounceTime = currentTime;
    }

    if (stableCounter >= stableThreshold) {
        ballSwitchActivated = true;
    } else {
        ballSwitchActivated = false;
    }

    float distance = getFilteredDistance();
    float distanceChange = abs(distance - previousDistance);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm, Change: ");
    Serial.print(distanceChange);
    Serial.print(" cm, Ball Switch: ");
    Serial.println(ballSwitchActivated ? "Activated (Good)" : "Stable (Bad)");

    if ((distanceChange >= distanceThreshold) && !ballSwitchActivated) {
        analogWrite(LED_PIN, 80);
        delay(200);
        analogWrite(LED_PIN, 0);
        delay(200);
        Serial.println("ALERT: Possible wall collapse detected!");
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    previousDistance = distance;
    delay(100);
}
