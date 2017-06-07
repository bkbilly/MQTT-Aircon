#include <IRremoteESP8266.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define DEBUG 1
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_password = "";
const int mqtt_port = 16825;

WiFiClient espClient;
PubSubClient client(espClient);

IRsend irsend(2);

unsigned rawmessage[139];
char mode[10] = "Cool";
char state[10] = "Off";
int speed = 0;
char swing[10] = "Off";
int sleep = 0;
int temp = 21;


void binaryAddLimit(char* out, char first[], char plus[], int binsize) {
  char result[binsize+1];

  int carrier = 0;
  int i;
  for (i = 0; i < binsize; ++i) {
    if (first[i] == '0' && plus[i] == '0' && carrier == 0) {
        result[i] = '0';
        carrier = 0;
    } else if (first[i] == '1' && plus[i] == '0' && carrier == 0) {
        result[i] = '1';
        carrier = 0;
    } else if (first[i] == '0' && plus[i] == '1' && carrier == 0) {
        result[i] = '1';
        carrier = 0;
    } else if (first[i] == '1' && plus[i] == '1' && carrier == 0) {
        result[i] = '0';
        carrier = 1;
    } else if (first[i] == '0' && plus[i] == '0' && carrier == 1) {
        result[i] = '1';
        carrier = 0;
    } else if (first[i] == '1' && plus[i] == '0' && carrier == 1) {
        result[i] = '0';
        carrier = 1;
    } else if (first[i] == '0' && plus[i] == '1' && carrier == 1) {
        result[i] = '0';
        carrier = 1;
    } else if (first[i] == '1' && plus[i] == '1' && carrier == 1) {
        result[i] = '1';
        carrier = 1;
    }
  }
  result[binsize] = '\0';
  i = 0;
  char tmp;
  while (carrier == 1 && i < binsize) {
      if (result[i] == '0') {
          tmp = '1';
          carrier = 0;
      }
      else if (result[i] == '1') {
          tmp = '0';
      }
      result[i] = tmp;
      i += 1;
  }
  strcpy(out, result);
}

