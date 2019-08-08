/*
PHAS1=PC0
PHAS2=PC1
PHAS3=PC2
PHAS4=PC3
PHAS5=PD3
PHAS6=PD4
PHAS7=PB6
PHAS8=PB7
*/

#include <avr/power.h>
#include <avr/sleep.h>
long unsigned int timer=0;
long unsigned int lastTimer=0;
unsigned int timerDelay=10;   //milliseconds per run
long unsigned int randomPhaserTimer=random(20000);
long unsigned int randomPhaserTimerBuffer=0;
//phaserArray=01234567
//byte phaserArray=B11110000;  //phaserArray=87654321
byte otherLEDs=B0000000;    //0,0,0,PNacelle,PCollector/NavLight/SCollector,SNacelle
// the setup function runs once when you press reset or power the board
void setup() {

    ADCSRA = 0; //disable ADC to save power
    POST();
}
void setOtherLEDs(){
    //0,0,0,PNacelle,PCollector/NavLight/SCollector,SNacelle
    //set PNacelle,PCollector
    int DDRBuf=DDRD&B10011111;  //get current LED pin state and blank out relivent LEDs
    int LEDMask=otherLEDs & B00011000;   //only look at LEDs for current port
    LEDMask=LEDMask<<2;               //shift right to align array with LED pins
    
    DDRBuf=DDRBuf|LEDMask;             //change new values while ignoring other pins
    DDRD=DDRBuf;
    
    //set nav light
    DDRBuf=DDRD&B11111011;  //get current LED pin state and blank out relivent LEDs
    LEDMask=otherLEDs & B00000100;   //only look at LEDs for current port
    DDRBuf=DDRBuf|LEDMask;             //change new values while ignoring other pins
    DDRD=DDRBuf;
    
    //set SCollector,SNacelle
    DDRBuf=DDRB&B11111001;  //get current LED pin state and blank out relivent LEDs
    LEDMask=otherLEDs & B00000011;   //only look at LEDs for current port
    LEDMask=LEDMask<<1;               //shift right to align array with LED pins
    DDRBuf=DDRBuf|LEDMask;             //change new values while ignoring other pins
    DDRB=DDRBuf;
}
void setPhasers(byte phaserArray=B00000000){
    //phaserArray=87654321
    //get PHAS1-PHAS4 into position
    int DDRBuf=DDRC&B11110000;    //get current LED pin state and blank out relivent LEDs
    int phasMask=phaserArray & B00001111;   //only look at LEDs for current port
    DDRBuf=DDRBuf|phasMask; //change new values while ignoring other pins
    
    DDRC=DDRBuf;
    PORTC=B00000000;
    
    //get PHAS5-PHAS6 into position
    DDRBuf=DDRD&B11100111;  //get current LED pin state and blank out relivent LEDs
    phasMask=phaserArray & B00110000;   //only look at LEDs for current port
    phasMask=phasMask>>1;               //shift right to align array with LED pins
    DDRBuf=DDRBuf|phasMask;             //change new values while ignoring other pins
    
    DDRD=DDRBuf;
    PORTD=B00000000;
    
    //get PHAS7-PHAS8 into position
    DDRBuf=DDRB&B00111111;
    phasMask=phaserArray & B11000000;
    DDRBuf=DDRBuf|phasMask;
    DDRB=DDRBuf;
    PORTB=B00000000;
    

}
void POST(){
    byte phaserTest=B0000001;
    otherLEDs=B00000001;
    for(int i=0; i<9; i++){
        setPhasers(phaserTest);
        delay(200);
        phaserTest=phaserTest<<1;
    }
    for(int i=0; i<6; i++){
        setOtherLEDs();
        delay(200);
        otherLEDs=otherLEDs<<1;
    }
}
void NavLight(bool navLightOn=false){
    //otherLEDs=otherLEDs^B00000100;
    if(navLightOn){
        otherLEDs=otherLEDs|B00000100;
    }
    else{
        otherLEDs=otherLEDs&B00011011;
    }
    setOtherLEDs();
}
void damageNacelles(int damage=0){
    if(damage==0){
        otherLEDs=otherLEDs&B00000100;
    }
    else if(damage==1){
        otherLEDs=otherLEDs&B00000111;
    }
    else if (damage==2){
        otherLEDs=otherLEDs&B00011100;
    }
    else{
        otherLEDs=otherLEDs|B00011011;
    }
    setOtherLEDs();
}
void loop() {
    if(millis()-lastTimer>timerDelay){
        lastTimer=millis();
        
    }

    if(millis()-randomPhaserTimerBuffer>randomPhaserTimer){
        randomPhaserTimerBuffer=millis();
        randomPhaserTimer=random(5000);
        damageNacelles(random(0,10));
        firePhasers();
        
    }
    //otherLEDs=B00011000;
    otherLEDs=B00011011;
    NavLight(1);
/*power_adc_disable(); // ADC converter
power_spi_disable(); // SPI
power_usart0_disable();// Serial (USART)
//power_timer0_disable();// Timer 0
//power_timer1_disable();// Timer 1
//power_timer2_disable();// Timer 2
power_twi_disable(); // TWI (I2C)
delay(1000);


set_sleep_mode(SLEEP_MODE_ADC);
sleep_enable();
sleep_mode();*/
  
}

void firePhasers(){
    static unsigned int phaserTimer;
    byte highValue=B10000000;
    byte lowValue=B0000001;
    int delayTime=500;
    for(int i=0; i<4; i++){
        //phaserArray=highValue+lowValue;
        setPhasers(highValue+lowValue);
        highValue=highValue>>1;
        lowValue=lowValue<<1;
        delay(delayTime);
        delayTime=delayTime*.7;
    }
    if(random(2)==0){
        setPhasers(B00001000);
    }
    else{
        setPhasers(B00010000);
    }
    //setPhasers();
    delay(600);
    setPhasers();
    //setPhasers();
}