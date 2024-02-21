#include <Keypad.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <String>

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns


uint8_t MacAddress2[] = {0xE8, 0xDB, 0x84, 0x00, 0xFB, 0x3C}; //ultrasonic
uint8_t MacAddress6[] = {0x24, 0x6F, 0x28, 0x28, 0x17, 0x1C}; //servo
uint8_t MacAddress4[] = {0xE8, 0x68, 0xE7, 0x23, 0x82, 0x1C}; //led
uint8_t MacAddress1[] = {0xE8, 0xD8, 0x84, 0x01, 0x07, 0x90}; //scan
uint8_t MacAddress3[] = {0x24, 0x6F, 0x28, 0x28, 0x15, 0x94}; //oled
typedef struct card_UID {
  String UID;
}card_UID;

typedef struct send_mode{
  int statuss; // 0 regis 1 reset 2 forget card
}send_mode;

typedef struct send_open{
  int statuss; // 1 open
}send_open;

typedef struct servo_status{
  int servo_status;
} servo_struct;

typedef struct led_status{
  int statuss; // 0 close 1 true 2 worng 3register
}led_status;

typedef struct keypad_oled{
  int text; 
}keypad_oled;

servo_struct receive_open;
send_mode mode_send;
card_UID receive_card;
led_status send_led;
keypad_oled send_oled;
send_open open_sen;


esp_now_peer_info_t peerInfo1; //1
esp_now_peer_info_t peerInfo2; //4 5 6
esp_now_peer_info_t peerInfo3;
esp_now_peer_info_t peerInfo4;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {19, 18, 5, 17}; // GPIO19, GPIO18, GPIO5, GPIO17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};   // GPIO16, GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

String password = "1234"; // change your password here
String input_password;
String nisit_id = "1234";
String input_nisit;
int door_open = 1;//0 open 1 close