void getBinState(char* out, char state[]) {
  if (strcmp(state,"Off")==0) {
    strcpy(out, "0");
  } else if (strcmp(state,"On")==0) {
    strcpy(out, "1");
  } else {
    strcpy(out, "Erro");
  }
}
void getBinMode(char* out, char mode[]) {
  if (strcmp(mode,"Auto")==0) {
    strcpy(out, "000");
  } else if (strcmp(mode,"Cool")==0) {
    strcpy(out, "100");
  } else if (strcmp(mode,"Dry")==0) {
    strcpy(out, "010");
  } else if (strcmp(mode,"Fan")==0) {
    strcpy(out, "110");
  } else if (strcmp(mode,"Heat")==0) {
    strcpy(out, "001");
  } else {
    strcpy(out, "Erro");
  }
}
void getBinSpeed1(char* out, int speed) {
  char* speeds[] = {"00", "10", "01", "11", "11"};
  if (0 <= speed && speed <= 4) 
    strcpy(out, speeds[speed]);
  else
    strcpy(out, "E");
}
void getBinSpeed2(char* out, int speed) {
  char* speeds[] = {"0", "0", "0", "0", "1"}; 
  if (0 <= speed && speed <= 4) 
    strcpy(out, speeds[speed]);
  else
    strcpy(out, "E");
}
void getBinSwing1(char* out, char swing[]) {
  if (strcmp(swing,"On")==0) {
    strcpy(out, "1");
  } else {
    strcpy(out, "0");
  }
}
void getBinSwing2(char* out, char swing[]) {
  if (strcmp(swing,"On")==0) {
    strcpy(out, "00110");
  } else {
    strcpy(out, "00000");
  }
}
void getBinSleep(char* out, int sleep) {
  if (sleep) {
    strcpy(out, "1");
  } else {
    strcpy(out, "0");
  }
}
void getBinTemp(char* out, int temp) {
  switch ( temp ) {
    case 16:
      strcpy(out, "0000");
      break;
    case 17:
      strcpy(out, "1000");
      break;
    case 18:
      strcpy(out, "0100");
      break;
    case 19:
      strcpy(out, "1100");
      break;
    case 20:
      strcpy(out, "0010");
      break;
    case 21:
      strcpy(out, "1010");
      break;
    case 22:
      strcpy(out, "0110");
      break;
    case 23:
      strcpy(out, "1110");
      break;
    case 24:
      strcpy(out, "0001");
      break;
    case 25:
      strcpy(out, "1001");
      break;
    case 26:
      strcpy(out, "0101");
      break;
    case 27:
      strcpy(out, "1101");
      break;
    case 28:
      strcpy(out, "0011");
      break;
    case 29:
      strcpy(out, "1011");
      break;
    case 30:
      strcpy(out, "0111");
      break;
    default:
      strcpy(out, "Error");
      break;
  }
}
void getBinChecksum(char* out, char bintemp[], char swing[], char state[], char mode[]) {
  char result[10];
  char plus[10];

  if (strcmp(mode,"Auto")==0) {
    strcpy(plus, "0010");
  } else if (strcmp(mode,"Cool")==0) {
    strcpy(plus, "1010");
  } else if (strcmp(mode,"Dry")==0) {
    strcpy(plus, "0110");
  } else if (strcmp(mode,"Fan")==0) {
    strcpy(plus, "1110");
  } else if (strcmp(mode,"Heat")==0) {
    strcpy(plus, "0001");
  }
  binaryAddLimit(result, bintemp, plus, 4);

  if (strcmp(mode,"On")==0) {
    strcpy(plus, "1000");
  } else {
    strcpy(plus, "0000");
  }
  binaryAddLimit(result, result, plus, 4);

  if (strcmp(state,"Off")==0) {
    strcpy(plus, "0001");
  } else if (strcmp(state,"On")==0) {
    strcpy(plus, "0000");
  }
  binaryAddLimit(result, result, plus, 4);

  if (DEBUG >= 3){
    printf("\n checksumprint \n");
    for (int i = 0; i < 4; ++i) {
      printf("bit: %c\n", result[i]);
    }
    printf("\n temp \n");
    for (int i = 0; i < 4; ++i) {
      printf("%c", bintemp[i]);
    }
    printf("\n \n");
  }

  strcpy(out, result);
}
void getBinSame1(char* out) {
  strcpy(out, "00000000");
}
void getBinSame2(char* out) {
  strcpy(out, "110000010100102");
}
void getBinSame3(char* out) {
  strcpy(out, "00000000100000000000000");
}
void getBinCommand(char* out, char state[], char mode[], int speed, char swing[], int sleep, int temp) {
  char binState[10];
  char binMode[10];
  char binSpeed1[10];
  char binSpeed2[10];
  char binSwing1[10];
  char binSwing2[10];
  char binSleep[10];
  char binTemp[10];
  char binChecksum[10];
  char binSame1[20];
  char binSame2[20];
  char binSame3[30];

  getBinState(binState, state);
  getBinMode(binMode, mode);
  getBinSpeed1(binSpeed1, speed);
  getBinSpeed2(binSpeed2, speed);
  getBinSwing1(binSwing1, swing);
  getBinSwing2(binSwing2, swing);
  getBinSleep(binSleep, sleep);
  getBinTemp(binTemp, temp);
  getBinChecksum(binChecksum, binTemp, swing, state, mode);
  getBinSame1(binSame1);
  getBinSame2(binSame2);
  getBinSame3(binSame3);

  sprintf(out, "%s%s%s%s%s%s%s%s%s%s%s%s", binMode, binState, binSpeed1, binSwing1, binSleep, binTemp, binSame1, binSpeed2, binSame2, binSwing2, binSame3, binChecksum);

  if (DEBUG >= 1) {
    printf("getBinState: %s\n", binState);
    printf("getBinMode: %s\n", binMode);
    printf("getBinSpeed1: %s\n", binSpeed1);
    printf("getBinSpeed2: %s\n", binSpeed2);
    printf("getBinSwing1: %s\n", binSwing1);
    printf("getBinSwing2: %s\n", binSwing2);
    printf("getBinSleep: %s\n", binSleep);
    printf("getBinTemp: %s\n", binTemp);
    printf("getBinChecksum: %s\n", binChecksum);
    printf("getBinSame1: %s\n", binSame1);
    printf("getBinSame2: %s\n", binSame2);
    printf("getBinSame3: %s\n", binSame3);
  }
  if (DEBUG >= 1)
    printf("%s\n", out);
}
void toRaw(unsigned *list, char bindata[]) {
  unsigned n = strlen(bindata);
  //  unsigned n = (unsigned)strlen(bindata);
  //  int size = (n*2) + 3;
  //  unsigned list[size];

  int position = 0;
  int datatowrite;
  list[position] = 9000;
  list[++position] = 4500;
  for (int i = 0; i < n; ++i) {
    list[++position] = 650;
    if (bindata[i] == '0')
      list[++position] = 540;
    else if (bindata[i] == '2')
      list[++position] = 20000;
    else
      list[++position] = 1640;
  }
  list[++position] = 650;

  if (DEBUG >= 2) {
    printf("\nPosition: %d\n", position);
    for (unsigned int i = 0; i != position; i+=2)
    {
      printf("%d %d\n", list[i], list[i+1]);
    }
    printf("%d\n", list[position]);
  }
}




