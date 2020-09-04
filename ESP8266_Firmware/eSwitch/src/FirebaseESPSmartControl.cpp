#include "FirebaseESPSmartControl.h"

String FirebaseESPSmartControlClass::WIFI_SSID       = "";
String FirebaseESPSmartControlClass::WIFI_PASSWORD   = "";
String FirebaseESPSmartControlClass::USERID   		 = "";
String FirebaseESPSmartControlClass::path            = "/FirebaseESP_Smart_Control";
  
DNSServer *FirebaseESPSmartControlClass::dnsServer; 
ESP8266WebServer *FirebaseESPSmartControlClass::server;
FirebaseData FirebaseESPSmartControlClass::firebaseData;
FirebaseData FirebaseESPSmartControlClass::firebaseRespond;

void (*FirebaseESPSmartControlClass::updateFunction)(void) = NULL;
int FirebaseESPSmartControlClass::modeFlag = -1;
unsigned long FirebaseESPSmartControlClass::ledStamp = millis();
unsigned long FirebaseESPSmartControlClass::updateStamp = millis();
unsigned long FirebaseESPSmartControlClass::tickStamp = millis();
unsigned long FirebaseESPSmartControlClass::updateTime = 3000;
unsigned long FirebaseESPSmartControlClass::tickTime = 50;
unsigned long FirebaseESPSmartControlClass::restartStamp = 0;

boolean FirebaseESPSmartControlClass::loadCmdFirstTime = true;
boolean FirebaseESPSmartControlClass::configMode = false;
boolean FirebaseESPSmartControlClass::userFound = false;
boolean FirebaseESPSmartControlClass::isStreaming = false;
DevicesList FirebaseESPSmartControlClass::devicesList;

FirebaseESPSmartControlClass FirebaseESPSmartControl;

FirebaseESPDevice::FirebaseESPDevice(int _devicePin, int _switchPin, String _unit){
	unit = _unit;
	devicePin = _devicePin;
	switchPin = _switchPin;
 
	pinMode(devicePin, OUTPUT);
	pinMode(switchPin, INPUT_PULLUP);
	digitalWrite(devicePin, HIGH);
	
	if(FirebaseESPSmartControl.devicesList.head == NULL){
		FirebaseESPSmartControl.devicesList.head = this;
		id = 1;
	}else{
		FirebaseESPSmartControl.devicesList.tail->next = this;
		id = FirebaseESPSmartControl.devicesList.tail->id + 1;
	}
	name = "Device " + String(id);
	FirebaseESPSmartControl.devicesList.tail = this;
	pressStamp = millis();
}

FirebaseESPDevice::FirebaseESPDevice(int _devicePin, int _switchPin){
	devicePin = _devicePin;
	switchPin = _switchPin;
	
	pinMode(devicePin, OUTPUT);
	pinMode(switchPin, INPUT_PULLUP);
	digitalWrite(devicePin, HIGH);
	
	if(FirebaseESPSmartControl.devicesList.head == NULL){
		FirebaseESPSmartControl.devicesList.head = this;
		id = 1;
	}else{
		FirebaseESPSmartControl.devicesList.tail->next = this;
		id = FirebaseESPSmartControl.devicesList.tail->id + 1;
	}
	name = "Device " + String(id);
	FirebaseESPSmartControl.devicesList.tail = this;
	pressStamp = millis();
}

void FirebaseESPDevice::load(String params){
	params.remove(0, params.indexOf(":") + 1);	// remove id
	params.remove(0, params.indexOf(":") + 1);	// remove value
	params.remove(0, params.indexOf(":") + 1);	// remove unit
	
	mode = params.substring(0, params.indexOf(":")).toInt() > 0;
	params.remove(0, params.indexOf(":") + 1);	// remove mode
	
	control = params.substring(0, params.indexOf(":")).toInt() > 0;
	params.remove(0, params.indexOf(":") + 1);	// remove control
	
	device = params.substring(0, params.indexOf(":")).toInt() > 0;
	params.remove(0, params.indexOf(":") + 1);	// remove device
	
	autoValue = params.substring(0, params.indexOf(":")).toInt();
	params.remove(0, params.indexOf(":") + 1);	// remove auto
	
	rangeValue = params.substring(0, params.indexOf(":")).toInt();
}

void FirebaseESPDevice::setName(String _name){
	name = _name;
	update();
}

void FirebaseESPDevice::setMode(bool _mode){
	mode = _mode;
	update();
}

void FirebaseESPDevice::setControl(bool _control){
	control = _control;
	update();
}

void FirebaseESPDevice::setDevice(bool _device, bool force){
	if(unit == "" or !control or force){
		device = _device;
		digitalWrite(devicePin, !device);
		EEPROM.write(200 + id, device);
		EEPROM.commit();
		update();
	}
}

