import processing.serial.*; //import the Serial library
import controlP5.*;

ControlP5 cp5;


int end = 10;    // the number 10 is ASCII for linefeed (end of serial.println), later we will look for this to break up individual messages
String serial;   // declare a new string called 'serial' . A string is a sequence of characters (data type know as "char")
Serial port;  // The serial port, this is a new instance of the Serial class (an Object)
PFont f;
Boolean getSettings;  Boolean gotSettings;
String KA_P, KA_D, KP_I, KP_P;
final String DEGREE  = "\u00b0";

String settings = "NO DATA";
Textlabel settingsText;

int i = 0;
void setup() {
  size(800, 500);
  background(255);
  f = createFont("Courier",16,true);
  textFont(f);
  //f = loadFont("andalemo.ttf", 32);
  port = new Serial(this, "/dev/tty.HC-06-DevB", 9600); // initializing the object by assigning a port and baud rate (must match that of Arduino)
  port.clear();  // function from serial library that throws out the first reading, in case we started reading in the middle of a string from Arduino
  serial = port.readStringUntil(end); // function that reads the string from serial port until a println and then assigns string to our string variable (called 'serial')
  serial = null; // initially, the string will be null (empty)
  cp5 = new ControlP5(this);
  

  
  cp5.addButton("GetSettings")
     .setValue(0)
     .setPosition(10,350)
     .setSize(100,25)
     ;
   cp5.addButton("SaveSettings")
     .setValue(0)
     .setPosition(10,380)
     .setSize(100,25)
     ;    
   
  cp5.addButton("ToggleTelemetry")
   .setValue(0)
   .setPosition(10,320)
   .setSize(100,25)
   ;   
   
  cp5.addButton("KapU")
   .setValue(0)
   .setPosition(150,390)
   .setSize(30,30)
   ;   
   
  cp5.addButton("KapD")
   .setValue(0)
   .setPosition(150,425)
   .setSize(30,30)
   ;   
  cp5.addButton("KadU")
   .setValue(0)
   .setPosition(250,390)
   .setSize(30,30)
   ;   
   
  cp5.addButton("KadD")
   .setValue(0)
   .setPosition(250,425)
   .setSize(30,30)
   ;   
   
  cp5.addButton("KpiU")
   .setValue(0)
   .setPosition(380,390)
   .setSize(30,30)
   ;   
   
  cp5.addButton("KpiD")
   .setValue(0)
   .setPosition(380,425)
   .setSize(30,30)
   ;   
   
  cp5.addButton("KppU")
   .setValue(0)
   .setPosition(500,390)
   .setSize(30,30)
   ;   
   
  cp5.addButton("KppD")
   .setValue(0)
   .setPosition(500,425)
   .setSize(30,30)
   ;   
   
  getSettings = false;
  
}

void draw() {
  
  
  
  
  while (port.available() > 0) { //as long as there is data coming from serial port, read it and store it 
    serial = port.readStringUntil(end);
  }
  if (serial != null) {  //if the string is not empty, print the following
    String[] data = split(serial, ',');  //a new array (called 'a') that stores values into separate cells (separated by commas specified in your Arduino program)
    
    if (data[0].equals("t")) {
      println(serial);
      updateTelemetry(data);
    } else {
      println(serial);
    }
    if (data[0].equals("s")) {
       updateSettings(data); 
       
    }
    serial = null;
  }
  if (getSettings) {
    getSettings = false;
    port.write("s");    
  }
  
}

public void GetSettings(int theValue) {
  getSettings = true;
}

public void SaveSettings(int theValue) {
  port.write("w");
}


public void KapU(int theValue) {
  float KA_Pfloat = float(KA_P); //<>//
  KA_Pfloat = KA_Pfloat + .01;
  KA_P = str(KA_Pfloat);
  sendSettings();
}
public void KapD(int theValue) {
  float KA_Pfloat = float(KA_P);
  KA_Pfloat = KA_Pfloat - .01;
  KA_P = str(KA_Pfloat);
  sendSettings();
}
public void KadU(int theValue) {
  float KA_Dfloat = float(KA_D);
  KA_Dfloat = KA_Dfloat + .01;
  KA_D = str(KA_Dfloat);
  sendSettings();
}
public void KadD(int theValue) {
  float KA_Dfloat = float(KA_D);
  KA_Dfloat = KA_Dfloat - .01;
  KA_D = str(KA_Dfloat);
  sendSettings();
}
public void KpiU(int theValue) {
  float KP_Ifloat = float(KP_I);
  KP_Ifloat = KP_Ifloat + .01;
  KP_I = str(KP_Ifloat);
  sendSettings();
}
public void KpiD(int theValue) {
  float KP_Ifloat = float(KP_I);
  KP_Ifloat = KP_Ifloat - .01;
  KP_I = str(KP_Ifloat);
  sendSettings();
}

public void KppU(int theValue) {
  float KP_Pfloat = float(KP_P);
  KP_Pfloat = KP_Pfloat + .01;
  KP_P = str(KP_Pfloat);
  sendSettings();
}
public void KppD(int theValue) {
  float KP_Pfloat = float(KP_P);
  KP_Pfloat = KP_Pfloat - .01;
  KP_P = str(KP_Pfloat);
  sendSettings();
}

void sendSettings() {
  String out = "u"+KA_P+","+KA_D+","+KP_I+","+KP_P+"\n";
  println(out);
  port.write(out);
}
public void ToggleTelemetry(int theValue) {
  port.write("t");
  port.clear();
  background(255);
}

void updateSettings(String[] data) {
  KA_P = data[1];
  KA_D = data[2];
  KP_I = data[3];
  KP_P = data[4];
  settings = "KA_P:"+KA_P+"  KA_D:"+KA_D+"  KP_I"+KP_I+"  KP_P"+KP_P;
  background(255);
  text(settings, 120,370);
  
}  
void updateTelemetry(String[] data) {
        background(255);
        float deg =float(data[1]);  // Converts and prints float
        deg = deg - 90;
        float rad = (deg * PI) /180;
        float x = 200 + 200*cos(rad);
        float y = 200 + 200*sin(rad);
        //println(deg);
        
        stroke(0);
        strokeWeight(1);
        line(200,0,200,200);
        strokeWeight(4);
        line(200,200,x,y);
        fill(0);
        String degOut = String.format("%.2f",(deg+90));
        text("Kalman Roll: "+degOut+DEGREE,100,220);
        
        float speedL = float(data[2]);
        float speedR = float(data[3]);
        float Position_AVG = float(data[4]);
        float Position_add = float(data[5]);
        float speedNeed = float(data[6]);
        float turbo = float(data[7]);
        
        
        strokeWeight(1);
        line(650,0,650,90);
        strokeWeight(20);
        text("Speed Left    : "+speedL, 350,30);
        line(650,25,650+(speedL*10),25);
        text("Speed Right   : "+speedR, 350,70);
        line(650,65,650+(speedR*10),65);
        
        text("Speed Need    : "+speedNeed, 350,120);
        text("Position_AVG  : "+Position_AVG, 350,140);
        text("Position_Add  : "+Position_add, 350,160);
        text("turbo         : "+turbo, 350,180);
        
        
        text(settings, 120,370);
        
}