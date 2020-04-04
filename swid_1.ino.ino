//  ====================================================
//
//   *******************************************
//   *        projet SWID ( Switch ID )        *
//   *******************************************
//
//           version 0.2
//
//   date debut : 15 mars 2020 
//   MODIF      : 21,22, 28 ,30 MARS ET 1 AVRIL 2020
//
//   programmation: Francois Blais Montreal QC
// 
//   capture de trame pour la decouverte de commutateur (lldp et cdp)
//
//   materiels
//   ---------
//   arduino  uno r3  32k prog   2k ram 
//   shield ethernet HanRun hr911105a
//   ecran affichage avec un OLED 128 par 64
//  
 
//    
//  bibliotheques
//  -------------
//  chemin de bibli shield w5100 lecture en mode raw   C:\Users\fblai\OneDrive\Documents\Arduino\libraries\Ethernet\src\utility
//
//  C:\Users\fblai\OneDrive\Documents\Arduino\libraries
//  #include "SSD1306Ascii.h"
//  #include "SSD1306AsciiAvrI2c.h"
//
//


// conflit i2c  et spi   ou bibliotheque  ( adafruit 1306 ??? )   jai pris ssd1306ascii
// 
//  i2c
//  ===
//  5v et gnd
//  A4 , A5  ( 2 dernier)
//  ou au dessus analog et (ref)
//
//  spi
//  ---
//  Numerique D10 ,D12,D13,D14  et  gnd et 3.3v
//
//  ====================================================

// IEEE 802.3 LLC / SNAP
  // https://www.safaribooksonline.com/library/view/ethernet-the-definitive/9781449362980/images/edg2_0401.png
  // http://godleon.blogspot.com/2007/06/link-layer-ip-ip-datagrams-arp-arp.html
  // http://www.infocellar.com/networks/ethernet/frame.htm
  //
  //
  //
  // http://www.ieee802.org/1/files/public/docs2002/LLDP%20Overview.pdf
  // http://enterprise.huawei.com/ilink/enenterprise/download/HW_U_149738
  // http://support.huawei.com/enterprise/docinforeader.action?contentId=DOC0100523127&partNo=10032
  // http://www.cnblogs.com/rohens-hbg/articles/4765763.html
  // 
  // Ethernet II (DIX)
  // +-----------------------------------------------------------------------------------------------------------------------------------------+
  // | destination_address | source_address | Frame type     |                  Data                                                     | CRC |
  // #=========================================================================================================================================#
  // # destination_address | source_address |  type          | LLDPPDU Data                                                              | CRC # LLDP with Ethernet II
  // # 6 byte              | 6              | 2(0x88CC lldp) | ~                                                                         | 4   # 
  // #=========================================================================================================================================#
  // 
  //
  // IEEE 802.3 with LLC
  // +-----------------------------------------------------------------------------------------------------------------------------------------+
  // | IEEE 802.3 MAC Header                                 | 802.2 LLC Header               |                  Data                    | CRC |
  // #=========================================================================================================================================#
  // # destination_address | source_address | length         | DSAP     | SSAP     | Ctrl     |  Data                                    | CRC #
  // # 6 byte              | 6              | 2              | 1        | 1        | 1        |  ~                                       | 4   #
  // #=========================================================================================================================================#
  //
  //
  // IEEE 802.3 with LLC / SNAP
  // +-----------------------------------------------------------------------------------------------------------------------------------------+
  // | IEEE 802.3 MAC Header                                 | 802.2 LLC Header               | 802.2 SNAP     | Data                    | CRC |
  // #=========================================================================================================================================#
  // # destination_address | source_address | length         | DSAP     | SSAP     | Ctrl     | org_code | type| Data                    | CRC # CDP, EDP, lldp 
  // # 6 byte              | 6              | 2              | 1 (0xAA) | 1 (0xAA) | 1 (0x03) | 3        | 2   | ~                       | 4   #
  // #=========================================================================================================================================#
  //     org_code = 0x00000c (cisco)    , type = 0x2000 (2)  cdp
  //     org_code = 0x00e02b (extreme)  , type = 0x00bb (187) edp
  //     org_code = 0x000000 (lldp)     , type = 0x88cc (35020) lldp. LLDP rarely used SNAP, Cisco and Extreme normally used with Ethernet II.
  //

  //
  //  port description   valeur tvl (4) 1 octets  et avec  grandeur 1 octets  donc 2 octets
  //  system name        valeur tvl (5)  et avec  grandeur
  //  vlan identifier    valeur tvl (127) grandeur 6 ==>>  3 octet ieee 00:80:ce , 1 octet subtype 0x01  donc   2 derniers octets
  //     335 bytes  
  //  destination broadcast lldp   01:80:c2:00:00:0e        0x88cc ?
  
 //  string   https://www.arduino.cc/reference/tr/language/variables/data-types/stringobject/
 //  conversion     String thisString = String(13, HEX);     String thisString = String(13, BIN);
 //  myString.indexOf(val, from)      myString.substring(from, to)


  