void FirebaseESPDevice::setAuto(int _autoValue){
	autoValue = _autoValue;
	update();
}

void FirebaseESPDevice::setRange(int _rangeValue){
	rangeValue = _rangeValue;
	update();
}

void FirebaseESPDevice::update(){
	static uint8_t key = 0;
	if(FirebaseESPSmartControl.isStreaming){
		key++;
		String str = String(id) + ":" + String(value) + ":" + unit + ":" + String(mode) + ":" + String(control) + ":" + String(device) + ":" + String(autoValue) + ":" + String(rangeValue) + ":" + String(key);
		FirebaseJson json;
		json.add("value", str);
		Firebase.updateNodeSilent(FirebaseESPSmartControl.firebaseData, FirebaseESPSmartControl.path + "/motherDevices/" + WiFi.macAddress() + "/Devices/" + String(id), json);
	}
}

void FirebaseESPSmartControlClass::begin(long baud){
	ESP.wdtDisable();
	ESP.wdtEnable(WDTO_8S);
	configMode = modeDetect();
	Serial.begin(baud);
	EEPROM.begin(512);
	Serial.println();
	Serial.println();
	Serial.println(F("**********************************************************************"));
	Serial.println(F("*                     FirebaseESP Smart Control                      *"));
	Serial.println(F("**********************************************************************"));
	Serial.println();
	loadConfig();
	path = path + "/" + USERID;
	pinMode(LED_BUILTIN, OUTPUT);
	
	FirebaseESPDevice *p = devicesList.head;
	while(p){
		p->setDevice(EEPROM.read(200 + p->id) != 0, false);
		p = p->next;
		yield();
	}
	/*WiFi setup*/
	if(configMode){
		Serial.println();
		Serial.println(F("Config mode..."));
		WiFi.mode(WIFI_AP);
		IPAddress apIP(8, 8, 8, 8);
		WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
		WiFi.softAP("eSwitch: " + WiFi.macAddress());

		IPAddress myIP = WiFi.softAPIP();
		Serial.print(F("AP Ip address: "));
		Serial.println(myIP);

		dnsServer = new DNSServer();
		dnsServer->start(53, "*", myIP);
		
		server = new ESP8266WebServer(80);
		server->on("/", handleRoot);
		server->onNotFound([]() {
			server->send(200, "text/html", redirect_html);
		});
		server->begin();
	}else{
		WiFi.softAPdisconnect(true);
		WiFi.mode(WIFI_STA);
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		digitalWrite(LED_BUILTIN, LOW);
		Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
		Firebase.enableClassicRequest(firebaseData, true);
		Firebase.enableClassicRequest(firebaseRespond, true);
		Firebase.reconnectWiFi(true);

		firebaseData.setBSSLBufferSize(1024, 1024);
		firebaseData.setResponseSize(1024);
		firebaseRespond.setBSSLBufferSize(1024, 1024);
		firebaseRespond.setResponseSize(1024);
		Firebase.setReadTimeout(firebaseData, 1000 * 5);
		Firebase.setwriteSizeLimit(firebaseData, "tiny");
	}
}

