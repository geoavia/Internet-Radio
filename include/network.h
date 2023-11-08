#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "main.hpp"

//WiFiClient client;
AsyncWebServer server(80);

bool async_hot = false;
int async_webvol = -1;
int async_fmvol = -1;
uint async_freq = 0;
String async_url = "";

void async_clear()
{
	async_hot = false;
	async_webvol = async_fmvol = -1;
	async_freq = 0;
	async_url = "";
}

void async_setFreq(uint f)
{
	async_clear();
	async_freq = f;
	async_hot = true;
}

void async_setUrl(String u)
{
	async_clear();
	async_url = u;
	async_hot = true;
}

void async_setVol(uint v, RADIO_TYPE t)
{
	async_clear();
	if (t == FM_RADIO) async_fmvol = v;
	else async_webvol = v;
	async_hot = true;
}


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
		tft.setTextColor(TFT_WHITE);
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
		tft.setTextColor(TFT_WHITE);
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
			DisplayRSSI(0, 10+(y+1)*8, WiFi.RSSI(y0 + y), ((yc == y) ? TFT_BLACK : TFT_WHITE));
		}
		

		while (!GetRemoteCode() || IsRepeat) delay(10); 
		if (IsCode(KEY_PLUS) || IsCode(KEY_NEXT) || IsCode (KEY_CH_MINUS))
		{
			if (yc > 0) yc--;
			else if (yc == 0 && y0 > 0) y0--;
			//else yc == y0 = 0;
		}
		if (IsCode(KEY_MINUS) || IsCode(KEY_PREV) || IsCode (KEY_CH_PLUS))
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
			blink = (blink+1)%20;
			tft.fillScreen(TFT_BLACK);
			tft.setCursor(0, 0);
			tft.setTextColor(TFT_WHITE);
			tft.println("Enter Password for");
			tft.println(curnet.ssid);
			tft.drawFastHLine(0, 40, 240, TFT_WHITE);
			tft.setCursor(0, 50);
			tft.println(pwd);

			tft.fillRect(xc*12, 66, 12, 4, (blink/10) ? TFT_WHITE : TFT_BLACK);

			tft.setCursor(0, 75);
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
			delay(10);
		}
	}

	return false;
}

void handle_NotFound(AsyncWebServerRequest *request)
{
	request->send(404, "text/plain", "Not found");
}

const char *AP_SSID = "Internet-Radio";

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

	server.reset();
	server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
		String html = html_header;
		if (request->hasParam("ssid", true))
		{
			int i = request->getParam("ssid", true)->value().toInt();
			if (request->hasParam("pwd", true))
			{
				String password = request->getParam("pwd", true)->value();
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
		html += html_footer;
		request->send(200, "text/html", html);
	});
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		String html = html_header;
		if (request->hasParam("ssid"))
		{
			int i = request->getParam("ssid")->value().toInt();
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
			for (int i = 0; i < n_SSID; ++i)
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
		request->send(200, "text/html", html);
	});
	server.onNotFound(handle_NotFound);
	server.begin();
	Serial.println("AP HTTP server started");

	DisplayHeader();
	tft.print("Setup WiFi Network\nAP: ");
	tft.println(AP_SSID);
	tft.printf("IP: %s\n", WiFi.softAPIP().toString().c_str());
	
}

void start_radio_server()
{
	server.reset();

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
		String html = html_header;
		if (request->hasParam("msg")) 
		{
			html += "<p><b>";
			html += request->getParam("msg")->value();
			html += "</b></p>";
		}
		html += "<div class='radio web'>";
		html += "<p>Current URL: <b>";
		html += WebStation.url;
		html += "</b></p>";
		html += R"===(<form action="/get">MP3 Radio URL: <input type="text" name="mp3url">
&nbsp;<input type="submit" value="Play" name="play">&nbsp;
Name: <input type="text" name="mp3name">&nbsp;<input type="submit" value="Add" name="add">
</form><br>)===";

		html += R"===(<form action="/vol"><input type="submit" value="Set Volume">&nbsp;
<input type="range" name="mp3vol" min="0" max=")===";
		html += MAX_WEB_VOLUME;
		html += "\" value=\"";
		html += WebVolume;
		html += "\">";
		html += "</form><br>";
		html += "<p>Playlist</p>";
		html += "<table><tr><th>#</th><th>Station</th><th>URL</th><th></th></tr>";
		int n = 1;
		for (uint i = 0; i < n_stations; i++)
		{
			if (IsType(i, WEB_RADIO))
			{
				if (Stations[i].url == WebStation.url) html += "<tr class='curr'><td>";
				else html += "<tr><td>";
				html += n;
				html += "</td><td><a href=\"/get?mp3url=";
				html += EncodeUrl(Stations[i].url);
				html += "\">";
				html += Stations[i].name;
				html += "</td><td>";
				html += Stations[i].url;
				html += "</td><td>";
				html += "<a href=\"/del?mp3url=";
				html += EncodeUrl(Stations[i].url);
				html += "\">Remove</a></td></tr>";
				n++;
			}
		}
		html += "</table></div><div class='radio fm'>";
		html += "<p>Current Frequency: <b>";
		html += String(((float)FMStation.freq)/10);
		html += "</b></p>";
		html += R"===(<form action="/get">FM Radio Frequency: <input type="text" name="fmfreq">