//
//    1- la bonne bibli mon shield est un w5100 Ethernet version 1.1.2 maximum sinon pas fonctions #include <utility/w5100.h> mettre include ethernet sinon marche pas  
//    2- les bibli pour modules  28j60        ethercard et ethershield  ----pas compatible  dans document/arduino
//    3- ecran oled 0.96  #include "SSD1306Ascii.h"  plus léger au adafruit
//    4-  calcul ordi  en binaire qui represente de lheaxdecimal de a base 16 0-9 et a-f
//    5- les include et define ne marche pas dans une fonction
//    6-  la carte a un buffer et doit etre rempli en un certain temp minimum 60 sec ou   60 000 miliseconde 
//    7- certaine erreur de compilation disparaisse si compile a nouveau
//     exemple  https://github.com/net-flow/Arduino-LLDP-Viewer/blob/master/lldp.ino
//  ======================================== 

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

String LCD_data[10];

//  ====================================================
//  affichage avec un OLED 128 par 64
//  ====================================================
void presentation(String L1,String L2,String L3,String L4,String L5,String L6,String L7,String L8){
     oled.clear();
     oled.print("1- ");  oled.println(L1);  
     oled.print("2- ");  oled.println(L2);  
     oled.print("3- ");  oled.println(L3); 
     oled.print("4- ");  oled.println(L4); 
     oled.print("5- ");  oled.println(L5);  
     oled.print("6- ");  oled.println(L6); 
     oled.print("7- ");  oled.println(L7); 
     oled.print("8- ");  oled.println(L8); 


     Serial.println(" function: presentation   ");
     Serial.println("===========================");
     Serial.println(L1);        Serial.println(L2);          Serial.println(L3);    Serial.println(L4);        Serial.println(L5);     Serial.println(L6); Serial.println(L7); Serial.println(L8);
     Serial.println("===========================");

     LCD_data[0] = ""; LCD_data[1] = "";  LCD_data[2] = "";  LCD_data[3] = ""; LCD_data[4] = "";  LCD_data[6] = ""; LCD_data[7] = ""; LCD_data[8] = "";
 } 

//  ======================================== 

//  335 octets
//  fin de la trame ethernet qui englobe  08-00-00
//  debut section lldp  \0x01\0x80\0xc2\0x00\0x00\0x0e
//  pout trouver localisation et ip switch   R104- EQUVALENT HEX 52-31-30-34-2d
//  vlan - preceder par hex 00-80-c2-01    2 prochains octets
//  port - unit  equivalent hex 55-6e-69-74
//  2 octets precedent 1-type  2-longueur

byte trame[] = {0x08,0x0d,0x55,0x6e,0x69,0x74,0x20,0x32,0x20,0x50,0x6f,0x72};

