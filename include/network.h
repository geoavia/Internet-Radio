#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "main.hpp"

WebServer server(80);

const char* mqtt_server = "broker.emqx.io";
const uint16_t mqtt_port = 1883;

#define MQTT_KEEPALIVE_SEC 100
#define MQTT_ID "ggm-web-radio-2"

char topic_command[32];
char topic_status[32];
char topic_list[32];

WiFiClient espClient;
//BearSSL::WiFiClientSecure espClient;
PubSubClient pubsub(mqtt_server, mqtt_port, espClient);

#define RECONNECT_INTERVAL_MS 5000
unsigned long lastReconnectTime = 0;
bool isConnected = false;

void shutdown()
{
	digitalWrite(PWR_PIN, LOW);
	delay(200);
	esp_deep_sleep_start();
}

// load all saved wifi credentials
void load_networks()
{
	String skey;
	String pkey;
	n_networks = 0;
	bool prefsOk = preferences.begin("webradio", true);
	while (prefsOk && (n_networks < MAX_NETWORKS))
	{
		skey = String("s") + (n_networks + 1);
		networks[n_networks].ssid = preferences.getString(skey.c_str(), "");
		pkey = String("p") + (n_networks + 1);
		networks[n_networks].password = preferences.getString(pkey.c_str(), "");

		if (networks[n_networks].ssid.length() == 0 ||
			networks[n_networks].password.length() == 0)
			break;

		Serial.printf("%s => %s\n", skey.c_str(), networks[n_networks].ssid.c_str());
		Serial.printf("%s => %s\n", pkey.c_str(), networks[n_networks].password.c_str());
		n_networks++;
	}
	preferences.end();
}

// save wifi credentials
void save_current_network()
{
	String skey;
	String pkey;
	preferences.begin("webradio", false);
	// check for password change
	for (int i = 0; i < n_networks; i++)
	{
		if (networks[i].ssid == curnet.ssid)
		{
			networks[i].password = curnet.password;
			skey = String("s") + (i + 1);
			pkey = String("p") + (i + 1);
			preferences.putString(skey.c_str(), curnet.ssid);
			preferences.putString(pkey.c_str(), curnet.password);
			Serial.printf("%s <= %s\n", skey.c_str(), curnet.ssid.c_str());
			Serial.printf("%s <= %s\n", pkey.c_str(), curnet.password.c_str());
			preferences.end();
			return;
		}
	}
	if (n_networks < MAX_NETWORKS)
	{
		networks[n_networks].ssid = curnet.ssid;
		networks[n_networks].password = curnet.password;
		n_networks++;
	}
	else
	{
		// replace last one
		networks[n_networks-1].ssid = curnet.ssid;
		networks[n_networks-1].password = curnet.password;
	}   
	skey = String("s") + n_networks;
	pkey = String("p") + n_networks;
	preferences.putString(skey.c_str(), curnet.ssid);
	preferences.putString(pkey.c_str(), curnet.password);
	Serial.printf("%s <= %s\n", skey.c_str(), curnet.ssid.c_str());
	Serial.printf("%s <= %s\n", pkey.c_str(), curnet.password.c_str());
	preferences.end();
}

void remove_network(int index)
{
	String skey;
	String pkey;
	if (index < n_networks)
	{
		preferences.begin("webradio", false);
		skey = String("s") + n_networks;
		pkey = String("p") + n_networks;
		preferences.remove(skey.c_str());
		preferences.remove(pkey.c_str());
		n_networks--;
		// shift subsequent networks if any
		for (int i = index; i < n_networks; i++)
		{
			skey = String("s") + (i + 1);
			pkey = String("p") + (i + 1);
			networks[i].ssid = networks[i + 1].ssid;
			networks[i].password = networks[i + 1].password;
			preferences.putString(skey.c_str(), networks[i].ssid);
			preferences.putString(pkey.c_str(), networks[i].password);
		}
		preferences.end();
	}
}

bool connect_ssid(String ssid, String password)
{
	DisplayHeader();
	tft.println("Connecting to");
	tft.println(ssid);
	
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.setHostname(RADIO_HOSTNAME);
	WiFi.begin(ssid.c_str(), password.c_str());
	int tries = 20;
	while (!WiFi.isConnected())
	{
		delay(500);
		tft.print(".");
		
		Serial.print(".");
		tries--;
		if (tries == 0)
		{
			Serial.println("");
			tft.println("\nConnection Error!");
			
			WiFi.disconnect();
			delay(1000);
			return false;
		}
	}
	Serial.println("");
	tft.println("\nConnected");
	
	//delay(1000);
	return true;

}

