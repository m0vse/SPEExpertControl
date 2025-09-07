#include "expertpackets.h"

#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"
#include "lvgl.h"
#include <ui.h>
#include "menuitems.h"

Arduino_H7_Video Display( 800, 480, GigaDisplayShield ); //( 800, 480, GigaDisplayShield );
Arduino_GigaDisplayTouch TouchDetector;

#define COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))

typedef struct {
    lv_obj_t **items;      
    int        count;
    int        selected;   
    bool       syncing;    
    lv_style_t st_checked; 
} menu_ctrl_t;

const char* outs[] = {"HALF","FULL"};

const char* tscales[] = {"°F","°C"};

const char* inputs[] = {"1","2"};

const char* ordinals[] = {"1st","2nd","3rd","4th"};

const char* antennas[] = {"1","2","3","4"};

const char* bands[] = {"160 m","80 m","40 m","30 m","20 m","17 m","15 m","12 m","10 m","6 m"};

const char* cats[] = {"SPE","ICOM","KENWD","YAESU","TTEC","FLEX","RS232","nNONE" };

const char* cat_icom[] = {"CI-V","VOLTAGE_BAND"};

const char* cat_yaesu[] = {"FT 100","FT 757 GX2","FT 817/847","FT 840/890","FT 897","FT 900","FT 920","FT 990","FT 1000","FT 1000 MP1",
                            "FT 1000 MP2","FT 1000 MP3","FT 2000","FT 9000D","BAND_DATA"};

const char* cat_tentec[] = {"OMNI VII","ORION I/II","JUPITER","ARGONAUT"};

const char* warnings[] = {"0x10: DEBUGGING (IGNORE)",        "POWER MANAGEMENT : V PA < 20 V", 
                          "POWER MANAGEMENT : V PA < 26 V",  "POWER MANAGEMENT : V PA > 50 V",
                          "POWER MANAGEMENT : V PA > 50 V",  "POWER MANAGEMENT : I PA > 40 A",  
                          "POWER MANAGEMENT : I PA > 50 A",  "OVER TEMPERATURE : TEMP > 90°C",
                          "P.A. MANAGEMENT : OVER DRIVING",  "0x19: DEBUGGING (IGNORE)",
                          "0x20: DEBUGGING (IGNORE)",        "P.A.MANAGEMENT : PW REV > 300W",  
                          "P.A. MANAGEMENT : PA PROTECTION" };
                          
const char* headings[] = { 
          " SETUP OPTIONS vs. IN %s ", 
          " SET ANTENNA vs. IN %s ",
          " SET CAT vs. IN %s ",
          " SET YAESU vs. IN %s ",
          " SET ICOM vs. IN %s ",
          " SET TEN-TEC vs. IN %s ",
          " SET BAUD RATE vs. IN %s ",
          "[←↑] [↓→]:SELECT          [SET]:CHANGE",
          "[←↑] [↓→]:SELECT         [SET]:CONFIRM"
};

const char* baud_rate[] = {"1200 Baud","2400 Baud","4800 Baud","9600 Baud"};

const char* setup_options[] = {"ANTENNA","CAT","MANUAL TUNE","BACKLIGHT","CONTEST","BEEP","START","TEMP","QUIT"};

const char* setup_messages[] = {
  "------- SET ANTENNAS vs. BANDS -------",
  "----- SET CAT INTERFACE FEATURES -----",
  "------- MANUAL TUNE OPERATIONS -------",
  "----- DISPLAY BACKLIGHT SETTINGS -----",
  "-------- CONTEST MODE On/Off ---------",
  "------ ACOUSTIC FEEDBACK On/Off ------",
  "------ SET STARTUP DEFAULT MODE ------",
  "-- TEMPERATURE VALUE SHOWN IN °C/°F --",
  "---------- LEAVE THIS MENU -----------"
};

