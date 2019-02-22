//connect scl pin in mpu to D6 in NodeMCU
//connect sda pin in mpu to D7 in NodeMCU
//connect vcc to 3.6 in MCU
//connect GND to GND in NodeMCU
#include <ESP8266WiFi.h>
#include <Wire.h>
int limit=200;
const uint8_t MPU6050SlaveAddress = 0x68;  // MPU6050 Slave Device Address
const uint8_t scl = D6;      // Select SDA and SCL pins for I2C communication 
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses 
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ,Temperature, GyroX, GyroY, GyroZ; //variables to save the raw values of MPU
double Ax, Ay, Az, T, Gx, Gy, Gz;                                //variables to save the exact values of MPU

String apiWritekey = "T12WOIVFQWGUNY2K"; // replace with your THINGSPEAK WRITEAPI key here
const char* ssid = "TP2"; // your wifi SSID name
const char* password = "1223334444" ;// wifi password
const char* server = "api.thingspeak.com"; //the saerver "we use thinkspeak cloud"
WiFiClient client;                         //make a wifiClient object named "client"

int fall=0;             //variable to represent falling "if equal zero that is mean not falling and if equal one that is mean falling"
  
void setup() {  
  Serial.begin(115200);  //intialize the serial comunication in buadrate 115200
  WiFi.disconnect();    //disconnect any wifi "to start again"
  delay(10); 
  WiFi.begin(ssid, password);  //connect with the wifi in ssid and passward 
  Wire.begin(sda, scl);        //intialize the wire communication and define the sda and scl lines "pins"
  MPU6050_Init();             //that is the function we made to intialize the communication with mpu6050
  WiFi.begin(ssid, password); //another trial to connect "just to be sure"
  while (WiFi.status() != WL_CONNECTED) {   //wait until connect correctly
    delay(500);
  }
  Serial.print("connected to  "); Serial.println(ssid);
}
 
void loop() {
  while(!threshold());        //still calling the function named "threshold" to still tracke the algorithm to detect the falling
  if (client.connect(server,80))  //connrct with the client at server "we defined the server above" at number 80 "80 related to web devolopment of the server"
  {  
    //here we arrange the word that transmite to the client that must start with apiwritekay and the field in the chanell then the data we want to transmite then, must ended by "\r\n\r\n"
    String tsData = apiWritekey;
           tsData +="&field1="; 
           tsData += String(fall);
           tsData += "\r\n\r\n";
     //raech the address of the field in the channel with api in the server in host in post 
     client.print("POST /update HTTP/1.1\n");    
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiWritekey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(tsData.length());
     client.print("\n\n");  // the 2 carriage returns indicate closing of Header fields & starting of data
     client.print(tsData);
  }
  delay(200);
  client.stop();   //stop the client after trasmitting the data 
  Serial.println("wait new value of the fall tracking ....");
  delay(15000);    // thingspeak needs minimum 15 sec delay between updates
} 
//that function to send data to the wired communication "and we use it to send tha addresses of the available regesters to receive the data"
void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}
//raed all of the raw data from the device mpu "but the raw data need to modifications to represent the angle and accelration"
//all this function in library wire 
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
//here we intialize the communication with mpu at the defined regesters "in line 13 in the code"
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
//this function made to modifay the raw value from mpu to diffritiate the real values
void mpu() {
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;
  Gx = (double)GyroX/GyroScaleFactor;
  Gy = (double)GyroY/GyroScaleFactor;
  Gz = (double)GyroZ/GyroScaleFactor;
  Az=Az-1;           //to callibrate the defult position at 0 in all direction "بسبب وضع السينسور المختلف"
   Serial.print("Ax: "); Serial.print(Ax);
  Serial.print(" Ay: "); Serial.print(Ay);
  Serial.print(" Az: "); Serial.print(Az);
  Serial.print(" Gx: "); Serial.print(Gx);
  Serial.print(" Gy: "); Serial.print(Gy);
  Serial.print(" Gz: "); Serial.println(Gz);
  delay(100);
}
//the main part of the code "threshold algorithm to make certain that the man aleardy fall"
int threshold() {
  mpu();  //first we get the real value from mpu 
  if(Gx > limit || Gy > limit || Gx < (limit)*-1 || Gy < (limit)*-1 )  //second we test if the angualar accelration of any direction greater than "limit"
  {
    delay(100);      //wait 100 ms
    mpu();           //and read a new value
    if(Ax > 0.7 || Az > 0.7 || Ax < -0.7 || Az < -0.7 )  //and test if the angle is greater than 0.7 in any diraction "the scale of angle is from -2 to 2 "
    {
      delay(2000);        //wait 2 s and test again
      mpu();              //read new values to test again 
      if((Ax > 0.7 || Az > 0.7 || Ax < -0.7 || Az < -0.7) && fall==0) {fall=1; Serial.println("fall"); return 1; }   //test again and if true that mean falling and return 1 to send to server 
      else {return 0;}      //if false return 0 to still checking 
    }
    else {return 0;}   //if false return 0 to still checking 
  }
    mpu();   //read new values
  if(Ax < 0.5 && Az < 0.5 && Ax > -0.5 && Az > -0.5 && fall==1) {fall=0; Serial.println("get up"); return 1; } //if back to the natural position return 1 to send to server and make fall=0 to tell the server that man get up
  else {return 0;} 
  
}