void list_networks()
{
	Serial.print("Scanning networks...");
	n_SSID = WiFi.scanNetworks();
	Serial.println("done");
	if (n_SSID == 0)
	{
		tft.setTextSize(1);
		tft.setTextColor(TFT_WHITE);
		tft.println("WiFi not detected!");
		Serial.println("No networks found");
	}
	else
	{
		Serial.print(n_SSID);
		Serial.println(" networks found");
	}
}

bool connect_saved_networks()
{
	for (int i = 0; i < n_SSID; ++i)
	{
		// Print SSID and RSSI for each network found
		Serial.print(i + 1);
		Serial.print(": ");
		Serial.print(WiFi.SSID(i));
		Serial.print(" (");
		Serial.print(WiFi.RSSI(i));
		Serial.print("=");
		Serial.print(Get4BarsFromRSSI(WiFi.RSSI(i)));
		Serial.print(")");
		Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
		delay(10);

		for (int j = 0; j < n_networks; j++)
		{
			if (WiFi.SSID(i) == networks[j].ssid)
			{
				if (connect_ssid(networks[j].ssid, networks[j].password)) return true;
				else remove_network(j);
			}
		}
	}
	return false;
}

bool want_display_ui()
{
	bool dui = false; // DUI - Display UI :)
	Serial.println("WiFi connection UI...");
	while (true) 
	{
		tft.fillScreen(TFT_BLACK);
		tft.setCursor(0, 0);
		tft.setTextColor(TFT_YELLOW);
		tft.println("WiFi Setup method");

		tft.setCursor(0, 20);
		tft.setTextWrap(false);

		// method: web - default & easy
		if (!dui) tft.setTextColor(TFT_BLACK, TFT_WHITE);
		else tft.setTextColor(TFT_WHITE, TFT_BLACK);
		tft.println("1. Web based Setup");
		// method: display - more finicky
		if (dui) tft.setTextColor(TFT_BLACK, TFT_WHITE);
		else tft.setTextColor(TFT_WHITE, TFT_BLACK);
		tft.println("2. Display UI Setup");
		
		while (!GetRemoteCode() || IsRepeat) delay(10); 
		if (IsCode(KEY_CH)) break;
		else if (IsCode(KEY_PLAYPAUSE)) shutdown();
		else dui = !dui;
	}
	return dui;
}

#define MIN_CHAR 32
#define MAX_CHAR 126

int shift_chr(char ach[], int i, char ch)
{
	int ret = ach[i] ? i : i+1;
	ach[i] = ch;
	return ret;
}