const char* ant_messages[] = {
  "--- Set %s ANTENNA FOR 160 m BAND ---",
  "---- Set %s ANTENNA FOR 80 m BAND ---",
  "---- Set %s ANTENNA FOR 40 m BAND ---",
  "---- Set %s ANTENNA FOR 30 m BAND ---",
  "---- Set %s ANTENNA FOR 20 m BAND ---",
  "---- Set %s ANTENNA FOR 17 m BAND ---",
  "---- Set %s ANTENNA FOR 15 m BAND ---",
  "---- Set %s ANTENNA FOR 12 m BAND ---",
  "---- Set %s ANTENNA FOR 10 m BAND ---",
  "---- Set %s ANTENNA FOR 6 m BAND ----",
  "------- SAVE SETTINGS AND EXIT -------"
};

const char* ant_num[] = { " 1", " 2", " 3", " 4","NO" };

enum ExpertStatus {Sync,Len,Data,Sum};

enum ExpertCommands {Key_On=0x10,Rcu_On=0x80,Rcu_Off=0x81,Cat_232=0x82};

enum ExpertKeys {L_Minus_Key=0x30,L_Plus_Key=0x31,C_Minus_Key=0x32,C_Plus_Key=0x33,Tune_Key=0x34,In_Key=0x28,Band_Minus_Key=0x29,Band_Plus_Key=0x2A,
                Ant_Key=0x2B,Cat_Key=0x2C,Left_Key=0x2D,Right_Key=0x2E,Set_Key=0x2F,Off_Key=0x18,Power_Key=0x1A,Display_Key=0x1B,Operate_Key=0x1C};

enum ExpertScreen {Receive_Screen=0x00,Operate_RX,Operate_TX,Cat_Screen,Data_Stored,Unused_Screen_A,Setup_Options,Set_Antenna,Set_Cat,Set_Yaesu,Set_Icom,Set_TenTec,
                  Set_BaudRate,Manual_Tune,Backlight,Unused_Screen_B,Unused_Screen_C,
                  Warning_V_Low_Half,Warning_V_Low_Full,Warning_V_High_Half,Warning_V_High_Full,Warning_A_High_Half,Warning_A_High_Full,Warning_Temp,Warning_Over_Driving,
                  Unused_Screen_D,Unused_Screen_E,Warning_Reverse,Warning_Protection,
                  Alarm_History=0x1D,Shutdown=0x1E,BootMessage=0xff};

// Setup options menu items
//static lv_obj_t *setup_options_items[] {ui_setupAntenna,ui_setupCat,ui_setupManualTune,ui_setupBacklight,ui_setupContest,ui_setupBeep,ui_setupStart,ui_setupTemp,ui_setupQuit};
menu_ctrl_t setup_options_ctrl;
static lv_obj_t *setup_options_items[9];

menu_ctrl_t setup_ant_ctrl;
static lv_obj_t *setup_ant_items[21];

menu_ctrl_t setup_cat_ctrl;
static lv_obj_t *setup_cat_items[8];

menu_ctrl_t setup_yaesu_ctrl;
static lv_obj_t *setup_yaesu_items[15];

menu_ctrl_t setup_icom_ctrl;
static lv_obj_t *setup_icom_items[2];

menu_ctrl_t setup_tentec_ctrl;
static lv_obj_t *setup_tentec_items[4];

menu_ctrl_t setup_baudrate_ctrl;
static lv_obj_t *setup_baudrate_items[4];

ExpertStatus expert = Sync;
ExpertScreen screen = BootMessage;

// Amplifier settings
Expert_Packet packet_in;
static uint8_t* const inp = reinterpret_cast<uint8_t*>(&packet_in);
uint8_t bytes = 0x00;

Expert_Packet last_status;

uint8_t checksum = 0x00;
uint8_t len_in = 0x00;
unsigned long last_rcu=0;
const unsigned long interval = 1000;

