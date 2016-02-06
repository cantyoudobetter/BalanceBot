import processing.serial.*; //import the Serial library

int end = 10;    // the number 10 is ASCII for linefeed (end of serial.println), later we will look for this to break up individual messages
String serial;   // declare a new string called 'serial' . A string is a sequence of characters (data type know as "char")
Serial port;  // The serial port, this is a new instance of the Serial class (an Object)
PFont f;
final String DEGREE  = "\u00b0";
void setup() {
  size(800, 400);
  background(255);
  f = createFont("Arial",16,true);
  port = new Serial(this, "/dev/tty.HC-06-DevB", 9600); // initializing the object by assigning a port and baud rate (must match that of Arduino)
  port.clear();  // function from serial library that throws out the first reading, in case we started reading in the middle of a string from Arduino
  serial = port.readStringUntil(end); // function that reads the string from serial port until a println and then assigns string to our string variable (called 'serial')
  serial = null; // initially, the string will be null (empty)
}

void draw() {
  while (port.available() > 0) { //as long as there is data coming from serial port, read it and store it 
    serial = port.readStringUntil(end);
  }
    if (serial != null) {  //if the string is not empty, print the following
      println(serial);
      String[] data = split(serial, ',');  //a new array (called 'a') that stores values into separate cells (separated by commas specified in your Arduino program)
      float deg =float(data[0]);  // Converts and prints float
      deg = deg - 90;
      float rad = (deg * PI) /180;
      float x = 200 + 200*cos(rad);
      float y = 200 + 200*sin(rad);
      println(deg);
      background(255);
      stroke(0);
      strokeWeight(1);
      line(200,0,200,200);
      strokeWeight(4);
      line(200,200,x,y);
      fill(0);
      String degOut = String.format("%.2f",(deg+90));
      text("Kalman Roll: "+degOut+DEGREE,150,220);
      
      float speedL = float(data[1]);
      float speedR = float(data[2]);
      strokeWeight(1);
      line(650,0,650,90);
      strokeWeight(20);
      text("Speed Left:  "+speedL, 450,30);
      line(650,25,650+(speedL*10),25);
      text("Speed Right: "+speedR, 450,70);
      line(650,65,650+(speedR*10),65);
      
    }
}