bool FirebaseESPSmartControlClass::onProcess(){
	ESP.wdtFeed();
	unsigned long currentTime = millis();
	
	FirebaseESPDevice *p = devicesList.head;
	
	p = devicesList.head;
	bool flag = true;

	while(p){
		bool state = digitalRead(p->switchPin);
		
		if(state == LOW && !p->trigger){
			p->setDevice(!p->device, false);
			p->pressStamp = currentTime;
			p->trigger = true;
		}
		
		if(state == HIGH || currentTime - p->pressStamp < LONG_PRESS){
			flag = false;
		}
		
		if(state == HIGH){
			p->trigger = false;
		}
				
		p = p->next;
		yield();
	}
	
	if(flag) {
		if(configMode){
			uint32_t data = 0x03040304;
			ESP.rtcUserMemoryWrite(0x00, &data, sizeof(data));
		}else{
			uint32_t data = 0x24112411;
			ESP.rtcUserMemoryWrite(0x00, &data, sizeof(data));
		}
		ESP.restart();
	}

	if(configMode){
		dnsServer->processNextRequest();
		server->handleClient();
		
		if(currentTime - ledStamp >= 500){
			ledBlink();
			ledStamp = currentTime;
		}
		
		if(restartStamp > 0 && millis() - restartStamp >= 1000){
			uint32_t data = 0x03040304;
			ESP.rtcUserMemoryWrite(0x00, &data, sizeof(data));
			ESP.restart();
		}
		
		return false;
	}
		
	if(currentTime - tickStamp >= tickTime){
		p = devicesList.head;
		while(p){

			if(p->control && p->unit != ""){
				if(p->mode){		// HIGH
					if(float(p->value/10.0) > float(p->autoValue/10.0) && p->device){
						p->setDevice(false, true);
					}else if(float(p->value/10.0) < float((p->autoValue - p->rangeValue)/10.0) && !p->device){
						p->setDevice(true, true);
					}
				}else{			// LOW
					if(float(p->value/10.0) < float(p->autoValue/10.0) && p->device){
						p->setDevice(false, true);
					}else if(float(p->value/10.0) > float((p->autoValue + p->rangeValue)/10.0) && !p->device){
						p->setDevice(true, true);
					}
				}
			}
			p = p->next;
		}

		tickStamp = currentTime;
		yield();
	}
	
	if(WiFi.status() != WL_CONNECTED && isStreaming){
		ESP.restart();
	}
	
	if(WiFi.status() != WL_CONNECTED){
		if(currentTime - ledStamp >= 1000){
			digitalWrite(LED_BUILTIN, LOW);
			delay(10);
			digitalWrite(LED_BUILTIN, HIGH);
			ledStamp = currentTime;
		}
	}else if (!userFound){
		if(currentTime - ledStamp >= 2000){
			digitalWrite(LED_BUILTIN, LOW);
			delay(10);
			digitalWrite(LED_BUILTIN, HIGH);
			ledStamp = currentTime;
		}
	}else if (!isStreaming){
		if(currentTime - ledStamp >= 3000){
			digitalWrite(LED_BUILTIN, LOW);
			delay(10);
			digitalWrite(LED_BUILTIN, HIGH);
			ledStamp = currentTime;
		}
	}
	
	if(currentTime - updateStamp >= updateTime && WiFi.status() == WL_CONNECTED){
		
		if(!userFound){
			Serial.print(F("Checking User... "));
			digitalWrite(LED_BUILTIN, LOW);
			if(Firebase.get(firebaseData, path)){
				if(!firebaseData.dataAvailable()){
					Serial.println(F("Failed. No user found."));
				}else{
					Serial.println(F("OK."));
					userFound = true;
				}
			}
			digitalWrite(LED_BUILTIN, HIGH);
		}
		
		if(userFound && !isStreaming){
			digitalWrite(LED_BUILTIN, LOW);
			Serial.print(F("Start streaming from Firebase... "));
			if(Firebase.beginStream(firebaseRespond, path + "/motherDevices/" + WiFi.macAddress() + "/cmd")){
				Firebase.setStreamCallback(firebaseRespond, streamCallback);
				Serial.println(F("OK"));
				isStreaming = true;
			}else{
				Serial.println(F("FAILED"));
			}
			digitalWrite(LED_BUILTIN, HIGH);
		}
		
		update();
		updateStamp = currentTime;
	}

	return true;
}

bool FirebaseESPSmartControlClass::modeDetect(){
	uint32_t data;
	ESP.rtcUserMemoryRead(0x00, &data, sizeof(data));
	return data==0x24112411;   
}

void FirebaseESPSmartControlClass::saveConfig(String WiFiSSID, String WiFiPassword, String UserID){
	Serial.print(F("Saving config... "));
	int addr = 0;
	bool overLoad = false;
	
	if(!overLoad){
		for(uint8_t j=0; j<WiFiSSID.length(); j++){
			EEPROM.write(addr++, WiFiSSID[j]);
			if(addr == 200){
				overLoad = true;
				break;
			}
			yield();
		}
		EEPROM.write(addr++, 0);
	}

	if(!overLoad){
		for(uint8_t j=0; j<WiFiPassword.length(); j++){
			EEPROM.write(addr++, WiFiPassword[j]);
			if(addr == 200){
				overLoad = true;
				break;
			}
			yield();
		}
		EEPROM.write(addr++, 0);
	}
	
	if(!overLoad){
		for(uint8_t j=0; j<UserID.length(); j++){
			EEPROM.write(addr++, UserID[j]);
			if(addr == 200){
				overLoad = true;
				break;
			}
			yield();
		}
		EEPROM.write(addr++, 0);
	}
	
	if(EEPROM.commit() && !overLoad) {
		WIFI_SSID = WiFiSSID;
		WIFI_PASSWORD = WiFiPassword;
		USERID = UserID;
		Serial.println(F("OK"));
		Serial.println();
		Serial.println(F("New Config:")); 
		Serial.print(F("+WiFi SSID: "));
		Serial.println(WiFiSSID);
		Serial.print(F("+WiFi Password: "));
		Serial.println(WiFiPassword);
		Serial.print(F("+User ID: "));
		Serial.println(UserID);
	} else {
		Serial.println(F("ERROR! EEPROM commit failed"));
	}

}