void setup() {
  Serial.begin(9600); // Debug logging

  Display.begin();
  TouchDetector.begin();
  ui_init();

  setupMenus();

  Serial1.begin(9600); // Amp connection
  while(!Serial && millis()<4000) {
    //wait for serial to connect, for up to 4 seconds (4000 milliseconds)
  }

  Serial.println(F("Starting Expert1K controller"));

  // Setup options menu items (must be done after ui_init())
  setup_options_items[0] = ui_setupAntenna;
  setup_options_items[1] = ui_setupCat;
  setup_options_items[2] = ui_setupManualTune;
  setup_options_items[3] = ui_setupBacklight;
  setup_options_items[4] = ui_setupContest;
  setup_options_items[5] = ui_setupBeep;
  setup_options_items[6] = ui_setupStart;
  setup_options_items[7] = ui_setupTemp;
  setup_options_items[8] = ui_setupQuit;
  menu_ctrl_init(&setup_options_ctrl, setup_options_items, COUNT_OF(setup_options_items));

  setup_ant_items[0] = ui_setup160Ant1;
  setup_ant_items[1] = ui_setup160Ant2;
  setup_ant_items[2] = ui_setup80Ant1;
  setup_ant_items[3] = ui_setup80Ant2;
  setup_ant_items[4] = ui_setup40Ant1;
  setup_ant_items[5] = ui_setup40Ant2;
  setup_ant_items[6] = ui_setup30Ant1;
  setup_ant_items[7] = ui_setup30Ant2;
  setup_ant_items[8] = ui_setup20Ant1;
  setup_ant_items[9] = ui_setup20Ant2;
  setup_ant_items[10] = ui_setup17Ant1;
  setup_ant_items[11] = ui_setup17Ant2;
  setup_ant_items[12] = ui_setup15Ant1;
  setup_ant_items[13] = ui_setup15Ant2;
  setup_ant_items[14] = ui_setup12Ant1;
  setup_ant_items[15] = ui_setup12Ant2;
  setup_ant_items[16] = ui_setup10Ant1;
  setup_ant_items[17] = ui_setup10Ant2;
  setup_ant_items[18] = ui_setup6Ant1;
  setup_ant_items[19] = ui_setup6Ant2;
  setup_ant_items[20] = ui_setupAntSave;
  menu_ctrl_init(&setup_ant_ctrl, setup_ant_items, COUNT_OF(setup_ant_items));

  setup_cat_items[0] = ui_setupCatSpe;
  setup_cat_items[1] = ui_setupCatIcom;
  setup_cat_items[2] = ui_setupCatKenwood;
  setup_cat_items[3] = ui_setupCatYaesu;
  setup_cat_items[4] = ui_setupCatTenTec;
  setup_cat_items[5] = ui_setupCatFlexRadio;
  setup_cat_items[6] = ui_setupCatRs232;
  setup_cat_items[7] = ui_setupCatNone;
  menu_ctrl_init(&setup_cat_ctrl, setup_cat_items, COUNT_OF(setup_cat_items));

  setup_yaesu_items[0] = ui_setupYaesuFT100;
  setup_yaesu_items[1] = ui_setupYaesuFT757;
  setup_yaesu_items[2] = ui_setupYaesuFT817;
  setup_yaesu_items[3] = ui_setupYaesuFT840;
  setup_yaesu_items[4] = ui_setupYaesuFT897;
  setup_yaesu_items[5] = ui_setupYaesuFT900;
  setup_yaesu_items[6] = ui_setupYaesuFT920;
  setup_yaesu_items[7] = ui_setupYaesuFT990;
  setup_yaesu_items[8] = ui_setupYaesuFT1000;
  setup_yaesu_items[9] = ui_setupYaesuFT1000MP1;
  setup_yaesu_items[10] = ui_setupYaesuFT1000MP2;
  setup_yaesu_items[11] = ui_setupYaesuFT1000MP3;
  setup_yaesu_items[12] = ui_setupYaesuFT2000;
  setup_yaesu_items[13] = ui_setupYaesuFT9000;
  setup_yaesu_items[14] = ui_setupYaesuBandData;
  menu_ctrl_init(&setup_yaesu_ctrl, setup_yaesu_items, COUNT_OF(setup_yaesu_items));

  setup_icom_items[0] = ui_setupIcomCiv;
  setup_icom_items[1] = ui_setupIcomVoltage;
  menu_ctrl_init(&setup_icom_ctrl, setup_icom_items, COUNT_OF(setup_icom_items));

  setup_tentec_items[0] = ui_setupTenTecOmni;
  setup_tentec_items[1] = ui_setupTenTecOrion;
  setup_tentec_items[2] = ui_setupTenTecJupiter;
  setup_tentec_items[3] = ui_setupTenTecArgonaut;
  menu_ctrl_init(&setup_tentec_ctrl, setup_tentec_items, COUNT_OF(setup_tentec_items));

  setup_baudrate_items[0] = ui_setupBaud1200;
  setup_baudrate_items[1] = ui_setupBaud2400;
  setup_baudrate_items[2] = ui_setupBaud4800;
  setup_baudrate_items[3] = ui_setupBaud1200;
  menu_ctrl_init(&setup_baudrate_ctrl, setup_baudrate_items, COUNT_OF(setup_baudrate_items));



  last_rcu=millis();
}

