/*
 * manage 7 segment display
 * http://richiardone.eu
 */


/* used by serial PIN_UART_RX = 2; // D4 on D1 Mini  and  PIN_UART_TX = 0; // D3 UNUSED
*/
#define PIN_D0 16
#define PIN_D1 5
#define PIN_D2 4
#define PIN_D5 14
#define PIN_D6 12
#define PIN_D7 13
#define PIN_D8 15

#define DIGIT0 0  // D3 on D1 Mini
#define DIGIT1 10 // no pin on D1 Mini, direct on ESP8266


namespace display7s {

    /*   0      D0
        1 2   D1  D2
         3      D5 
        4 5   D6  D7
         6      D8
    */
    constexpr static const uint8_t displayio[] = {PIN_D0, PIN_D1, PIN_D2, PIN_D5, PIN_D6, PIN_D7, PIN_D8};
    constexpr static const uint8_t numtable[10][7] = {
    {HIGH,HIGH,HIGH,LOW, HIGH,HIGH,HIGH},
    {LOW, LOW, HIGH,LOW, LOW, HIGH,LOW},
    {HIGH,LOW, HIGH,HIGH,HIGH,LOW, HIGH},
    {HIGH,LOW, HIGH,HIGH,LOW, HIGH,HIGH},
    {LOW, HIGH,HIGH,HIGH,LOW, HIGH,LOW},
    {HIGH,HIGH,LOW, HIGH,LOW, HIGH,HIGH},
    {LOW, HIGH,LOW, HIGH,HIGH,HIGH,HIGH},
    {HIGH,LOW, HIGH,LOW, LOW, HIGH,LOW},
    {HIGH,HIGH,HIGH,HIGH,HIGH,HIGH,HIGH},
    {HIGH,HIGH,HIGH,HIGH,LOW, HIGH,LOW},
    };
    constexpr static const uint8_t nan[] = {LOW,LOW,LOW,HIGH,LOW,LOW,LOW};
    
    void setup() {
      int i;
    	
      pinMode(PIN_D0, OUTPUT);
      pinMode(PIN_D1, OUTPUT);
    	pinMode(PIN_D2, OUTPUT);
	pinMode(PIN_D5, OUTPUT);
	pinMode(PIN_D6, OUTPUT);
	pinMode(PIN_D7, OUTPUT);
	pinMode(PIN_D8, OUTPUT);
	
      pinMode(DIGIT0, OUTPUT);
      pinMode(DIGIT1, OUTPUT);
      	
      digitalWrite(DIGIT0, HIGH);
      digitalWrite(DIGIT1, HIGH);
    }

    /* pos = 0, 1 is digit position 
     * digit ca be -1 to mean nan
    */
    void write(uint8_t pos, short digit) {
        byte i;
        
        if(pos == 0){
            digitalWrite(DIGIT1, HIGH);
            digitalWrite(DIGIT0, LOW);
        } else {
            digitalWrite(DIGIT1, LOW);
            digitalWrite(DIGIT0, HIGH);
        }
        
      
      if(digit < 0){
    	    for(i=0; i<7; i++){
	        digitalWrite(displayio[i], nan[i]);	
            }    
        }
        
        if(digit >= 0 && digit <= 9){
    	    for(i=0; i<7; i++){
	        digitalWrite(displayio[i], numtable[digit][i]);	
            }
        }
    }


}
