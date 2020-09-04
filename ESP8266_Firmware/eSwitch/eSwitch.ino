#include "src/FirebaseESPSmartControl.h"


//FirebaseESPDevice deviceVariable(devicePin, switchPin, unit); for auto/manual controlling devices with sensors.
//FirebaseESPDevice tempDevice(/*devicePin = */13, /*switchPin = */16, /*unit = */"&#186;C"); // "&#186;C" = *C in HTML4

//FirebaseESPDevice deviceVariable(devicePin, switchPin); for controlling devices remotely
FirebaseESPDevice light1(/*devicePin = */13, /*switchPin = */16);
FirebaseESPDevice light2(/*devicePin = */12, /*switchPin = */14);


void updateValues(){	// this function to read/update sensor values, WARNNING: NO DELAY USED!!
	//tempDevice.value = random(1000);
}

void setup()
{
	// Begin FirebaseESPSmartControl with HW Serial baudrate.
	FirebaseESPSmartControl.begin(115200);	
	
	// Initialize update function with time = 5s (5000ms) interval to update to firebase;
	FirebaseESPSmartControl.setFirebaseUpdate(3000, updateValues); //NO DELAY in update function, 5000 = 5 second.
}

void loop()
{
	// Loop for FirebaseESPSmartControl
	if(!FirebaseESPSmartControl.onProcess()) return;
}
