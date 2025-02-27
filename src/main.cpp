#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <SpeicherLib.h>


ESP8266WebServer server(80);
const char* ssidap = "AP-Speicher";
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

Daten daten = Daten();
Speicher speicher = Speicher(&daten);

#define HTML "<!DOCTYPE html><html lang='de'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'><style>body{text-align:center;font-family:verdana,sans-serif;background:#252525;}div{text-align:center;font-size: 1.1em;display:inline-block;color:#eaeaea;width:340px;}div.div1{text-align:left;font-size: 1.0em;font-weight:bold;display:flex;padding:5px;}.db{display:block;}span.span1{text-align:right;font-weight:normal;width:50px;margin-left:auto;}span.span2{width:30px;text-align:center;font-weight:normal;margin-right:10px}button{border:0;border-radius:0.3rem;background:#1fa3ec;color:#faffff;line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;cursor:pointer;}button:hover{background:#0e70a4;}.bred{background:#d43535;}.bred:hover{background:#931f1f;}a{color:#1fa3ec;text-decoration:none;}</style><title>Speicher M01 - Hauptmenü</title></head><body><div><div><h3>ESP8266 D1 Mini</h3><h2>Speicher M01</h2></div><div class='div1'>Spannung:<span class='span1' id='sp'>52.8</span><span class='span2'>V</span></div><div class='div1'>Ladezustand (SOC):<span class='span1' id='lz'>71</span><span class='span2'>%</span></div><div class='div1'>Strom Akku:<span class='span1' id='s1'>2.8</span><span class='span2'>A</span></div><div class='div1' id='s2d'>Strom PV:<span class='span1' id='s2'>0.0</span><span class='span2'>A</span></div><div class='div1' id='tpd'>Temperatur:<span class='span1' id='tp'>23</span><span class='span2'>°C</span></div><div id=but3d class='db'></div><p><form id=but2 class='db' action='md4' method='get'><button>mehr Daten</button></form></p><p><form id=but1 class='db' action='einst' method='get'><button>Einstellungen</button></form></p><p><form id=but0 class='db' action='.' method='get' onsubmit='return confirm(\"Wirklich neustarten?\");'><button name='rst' class='button bred'>Neustart</button></form></p><div style='text-align:right;font-size:11px;'><hr/><a href='' target='_blank' style='color:#aaa;'>Speicher2Mqtt 0.1 von Martin Dienert</a></div></div></body><script>let eB=s=>document.getElementById(s);let to;function dho(){clearTimeout(to);let x=new XMLHttpRequest();x.overrideMimeType('application/json');x.open('GET','/daten.json',true);x.onreadystatechange=()=>{if(x.readyState==4&&x.status==200){sD(x.responseText);to=setTimeout(()=>{dho();},3000);}};x.send();}function sD(t){od = JSON.parse(t);eB('sp').innerHTML = od.Spannung;eB('lz').innerHTML = od.Ladezustand;eB('s1').innerHTML = od.StromAkku;if(od.Typ=='1'){eB('s2d').style.display='none';eB('tpd').style.display='none';}else{eB('s2d').style.display='flex';eB('tpd').style.display='flex';eB('s2').innerHTML=od.StromPV;eB('tp').innerHTML=od.Temperatur;}}dho();</script></html>"
//#define EINST "<!DOCTYPE html><html lang='de'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/><style>body{text-align:center;font-family:verdana,sans-serif;background:#252525;}div{text-align:center;font-size:1.1em;display:inline-block;color:#eaeaea;width:340px;}fieldset{background:#4f4f4f;}input{font-size:1.1em;width:100%;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;background:#dddddd;color:#000000;}button{border:0;border-radius:0.3rem;background:#1fa3ec;color:#faffff;line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;cursor:pointer;}button:hover{background:#0e70a4;}.bgrn{background:#47c266;}.bgrn:hover{background:#5aaf6f;}a{color:#1fa3ec;text-decoration:none;}</style><title>Speicher M01 - Einstellungen</title></head><body><div><div><h3>ESP8266 D1 Mini</h3><h2>Speicher M01</h2></div><fieldset><legend><b>&nbsp;WLAN-Einstellungen&nbsp;</b></legend><form method='get' action='einst'><p><b>WLAN - SSID</b><br><input id='s1' placeholder='WiFi Netzwerk eingeben' name='ssid' value=''></p><p><b>WLAN - Passwort</b><br><input id='p1' type='password' placeholder='WiFi Passwort eingeben' name='pwd' value=''></p><button name='save' value='1' type='submit' class='button bgrn'>Speichern</button></form></fieldset><div id=but3d style='display: block;'></div><p><form id=but3 style='display: block;' action='/' method='get'><button>Hauptmen&uuml;</button></form><div style='text-align:right;font-size:11px;'><hr/><a href='' target='_blank' style='color:#aaa;'>Speicher2Mqtt 0.1 von Martin Dienert</a></div></div></body></html>"

void hauptseite(){
  server.send(200, "text/html", HTML);
}

void einstellungsseite(){
  File datei = LittleFS.open("/einst.html", "r");
  if(!datei){
    server.send(404, "text/plain", "Datei nicht gefunden.");
    return;
  }
  server.streamFile(datei, "text/html");
  if(server.hasArg("ssid") && server.hasArg("pwd")){
    File dat = LittleFS.open("ssidpwd", "w");
    if(dat){
      dat.print(server.arg("ssid"));
      dat.print(";");
      dat.print(server.arg("pwd"));
      dat.print(";");
      dat.close();
    }
  }
}

void sendedaten(){
  server.send(200, "application/json", daten.json);
}

void mehrdaten(){
  server.send(200, "text/html", HTML);
  speicher.sendeTel(1);
}

void softreset(){
  server.send(200, "text/html", "Restart_Ok");
  delay(500);
  ESP.restart();
}

void setup(){
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  Serial.setTimeout(40);
  Serial.begin(115200);
  if(LittleFS.begin()){
    File dat = LittleFS.open("ssidpwd", "r");
    if(dat){
      String ssid = dat.readStringUntil(';');
      String pwd = dat.readStringUntil(';');
      dat.close();
      WiFi.begin(ssid, pwd);
      int i = 0;
      while(i < 60 && WiFi.status() != WL_CONNECTED){
        delay(500);
        i++;
      }
    }
  }
  if(WiFi.status() != WL_CONNECTED){
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssidap);
    delay(500);
  }

  server.onNotFound([](){
    server.send(404, "text/plain", "Link wurde nicht gefunden!");  
  });
  server.on("/", hauptseite);
  server.on("/einst", einstellungsseite);
  server.on("/daten.json", sendedaten);
  server.on("/md4", mehrdaten);
  server.on("/.", softreset);
  server.on("/cn", []() {
    server.send(200, "text/plain", "Nur eine Beispiel-Route");
  });
  server.begin();

  while(Serial.available()){      // Puffer leeren
    Serial.read();
    delay(5);
  }
}

void loop(){
  speicher.run();
  server.handleClient();  
}
