#include <Picovoice_EN.h>
#include <Adafruit_NeoPixel.h>
#include "params.h"
#include "DFRobotDFPlayerMini.h"

#define MEMORY_BUFFER_SIZE (70 * 1024)
#define PIN1 5
#define PIN2 7
#define NUM_LEDS1 24
#define NUM_LEDS2 12
#define SENSOR_PIN 2
#define MIN_TOUCH_DURATION 1000 // 1 second

volatile bool val = false;

static const char *ACCESS_KEY = "ZYVuceCnoLjpOZHD0FJrlIaFbrs14HpeCJGdsaS0qA/FE3hY6KC9sg=="; // AccessKey string Picovoice Console

static pv_picovoice_t *handle = NULL;

static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

static const float PORCUPINE_SENSITIVITY = 0.75f;
static const float RHINO_SENSITIVITY = 0.5f;
static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
static const bool RHINO_REQUIRE_ENDPOINT = true;
volatile bool wake_word_detected = false;
volatile bool intent_detected = false;
volatile bool is_understood = false;
volatile int num_leds_on = 0;

unsigned long lastTouchTime = 0;
int currentPixel1 = 0;
int currentPixel2 = 0;
bool firstRingComplete = false;



unsigned long startTime = 0; 
unsigned long endTime = 0; 
bool isTouched = false;     
int touchCount = 0;

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LEDS1, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_LEDS2, PIN2, NEO_GRB + NEO_KHZ800);
DFRobotDFPlayerMini myDFPlayer;

volatile bool is_drink = false;