bool get_network_ui()
{
	int yc = 0;
	int y0 = 0;
	curnet.ssid = "";
	curnet.password = "";

	// List Networks UI
	while(curnet.ssid == "")
	{
		tft.fillScreen(TFT_BLACK);
		tft.setCursor(0, 0);
		tft.setTextColor(TFT_YELLOW);
		tft.println("Select WiFi Network");

		tft.setCursor(0, 20);
		tft.setTextWrap(false);
		for (int y = 0; (y0 + y) < n_SSID && y < 6; y++)
		{
			if (yc == y) tft.setTextColor(TFT_BLACK, TFT_WHITE);
			else tft.setTextColor(TFT_WHITE, TFT_BLACK);
			tft.print("  ");
			//tft.print((WiFi.encryptionType(y0 + y) == WIFI_AUTH_OPEN) ? "  " : "* ");
			tft.println(WiFi.SSID(y0 + y));
			DisplayRSSI(5, 20+(y+1)*16, WiFi.RSSI(y0 + y), ((yc == y) ? TFT_BLACK : TFT_WHITE));
		}
		

		while (!GetRemoteCode() || IsRepeat) delay(10); 
		if (IsCode(KEY_PLUS) || IsCode(KEY_NEXT) || IsCode (KEY_CH_PLUS))
		{
			if (yc > 0) yc--;
			else if (yc == 0 && y0 > 0) y0--;
			//else yc == y0 = 0;
		}
		if (IsCode(KEY_MINUS) || IsCode(KEY_PREV) || IsCode (KEY_CH_MINUS))
		{
			if (yc < 5 && (yc + 1) < n_SSID) yc++;
			else if (yc == 5 && y0 + 6 < n_SSID) y0++;
			else yc = y0 = 0;
		}
		if (IsCode(KEY_CH))
		{
			curnet.ssid = WiFi.SSID(y0 + yc);
		}
		if (IsCode(KEY_PLAYPAUSE)) shutdown();
	}

	// Enter Password UI
	if (WiFi.encryptionType(y0 + yc) != WIFI_AUTH_OPEN) 
	{
		int xc = 0;
		char pwd[20] = "";
		int blink = 0;

		memset(pwd, 0, 20);
		
		//tft.cp437(true);         // Use full 256 char 'Code Page 437' font
		while(curnet.password == "")
		{
			blink = (blink+1)%2;
			tft.fillScreen(TFT_BLACK);
			tft.setCursor(0, 0);
			tft.setTextColor(TFT_YELLOW);
			tft.println("Enter Password for");
			tft.setTextColor(TFT_WHITE);
			tft.println(curnet.ssid);
			tft.drawFastHLine(0, 40, 240, TFT_CYAN);
			tft.setCursor(0, 50);
			tft.setTextColor(TFT_WHITE);
			tft.println(pwd);

			tft.fillRect(xc*12, 66, 12, 4, blink ? TFT_YELLOW : TFT_BLACK);

			tft.setCursor(0, 75);
			tft.setTextColor(TFT_DARKCYAN);
			tft.println("+/-: Change letter");
			tft.println("prev/next: Cursor");
			tft.println("100/200: Case/Symbol");
			

			if (GetRemoteCode())
			{
				if (IsCode(KEY_1)) xc = shift_chr(pwd, xc,'1');
				if (IsCode(KEY_2)) xc = shift_chr(pwd, xc,'2');
				if (IsCode(KEY_3)) xc = shift_chr(pwd, xc,'3');
				if (IsCode(KEY_4)) xc = shift_chr(pwd, xc,'4');
				if (IsCode(KEY_5)) xc = shift_chr(pwd, xc,'5');
				if (IsCode(KEY_6)) xc = shift_chr(pwd, xc,'6');
				if (IsCode(KEY_7)) xc = shift_chr(pwd, xc,'7');
				if (IsCode(KEY_8)) xc = shift_chr(pwd, xc,'8');
				if (IsCode(KEY_9)) xc = shift_chr(pwd, xc,'9');
				if (IsCode(KEY_0)) xc = shift_chr(pwd, xc,'0');

				if (IsCode(KEY_100))
				{
					if (isupper(pwd[xc])) pwd[xc] = tolower(pwd[xc]);
					else pwd[xc] = toupper(pwd[xc]);
				}
				if (IsCode(KEY_200)) 
				{
					if (isalnum(pwd[xc])) pwd[xc] = '!';
					else pwd[xc] = 'A';
				}
				if (IsCode(KEY_MINUS, false))
				{
					if (pwd[xc] == 0) pwd[xc] = 'A';
					else if (pwd[xc] < MAX_CHAR) pwd[xc]++;
				}
				if (IsCode(KEY_PLUS, false))
				{
					if (pwd[xc] == 0) pwd[xc] = 'Z';
					else if (pwd[xc] > MIN_CHAR) pwd[xc]--;
				}
				if (IsCode(KEY_CH))
				{
					curnet.password = String(pwd);
					curnet.password.trim();
					return true;
				}
				if (IsCode(KEY_CH_MINUS) || IsCode(KEY_PREV))
				{
					if (xc > 0) xc--;
				}
				if (IsCode(KEY_CH_PLUS) || IsCode(KEY_NEXT))
				{
					if (xc < 19 && pwd[xc]) xc++;
				}
				if (IsCode(KEY_PLAYPAUSE)) shutdown();
			}
			delay(100);
		}
	}

	return false;
}


const char *AP_SSID = "Internet-Radio";

void handle_root()
{
	String html = html_header;
	if (server.hasArg("ssid"))
	{
		int i = server.arg("ssid").toInt();
		html += "<p>Connect to WiFi:";
		html += WiFi.SSID(i);
		html += "</p>";
		html += "<form action=\"/\" method=\"post\">Password: <input type=\"text\" name=\"pwd\">";
		html += "<input type=\"hidden\" name=\"ssid\" value=\"";
		html += i;
		html += "\"><input type=\"submit\" value=\"Connect\">";
		html += "</form><br>";
		html += "<a href=\"/\">Back to WiFi List</a>";
	}
	else
	{
		html += "<p>WiFi Networks</p>";
		html += "<ol>";
		for (uint i = 0; i < n_SSID; i++)
		{
			// Print SSID and RSSI for each network found
			html += "<li><a href=\"/?ssid=";
			html += i;
			html += "\">";
			html += WiFi.SSID(i);
			html += "</a> (";
			html += WiFi.RSSI(i);
			html += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? ") " : ")*";
			html += "</li>";
		}
		html += "</ol>";
	}
	html += html_footer;
	server.send(200, "text/html", html);
}

