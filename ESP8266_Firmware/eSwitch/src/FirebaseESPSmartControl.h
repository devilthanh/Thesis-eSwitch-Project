#ifndef FIREBASEESP_SMARTCONTROL
#define FIREBASEESP_SMARTCONTROL

#include <EEPROM.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "FirebaseESPSmartControlHTML.h"

#define LED_BUILTIN 2
#define LONG_PRESS 5000

#define FIREBASE_HOST "https://esp8266-smartcontrol.firebaseio.com/"
#define FIREBASE_AUTH "8ajac460CNzzUzGVdgKDeRw39rio0c7sGBzLbhnv"
#define URL_ROOT "https://raw.githubusercontent.com/devilthanh/SmartControlFirmwares/master"

class FirebaseESPDevice {
	public:
		FirebaseESPDevice *next = NULL;
		String name = "Device";
		int id = 0;
		int value = 0;
		int autoValue = 0;
		int rangeValue = 0;
		String unit = "";
		int devicePin;
		int switchPin;
		bool mode = false;
		bool control = false;
		bool device = true;
		bool trigger = false;
		unsigned long pressStamp = 0;
			
		FirebaseESPDevice(int _devicePin, int _switchPin, String _unit);
		FirebaseESPDevice(int _devicePin, int _switchPin);
		void load(String params);
		void setName(String _name);
		void setMode(bool _mode);
		void setControl(bool _control);
		void setDevice(bool _device, bool force);
		void setAuto(int _autoValue);
		void setRange(int _rangeValue);
		void update();
};

struct DevicesList{
	public:
		DevicesList(){};
		FirebaseESPDevice *head = NULL;
		FirebaseESPDevice *tail = NULL;
};

class FirebaseESPSmartControlClass {
	public:
		FirebaseESPSmartControlClass(){};
		static String WIFI_SSID;
		static String WIFI_PASSWORD;
		static String USERID;
		static String path;
		
		static DNSServer* dnsServer;
		static ESP8266WebServer* server; 
		static FirebaseData firebaseData;
		static FirebaseData firebaseRespond;

    static int modeFlag;
		static unsigned long ledStamp;
		static unsigned long updateStamp;
		static unsigned long tickStamp;
		static unsigned long updateTime;
		static unsigned long tickTime;
		static unsigned long restartStamp;

		static bool loadCmdFirstTime;
		static bool configMode;
		static bool userFound;
		static bool isStreaming;
		static DevicesList devicesList;
		static void (*updateFunction)(void);
				
		static void streamCallback(StreamData data);
		static bool onProcess();
		static bool modeDetect();
		static void saveConfig(String wifiSSID, String wifiPassword, String UserID);
		static void loadConfig();
		static void handleRoot();
		static void ledBlink();
		static void ledBlink(int count);	
		static void update();
		static void setFirebaseUpdate(int time, void (*_updateFunction)(void));
		static void begin(long baud);
};

extern FirebaseESPSmartControlClass FirebaseESPSmartControl;

#endif