bool compareMac(const uint8_t * a, uint8_t * b){
  for(int i=0;i<6;i++){
    if(a[i]!=b[i])
      return false;    
  }
  return true;
}
void send_keypadstring(){
  send_oled.text = 0; //*
  esp_err_t result = esp_now_send(MacAddress3, (uint8_t *) &send_oled, sizeof(keypad_oled)); //mode 2 reset
  if (result == ESP_OK) {
    Serial.println("Sent with success");
    Serial.println(send_oled.text);
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(500);
  return;
}
void creatpass(){
  while(true){
    char key = keypad.getKey();
    if(key){
        Serial.println(key);
        if (key == '#'){
          break;
        if (key == '*') {
          password = ""; // clear input password
        } 
        else {
          send_keypadstring(); //send * to oled
          password += key;// append new character to input password string
        }
      }
  }
  return;
}
}
void Regis(){
  //Serial.println("No owner if you want this locker press A to Register");
  while(true){
    char key = keypad.getKey();
    if(key){
      Serial.println(key);
      //If User press A go to inputpass
      if(key == 'A'){
        //send mode 0 register
         mode_send.statuss = 0;
        esp_err_t result2 = esp_now_send(MacAddress1, (uint8_t *) &mode_send, sizeof(send_mode)); //mode 0 register
        if (result2 == ESP_OK) {
          Serial.println("Sent with success");
          Serial.println(send_oled.text);
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(500);
        send_led.statuss = 0;
        esp_err_t result1 = esp_now_send(MacAddress4, (uint8_t *) &send_led, sizeof(led_status)); 
        if (result1 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
       delay(500);
         
        break;  
      }
    }
  }
  return; 
}

void inputpass(){
  input_password = "";
  send_oled.text = "Enter Password";
  esp_err_t result = esp_now_send(MacAddress3, (uint8_t *) &send_oled, sizeof(keypad_oled)); 
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(500);
  while(true){
    char key = keypad.getKey();
    if (key) {
    Serial.println(key);
     //ch+=1;
    if (key == '*') {
      input_password = ""; // clear input password
    } 
    else if (key == '#') { // check password and # to confirm
      if (password == input_password) {
        Serial.println("The password is correct");
        send_led.statuss = 1;
        esp_err_t result1 = esp_now_send(MacAddress4, (uint8_t *) &send_led, sizeof(led_status)); 
        if (result1 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
       delay(500);
        break;

      } else {
        Serial.println("The password is incorrect, Please try again");
        send_led.statuss = 2;
        esp_err_t result1 = esp_now_send(MacAddress4, (uint8_t *) &send_led, sizeof(led_status)); 
        if (result1 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
       delay(500);
      }

      input_password = ""; // clear input password
      
    }
    
    else {
      send_keypadstring(); //send * to oled
      input_password += key;// append new character to input password string
    } 
   }
  }
}

int confirmreset(){
  send_oled.text = "Confirm reset press # to reset or * to cancel";
  esp_err_t result = esp_now_send(MacAddress3, (uint8_t *) &send_oled, sizeof(keypad_oled)); //mode 2 reset
  if (result == ESP_OK) {
      Serial.println("Sent with success");
      Serial.println(send_oled.text);
  }
  else {
      Serial.println("Error sending the data");
  }
  delay(500);
  while(true){
    char key = keypad.getKey();
    if(key){
      Serial.println(key);
      if(key == '#'){
        send_oled.text = "Reset";
        esp_err_t result = esp_now_send(MacAddress3, (uint8_t *) &send_oled, sizeof(keypad_oled)); //mode 2 reset
        if (result == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(500);
        return 1; //reset
      }
      else if(key == '*'){
        send_oled.text = "Cancel reset";
        esp_err_t result = esp_now_send(MacAddress3, (uint8_t *) &send_oled, sizeof(keypad_oled)); //mode 2 reset
        if (result == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
        return 2; //No reset 
      }
    }
  }
}
void inputnisit(){
  input_nisit = "";
  while(true){
    char key = keypad.getKey();
    if (key) {
    Serial.println(key);
    if (key == '*') {
      input_nisit = ""; // clear input password
    } 
    else if (key == '#') { // check password
      if (nisit_id == input_nisit) {
        Serial.println("The nisit is correct");
        send_led.statuss = 1;
        esp_err_t result2 = esp_now_send(MacAddress4, (uint8_t *) &send_led, sizeof(led_status)); 
        if (result2 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(500);
        inputpass();
        break;

      } else {
        Serial.println("Nisit ID is incorrect, Please try again");
        send_led.statuss = 2;
        esp_err_t result2 = esp_now_send(MacAddress4, (uint8_t *) &send_led, sizeof(led_status)); 
        if (result2 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
       delay(500);
      }

      input_nisit = ""; // clear input password
    }
    
    else {
      input_nisit += key;// append new character to input password string
    } 
   }
  }  
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);

  
  if(compareMac(mac_addr,MACAddress1)){
    memcpy(&receive_card, incomingData, sizeof(card_UID));
  }
  
  if(compareMac(mac_addr,MacAddress6)){
     memcpy(&receive_open, incomingData, sizeof(receive_open));
     if(receive_open.servo_status == 0){
      door_open = 0;
      Serial.println("Close");  
       Serial.println(door_open);  
     }
     else{
      door_open = 1;
      Serial.println("open");
      Serial.println(door_open); 
     }
  }
}


void setup() {
  Serial.begin(115200);
  input_password.reserve(32); // maximum input characters is 33, change if needed
   WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo1.peer_addr, MacAddress1, 6);
  peerInfo1.channel = 0;  
  peerInfo1.encrypt = false;
  if (esp_now_add_peer(&peerInfo1) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  memcpy(peerInfo2.peer_addr, MacAddress3, 6);
  peerInfo2.channel = 0;  
  peerInfo2.encrypt = false;
  if (esp_now_add_peer(&peerInfo2) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  memcpy(peerInfo3.peer_addr, MacAddress4, 6);
  peerInfo3.channel = 0;  
  peerInfo3.encrypt = false;
  if (esp_now_add_peer(&peerInfo3) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo4.peer_addr, MacAddress6, 6);
  peerInfo4.channel = 0;  
  peerInfo4.encrypt = false;
  if (esp_now_add_peer(&peerInfo4) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
}
void loop() {
  char key = keypad.getKey();
 if(door_open == 1){
  if (password == "" ){ // start 
    Regis(); //register
    //receive card UID 
    if(receive_card.UID){ //if scan
      Serial.println("Create Password");
      creatpass();
    }
  }
  else{
    if (key) {
    Serial.println(key);
    if (key == 'C'){ //use password to enter locker
      Serial.println("Forget Card ");
       send_oled.text = "Forget Card Please input your nisit number";
       esp_err_t result = esp_now_send(MacAddress3, (uint8_t *) &send_oled, sizeof(keypad_oled)); 
       if (result == ESP_OK) {
          Serial.println("Sent with success");
          Serial.println(send_oled.text);
          
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(500);
      mode_send.statuss = 2;
      esp_err_t result = esp_now_send(MacAddress1, (uint8_t *) &mode_send, sizeof(mode_send)); //mode 2 reset
       if (result == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(500);
      inputnisit();
      open_send.statuss = 1; //send open
      door_open = 0;
      esp_err_t result1 = esp_now_send(MacAddress1, (uint8_t *) &open_send, sizeof(send_open)); 
       if (result1 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
       delay(500);
    }
    else if(key == 'A' && password != ""){ //press register again
      //send mode 0
      mode_send.statuss = 0;
      esp_err_t result2 = esp_now_send(MacAddress1, (uint8_t *) &mode_send, sizeof(send_mode)); //mode 0 register
       if (result2 == ESP_OK) {
          Serial.println("Sent with success");
          Serial.println(send_oled.text);
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(500);
      //send status to led
      send_led.statuss = 3;
        esp_err_t result2 = esp_now_send(MacAddress4, (uint8_t *) &send_led, sizeof(led_status)); 
        if (result2 == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
       delay(500);
        
    }
    else if(key == 'B'){ // reset  owner
      mode_send.statuss = 1;
      int x = confirmreset();
      if(x == 1){
       esp_err_t result = esp_now_send(MacAddress1, (uint8_t *) &mode_send, sizeof(send_mode)); //mode 1 reset
       if (result == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
        
        delay(500);
        password = "";
      }
    }
  }
 }  
}
}