void handle_post()
{
	String html = html_header;
	if (server.hasArg("ssid"))
	{
		int i = server.arg("ssid").toInt();
		if (server.hasArg("pwd"))
		{
			String password = server.arg("pwd");
			Serial.print("SSID: ");
			Serial.println(WiFi.SSID(i));
			Serial.print("Password: ");
			Serial.println(password);

			curnet.ssid = WiFi.SSID(i);
			curnet.password = password;
			html += "<p>Connecting to: ";
			html += curnet.ssid;
			html += "...</p>";
		}
	}

	html += "<br><a href=\"https://pilot.ge/radio\">Go to Control Interface</a>";
	html += html_footer;
	server.send(200, "text/html", html);
}

void handle_notfound()
{
	server.send(404, "text/plain", "Not found");
}

void start_ap_server()
{
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();

	curnet.ssid = "";
	curnet.password = "";

	Serial.print("Setting WiFi Access Point...");
	WiFi.softAP(AP_SSID);
	Serial.print("AP IP address: ");
	Serial.println(WiFi.softAPIP());

	//server.reset();
	server.on("/", HTTP_POST, handle_post);
	server.on("/", HTTP_GET, handle_root);
	server.onNotFound(handle_notfound);
	server.begin();
	Serial.println("AP HTTP server started");

	DisplayHeader();
	tft.print("Setup WiFi Network\nAP: ");
	tft.println(AP_SSID);
	tft.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());	
}

String getStatus()
{
	static char r[32];
	ulong m = millis()/60000;
	uint h = m/60;
	uint d = h/24;
	if (d > 0) sprintf(r, "Ready (%d day %d hr %ld min)", d, h%24, m%60);
	else if (h > 0) sprintf(r, "Ready (%d hr %ld min)", h%24, m%60);
	else sprintf(r, "Ready (%ld min)", m%60);
	return String(r);
}

void initTopics()
{
	strcpy(topic_command, MQTT_ID);
	strcpy(topic_status, MQTT_ID);
	strcpy(topic_list, MQTT_ID);
	
	strcat(topic_command, "/command");
	strcat(topic_status, "/status");
	strcat(topic_list, "/list");
}

void publishStatus()
{
	String json = "{ \"status\":\"" +  getStatus() + "\"";
	json += ",\"voltage\":";
	json += getVbat();
	json += ",\"ssid\":\"";
	json += WiFi.SSID();
	json += "\",\"rssi\":";
	json += WiFi.RSSI();
	json += ",\"webvol\":";
	json += WebVolume;
	json += ",\"fmvol\":";
	json += FMVolume;
	if (CurrentRadio == WEB_RADIO) {
		json += ",\"source\":\"web\"";
		json += ",\"url\":\"";
		json += WebStation.url;
		json += "\",\"name\":\"";
		json += WebStation.name;
		json += "\",\"title\":\"";
		json += WebStation.title;
		json += "\"";
	} 
	else 
	{
		json += ",\"source\":\"fm\"";
		json += ",\"freq\":";
		json += FMStation.freq;
		json += ",\"name\":\"";
		json += FMStation.name;
		json += "\"";
	}
	json += "}";
	pubsub.publish(topic_status, json.c_str());
}

void publishList()
{
	pubsub.publish(topic_list, "-");
	for (uint i = 0; i < n_stations; i++)
	{
		String line = ((CurrentRadio == WEB_RADIO && Stations[i].url == WebStation.url) ||	
			(CurrentRadio == FM_RADIO && Stations[i].freq == FMStation.freq)) ? "1" : "0";
		line += ",";
		line += Stations[i].freq;
		line += ",";
		line += Stations[i].url;
		line += ",";
		line += Stations[i].name;
		pubsub.publish(topic_list, line.c_str());
	}
	pubsub.publish(topic_list, "+");
}

