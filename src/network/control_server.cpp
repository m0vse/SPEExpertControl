/*
 * Lightweight HTTP server that mirrors the LCD UI and exposes amplifier control endpoints.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "network/control_server.h"

#include "amp_control.h"
#include "app_status.h"
#include "network/spe_logo_svg.h"
#include <Arduino.h>
#include <WiFi.h>

class ControlServer {
public:
    void service()
    {
        const int wifi_status = WiFi.status();
        if (wifi_status != WL_CONNECTED) {
            last_wifi_status_ = wifi_status;
            return;
        }

        if (last_wifi_status_ != WL_CONNECTED) {
            printWifiStatus();
            last_wifi_status_ = wifi_status;
            begin();
        }

        WiFiClient client = server_.accept();
        if (!client) {
            return;
        }

        handleClient(client);
    }

private:
    void begin()
    {
        if (started_) {
            return;
        }

        server_.begin();
        started_ = true;
        Serial.println(F("HTTP control server started"));
    }

    void printWifiStatus()
    {
        IPAddress ip = WiFi.localIP();
        Serial.print(F("WiFi connected: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("IP address: "));
        Serial.println(ip);
        Serial.print(F("RSSI: "));
        Serial.print(WiFi.RSSI());
        Serial.println(F(" dBm"));
        Serial.print(F("Web UI: http://"));
        Serial.println(ip);
    }

    void handleClient(WiFiClient &client)
    {
        String request_line;
        const unsigned long started = millis();

        while (client.connected() && millis() - started < 1000) {
            if (!client.available()) {
                delay(1);
                continue;
            }

            char c = client.read();
            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                break;
            }
            request_line += c;
        }

        while (client.available()) {
            client.read();
        }

        if (request_line.startsWith("GET /spe-logo.svg")) {
            sendLogo(client);
        } else if (request_line.startsWith("GET /api/status") || request_line.startsWith("GET /status.json")) {
            sendStatusJson(client);
        } else if (request_line.startsWith("GET /api/key?name=")) {
            handleKeyRequest(client, request_line);
        } else {
            sendIndex(client);
        }

        client.flush();
        delay(25);
        client.stop();
    }

    void sendStatusJson(WiFiClient &client)
    {
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: application/json"));
        client.println(F("Connection: close"));
        client.println();
        client.print(F("{\"wifi\":{\"status\":\"connected\",\"ssid\":\""));
        client.print(WiFi.SSID());
        client.print(F("\",\"ip\":\""));
        client.print(WiFi.localIP());
        client.print(F("\",\"rssi\":"));
        client.print(WiFi.RSSI());
        client.print(F("},\"amp\":"));
        app_status_print_json(client);
        client.println(F("}"));
    }

    void sendLogo(WiFiClient &client)
    {
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: image/svg+xml"));
        client.print(F("Content-Length: "));
        client.println(SPE_LOGO_SVG_SIZE);
        client.println(F("Cache-Control: max-age=86400"));
        client.println(F("Connection: close"));
        client.println();
        client.print(SPE_LOGO_SVG);
    }

    void handleKeyRequest(WiFiClient &client, const String &request_line)
    {
        String name = request_line.substring(strlen("GET /api/key?name="));
        const int end = name.indexOf(' ');
        if (end >= 0) {
            name.remove(end);
        }

        const bool ok = amp_control_press_key(name.c_str());
        client.println(ok ? F("HTTP/1.1 200 OK") : F("HTTP/1.1 400 Bad Request"));
        client.println(F("Content-Type: application/json"));
        client.println(F("Connection: close"));
        client.println();
        client.print(F("{\"ok\":"));
        client.print(ok ? F("true") : F("false"));
        client.print(F(",\"key\":\""));
        client.print(name);
        client.println(F("\"}"));
    }

    void sendIndex(WiFiClient &client)
    {
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-Type: text/html; charset=utf-8"));
        client.println(F("Connection: close"));
        client.println();
        client.println(F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><title>SPE Expert Control</title>"));
        client.println(F("<style>body{margin:0;background:#0b1014;color:#d9f7ff;font-family:system-ui,Arial,sans-serif}.wrap{width:min(100vw,900px);margin:auto;padding:12px;box-sizing:border-box}.lcd{position:relative;width:800px;height:480px;background:#00b3fe;color:#050505;font:24px ui-monospace,Consolas,monospace;font-weight:700;transform-origin:top left}.panel{position:absolute;left:17px;top:23px;width:763px;height:149px;border:4px solid #000;box-sizing:border-box}.setup{top:15px;height:232px;border-width:4px 0 0 0}.cap{position:absolute;top:-18px;left:50%;transform:translateX(-50%);background:#00b3fe;padding:0 8px;white-space:nowrap}.line{position:absolute;white-space:pre}.center{text-align:center}.sel{background:#000;color:#00b3fe}.bar{position:absolute;height:25px;background:#000;outline:1px solid #000}.fill{height:100%;background:#fff}.amp{position:absolute;left:17px;top:183px;width:763px;height:75px;border-top:4px solid #000;display:flex}.amp div{height:65px;border-right:4px solid #000;text-align:center;box-sizing:border-box;padding-top:5px;white-space:pre-line;line-height:27px}.hidden{display:none}.btn{position:absolute;background:#5a5a5a;color:#fff;border:0;outline:1px solid #ff5f00;outline-offset:2px;border-radius:0;font:28px system-ui,Arial,sans-serif;font-weight:700}.btn.hit,.btn:active{background:#ff5f00;color:#111}.btn:disabled{background:#2f3437;color:#8c9aa0;outline-color:#6f7c82}.small{margin-top:8px;color:#8fb2bf;font-size:14px}.speLogo{position:absolute;left:0;top:0;width:380px;height:137px;object-fit:fill}.antc{position:absolute;top:40px;height:120px;border-right:4px solid #000}.antrow{position:relative;height:30px}.antrow span{position:absolute}.menuitem{position:absolute;padding-left:10px}.warnctl{position:absolute;left:5px;top:184px;width:790px;height:65px;border-top:4px solid #000;display:flex}.warnctl div{height:60px;border-right:4px solid #000;text-align:center;box-sizing:border-box;padding-top:5px}@media(max-width:824px){.lcd{transform:scale(calc((100vw - 24px)/800));margin-bottom:calc(480px * ((100vw - 24px)/800 - 1))}}</style>"));
        client.println(F("</head><body><div class='wrap'><div class='lcd'><div id='topPanel' class='panel'></div><div id='amp' class='amp'><div id='ain' style='width:90px'>IN<br>-</div><div id='aband' style='width:130px'>BAND<br>----</div><div id='aant' style='width:80px'>ANT<br>-</div><div id='acat' style='width:125px'>CAT<br>-----</div><div id='aout' style='width:105px'>OUT<br>----</div><div id='aswr' style='width:120px'>SWR<br>--.--</div><div id='atemp' style='width:110px;border-right:0'>TEMP<br>----</div></div>"));
        client.println(F("<button class='btn' style='left:14px;top:272px;width:101px;height:59px' data-k='l_down'>&lt; L</button><button class='btn' style='left:120px;top:272px;width:101px;height:59px' data-k='l_up'>L &gt;</button><button class='btn' style='left:14px;top:336px;width:101px;height:59px' data-k='c_down'>&lt; C</button><button class='btn' style='left:120px;top:336px;width:101px;height:59px' data-k='c_up'>C &gt;</button><button class='btn' style='left:14px;top:400px;width:207px;height:59px' data-k='tune'>TUNE</button>"));
        client.println(F("<button class='btn' style='left:247px;top:272px;width:97px;height:59px' data-k='input'>INPUT</button><button class='btn' style='left:349px;top:272px;width:96px;height:59px;font-size:24px' data-k='band_down'>&lt; BAND</button><button class='btn' style='left:450px;top:272px;width:97px;height:59px;font-size:24px' data-k='band_up'>BAND &gt;</button><button class='btn' style='left:247px;top:336px;width:97px;height:59px' data-k='ant'>ANT</button><button class='btn' style='left:349px;top:336px;width:96px;height:59px' data-k='left'>&lt; ^</button><button class='btn' style='left:450px;top:336px;width:97px;height:59px' data-k='right'>v &gt;</button><button class='btn' style='left:247px;top:400px;width:97px;height:59px' data-k='cat'>CAT</button><button class='btn' style='left:349px;top:400px;width:198px;height:59px' data-k='set'>SET</button>"));
        client.println(F("<button class='btn' style='left:573px;top:272px;width:101px;height:59px' data-k='off'>OFF</button><button class='btn' style='left:679px;top:272px;width:101px;height:59px' data-k='on'>ON</button><button class='btn' style='left:573px;top:336px;width:101px;height:59px;font-size:24px' data-k='power'>POWER</button><button class='btn' style='left:679px;top:336px;width:101px;height:59px;font-size:22px' data-k='display'>DISPLAY</button><button class='btn' style='left:573px;top:400px;width:207px;height:59px' data-k='operate'>OPERATE</button></div><div class='small'>Screen <span id='screen'>--</span> | WiFi <span id='wifi'>--</span> | IP <span id='ip'>--</span> | RSSI <span id='rssi'>--</span></div></div>"));
        client.println(F("<script>const opt=['ANTENNA','CAT','MANUAL TUNE','BACKLIGHT','CONTEST','BEEP','START','TEMP','QUIT'],msg=['------- SET ANTENNAS vs. BANDS -------','----- SET CAT INTERFACE FEATURES -----','------- MANUAL TUNE OPERATIONS -------','----- DISPLAY BACKLIGHT SETTINGS -----','-------- CONTEST MODE On/Off ---------','------ ACOUSTIC FEEDBACK On/Off ------','------ SET STARTUP DEFAULT MODE ------','-- TEMPERATURE VALUE SHOWN IN C/F --','---------- LEAVE THIS MENU -----------'],cats=['SPE','ICOM','KENWD','YAESU','TTEC','FLEX','RS232','NONE'],yaesu=['FT 100','FT 757 GX2','FT 817/847','FT 840/890','FT 897','FT 900','FT 920','FT 990','FT 1000','FT 1000 MP1','FT 1000 MP2','FT 1000 MP3','FT 2000','FT 9000D','BAND DATA'],icom=['CI-V','VOLTAGE BAND'],tt=['OMNI VII','ORION I/II','JUPITER','ARGONAUT'],baud=['1200 Baud','2400 Baud','4800 Baud','9600 Baud'],bands=['160 m','80 m','40 m','30 m','20 m','17 m','15 m','12 m','10 m','6 m'],ant=['1','2','3','4','NO'],warn=['DEBUGGING','POWER MANAGEMENT : V PA < 20 V','POWER MANAGEMENT : V PA < 26 V','POWER MANAGEMENT : V PA > 50 V','POWER MANAGEMENT : V PA > 50 V','POWER MANAGEMENT : I PA > 40 A','POWER MANAGEMENT : I PA > 50 A','OVER TEMPERATURE : TEMP > 90 C','P.A. MANAGEMENT : OVER DRIVING','DEBUGGING','DEBUGGING','P.A.MANAGEMENT : PW REV > 300W','P.A. MANAGEMENT : PA PROTECTION'];"));
        client.println(F("function s(id,v){document.getElementById(id).textContent=v}function e(v){return String(v==null?'':v).replace(/[&<>]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]))}function div(c,x,y,t){return '<div class=\"'+c+'\" style=\"left:'+x+'px;top:'+y+'px\">'+t+'</div>'}function bar(x,y,w,max,val){let p=Math.max(0,Math.min(100,Math.round((Number(val)||0)*100/max)));return '<div class=\"bar\" style=\"left:'+x+'px;top:'+y+'px;width:'+w+'px\"><div class=\"fill\" style=\"width:'+p+'%\"></div></div>'}function pF(st){let w=[5.2,6.5,12.5,21,40.9,86.5,168,342,693.5,1384.5],r=((st[3]&3)<<8)|st[2],v=0;for(let i=0;i<10;i++)if(r&(1<<i))v+=w[i];return v.toFixed(1)}"));
        client.println(F("function setAmp(a,show){let m=document.getElementById('amp');if(!document.getElementById('ain'))m.innerHTML='<div id=\"ain\" style=\"width:90px\">IN<br>-</div><div id=\"aband\" style=\"width:130px\">BAND<br>----</div><div id=\"aant\" style=\"width:80px\">ANT<br>-</div><div id=\"acat\" style=\"width:125px\">CAT<br>-----</div><div id=\"aout\" style=\"width:105px\">OUT<br>----</div><div id=\"aswr\" style=\"width:120px\">SWR<br>--.--</div><div id=\"atemp\" style=\"width:110px;border-right:0\">TEMP<br>----</div>';m.className=show?'amp':'amp hidden';s('ain','IN\\n'+e(a.input));s('aband','BAND\\n'+e(a.band));s('aant','ANT\\n'+e(a.antenna));s('acat','CAT\\n'+e(a.cat));s('aout','OUT\\n'+e(a.out));s('aswr','SWR\\n'+e(a.swr));s('atemp','TEMP\\n'+e(a.temp))}function setTop(c,h){let p=document.getElementById('topPanel');p.className='panel '+c;p.innerHTML=h}"));
        client.println(F("function receive(a){setTop('',div('line sel',330,126,' Standby ')+'<img class=\"speLogo\" src=\"/spe-logo.svg\">'+div('line',470,45,'EXPERT 1K-FA<br>Solid State<br>Fully Automatic'));setAmp(a,true)}function tx(a){let op=a.screenName=='operate_rx'||a.screenName=='operate_tx';let pl=op?'PA OUT':'  OUT ';let vl=op?'  I PA':'V PA';setTop('',div('line sel',323,126,op?' Operate ':' TX Exciter ')+div('line',7,10,pl)+bar(136,10,360,1200,a.power)+div('line',504,10,e(a.power)+' W pep')+div('line',7,82,vl)+bar(136,82,360,op?60:60,op?a.current:a.voltage)+div('line',504,82,e(op?a.current:a.voltage)+(op?' A':' v')));setAmp(a,true)}"));
        client.println(F("function catScreen(a,st){let c1=st[0]&15,c2=st[3]&15,t1=c1==1?icom[st[1]&15]:c1==3?yaesu[st[1]&15]:c1==4?tt[st[1]&15]:'',t2=c2==1?icom[st[4]&15]:c2==3?yaesu[st[4]&15]:c2==4?tt[st[4]&15]:'';let ver=((st[6]>>4)&15)+''+(st[6]&15)+'_'+((st[7]>>4)&15)+(st[7]&15)+'_'+((st[8]>>4)&15)+(st[8]&15)+'_'+String.fromCharCode(st[9]||32);setTop('',div('cap',185,-18,' IN 1 ')+div('cap',552,-18,' IN 2 ')+'<div style=\"position:absolute;left:381px;top:0;width:4px;height:149px;background:#000\"></div>'+div('line',0,25,' CAT: '+e(cats[c1]||'?'))+div('line',0,55,t1?'TYPE: '+e(t1):'')+div('line',0,114,' VER:'+ver)+div('line',391,25,' CAT: '+e(cats[c2]||'?'))+div('line',391,55,t2?'TYPE: '+e(t2):''));setAmp(a,true)}"));
        client.println(F("function setupItems(a,st){let sel=st[1]&15;let vals=['ANTENNA','CAT','MANUAL TUNE','BACKLIGHT','CONTEST '+((a.flags>>5)&1?'On ':'Off'),'BEEP    '+((a.flags>>6)&1?'On ':'Off'),'START   '+((a.flags>>1)&1?'Operate':'Standby'),'TEMP    '+((a.flags>>7)&1?'C':'F'),'QUIT'];let pos=[[0,25],[0,60],[0,95],[0,130],[285,25],[285,60],[285,95],[285,130],[660,25]],h='<div class=\"cap\"> SETUP OPTIONS vs. IN '+e(a.input)+' </div>';for(let i=0;i<vals.length;i++)h+=div('menuitem '+(i==sel?'sel':''),pos[i][0],pos[i][1],e(vals[i]));h+=div('line',7,163,e(msg[sel]||''))+div('line',7,196,'[&lt;^] [v&gt;]:SELECT         [SET]:CONFIRM');setTop('setup',h);setAmp(a,false)}"));
        client.println(F("function antScreen(a,st){let idx=Math.min(st[0]||0,20),h='<div class=\"cap\"> SET ANTENNA vs. IN '+e(a.input)+' </div>';let groups=[[0,255,0,4],[270,235,4,8],[530,235,8,10]];for(let g of groups){h+='<div class=\"antc\" style=\"left:'+g[0]+'px;width:'+g[1]+'px\">';for(let i=g[2];i<g[3];i++){h+='<div class=\"antrow\"><span style=\"left:0px;top:0px\">'+(i<4?['160 m:',' 80 m:',' 40 m:',' 30 m:'][i]:i<8?['20 m:','17 m:','15 m:','12 m:'][i-4]:['10 m:',' 6 m:'][i-8])+'</span><span class=\"'+(idx==i*2?'sel':'')+'\" style=\"left:'+(g[0]?100:120)+'px;top:0px;width:55px\">'+e(ant[(st[i+1]&7)]||'?')+'</span><span class=\"'+(idx==i*2+1?'sel':'')+'\" style=\"left:'+(g[0]?165:185)+'px;top:0px;width:55px\">'+e(ant[((st[i+1]>>4)&7)]||'?')+'</span></div>'}if(g[0]==530)h+='<span class=\"'+(idx==20?'sel':'')+'\" style=\"position:absolute;left:110px;top:90px;width:110px;padding-left:15px\">SAVE</span>';h+='</div>'}let b=Math.min(Math.floor(idx/2),10),ord=idx%2,am=['--- Set 1st ANTENNA FOR 160 m BAND ---','---- Set 1st ANTENNA FOR 80 m BAND ---','---- Set 1st ANTENNA FOR 40 m BAND ---','---- Set 1st ANTENNA FOR 30 m BAND ---','---- Set 1st ANTENNA FOR 20 m BAND ---','---- Set 1st ANTENNA FOR 17 m BAND ---','---- Set 1st ANTENNA FOR 15 m BAND ---','---- Set 1st ANTENNA FOR 12 m BAND ---','---- Set 1st ANTENNA FOR 10 m BAND ---','---- Set 1st ANTENNA FOR 6 m BAND ----','------- SAVE SETTINGS AND EXIT -------'];if(idx<20&&ord)am[b]=am[b].replace('1st','2nd');h+=div('line',7,163,e(am[b]))+div('line',7,196,idx==20?'[&lt;^] [v&gt;]:SELECT         [SET]:CONFIRM':'[&lt;^] [v&gt;]:SELECT          [SET]:CHANGE');setTop('setup',h);setAmp(a,false)}"));
        client.println(F("function listSetup(a,title,items,sel,foot){let h='<div class=\"cap\"> '+title+' vs. IN '+e(a.input)+' </div>';for(let i=0;i<items.length;i++)h+=div('menuitem '+(i==sel?'sel':''),i<4?0:285,i<4?45+i*30:45+(i-4)*30,e(items[i]));h+=div('line',7,163,foot)+div('line',7,196,'[&lt;^] [v&gt;]:SELECT         [SET]:CONFIRM');setTop('setup',h);setAmp(a,false)}function manual(a,st){setTop('',div('line',7,25,'FREQ:')+div('line',160,25,(a.freq/1000).toFixed(3)+' MHz')+div('line',555,25,'SUB BAND:')+div('line',695,25,e(a.subBand))+div('line',7,55,'L:')+bar(147,60,273,127,st[1]||0)+div('line',510,55,((st[1]||0)/10).toFixed(1)+' uH')+div('line',7,85,'C:')+bar(147,90,273,2760,Number(pF(st)))+div('line',510,85,pF(st)+' pF'));setAmp(a,true)}"));
        client.println(F("function render(a){let st=a.setup||[],sn=a.screenName,sel=st[1]||0;if(!a.valid){setTop('',div('line sel',290,65,' INITIALIZING '));setAmp(a,false);return}if(sn=='receive')return receive(a);if(sn=='operate_rx'||sn=='operate_tx')return tx(a);if(sn=='cat')return catScreen(a,st);if(sn=='setup_options')return setupItems(a,st);if(sn=='set_antenna')return antScreen(a,st);if(sn=='set_cat')return listSetup(a,'SET CAT',cats,sel,'--------- SET CAT INTERFACE ----------');if(sn=='set_yaesu')return listSetup(a,'SET YAESU',yaesu,sel,'------- SET PROPER YAESU MODEL -------');if(sn=='set_icom')return listSetup(a,'SET ICOM',icom,sel,'----- SET PROPER ICOM INTERFACE ------');if(sn=='set_tentec')return listSetup(a,'SETUP TEN-TEC',tt,sel,'------ SET PROPER TEN-TEC MODEL ------');if(sn=='set_baudrate')return listSetup(a,'SET BAUD RATE',baud,sel,'-------- SET PROPER BAUD RATE --------');if(sn=='manual_tune')return manual(a,st);if(sn=='backlight'){setTop('setup','<div class=\"cap\"> DISPLAY BACKLIGHT </div>'+bar(205,75,354,255,st[1]||0)+div('line',7,163,'-------------- SETTINGS --------------')+div('line',7,196,'[&lt;^] [v&gt;]:ADJUST            [SET]:SAVE'));setAmp(a,false);return}if(sn=='alarm_history'){let n=st[0]&15,i=(st[0]>>4)&15,h='<div class=\"cap\"> ALARM HISTORY </div>';for(let x=0;x<4;x++){let j=i-x;if(n>x&&j>=0)h+=div('line',0,26+x*30,e(j+')IN '+(((st[j]>>7)&1)+1)+' '+(warn[st[j]&15]||'')))}setTop('',h);document.getElementById('amp').outerHTML='<div id=\"amp\" class=\"warnctl\"><div style=\"width:235px\">[&lt;^] [v&gt;]<br>SCROLL</div><div style=\"width:325px\">[TUNE] &amp; [OP]<br>CLEAR LIST</div><div style=\"width:220px;border-right:0\">[DISPLAY]<br>QUIT</div></div>';return}if(sn=='shutdown'||sn=='data_stored'){setTop('setup','<div class=\"cap\"> SYSTEM MESSAGE </div>'+div('line center',0,110,sn.toUpperCase().replace('_',' ')));setAmp(a,false);return}if(a.screen>=17&&a.screen<=28){setTop('', '<div class=\"cap\"> WARNING! </div>'+div('line',0,57,e(warn[a.displayCtx&15]||'')));document.getElementById('amp').outerHTML='<div id=\"amp\" class=\"warnctl\"><div style=\"width:790px;border-right:0\">[DISPLAY]<br>RESET ALARM</div></div>';return}setTop('',div('line',0,60,e(sn)));setAmp(a,false)}"));
        client.println(F("let busy=false,commandPending=false,fail=0;function setBusy(v){commandPending=v;document.querySelectorAll('button[data-k]').forEach(b=>b.disabled=v);if(v)s('screen','BUSY')}async function u(){if(busy)return;busy=true;try{let r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error(r.status);let j=await r.json(),a=j.amp,w=j.wifi;fail=0;s('screen',a.valid?a.screenName:'WAITING FOR AMP');render(a);s('wifi',w.ssid||w.status);s('ip',w.ip);s('rssi',w.rssi+' dBm');if(commandPending)setBusy(false)}catch(e){if(++fail>=4){s('screen','OFFLINE');setTop('',div('line sel',330,65,' OFFLINE '));setAmp({},false)}}finally{busy=false}}async function k(b){if(commandPending)return;b.classList.add('hit');setBusy(true);try{let r=await fetch('/api/key?name='+b.dataset.k,{cache:'no-store'});if(!r.ok)throw Error(r.status)}catch(e){setBusy(false);throw e}finally{setTimeout(()=>b.classList.remove('hit'),160);setTimeout(u,250)}}document.querySelectorAll('button[data-k]').forEach(b=>b.onclick=()=>k(b));u();setInterval(u,1500)</script></body></html>"));
    }

    WiFiServer server_{80};
    int last_wifi_status_ = WL_IDLE_STATUS;
    bool started_ = false;
};

static ControlServer control_server;

void control_server_service(void)
{
    control_server.service();
}