byte trame1[] = {
"0x01,0x80,0xc2,0x00,0x00,0x0e,0x10,0xcd,0xae,0xe7,0x18,0x00,0x88,0xcc,0x02,0x07" \
",0x04,0x10,0xcd,0xae,0xe7,0x18,0x01,0x04,0x07,0x03,0x10,0xcd,0xae,0xe7,0x18,0x49" \
",0x06,0x02,0x00,0x78,0x08,0x0d,0x55,0x6e,0x69,0x74,0x20,0x32,0x20,0x50,0x6f,0x72" \
",0x74,0x20,0x38,0x0a,0x17,0x52,0x31,0x30,0x34,0x2d,0x43,0x41,0x52,0x43,0x33,0x31" \
",0x35,0x36,0x2d,0x31,0x31,0x35,0x2e,0x31,0x2e,0x32,0x30,0x30,0x0c,0x4c,0x45,0x74" \
",0x68,0x65,0x72,0x6e,0x65,0x74,0x20,0x52,0x6f,0x75,0x74,0x69,0x6e,0x67,0x20,0x53" \
",0x77,0x69,0x74,0x63,0x68,0x20,0x34,0x38,0x35,0x30,0x47,0x54,0x53,0x2d,0x50,0x57" \
",0x52,0x2b,0x20,0x20,0x48,0x57,0x3a,0x31,0x34,0x20,0x20,0x20,0x20,0x20,0x20,0x20" \
",0x46,0x57,0x3a,0x35,0x2e,0x38,0x2e,0x30,0x2e,0x31,0x20,0x20,0x20,0x53,0x57,0x3a" \
",0x76,0x35,0x2e,0x38,0x2e,0x33,0x2e,0x30,0x31,0x37,0x0e,0x04,0x00,0x14,0x00,0x04" \
",0x10,0x15,0x05,0x01,0x0a,0x73,0x01,0xc8,0x01,0x00,0x00,0x00,0x00,0x09,0x2b,0x06" \
",0x01,0x04,0x01,0x2d,0x03,0x4e,0x02,0xfe,0x06,0x00,0x80,0xc2,0x01,0x0b,0xfc,0xfe" \
",0x0c,0x00,0x12,0x0f,0x02,0x07,0x01,0x01,0x13,0x00,0x00,0x00,0x00,0xfe,0x07,0x00" \
",0x12,0xbb,0x01,0x00,0x2f,0x04,0xfe,0x07,0x00,0x12,0xbb,0x04,0x13,0x01,0x40,0xfe" \
",0x0c,0x00,0x12,0xbb,0x05,0x31,0x34,0x20,0x20,0x20,0x20,0x20,0x20,0xfe,0x0d,0x00" \
",0x12,0xbb,0x06,0x35,0x2e,0x38,0x2e,0x30,0x2e,0x31,0x20,0x20,0xfe,0x0e,0x00,0x12" \
",0xbb,0x07,0x76,0x35,0x2e,0x38,0x2e,0x33,0x2e,0x30,0x31,0x37,0xfe,0x10,0x00,0x12" \
",0xbb,0x08,0x31,0x35,0x4a,0x50,0x31,0x39,0x35,0x48,0x37,0x33,0x57,0x44,0xfe,0x09" \
",0x00,0x12,0xbb,0x09,0x41,0x76,0x61,0x79,0x61,0xfe,0x10,0x00,0x12,0xbb,0x0a,0x34" \
",0x38,0x35,0x30,0x47,0x54,0x53,0x2d,0x50,0x57,0x52,0x2b,0xfe,0x10,0x00,0x40,0x0d" \
",0x08,0x36,0xa5,0x10,0xcd,0xae,0xe7,0x18,0x00,0x00,0x00,0x02,0x08,0x00,0x00"
  };

#include <SPI.h>         
#include <Ethernet.h>
#include <utility/w5100.h>

SOCKET s; // the socket that will be openend in RAW mode
byte rbuf[500+14]; //receive buffer (was 1500+14, but costs way too much SRAM)
int rbuflen; // length of data to receive
bool silldp(const byte a[], unsigned int offset, const byte b[], unsigned int length);
  
byte lldp_mac[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e};  // dst mac for lldp traffic, we look for this