static void turn_off_check(void) {
  
    for (int i = 0; i < NUM_LEDS1; i++) {
        strip1.setPixelColor(i, strip1.Color(0, 0, 0)); 
    strip1.show(); 

    for (int i = 0; i < NUM_LEDS2; i++) {
        strip2.setPixelColor(i, strip2.Color(0, 0, 0)); 
    }
    strip2.show(); 

    Serial.print("turnoffcheckok");
}

static void listening(void) {
    static unsigned long lastChangeTime = 0;
    static int state = 0;

    unsigned long currentTime = millis();

    if (currentTime - lastChangeTime >= 500) { // 5000 ms = 5 secondi
        lastChangeTime = currentTime;
        state = (state + 1) % 2; // Alterna tra 0 e 1

        if (state == 0) {
          
            for (int i = 0; i < NUM_LEDS2; i++) {
                strip2.setPixelColor(i, strip2.Color(0, 0, 100)); 
            }
            
        } else {
            // Stato 1: Diminuire la luminosità dei LED a 10
            for (int i = 0; i < NUM_LEDS2; i++) {
                strip2.setPixelColor(i, strip2.Color(0, 0, 80));
            }
            
        }

        strip1.show();
        strip2.show();
    }
}

static void Speaking(void) {
    static unsigned long lastChangeTime = 0;
    static int state = 0;

    unsigned long currentTime = millis();

    if (currentTime - lastChangeTime >= 500) { 
        lastChangeTime = currentTime;
        state = (state + 1) % 2; // Alterna tra 0 e 1

        if (state == 0) {
            for (int i = 0; i < NUM_LEDS2; i++) {
                strip2.setPixelColor(i, strip2.Color(20, 20, 20)); 
            }
            
        } else {
            for (int i = 0; i < NUM_LEDS2; i++) {
                strip2.setPixelColor(i, strip2.Color(10, 10, 10)); 
            }
            
        }

        strip1.show();
        strip2.show();
    }
}

static void Stack_mode(void) {
    static unsigned long lastChangeTime = 0;
    static int state = 0;

    unsigned long currentTime = millis();

    if (currentTime - lastChangeTime >= 500) { // 5000 ms = 5 secondi
        lastChangeTime = currentTime;
        state = (state + 1) % 2; // Alterna tra 0 e 1

        if (state == 0) {
            // Stato 0: Imposta la luminosità dei LED a 20
            for (int i = 0; i < NUM_LEDS2; i++) {
                strip2.setPixelColor(i, strip2.Color(20, 0, 0)); // Imposta il colore blu
            }
            
        } else {
            // Stato 1: Diminuire la luminosità dei LED a 10
            for (int i = 0; i < NUM_LEDS2; i++) {
                strip2.setPixelColor(i, strip2.Color(10, 0, 0)); // Dim LED blu
            }
            
        }

        strip1.show();
        strip2.show();
    }
}

static void wake_word_callback(void) {
    Serial.println("ok luna is detected!");
    wake_word_detected = true;
    intent_detected = false;
    

    turn_off_check();

    for (int i = 0; i < 12; i++) {
        strip2.setPixelColor(i, strip2.Color(100, 100, 100));
        strip2.show();
        delay(25); // Attende 25 ms
        strip2.setPixelColor(i, strip2.Color(0, 0, 0)); 
        strip2.show();
    }
    delay(500);

    for (int i = 0; i < NUM_LEDS2; i++) {
        strip2.setPixelColor(i, strip2.Color(0, 0, 0));
    }

    for (int i = 0; i < NUM_LEDS1; i++) {
        strip1.setPixelColor(i, strip1.Color(0, 0, 0));
    }
    strip1.show();
    strip2.show();
    delay(500);
}

static void inference_callback(pv_inference_t *inference) {
    Serial.println("{");
    Serial.print("    is_understood : ");
    intent_detected = true;
    wake_word_detected = false;
    Serial.println(inference->is_understood ? "true" : "false");

    if (inference->is_understood) {
        is_understood = true;
        Serial.print("    intent : ");
        Serial.println(inference->intent);

        if (inference->num_slots > 0) {
            Serial.println("    slots : {");
            for (int32_t i = 0; i < inference->num_slots; i++) {
                Serial.print("        ");
                Serial.print(inference->slots[i]);
                Serial.print(" : ");
                Serial.println(inference->values[i]);
            }
            Serial.println("    }");
        }



            //Show me the water drunk
            if (strcmp(inference->intent, "show_water") == 0) {
              turn_off_check();
              val = true;
              Serial.println("Intent: Show_water");
              Serial.println("val=true");
              Speaking();
              myDFPlayer.play(15);
              delay (4000);
              turn_off_check();

            }
            //Show Me My  period Diary
            else if (strstr(inference->intent, "show_menstrual") != NULL) {
            Serial.println("Intent: show_menstrual");
            turn_off_check();
            Speaking();
            myDFPlayer.play(14);
            delay (7000);
            turn_off_check();
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 3; j++) {
                    int ledIndex = i * 4 + j; // Calcola l'indice del LED
                    if (ledIndex < NUM_LEDS1) {
                        strip1.setPixelColor(ledIndex, strip1.Color(25, 0, 0)); // Imposta il colore rosso
                        strip1.show();
                        delay(100);
                    }
                }

                int ledIndex = i * 4 + 3;
                if (ledIndex < NUM_LEDS1) {
                    strip1.setPixelColor(ledIndex, strip1.Color(0, 0, 0)); // Spegni il LED
                    strip1.show();
                    
                    delay(100);
                }
            }
        }

        //Show Me my antibiotics diary
        else if (strstr(inference->intent, "show_pills") != NULL) {
        Serial.println("Intent: show_pills");
        turn_off_check();
        myDFPlayer.play(16);
        delay (4000);
        turn_off_check();
        
    
        int count = 0;  // reset after showing antibiotics diary
    
        for (int i = 0; i < 17 && count < 7; i += 2) {
            strip1.setPixelColor(i, strip1.Color(0, 100, 0)); // blu
            strip1.show();
            delay(100);
            count++;
        }
   
    
    // 4 pills per day
            for (int i = 0; i < 4; i++) {
                strip2.setPixelColor(i, strip2.Color(0, 100, 0)); // green
                strip2.show();
                delay(100);
            }

            // how many days finished
            for (int i = 8; i < 12; i++) {
                strip2.setPixelColor(i, strip2.Color(0, 0, 100)); // blue
                strip2.show();
                delay(100);
            }
        }


        //Good morning activation
        else if (strstr(inference->intent, "Good_morning") != NULL) {
          
          Serial.println("Intent: Good_morning");
          turn_off_check();
          myDFPlayer.play(3);
          Speaking();
          delay (4000);
          turn_off_check();

        }

        //i am going to school
        else if (strstr(inference->intent, "water_1") != NULL) {
            Serial.println("Intent: water_1");
            turn_off_check();
           myDFPlayer.play(4);
           Speaking();
          delay (9000);
          turn_off_check();
        }
        //I am at home
        else if (strstr(inference->intent, "water_2") != NULL) {
            Serial.println("Intent: water_2");
            turn_off_check();

            // Set LED in blue colour

            myDFPlayer.play(5);
            Speaking();
          delay (5000);
          turn_off_check();

        }
           //i fell tired after school
         else if (strstr(inference->intent, "water_3") != NULL) {
            Serial.println("Intent: water_3");
            turn_off_check();
            // Set LED in blue colour
            myDFPlayer.play(10);
            Speaking();
          delay (10000);
          turn_off_check();

        }
        //my period started today
         else if (strstr(inference->intent, "Menstrual_1") != NULL) {
            Serial.println("Intent: Menstrual_1");
            turn_off_check();
            // Set LED in blue colour
            myDFPlayer.play(13);
            Speaking();
          delay (9000);
          turn_off_check();

        }
        //update my period schedule
         else if (strstr(inference->intent, "Menstrual_2") != NULL) {
            Serial.println("Intent: Menstrual_2");
            turn_off_check();
            // Set LED in blue colour
            myDFPlayer.play(6);
            Speaking();
          delay (13000);
          turn_off_check();

        }
        //i have pain during peeing
        else if (strstr(inference->intent, "Antibiotics_1") != NULL) {
            Serial.println("Intent: Antibiotics_1");
            turn_off_check();
            // CSet LED in blue colour 
           myDFPlayer.play(1);
           Speaking();
          delay (15000);
          turn_off_check();
        }
        //Is it possible to treat without visiting doctor
        else if (strstr(inference->intent, "Antibiotics_2") != NULL) {
            Serial.println("Intent: Antibiotics_2");
            turn_off_check();
            // Set LED in blue colour
           myDFPlayer.play(2);
           Speaking();
          delay (24000);
          turn_off_check();
        }
         //doctor prescribe me antibiotics
         else if (strstr(inference->intent, "Antibiotics_3") != NULL) {
            Serial.println("Intent: Antibiotics_3");
            turn_off_check();
            myDFPlayer.play(9);
            Speaking();
          delay (5000);
          turn_off_check();
           
        }
        //I have to take 3 pills a day next 7 days
         else if (strstr(inference->intent, "Antibiotics_4") != NULL) {
            Serial.println("Intent: Antibiotics_4");
            turn_off_check();
            myDFPlayer.play(12);
            Speaking();
          delay (16000);
          turn_off_check();

        }
    }else{
      is_understood=false;
    }
    Serial.println("}\n");
    pv_inference_delete(inference);
}