void loop()
{
  // Check for serial data
  while (Serial1.available() > 0) {
    int in = Serial1.read();
    switch (expert) {
      case Sync:
        if ((in & 0xff) != 0xAA) {
          bytes = 0x00;
        } else if (bytes == 2) {
          expert = Len;
        } else {
          bytes++;
        }        
        break;
      case Len:
        if (in <= MAX_DATA) {
          len_in = (in & 0xFF);
          expert = Data;
        } else {
          expert = Sync;
        }
        bytes=0x00;
        break;
      case Data:
        inp[bytes++] = static_cast<uint8_t>(in);
        checksum += (in & 0xFF);
        if (bytes == len_in) {
          expert = Sum;
        }
        break;
      case Sum:
        if (checksum == (in & 0xff)) {
          process_packet();
        } else {
          Serial.println("Invalid checksum");
        }
        // Reset
        bytes = 0x00;
        checksum = 0x00;
        expert = Sync;
        len_in=0x00; // Shouldn't be needed...
        break;
    }
  }  
  lv_timer_handler();
// Send Rcu_On command if nothing received for over ten seconds.
  unsigned long now = millis();
  if (now - last_rcu >= interval)
  {
    //Serial1.end();      // close serial port
    //delay(100);        //wait 100 millis
    //Serial1.begin(9600);
    send_command({Rcu_On});
    last_rcu=now;
  }
}