void callback(char *topic, byte *payload, unsigned int length)
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	String sjson = "";
	for (uint i = 0; i < length; i++)
	{
		sjson += (char)payload[i];
	}
	Serial.println(sjson);

	DynamicJsonDocument doc(256);
	DeserializationError error = deserializeJson(doc, sjson);

	if (error)
	{
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
		return;
	}

	//blink(1, true);

	if (!strcmp(topic_command, topic))
	{
		if (!doc["status"].isNull())
		{
			publishStatus();
		}
		if (!doc["list"].isNull())
		{
			publishList();
		}
		if (!doc["webvol"].isNull())
		{
			SetWebVolume(doc["webvol"], false);
		}
		if (!doc["fmvol"].isNull())
		{
			SetFMVolume(doc["fmvol"], false);
		}
		if (!doc["play"].isNull())
		{
			String src = doc["play"];
			uint freq = src.toInt();
			if (freq >= MIN_FREQ && freq <= MAX_FREQ)
			{
				TuneFMStation(freq, "FM " + String(((float)freq)/10), true);
			}
			else if (src.length() > 0)
			{
				PlayWebStation(src, DefaultWebStationName);
			}
		}
		if (!doc["add"].isNull())
		{
			String src = doc["add"];
			uint freq = src.toInt();
			String name = doc["name"];
			name.trim();
			if (name == "") name = "Station " + n_stations;
			if (freq >= MIN_FREQ && freq <= MAX_FREQ)
			{
				AddStation(freq, "", name);
				SaveRadioStations();
				publishList();
			}
			else if (src.length() > 0)
			{
				AddStation(0, src, name);
				SaveRadioStations();
				publishList();
			}
		}
		if (!doc["up"].isNull())
		{
			uint index = doc["up"];
			ShiftStation(index, -1);
			SaveRadioStations();
			publishList();
		}
		if (!doc["down"].isNull())
		{
			uint index = doc["down"];
			ShiftStation(index, 1);
			SaveRadioStations();
			publishList();
		}
		if (!doc["remove"].isNull())
		{
			uint index = doc["remove"];
			RemoveStation(index);
			SaveRadioStations();
			publishList();
		}
		if (!doc["sleep"].isNull())
		{
			shutdown();
		}
		if (!doc["display"].isNull())
		{
			DisplayCurrentMode(doc["display"]);
			lastKeyTime = millis();
		}
	}
}

void reconnect_mqtt()
{
	Serial.print("Attempting MQTT connection...");
	// Attempt to connect
	if (pubsub.connect(MQTT_ID))
	{
		Serial.println("connected");
		// Once connected, publish an announcement...
		pubsub.subscribe(topic_command);
		Serial.printf("Subscribed to: %s\n",topic_command);
		//blink(2);
	}
	else
	{
		Serial.print("failed, rc=");
		Serial.print(pubsub.state());
		Serial.println(" try again later...");
		//blink(5);
	}
}

void connect_network()
{
	list_networks();

	if (!connect_saved_networks())
	{
		do {
			if (want_display_ui())
			{
				// run Wifi setup on display UI
				get_network_ui();
			}
			else
			{
				// run web based Wifi setup
				start_ap_server();
				while (curnet.ssid == "") server.handleClient();
				server.handleClient();
				server.stop();
				WiFi.mode(WIFI_STA);
				WiFi.disconnect();
			}
		} while (!connect_ssid(curnet.ssid, curnet.password));

		// store current to saved to networks
		save_current_network();
	}

	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void NetworkInit()
{
	DisplayHeader();

	// Set WiFi to station mode and disconnect from an AP if it was previously connected
	WiFi.setAutoReconnect(true);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);

	load_networks();
	connect_network();
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

	initTopics();
	pubsub.setKeepAlive(MQTT_KEEPALIVE_SEC);
	pubsub.setCallback(callback);
}

void NetworkJob()
{
	if (!pubsub.connected()) 
	{
		isConnected = false;
		
		if (millis() - lastReconnectTime > RECONNECT_INTERVAL_MS)
		{
			reconnect_mqtt();
			lastReconnectTime = millis();
		}
	}
	else pubsub.loop();
}

// 	server.on("/dec", HTTP_GET, [](AsyncWebServerRequest *request) {
// 		if (request->hasParam("num"))
// 		{
// 			ShiftStation(request->getParam("num")->value().toInt(), -1);
// 			SaveRadioStations();
// 			request->redirect("/");
// 		}
// 		else 
// 		{
// 			request->redirect("/?msg=Incorrect Param");
// 		}
// 	});



#endif //__NETWORK_H__