static void drinking(void) {
    // touch controller (water intake)
    static int touchCount = 0;
    

    Serial.println("is pressed");

    // controlling the touch sensor
    touchCount++;
    Serial.print("Touchings: ");
    Serial.println(touchCount);

    // Accendi i LED in base al numero di tocchi
    int numLEDsToLight = map(touchCount, 0, 5, 0, NUM_LEDS1);

    for (int i = 0; i < NUM_LEDS1; i++) {
        if (i < numLEDsToLight) {
            strip1.setPixelColor(i, strip1.Color(0, 0, 20)); // Green color
        } else {
            strip1.setPixelColor(i, strip1.Color(0, 0, 0)); // reset LED
        }
    }
   
    strip1.show();

    // if touchcount is filled (1st ring), play the water message
    if (touchCount >= 5) {
        firstRingComplete = true;
        
        Speaking();
        myDFPlayer.play(12); //vocal when the device is attatched for the first time
        delay(4000);
        turn_off_check();

        // Reset touch count after playing audio (water intake)
        touchCount = 0;
        currentPixel1 = 0; // Reset the LED index
    }
}


static void print_error_message(char **message_stack, int32_t message_stack_depth) {
    for (int32_t i = 0; i < message_stack_depth; i++) {
        Serial.println(message_stack[i]);
    }
}

static void starting() {
    for (int i = 0; i < NUM_LEDS1; i++) {
        strip1.setPixelColor(i, strip1.Color(50, 50, 50)); // Imposta il colore nero per spegnere il LED
    }
    strip1.show(); // Aggiorna il display del primo anello

    for (int i = 0; i < NUM_LEDS2; i++) {
        strip2.setPixelColor(i, strip2.Color(50, 50, 50)); // Imposta il colore nero per spegnere il LED
    }
    strip2.show(); // Aggiorna il display del secondo anello

    Serial.print("turnoffcheckok");
}