void FirebaseESPSmartControlClass::loadConfig(){
	Serial.print(F("Loading Config... "));
	bool overLoad = false;
	int addr = 0;
    char ch = 0;
	
	String _WiFiSSID = "";
	String _WiFiPassword = "";
	String _UserID = "";
	
	if(!overLoad){
		do{
			ch = EEPROM.read(addr++);
			if(ch) _WiFiSSID += ch;
			if(addr == 200){
				overLoad = true;
				break;
			}
		}while(ch);
	}
	
	if(!overLoad){
		do{
			ch = EEPROM.read(addr++);
			if(ch) _WiFiPassword += ch;
			if(addr == 200){
				overLoad = true;
				break;
			}
		}while(ch);
	}
	
	if(!overLoad){
		do{
			ch = EEPROM.read(addr++);
			if(ch) _UserID += ch;
			if(addr == 200){
				overLoad = true;
				break;
			}
		}while(ch);
	}
	
	if(!overLoad){
		WIFI_SSID = _WiFiSSID;
		WIFI_PASSWORD = _WiFiPassword;
		USERID = _UserID;
		
		Serial.println(F("OK"));
		Serial.println();
		Serial.println(F("Current Config:")); 
		Serial.print(F("+WiFi SSID: "));
		Serial.println(WIFI_SSID);
		Serial.print(F("+WiFi Password: "));
		Serial.println(WIFI_PASSWORD);
		Serial.print(F("+User ID: "));
		Serial.println(USERID);
	}else{
		saveConfig(WIFI_SSID, WIFI_PASSWORD, USERID);
	}

}

void FirebaseESPSmartControlClass::handleRoot(){
	if(server->args() > 0){
		bool onSave = false;
		String WiFiSSID = WIFI_SSID, WiFiPassword = WIFI_PASSWORD, UserID = USERID;

		for(int i = 0; i < server->args(); i++) {
			if(server->argName(i) == "wifiSSID"){
				WiFiSSID = server->arg(i);
			}else if(server->argName(i) == "wifiPassword"){
				WiFiPassword = server->arg(i);
			}else if(server->argName(i) == "userID"){
				UserID = server->arg(i);
			}else if(server->argName(i) == "submit" && server->arg(i) == "Save"){
				onSave = true;
			}
			yield();
		}

		if(onSave){
			saveConfig(WiFiSSID, WiFiPassword, UserID);
			String headerPack = "/?wifiSSID=" + WiFiSSID + "&wifiPassword=" + WiFiPassword + "&userID=" + UserID + "&status=saved";
			server->sendHeader("Location", headerPack, true);
			server->send(302, "text/html", index_html);
			restartStamp = millis();
		}else{
			server->send(200, "text/html", index_html);
		}
	}else{
		String headerPack = "/?wifiSSID=" + WIFI_SSID + "&wifiPassword=" + WIFI_PASSWORD + "&userID=" + USERID;
		server->sendHeader("Location", headerPack, true);
		server->send(302, "text/html", index_html);
	}  
	
}

void FirebaseESPSmartControlClass::ledBlink(){
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
}

void FirebaseESPSmartControlClass::ledBlink(int count){
	for(int i = 0; i < count; i++){
		ledBlink();
		delay(150);
		ledBlink();
		delay(150);
	}
}

void FirebaseESPSmartControlClass::update(){
	if(updateFunction) updateFunction();
	
	FirebaseESPDevice *p = devicesList.head;
	while(p){
		p->update();
		p = p->next;
		yield();
	}
}

void FirebaseESPSmartControlClass::setFirebaseUpdate(int time, void (*_updateFunction)(void)){
	if(time >= 500){
		updateTime = time;
		updateFunction = _updateFunction;
	}
}

void FirebaseESPSmartControlClass::streamCallback(StreamData data){
	if(loadCmdFirstTime){
		loadCmdFirstTime = false;
		return;
	}
	
	String str = data.stringData();

	int index = str.indexOf(":");
	String cmd = str.substring(0, index);
	str.remove(0, index+1);
	index = str.indexOf(":");
	int id = str.substring(0, index).toInt();
	String param = "";
	if(cmd == "A" || cmd == "R"){
		str.remove(0, index+1);
		index = str.indexOf(":");
		param = str.substring(0, index);
	}else if(cmd == "C"){
		update();
		return;
	}
	
	int i=1;
	FirebaseESPDevice *p = devicesList.head;
	
	while(p){
		if(i == id){
			if(cmd == "D"){
				p->setDevice(!p->device, false);
			}else if(cmd == "M"){
				p->setMode(!p->mode);
			}else if(cmd == "C"){
				p->setControl(!p->control);
			}else if(cmd == "A"){
				p->setAuto(param.toInt());
			}else if(cmd == "R"){
				p->setRange(param.toInt());
			}
			return;
		}
		i++;
		p = p->next;
		yield();
	}
	
}