byte mac[] = {  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
int NBR = 0;
  
// =============== fonctionne au debut du script seulement ============================

void setup()
{

  Serial.begin(9600);
 
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  oled.setFont(Adafruit5x7);
  
  
  W5100.init();
  W5100.writeSnMR(s, SnMR::MACRAW);
  W5100.execCmdSn(s, Sock_OPEN);
  
  
  presentation( "SWID v 3.0 " ,    " "  ,"Capteur  LLDP"," " ," " ," "," ","F.Blais MTL 2020" );
  delay(10000); 

}

// ==================== en boucle continue ============================== 


void loop()
{

 NBR++;   

 rbuflen = W5100.getRXReceivedSize(s);
    if(rbuflen>0) {
    if(rbuflen > sizeof(rbuf))
    rbuflen = sizeof(rbuf);
    W5100.recv_data_processing(s, rbuf, rbuflen);
    W5100.execCmdSn(s, Sock_RECV);
 }

  W5100.execCmdSn(s, Sock_CLOSE);

  
 // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
   Serial.println("Failed to configure Ethernet using DHCP");
    }
  
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
     
 
  // presentation( "-Data-", "B:" + String(NBR) + "   G:" + String(rbuflen),"" ,  "ID:" + Ethernet.localIP()  ," " , "Port:" + LCD_data[3] ," ",  "Vlan:" + LCD_data[6]   );
  delay(30000);
  
}

 //  ===============================================================================

void test(){
  int local=0;
  // equivalent Unit
  byte port[]={0x55,0x6e,0x69,0x74}; 
local = trouve( trame, 0, port);
LCD_data[6] = decoupe( trame, local );
Serial.println("local:" + String(local)+ " data:" + LCD_data[6]);
Serial.println( String(trame[2],HEX) );
Serial.println( String(trame[3],HEX) );
Serial.println( String(port[0],HEX) );
Serial.println( String(port[1],HEX) );
}


void lldp(){

   int local=0;
 
  // if ( rbuflen > 0 ) {
   
   // if (silldp(trame, 0, lldp_mac, sizeof(lldp_mac))) {    
      Serial.println("LLPD trouvé");

 // equivalent de R104
  byte description[]={0x52,0x31,0x30,0x34};
 local = trouve(trame, 32, description );
LCD_data[1] = decoupe(trame, local );
Serial.println("local:" + String(local)+ " data:" + LCD_data[1]);

 // equivalent 00-80-c2-01
  byte vlan[]={0x00,0x80,0xc2,0x01}; 
local = trouve( trame, 32, vlan);
LCD_data[3] = decoupe( trame, local );
Serial.println("local:" + String(local)+ " data:" + LCD_data[3]);

 // equivalent Unit
  byte port[]={0x55,0x6e,0x69,0x74}; 
local = trouve( trame, 32, port);
LCD_data[6] = decoupe( trame, local );
Serial.println("local:" + String(local)+ " data:" + LCD_data[6]);
  //  }
 // }
}
 // =======================================================================================

 //
 // VERIFI SIL Y A UNE PARTIE DE LA TRAME QUI CORRESPOND AU ID LLDP
 //

bool silldp(const byte a[], unsigned int offset, const byte b[], unsigned int length) {
  for (unsigned int i = offset, j = 0; j < length; ++i, ++j) {
    if (a[i] != b[j]) {
      return false;
    }
  }
  return true;
}
//  ========================================================

//
//  RECHERCHE UN MORCEAU SPECIFIQUE DANS LA TRAME
//

 int trouve(const byte trame[], unsigned int offset, const byte rech[] ) {
  
  for (unsigned int i = offset ; i < 335  ; ++i ) {
    if (trame[i] == rech[0])
      if( (trame[i+1] == rech[1]) and (trame[i+2] == rech[2]) and (trame[i+3] == rech[3]) )
      return i;
   else
      return 0; 

      Serial.println("trouve" + String(i));
      
  }  
}
//  ========================================================

//
//  EXTRAIT UN MORCEAU EN HEXA ET LE RENVOI EN STRING
//

 String decoupe(const byte trame[], unsigned int offset  ) {

   Serial.println("decoupe");
     
  int grandeur = trame[offset - 5] ;
  String texte=" ";
  
  for (unsigned int i = offset   ; i <= grandeur ; ++i  ) 
     texte += String(trame[i],HEX);  
       
  return texte;  
}
 // ================= fin script =====================================================================================================  