&nbsp;<input type="submit" value="Tune" name="tune">&nbsp;
Name: <input type="text" name="fmname">&nbsp;<input type="submit" value="Add" name="add">
</form><br>)===";

		html += R"===(<form action="/vol"><input type="submit" value="Set Volume">&nbsp;
<input type="range" name="fmvol" min="0" max=")===";
		html += MAX_FM_VOLUME;
		html += "\" value=\"";
		html += FMVolume;
		html += "\">";
		html += "</form><br>";
		html += "<p>Playlist</p>";
		html += "<table><tr><th>#</th><th>Station</th><th>Frequency</th><th></th></tr>";
		n = 1;
		for (uint i = 0; i < n_stations; i++)
		{
			if (IsType(i,FM_RADIO))
			{
				if (Stations[i].freq == FMStation.freq) html += "<tr class='curr'><td>";
				else html += "<tr><td>";
				html += n;
				html += "</td><td><a href=\"/get?fmfreq=";
				html += String(((float)Stations[i].freq)/10);
				html += "\">";
				html += Stations[i].name;
				html += "</td><td>";
				html += String(((float)Stations[i].freq)/10);
				html += "</td><td>";
				html += "<a href=\"/del?fmfreq=";
				html += Stations[i].freq;
				html += "\">Remove</a></td></tr>";
				n++;
			}
		}
		html += "</table></div>";
		html += html_footer;
		request->send(200, "text/html", html);
	});
			
	server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {
		if (request->hasParam("mp3url"))
		{
			String url = request->getParam("mp3url")->value();
			if (request->hasParam("add")) 
			{
				String name = "";
				if (request->hasParam("mp3name"))
					name = request->getParam("mp3name")->value();
				name.trim();
				if (name == "")
				{
					name = "Station ";
					name += n_stations;
				}
				
				AddStation(0, url, name);
				SaveRadioStations();
			} 
			else // play
			{
				async_setUrl(url);
				delay(1000); // for Job to finish
			}
			request->redirect("/");
		}
		else if (request->hasParam("fmfreq"))
		{
			uint freq = request->getParam("fmfreq")->value().toFloat() * 10;
			if (request->hasParam("add")) 
			{
				String name = "";
				if (request->hasParam("fmname"))
					name = request->getParam("fmname")->value();
				name.trim();
				if (name == "")
				{
					name = "Station ";
					name += n_stations;
				}
				
				AddStation(freq, "", name);
				SaveRadioStations();
			} 
			else // tune
			{
				async_setFreq(freq);
				delay(1000); // for Job to finish
			}
			request->redirect("/");
		}
		else 
		{
			request->redirect("/?msg=Incorrect Param");
		}
	});

	server.on("/del", HTTP_GET, [](AsyncWebServerRequest *request) {
		if (request->hasParam("mp3url"))
		{
			String url = request->getParam("mp3url")->value();
			RemoveStationByUrl(url);
			SaveRadioStations();
			request->redirect("/");
		}
		else if (request->hasParam("fmfreq"))
		{
			uint freq = request->getParam("fmfreq")->value().toInt();
			RemoveStationByFreq(freq);
			SaveRadioStations();
			request->redirect("/");
		}
		else 
		{
			request->redirect("/?msg=Incorrect Param");
		}
	});

	server.on("/vol", HTTP_GET, [](AsyncWebServerRequest *request) {
		if (request->hasParam("mp3vol"))
		{
			async_setVol(request->getParam("mp3vol")->value().toInt(), WEB_RADIO);
			delay(1000); // for Job to finish
			request->redirect("/");
		}
		if (request->hasParam("fmvol"))
		{
			async_setVol(request->getParam("fmvol")->value().toInt(), FM_RADIO);
			delay(1000); // for Job to finish
			request->redirect("/");
		}
		else 
		{
			request->redirect("/?msg=Incorrect Param");
		}
	});

	server.on("/radio.css", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/radio.css", "text/css");
	});
	server.onNotFound(handle_NotFound);

	server.begin();

	Serial.println("Radio HTTP server started");
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
				while (curnet.ssid == "") delay(100);
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
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);

	load_networks();
	connect_network();
	start_radio_server();
	WiFi.setAutoReconnect(true);
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}


#endif //__NETWORK_H__