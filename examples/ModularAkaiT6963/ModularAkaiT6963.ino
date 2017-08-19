#include <VLCD.h>
#include <EmuT6963.h>


EmuT6963 pmpReceiver;
VLCD vtft;

void setup() {

    pmpReceiver.initializeDevice();

    Serial.begin(115200);
    vtft.initializeDevice(Serial);    
}

void loop() {    
    static boolean serialConnected = false;

    if (Serial) {      
        if (!serialConnected) {
            serialConnected = true;
            vtft.setSize(240, 64);
            vtft.setBackground(Color::DarkBlue);
            vtft.setForeground(Color::Snow);
        }
    } else {
        serialConnected = false;
    }

    pmpReceiver.draw(vtft, 0, 0);
}