void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected to MQTT");
      // Once connected, publish an announcement...
      client.publish("outTopic", "connected to MQTT");
      // ... and resubscribe
      client.subscribe("/aircond/myroom/getall");
      client.subscribe("/aircond/myroom/mode/set");
      client.subscribe("/aircond/myroom/speed/set");
      client.subscribe("/aircond/myroom/swing/set");
      client.subscribe("/aircond/myroom/sleep/set");
      client.subscribe("/aircond/myroom/temp/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendToIR() {
  printf("\n\n%s - %s - %d - %s - %d - %d \n\n", state, mode, speed, swing, sleep, temp);

  char binCommand[71];
  getBinCommand(binCommand, state, mode, speed, swing, sleep, temp);
  toRaw(rawmessage, binCommand);

  Serial.println("\nSending RAW data... ");
  irsend.sendRaw(rawmessage, 139, 38);
}

void callback(char* topic, byte* payload, unsigned int length) {
  char mqttmessage[300];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i;
  for (i = 0; i < length; i++) {
    mqttmessage[i] = (char)payload[i];
    Serial.print(mqttmessage[i]);
  }
  mqttmessage[i] = '\0';

  if (strcmp(topic,"/aircond/myroom/state/set")==0) {
    strcpy(state, mqttmessage);
    sendToIR();
    client.publish("/aircond/myroom/state", state);
  }
  else if (strcmp(topic,"/aircond/myroom/mode/set")==0) {
    if (strcmp(mqttmessage, "Off")==0) {
      strcpy(state, mqttmessage);
      client.publish("/aircond/myroom/mode", "Off");
    } else {
      strcpy(state, "On");
      strcpy(mode, mqttmessage);
      client.publish("/aircond/myroom/mode", mode);
    }
    sendToIR();
  }
  else if (strcmp(topic,"/aircond/myroom/speed/set")==0) {
    speed = (int) strtol(mqttmessage, (char **)NULL, 10);
    sendToIR();
    char speedstr[5];
    sprintf(speedstr, "%d", speed);
    client.publish("/aircond/myroom/speed", speedstr);
  }
  else if (strcmp(topic,"/aircond/myroom/swing/set")==0) {
    strcpy(swing, mqttmessage);
    sendToIR();
    client.publish("/aircond/myroom/swing", swing);
  }
  else if (strcmp(topic,"/aircond/myroom/sleep/set")==0) {
    sleep = (int) strtol(mqttmessage, (char **)NULL, 10);
    sendToIR();
    char sleepstr[5];
    sprintf(sleepstr, "%d", sleep);
    client.publish("/aircond/myroom/sleep", sleepstr);
  }
  else if (strcmp(topic,"/aircond/myroom/temp/set")==0) {
    temp = (int) strtol(mqttmessage, (char **)NULL, 10);
    sendToIR();
    char tempstr[5];
    sprintf(tempstr, "%d", temp);
    client.publish("/aircond/myroom/temp", tempstr);
  }
  else if (strcmp(topic,"/aircond/myroom/getall")==0) {
    char speedstr[5];
    char sleepstr[5];
    char tempstr[5];
    sprintf(speedstr, "%d", speed);
    sprintf(sleepstr, "%d", sleep);
    sprintf(tempstr, "%d", temp);
    client.publish("/aircond/myroom/state", state);
    client.publish("/aircond/myroom/mode", mode);
    client.publish("/aircond/myroom/speed", speedstr);
    client.publish("/aircond/myroom/swing", swing);
    client.publish("/aircond/myroom/sleep", sleepstr);
    client.publish("/aircond/myroom/temp", tempstr);
  }
}




void setup() {
  // digitalWrite(4, LOW);
  irsend.begin();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}