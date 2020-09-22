#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

int sensor_pin = 1;                

int led_pin = 13;                  

float temp;

int tempPin = 2;

volatile int heart_rate;          

volatile int analog_data;              

volatile int time_between_beats = 600;            

volatile boolean pulse_signal = false;    

volatile int beat[10];         //heartbeat values will be sotred in this array    

volatile int peak_value = 512;          

volatile int trough_value = 512;        

volatile int thresh = 525;              

volatile int amplitude = 100;                 

volatile boolean first_heartpulse = true;      

volatile boolean second_heartpulse = false;    

volatile unsigned long samplecounter = 0;   //This counter will tell us the pulse timing

volatile unsigned long lastBeatTime = 0;

volatile int code[3];

volatile int length=0;

unsigned long previousMillies=0;



int fsrPin = 0;     // the FSR and 10K pulldown are connected to a0
int fsrReading;
volatile int sum;

void setup()

{

  pinMode(led_pin,OUTPUT);        

  Serial.begin(9600);           

  interruptSetup();   
  if (!driver.init())
         Serial.println("RF init failed");               

}



void loop()

{ 
      if(millis()-previousMillies>=350){
        previousMillies=millis();
        temp = analogRead(tempPin);
        temp = temp * 0.48828125;
  
        fsrReading = analogRead(fsrPin);
        
        if(fsrReading >0 && fsrReading <200){
          code[length++]=0;
        }
        else if(fsrReading >200){
          code[length++]=1;
        }
     
        if(length==3){
          length=0;
          int num=4;
          sum=0;
          for(int x=0;x<3;x++){
            
            sum=sum+(num*code[x]);
            num= num/2;
          }
          
        }
      }
      String space=" ";
      String msg1=String(temp)+"*c "+space+heart_rate+space+fsrReading+space+12.936+space+77.533+space+sum;
      int length1= msg1.length()+1;
      char msg[length1];
      msg1.toCharArray(msg,length1);
      Serial.println(String(temp)+"*c "+space+heart_rate+space+fsrReading+space+12.936+space+77.533+space+sum);
      driver.send((uint8_t *)msg, strlen(msg));
      driver.waitPacketSent();
      
}



void interruptSetup()

{    

  TCCR2A = 0x02;  // This will disable the PWM on pin 3 and 11

  OCR2A = 0X7C;   // This will set the top of count to 124 for the 500Hz sample rate

  TCCR2B = 0x06;  // DON'T FORCE COMPARE, 256 PRESCALER

  TIMSK2 = 0x02;  // This will enable interrupt on match between OCR2A and Timer

  sei();          // This will make sure that the global interrupts are enable

}


ISR(TIMER2_COMPA_vect)

{ 

  cli();                                     

  analog_data = analogRead(sensor_pin);            

  samplecounter += 2;                        

  int N = samplecounter - lastBeatTime;      


  if(analog_data < thresh && N > (time_between_beats/5)*3)

    {     

      if (analog_data < trough_value)

      {                       

        trough_value = analog_data;

      }

    }


  if(analog_data > thresh && analog_data > peak_value)

    {        

      peak_value = analog_data;

    }                          



   if (N > 250)

  {                            

    if ( (analog_data > thresh) && (pulse_signal == false) && (N > (time_between_beats/5)*3) )

      {       

        pulse_signal = true;          

        digitalWrite(led_pin,HIGH);

        time_between_beats = samplecounter - lastBeatTime;

        lastBeatTime = samplecounter;     



       if(second_heartpulse)

        {                        

          second_heartpulse = false;   

          for(int i=0; i<=9; i++)    

          {            

            beat[i] = time_between_beats; //Filling the array with the heart beat values                    

          }

        }


        if(first_heartpulse)

        {                        

          first_heartpulse = false;

          second_heartpulse = true;

          sei();            

          return;           

        }  


      word runningTotal = 0;  


      for(int i=0; i<=8; i++)

        {               

          beat[i] = beat[i+1];

          runningTotal += beat[i];

        }


      beat[9] = time_between_beats;             

      runningTotal += beat[9];   

      runningTotal /= 10;        

      heart_rate = 60000/runningTotal;

    }                      

  }




  if (analog_data < thresh && pulse_signal == true)

    {  

      digitalWrite(led_pin,LOW); 

      pulse_signal = false;             

      amplitude = peak_value - trough_value;

      thresh = amplitude/2 + trough_value; 

      peak_value = thresh;           

      trough_value = thresh;

    }


  if (N > 2500)

    {                          

      thresh = 512;                     

      peak_value = 512;                 

      trough_value = 512;               

      lastBeatTime = samplecounter;     

      first_heartpulse = true;                 

      second_heartpulse = false;               

    }


  sei();                                

}