void process_packet()
{
  // We have a valid packet!
  if (len_in == 1) {
    // This is a response packet
    //if (packet_in.status_code == 0x06)
    //Serial.println("ACK");
    if (packet_in.status_code == 0x20)
      Serial.println("NAK");
    else if (packet_in.status_code != 0x06)
      Serial.println("UNK");
  } else if (len_in == 30) {
    // Show status
    last_rcu=millis();
    // Only upate if packet has changed
    if (memcmp(&last_status,&packet_in,sizeof last_status))
    {
      // Select current screen
      // Receive_Screen=0x00,Operate_RX,Operate_TX,Cat_Screen,UnusedA,Data_Stored,Setup_Options,Set_Antenna,Set_Cat,Set_Yaesu,Set_Icom,Set_TenTec,
      // Set_BaudRate,Manual_Tune,Backlight,UnusedB,UnusedC,Alarm_History,Shutdown
      ExpertScreen scr = static_cast<ExpertScreen>(packet_in.display_ctx);
      
      // Open the mainScreen if currently displaying bootMessage
      if (screen == BootMessage) {
          lv_disp_load_scr(ui_mainScreen);
      }

      if (screen != scr) {
        // Screen has changed, so lets display it, hide everything first.
        // Need to tidy this, probably create an array of all screen objects?
        lv_obj_add_flag(ui_receive, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_manualTune, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_backlight, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_transmit, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_alarmHistory, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_alarmControl, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_warningScreen, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_warningControl, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_systemMessage, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupAntOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupCatOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupYaesuOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupIcomOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupTenTecOptions, LV_OBJ_FLAG_HIDDEN); 
        lv_obj_add_flag(ui_setupBaudRateOptions, LV_OBJ_FLAG_HIDDEN); 
        switch (scr) {
          case Receive_Screen:
            lv_obj_remove_flag(ui_receive, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            break;
          case Operate_RX:
          case Operate_TX:
            lv_obj_remove_flag(ui_transmit, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            break;
          case Alarm_History:
            lv_obj_remove_flag(ui_alarmHistory, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_alarmControl, LV_OBJ_FLAG_HIDDEN);
            last_status.setup[0]=0xff; // Make sure it updates!
            break;
          case Setup_Options:
            lv_obj_remove_flag(ui_setupOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupHeaderText,headings[0],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_Antenna:
            lv_obj_remove_flag(ui_setupAntOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupAntHeaderText,headings[1],inputs[packet_in.band_input & 0x01]);
            // Populate all labels
            for (int f=0;f<10;f++)
            {
              for (int g=0;g<2;g++)
              {
                lv_label_set_text(setup_ant_items[(f*2)+g],ant_num[(packet_in.setup[f+1] >> (g*4)) & 0x07]);
              }
            }
            break;
          case Set_Cat:
            lv_obj_remove_flag(ui_setupCatOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupCatHeaderText,headings[2],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_Yaesu:
            lv_obj_remove_flag(ui_setupYaesuOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupIcomHeaderText,headings[3],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_Icom:
            lv_obj_remove_flag(ui_setupIcomOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupIcomHeaderText,headings[4],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_TenTec:
            lv_obj_remove_flag(ui_setupTenTecOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupTenTecHeaderText,headings[5],inputs[packet_in.band_input & 0x01]);
            break;
          case Set_BaudRate:
            lv_obj_remove_flag(ui_setupBaudRateOptions, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_setupBaudRateHeaderText,headings[6],inputs[packet_in.band_input & 0x01]);
            break;
          case Manual_Tune:
            lv_obj_remove_flag(ui_manualTune, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_ampStatus, LV_OBJ_FLAG_HIDDEN);
            break;
          case Backlight:
            lv_obj_remove_flag(ui_backlight, LV_OBJ_FLAG_HIDDEN);
            break;
          case Shutdown:
            lv_obj_remove_flag(ui_systemMessage, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_systemMessageText,"SHUTDOWN");
            break;
          case Data_Stored:
            lv_obj_remove_flag(ui_systemMessage, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_systemMessageText,"DATA STORED");
            break;
          case Warning_V_Low_Half:
          case Warning_V_Low_Full:
          case Warning_V_High_Half:
          case Warning_V_High_Full:
          case Warning_A_High_Half:
          case Warning_A_High_Full:
          case Warning_Temp:
          case Warning_Over_Driving:
          case Warning_Reverse:
          case Warning_Protection:
            lv_obj_remove_flag(ui_warningScreen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui_warningControl, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(ui_warningText,"%s",warnings[packet_in.display_ctx & 0x0f]);
            break;
          default:
          break;
        }
        screen = scr;
        // If screen has changed, last_status is now invalid;
        memset(&last_status,0xff,sizeof last_status);
      }

      if (scr == Alarm_History && last_status.setup[0] != packet_in.setup[0]) 
      {
        // Refresh warnings as currently displayed has changed.
        uint8_t wrn_idx = (packet_in.setup[0]) >> 4 & 0x0f;
        uint8_t wrn_no = packet_in.setup[0] & 0x0f;
        if (wrn_no && (packet_in.setup[wrn_idx] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine1, "%*d)IN %s %s",2,wrn_idx,inputs[(packet_in.setup[wrn_idx] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx] & 0x0f]);
        if (wrn_no > 1 && (packet_in.setup[wrn_idx-1] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine2, "%*d)IN %s %s",2,wrn_idx-1,inputs[(packet_in.setup[wrn_idx-1] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx-1] & 0x0f]);
        if (wrn_no > 2 && (packet_in.setup[wrn_idx-2] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine3, "%*d)IN %s %s",2,wrn_idx-2,inputs[(packet_in.setup[wrn_idx-2] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx-2] & 0x0f]);
        if (wrn_no > 3 && (packet_in.setup[wrn_idx-3] & 0x0f) < 0x0D)
          lv_label_set_text_fmt(ui_alarmLine4, "%*d)IN %s %s",2,wrn_idx-3,inputs[(packet_in.setup[wrn_idx-3] >> 7) & 0x01],warnings[packet_in.setup[wrn_idx-3] & 0x0f]);
      } 

      if (memcmp(&last_status.setup,&packet_in.setup,sizeof last_status.setup))
      {
        // Something has changed!
        if (scr == Setup_Options) 
        {
          // Setup_Options Menu has changed.
          lv_label_set_text_fmt(ui_setupFooterText,"%s",setup_messages[packet_in.setup[1] & 0x0f]);        
          menu_apply_selection(&setup_options_ctrl, packet_in.setup[1] & 0x0f);
        }
        else if (scr == Set_Antenna) 
        {
          // Setup_Options Menu has changed.
          // The documentation appears to be incorrect. It suggests that the setup[0] defines the band 
          // and in each other setup bit 7 shows the current selected ant.
          // This does not appear to be the case, as setup[0] goes up to 20!
          uint8_t index = packet_in.setup[0];
          uint8_t idx = index / 2;
          uint8_t ord = index % 2;

          lv_label_set_text_fmt(ui_setupAntFooterText,ant_messages[idx],ordinals[ord]); 
          if (index < 20) // Don't change label of SAVE!     
          {
            lv_label_set_text(setup_ant_items[index],ant_num[(packet_in.setup[idx+1] >> (ord*4)) & 0x07]);
            if (setup_ant_ctrl.selected == 20) { // Was previously SAVE
              lv_label_set_text(ui_setupAntBottomLabel,headings[7]);
            }
          } else {
            lv_label_set_text(ui_setupAntBottomLabel,headings[8]);
          }
          menu_apply_selection(&setup_ant_ctrl, index);
        }
        else if (scr == Set_Cat) 
        {
          menu_apply_selection(&setup_cat_ctrl, packet_in.setup[1]);
        }
        else if (scr == Set_Yaesu) 
        {
          menu_apply_selection(&setup_yaesu_ctrl, packet_in.setup[1]);
        }
        else if (scr == Set_Icom) 
        {
          menu_apply_selection(&setup_icom_ctrl, packet_in.setup[1]);
        }
        else if (scr == Set_TenTec) 
        {
          menu_apply_selection(&setup_tentec_ctrl, packet_in.setup[1]);
        }
        else if (scr == Set_BaudRate) 
        {
          menu_apply_selection(&setup_baudrate_ctrl, packet_in.setup[1]);
        }
        else if (scr == Manual_Tune)
        {
          // Update ManualTune contents
          lv_label_set_text_fmt(ui_manualTuneFreq,"%*.3f MHz",6,float(packet_in.freq)/1000.0);
          lv_label_set_text_fmt(ui_manualTuneSubBand,"%*d",3,packet_in.sub_band);

          lv_label_set_text_fmt(ui_manualTuneuHLabel,"%*.1f uH",7,float(packet_in.setup[1])/10.0);
          lv_bar_set_value(ui_manualTuneuH, packet_in.setup[1], LV_ANIM_ON);

          uint16_t mask = (static_cast<uint16_t>(packet_in.setup[3]) << 8) | packet_in.setup[2];
          static const double weights[10] = { 3.6, 6.4, 12.1, 18.9, 40.8, 81.5, 158.0, 321.5, 641.6, 1250.0 };

          double pF = 0.0;
          for (int i = 0; i < 10; i++) {
              if (mask & (1u << i)) {
                  pF += weights[i];
              }
          }

          lv_label_set_text_fmt(ui_manualTunepFLabel,"%*.1f pF",7,pF);
          lv_bar_set_value(ui_manualTunepF, pF, LV_ANIM_ON);
        }
        else if (scr == Backlight)
        {
          // Update ManualTune contents
          lv_bar_set_value(ui_backlightLevel, packet_in.setup[1], LV_ANIM_ON);
        }
      }
      // No point updating any other ui elements unless they have actually changed since the last update.

      // Standby/Operate screen data
      if (last_status.band_input != packet_in.band_input) {
        lv_label_set_text_fmt(ui_inLabel,"IN\n%s",inputs[packet_in.band_input & 0x01]);      
        lv_label_set_text_fmt(ui_bandLabel,"BAND\n%s",bands[(packet_in.band_input >> 4) & 0x0f]);      
      }

      if (last_status.antenna_cat != packet_in.antenna_cat) {
        lv_label_set_text_fmt(ui_antLabel,"ANT\n%s",antennas[packet_in.antenna_cat & 0x07]); // only 3 bits
        lv_label_set_text_fmt(ui_catLabel,"CAT\n%s",cats[(packet_in.antenna_cat >> 4) & 0x07]); // only 3 bits     
      }

      if (last_status.flags != packet_in.flags) {
        lv_label_set_text_fmt(ui_outLabel,"OUT\n%s",outs[(packet_in.flags >> 4) & 0x01]);      
      }

      if (last_status.power != packet_in.power) {
        lv_label_set_text_fmt(ui_pep,"%*.1f W pep",6,float(packet_in.power)/10.0);      
      }

      if (last_status.voltage != packet_in.voltage) {
        lv_label_set_text_fmt(ui_vPA,"%*.1f v",4,float(packet_in.voltage)/10.0);      
        lv_bar_set_value(ui_vBar, packet_in.voltage/10, LV_ANIM_ON);
      }

      if (last_status.swr_gain != packet_in.swr_gain) {
        if (packet_in.swr_gain == 0)
          lv_label_set_text(ui_swrLabel,"SWR\n--.--");
        else
          lv_label_set_text_fmt(ui_swrLabel,"SWR\n%*.2f",5,float(packet_in.swr_gain)/100.0);
      }

      if (last_status.temp != packet_in.temp) {
        lv_label_set_text_fmt(ui_tempLabel,"TEMP\n%d%s",packet_in.temp,tscales[(packet_in.flags >> 7) & 0x01]);
      }

      Serial.print("SWR/Gain:");
      Serial.print(float(packet_in.swr_gain)/10);
      Serial.print(" Temp:");
      Serial.print(packet_in.temp);
      Serial.print(" Power:");
      Serial.print(float(packet_in.power)/10);
      Serial.print(" Reverse:");
      Serial.print(float(packet_in.rev_power)/10);
      Serial.print(" Voltage:");
      Serial.print(float(packet_in.voltage)/10);
      Serial.print(" Current:");
      Serial.println(float(packet_in.current)/10);    
      memcpy(&last_status,&packet_in,sizeof last_status);
    }
  }
}


bool menu_ctrl_init(menu_ctrl_t *mc, lv_obj_t **items, int count) {
    if (!mc || !items || count <= 0) return false;

    mc->items    = items;
    mc->count    = count;
    mc->selected = -1;
    mc->syncing  = false;

    lv_style_init(&mc->st_checked);
    lv_style_set_bg_opa(&mc->st_checked, LV_OPA_COVER);
    lv_style_set_bg_color(&mc->st_checked, lv_color_black());
    lv_style_set_text_color(&mc->st_checked, lv_color_white());

    for (int i = 0; i < count; ++i) {
        lv_obj_t *it = items[i];
        if (!it || !lv_obj_is_valid(it)) {
            // Fail fast: bad pointer in the array
            Serial.print("Invalid menu item, cannot continue!");
            return false;
        }

        lv_obj_add_style(it, &mc->st_checked, LV_PART_MAIN | LV_STATE_CHECKED);
        // lv_obj_add_event_cb(it, menu_item_event_cb, LV_EVENT_CLICKED, mc);
        // lv_obj_add_event_cb(it, menu_item_event_cb, LV_EVENT_FOCUSED, mc);
    }
    return true;
}

static void menu_apply_selection(menu_ctrl_t *mc, int new_index) {
    if (mc->syncing) return;          
    mc->syncing = true;

    if (new_index < -1) new_index = -1;
    if (new_index >= mc->count) new_index = mc->count - 1;

    if (new_index == mc->selected) {
        mc->syncing = false;
        return;
    }
 
    // Clear previous
    if (mc->selected >= 0 && mc->selected < mc->count) {
        lv_obj_clear_state(mc->items[mc->selected], LV_STATE_CHECKED);
    }
    mc->selected = new_index;

    // Set new
    if (mc->selected >= 0) {
        lv_obj_add_state(mc->items[mc->selected], LV_STATE_CHECKED);
    }
    mc->syncing = false;
}

void send_command(std::initializer_list<uint8_t> cmd) {
  Serial1.write(0x55); Serial1.write(0x55); Serial1.write(0x55);
  Serial1.write(cmd.size() & 0xff);
  uint8_t sum=0;
  for (uint8_t c : cmd) {
    Serial1.write(c);
    sum += c;
  }
  Serial1.write(sum);
}

/* Rather than have a separate function for every button, we check who the calling object it and act on it
L_Plus_Key=0x30,L_Minus_Key=0x31,C_Minus_Key=0x32,C_Plus_Key=0x33,Tune_Key=0x34,In_Key=0x28,Band_Minus_Key=0x29,Band_Plus_Key=0x2A,
Ant_Key=0x2B,Cat_Key=0x2C,Left_Key=0x2D,Right_Key=0x2E,Set_Key=0x2F,Off_Key=0x18,Power_Key=0x1A,Display_Key=0x1B,Operate_Key=0x1C
*/

void button_pressed(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target_obj(e);
  if(code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    if (obj == ui_buttonLowerL)
      send_command({Key_On,L_Minus_Key});
    else if (obj == ui_buttonHigherL)
      send_command({Key_On,L_Plus_Key});
    else if (obj == ui_buttonLowerC)
      send_command({Key_On,C_Minus_Key});
    else if (obj == ui_buttonHigherC)
      send_command({Key_On,C_Plus_Key});
    else if (obj == ui_buttonTune)
      send_command({Key_On,Tune_Key});
    else if (obj == ui_buttonInput)
      send_command({Key_On,In_Key});
    if (obj == ui_buttonBandDown)
      send_command({Key_On,Band_Minus_Key});
    else if (obj == ui_buttonBandUp)
      send_command({Key_On,Band_Plus_Key});
    if (obj == ui_buttonAnt)
      send_command({Key_On,Ant_Key});
    if (obj == ui_buttonLeftUp)
      send_command({Key_On,Left_Key});
    else if (obj == ui_buttonRightDown)
      send_command({Key_On,Right_Key});
    else if (obj == ui_buttonCat)
      send_command({Key_On,Cat_Key});
    else if (obj == ui_buttonSet)
      send_command({Key_On,Set_Key});
    else if (obj == ui_buttonOff)
      send_command({Key_On,Off_Key});
    else if (obj == ui_buttonOn) 
      Serial.println("On key pressed");
    else if (obj == ui_buttonPower)
      send_command({Key_On,Power_Key});
    else if (obj == ui_buttonDisplay)
      send_command({Key_On,Display_Key});
    else if (obj == ui_buttonOperate)
      send_command({Key_On,Operate_Key});
  }
}