void setup() {
    

    Serial.begin(9600);
    Serial1.begin(9600);
    while (!Serial);
    Serial.begin(115200);
    // startTime = millis();
    // isLunaSpeakingActive = true;
    pinMode(SENSOR_PIN, INPUT);

    strip1.begin();
    strip1.show(); // Inizializza tutti i LED del primo anello a spenti

    strip2.begin();
    strip2.show(); // Inizializza tutti i LED del secondo anello a spenti

    if (!myDFPlayer.begin(Serial1, true, true)) {
        Serial.print("Connecting ...");
        while (!myDFPlayer.begin(Serial1)) {
            Serial.print(".");
        }
        Serial.println("!");
    }
    Serial.println("Connected!");
    myDFPlayer.volume(29);  // Set volume value. From 0 to 30
    starting();
    myDFPlayer.play(7); //vocal when the device is attatched for the first time
    delay(5000);
    turn_off_check();
    

    

    pv_status_t status = pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        Serial.print("Audio init failed with ");
        Serial.println(pv_status_to_string(status));
        while (1);
    }

    char **message_stack = NULL;
    int32_t message_stack_depth = 0;
    pv_status_t error_status;

    status = pv_picovoice_init(
        ACCESS_KEY,
        MEMORY_BUFFER_SIZE,
        memory_buffer,
        sizeof(KEYWORD_ARRAY),
        KEYWORD_ARRAY,
        PORCUPINE_SENSITIVITY,
        wake_word_callback,
        sizeof(CONTEXT_ARRAY),
        CONTEXT_ARRAY,
        RHINO_SENSITIVITY,
        RHINO_ENDPOINT_DURATION_SEC,
        RHINO_REQUIRE_ENDPOINT,
        inference_callback,
        &handle);
    if (status != PV_STATUS_SUCCESS) {
        Serial.print("Picovoice init failed with ");
        Serial.println(pv_status_to_string(status));

        error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
        if (error_status != PV_STATUS_SUCCESS) {
            Serial.println("Unable to get Porcupine error state");
            while (1);
        }
        print_error_message(message_stack, message_stack_depth);
        pv_free_error_stack(message_stack);
        while (1);
    }

    const char *rhino_context = NULL;
    status = pv_picovoice_context_info(handle, &rhino_context);
    if (status != PV_STATUS_SUCCESS) {
        Serial.print("retrieving context info failed with");
        Serial.println(pv_status_to_string(status));
        while (1);
    }
    Serial.println("Wake word: 'ok luna'");
    Serial.println(rhino_context);
   
}

void loop() {
    if (val == true) {
        unsigned long currentTime = millis();
        int state = digitalRead(SENSOR_PIN);

        if (state == HIGH) {
            // Serial.println("tocco");
            if (!isTouched) {
                startTime = millis();
                isTouched = true;
            } else if (millis() - startTime > 1000) {
                touchCount++;
                Serial.print("Tocchi: ");
                Serial.println(touchCount);
                startTime = millis();
                if (touchCount == 1) {
                    Serial.println("1 audio");
                    for (int i = 0; i < 5; i++) {
                        strip1.setPixelColor(i, strip1.Color(0, 0, 100));
                    }
                    strip1.show();
                } else if (touchCount == 2) {
                    Serial.println("2 audio");
                    for (int i = 5; i < 10; i++) {
                        strip1.setPixelColor(i, strip1.Color(0, 0, 100));
                    }
                    strip1.show();
                } else if (touchCount == 3) {
                    Serial.println("3 audio");
                    for (int i = 10; i < 15; i++) {
                        strip1.setPixelColor(i, strip1.Color(0, 0, 100));
                    }
                    strip1.show();
                } else if (touchCount == 4) {
                    Serial.println("4 audio");
                    for (int i = 15; i < 20; i++) {
                        strip1.setPixelColor(i, strip1.Color(0, 0, 100));
                    }
                    strip1.show();
                } else if (touchCount == 5) {
                    Serial.println("5 audio");
                    for (int i = 20; i < 24; i++) {
                        strip1.setPixelColor(i, strip1.Color(0, 0, 100));
                    }
                    myDFPlayer.play(11);
                    Speaking();
                    delay(4000);
                    turn_off_check();
                    strip1.show();

                    // Reset variables
                    val = false;
                    Serial.println("val=false");
                    touchCount = 0;
                    isTouched = false;
                }
            }
        } else {
            isTouched = false;
            for (int i = 23; i >= 0; i--) {
                strip1.setPixelColor(i, strip1.Color(0, 0, 0));
            }
            strip1.show();
        }
    }

    

   
    const int16_t *buffer = pv_audio_rec_get_new_buffer();
    if (buffer) {
        const pv_status_t status = pv_picovoice_process(handle, buffer);
        if (status != PV_STATUS_SUCCESS) {
            Serial.print("Picovoice process failed with ");
            Serial.println(pv_status_to_string(status));
            char **message_stack = NULL;
            int32_t message_stack_depth = 0;
            pv_get_error_stack(&message_stack, &message_stack_depth);
            for (int32_t i = 0; i < message_stack_depth; i++) {
                Serial.println(message_stack[i]);
            }
            pv_free_error_stack(message_stack);
            while (1);
        }
    }

    if (wake_word_detected && !intent_detected && !is_understood) {
        listening();
    } else if (wake_word_detected ) {
        listening();
        // Serial.println("parla l'assistente");
    } else if (!wake_word_detected && intent_detected && !is_understood) {
        Stack_mode();
        // Serial.println("errore");
    }
}

