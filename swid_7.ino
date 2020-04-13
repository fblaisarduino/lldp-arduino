//  ====================================================
//
//   *******************************************
//   *        projet SWID ( Switch ID )        *
//   *******************************************
//
//           version 7.1
//
//   date debut : 15 mars 2020 
//   MODIF      : 21,22, 28 ,30 MARS ET 1 , 2 , 3 , 13 AVRIL 2020
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
//  bibliotheques
//  -------------
//  chemin de bibli shield w5100 lecture en mode raw   C:\Users\fblai\OneDrive\Documents\Arduino\libraries\Ethernet\src\utility
//
//  C:\Users\fblai\OneDrive\Documents\Arduino\libraries
//  ===============  problem  ================================ 
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
//===============================================
// outil de debugage arduino
//===============================================
// exemple: ("pos:" ,String( ),"" + String(  ,DEC ),"" + String(  ,HEX ),"" + (char) abc );
// debug("" ,"","" ,"");
// void debug(String POS,String L2,String L3,String L4);
// if ( (L2 == "") and or (L3 == "") and or (L4 == "")  )
//    Serial.println("");
//
//    switch (L2) {
//    case "":
//     Serial.println(""); 
//    break;
//    case "":
//      Serial.println("");
//    break;
//    default:
//      Serial.println("");
//    break;
//   }
int nbrdbg;
void debug(String POS,String L2,String L3,String L4){
  nbrdbg++;
     Serial.println("========== Debug boucle:" + String(nbrdbg) + "=================");
     Serial.println("pos:" + POS);   Serial.println("L2:" + L2);   Serial.println("L3:" + L3);  Serial.println("L4:" + L4);  
     Serial.println("=========== fin ================"); 
 } 
//=============================================================================
  //  port description   valeur tvl (4) 1 octets  et avec  grandeur 1 octets  donc 2 octets
  //  system name        valeur tvl (5)  et avec  grandeur
  //  vlan identifier    valeur tvl (127) grandeur 6 ==>>  3 octet ieee 00:80:ce , 1 octet subtype 0x01  donc   2 derniers octets
  //     335 bytes  
  //  destination broadcast lldp   01:80:c2:00:00:0e        0x88cc ?
  
 //  string   https://www.arduino.cc/reference/tr/language/variables/data-types/stringobject/
 // operateur  https://www.arduino.cc/en/Tutorial/StringAppendOperator
 //  conversion     String thisString = String(13, HEX);     String thisString = String(13, BIN);
 //  myString.indexOf(val, from)      myString.substring(from, to)
   
//    ====================== apprentissage ===============================  
//    1- la bonne bibli mon shield est un w5100 Ethernet version 1.1.2 maximum sinon pas fonctions #include <utility/w5100.h> mettre include ethernet sinon marche pas  
//    2- les bibli pour modules  28j60        ethercard et ethershield  ----pas compatible  dans document/arduino
//    3- ecran oled 0.96  #include "SSD1306Ascii.h"  plus léger au adafruit
//    4-  calcul ordi  en binaire qui represente de lheaxdecimal de a base 16 0-9 et a-f
//    5- les include et define ne marche pas dans une fonction
//    6-  la carte a un buffer et doit etre rempli en un certain temp minimum 60 sec ou   60 000 miliseconde 
//    7- certaine erreur de compilation disparaisse si compile a nouveau
//    8- exemple  https://github.com/net-flow/Arduino-LLDP-Viewer/blob/master/lldp.ino
//    9- inString += (char)inChar;   convertir int en char et ajoute a un string
//    10- inString.toInt()    char a int  
//    11- char a[] = {0x052,0x052};  valeur en hexa vers tableau de car >> converti hexa en ascii donne  RR
//    12- pour acceder place le a dans serial.println(a)
//    13-  int grandeur = trame[local-1] ;  converti hex a int
//    14- ethernet attente de 30 seconde puis passe et reboot le system ( donc une tentative )
//    15- socket init dans setup
//    16- String lcd_date  definit global , lcd_date="" initialiser localement ==== Gele le deroulement ?
//    17- attend creation de la class    while (!Serial) 
//    18- global  seulement des declaration/assignation mais pas assignation seul
//    19- Serial.println (x2i ("0bfc"), DEC);  et  String( ,HEX)
//

// donc id  et grandeur du contenu ( utiliser wireshark pour avoir position ) VALEUR NON STATIC
// (1)  byte 14-15  chassis  grandeur 7 + 2 (id et grandeur en HEXA ) si grandeur 7
// (2)  byte subtype mac 23-24   si grandeur 7
// (3)  time to live  32-33      si grandeur 2
// (4)  description  36-37       si grandeur 13
// (5)  system name  51-52       si grandeur 23
//
//  avant info ethernet , offset  14 - 23 - 32 -36 -51

//  ====================================================
//     affichage avec un OLED 128 par 64
//  ====================================================
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

String LCD_data[7];

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

    // Serial.println(" function: presentation   ");
    // Serial.println("===========================");
    // Serial.println(L1);        Serial.println(L2);          Serial.println(L3);    Serial.println(L4);        Serial.println(L5);     Serial.println(L6); Serial.println(L7); Serial.println(L8);
    // Serial.println("===========================");
 } 

//  ================ Ethernet  ======================== 
//  335 octets
//  fin de la trame ethernet qui englobe  08-00-00
//  debut section lldp  \0x01\0x80\0xc2\0x00\0x00\0x0e
//  pout trouver localisation et ip switch   R104- EQUVALENT HEX 52-31-30-34-2d
//  vlan - preceder par hex 00-80-c2-01    2 prochains octets
//  port - unit  equivalent hex 55-6e-69-74
//  2 octets precedent 1-type  2-longueur
// byte rech[]={0x52,0x31,0x30,0x34}; ==pas de retour de ligne ==virgule == pas text""  == hexa vers octet
// char myArray[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; traduit  hex en ascii
// ethernet.begin    reboot arduino si pas de connection apres 30 seconde

#include <SPI.h>         
#include <Ethernet.h>
#include <utility/w5100.h>

SOCKET s; // the socket that will be openend in RAW mode
 
int rbuflen;  
byte mac[] = {  0xFB, 0xAA, 0xBB, 0xFB, 0xDE, 0xFB };
int NBR = 0;
int position[8];

bool silldp(const byte a[], unsigned int offset, const byte b[], unsigned int length);
String decoupe( unsigned int local  );  
int trouve( const byte rech[] );
String decoupevlan( unsigned int local  );
void trouvetlv();
String decompte;

// ==== pour faire des tests =======
 // byte trame[335+14] = {0x01,0x80,0xc2,0x00,0x00,0x0e,0x10,0xcd,0xae,0xe7,0x18,0x00,0x88,0xcc,0x02,0x07,0x04,0x10,0xcd,0xae,0xe7,0x18,0x01,0x04,0x07,0x03,0x10,0xcd,0xae,0xe7,0x18,0x49,0x06,0x02,0x00,0x78,0x08,0x0d,0x55,0x6e,0x69,0x74,0x20,0x32,0x20,0x50,0x6f,0x72,0x74,0x20,0x38,0x0a,0x17,0x52,0x31,0x30,0x34,0x2d,0x43,0x41,0x52,0x43,0x33,0x31,0x35,0x36,0x2d,0x31,0x31,0x35,0x2e,0x31,0x2e,0x32,0x30,0x30,0x0c,0x4c,0x45,0x74,0x68,0x65,0x72,0x6e,0x65,0x74,0x20,0x52,0x6f,0x75,0x74,0x69,0x6e,0x67,0x20,0x53,0x77,0x69,0x74,0x63,0x68,0x20,0x34,0x38,0x35,0x30,0x47,0x54,0x53,0x2d,0x50,0x57,0x52,0x2b,0x20,0x20,0x48,0x57,0x3a,0x31,0x34,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x46,0x57,0x3a,0x35,0x2e,0x38,0x2e,0x30,0x2e,0x31,0x20,0x20,0x20,0x53,0x57,0x3a,0x76,0x35,0x2e,0x38,0x2e,0x33,0x2e,0x30,0x31,0x37,0x0e,0x04,0x00,0x14,0x00,0x04,0x10,0x15,0x05,0x01,0x0a,0x73,0x01,0xc8,0x01,0x00,0x00,0x00,0x00,0x09,0x2b,0x06,0x01,0x04,0x01,0x2d,0x03,0x4e,0x02,0xfe,0x06,0x00,0x80,0xc2,0x01,0x0b,0xfc,0xfe,0x0c,0x00,0x12,0x0f,0x02,0x07,0x01,0x01,0x13,0x00,0x00,0x00,0x00,0xfe,0x07,0x00,0x12,0xbb,0x01,0x00,0x2f,0x04,0xfe,0x07,0x00,0x12,0xbb,0x04,0x13,0x01,0x40,0xfe,0x0c,0x00,0x12,0xbb,0x05,0x31,0x34,0x20,0x20,0x20,0x20,0x20,0x20,0xfe,0x0d,0x00,0x12,0xbb,0x06,0x35,0x2e,0x38,0x2e,0x30,0x2e,0x31,0x20,0x20,0xfe,0x0e,0x00,0x12,0xbb,0x07,0x76,0x35,0x2e,0x38,0x2e,0x33,0x2e,0x30,0x31,0x37,0xfe,0x10,0x00,0x12,0xbb,0x08,0x31,0x35,0x4a,0x50,0x31,0x39,0x35,0x48,0x37,0x33,0x57,0x44,0xfe,0x09,0x00,0x12,0xbb,0x09,0x41,0x76,0x61,0x79,0x61,0xfe,0x10,0x00,0x12,0xbb,0x0a,0x34,0x38,0x35,0x30,0x47,0x54,0x53,0x2d,0x50,0x57,0x52,0x2b,0xfe,0x10,0x00,0x40,0x0d,0x08,0x36,0xa5,0x10,0xcd,0xae,0xe7,0x18,0x00,0x00,0x00,0x02,0x08,0x00,0x00};
  byte trame[335+14];
 
// ========================================================================================================
// =============== fonctionne au debut du script seulement one shot ============================================
// ========================================================================================================
// debug("" ,"","" ,"");
void setup()
{
  Serial.begin(9600);
 
 // ============ initialise lecran oled ===================
 #if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  oled.setFont(Adafruit5x7);
   
   presentation( "SWID v7.1 " ,    " "  ,"Capteur  LLDP"," " ," " ," "," ","F.Blais MTL 2020" );
  delay(5000); 
  
 // ============ initialise ethernet ===================  
 
//       if (Ethernet.begin(mac) == 0) {
//     // reboot arduino si pas de connection apres 30 seconde
//     presentation( " Pas de DHCP " ,    " "  ," "," " ," " ," "," ","F.Blais MTL 2020" );
//     }
//     else
//     {
//      //  ====================== trouve info de connexion ============================ 
//  // debug("" ,"","" ,"");
//     LCD_data[0] = "-Data-", "B:" + String(NBR) + "   G:" + String(rbuflen);
//     LCD_data[3] = "IP" +  String(Ethernet.localIP());
//     LCD_data[5] = "DNS:" + String(Ethernet.dnsServerIP());
//     LCD_data[7] = "GW:" + String(Ethernet.gatewayIP());
//  presentation( LCD_data[0], LCD_data[1],LCD_data[2] ,LCD_data[3] ,LCD_data[4] , LCD_data[5] ,LCD_data[6], "F.Blais MTL 2020");
//  delay(60000); 
//      }

  W5100.init();
  W5100.writeSnMR(s, SnMR::MACRAW);
  W5100.execCmdSn(s, Sock_OPEN);
}
// ===================================================================================================== 
// ==================== en boucle continue ============================================================== 
// ===================================================================================================== 
void loop()
{
 NBR++;   
 byte lldp_mac[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e};  // dst mac for lldp traffic, we look for this
 
// ===================== info sur la switch  =======================
//  debug("" ,"","" ,"");


 rbuflen = W5100.getRXReceivedSize(s);
    if(rbuflen>0) {
    if(rbuflen > sizeof(trame))
    rbuflen = sizeof(trame);
    W5100.recv_data_processing(s, trame, rbuflen);
    W5100.execCmdSn(s, Sock_RECV);
 }

//  =========== test avec les donnees  ===========
// rbuflen = 10;
// =========== test ===========

 decompte = "b:" + String(NBR) + "   g:" +  String(rbuflen) ;

if ( rbuflen > 0 ) {
  
  if (silldp(trame, 0, lldp_mac, sizeof(lldp_mac))) {
     lldp();
     presentation( decompte , LCD_data[1],LCD_data[2] ,LCD_data[3] ,LCD_data[4] , LCD_data[5] ,LCD_data[6], LCD_data[7]);
  }
  else
  {
    LCD_data[2] = "pas une trame lldp";
    presentation( decompte , LCD_data[1],LCD_data[2] ,LCD_data[3] ,LCD_data[4] , LCD_data[5] ,LCD_data[6], "F.Blais MTL 2020");
  }
  
  delay(15000);
}
else
{
   LCD_data[2] = "pas de trame";
   presentation( decompte , LCD_data[1],LCD_data[2] ,LCD_data[3] ,LCD_data[4] , LCD_data[5] ,LCD_data[6], "F.Blais MTL 2020");
   delay(15000);
}
 
}

//  ===============================================================================================
//  ==================================  mes fonctions    ====================================
//  ===============================================================================================

void lldp(){
// debug("" ,"","" ,"");
 
  
   int local=0;
   String partie;
    
     
     trouvetlv();     

 //   description 
  partie = decoupe(   position[5] ) ;
  LCD_data[2] = "";
  LCD_data[2] =  partie.substring(6,14);
  LCD_data[3] = "";
  LCD_data[3] += "10.";
  LCD_data[3] +=  partie.substring(15,24);

 // equivalent 00-80-c2-01 signature de la ligne vlan
  byte vlan[]={0x00,0x80,0xc2,0x01}; 
  local = trouve(vlan);
  partie = decoupevlan(local) ;
  LCD_data[6] = "";
  LCD_data[6] = "Vlan:"  + partie ;

 // equivalent Unit/port
  partie = decoupe(position[4] ) ;
  LCD_data[5] = "";
  LCD_data[5] += "U" ;
  LCD_data[5] +=  partie.substring(5,15);
 
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
//  EXTRAIT UN MORCEAU EN HEXA ET LE RENVOI EN STRING
//
// debug("" ,"","" ,"");

 String decoupe( unsigned int local  ) {
 unsigned int i;

  int grandeur = trame[local+1] ;                                                           
  
  String texte=" "; 
  for ( i = ( local + 2 )   ; i < (local + grandeur + 2)  ; i++  ) 
        texte +=   (char)trame[i] ;
   
  return texte;  
}
//  ========================================================
//
//  EXTRAIT UN MORCEAU EN HEXA ET LE RENVOI EN STRING
//
// debug("" ,"","" ,"");
 String decoupevlan( unsigned int local  ) {
unsigned int i;
 int grandeur = trame[local-1] ;
  grandeur = grandeur - 4 ;

  int res;
  String texte ; 
  for ( i = (local+4)   ; i< ( (local+4) + ( grandeur) )   ; i++  ) 
    texte += String(trame[i],HEX);

  texte = x2i(texte);
    
  return texte;  
}
//  ========================================================
//
//  RECHERCHE UN MORCEAU SPECIFIQUE DANS LA TRAME
//
// =======================================================
 int trouve(const byte rech[] ) {
  unsigned int i = 0;
  for ( i = 0 ; i < 335  ; ++i ) {
    if (trame[i] == rech[0])
      if( (trame[i+1] == rech[1]) and (trame[i+2] == rech[2]) and (trame[i+3] == rech[3]) )
      return i;
       }         
  }  

  // =================================================================
  // trouve les deplacements 
  // 
  
 void trouvetlv(){
       int nbr=0;
      int pos = 14;       position[0]=0;   position[1]=14;
   
  for ( nbr = 2 ; nbr < 9  ; nbr++ ) {
    position[nbr] =  ( (pos + 2) + trame[pos+1]   );
    pos  = position[nbr];
   }
    return position ;
 }
  
// ======================================================================
   String print_ip(const byte a[], unsigned int offset, unsigned int length) {
  String ip;
  for (unsigned int i = offset; i < offset + length; ++i) {
    //    if(i>offset) Serial.print('.');
    //   Serial.print(a[i], DEC);
    if (i > offset) ip = ip + '.';
    ip = ip + String (a[i]);
  }
  int iplentgh;
  return ip;
}
// ======================================================================
String print_mac(const byte a[], unsigned int offset, unsigned int length) {
  String Mac;
  char temp [40];
  LCD_data[1] = "";
  for (unsigned int i = offset; i < offset + length; ++i) {

    if (i > offset) {
      //  LCD_data[1] = LCD_data[1] + Mac + ':';
      Mac = Mac + ':';
    }
    if (a[i] < 0x10) {
      Mac = Mac + '0';
      //    LCD_data[1] = LCD_data[1] + Mac + '0';
    }
    Mac = Mac + String (a[i], HEX);
  }
  LCD_data[1] = LCD_data[1]  + Mac;
  return Mac;
}
//  ====================================================
//
//  converti hexa vers dec en type string
//
//  ====================================================
String x2i(String monhexa)
{
  int x = 0; int nbr;
  for(nbr=0 ; nbr < monhexa.length() ; nbr++) {
    
    if (monhexa[nbr] >= '0' && monhexa[nbr] <= '9') {
      x *= 16;
      x += monhexa[nbr] - '0';
    }
    else if (monhexa[nbr] >= 'A' && monhexa[nbr] <= 'F') {
      x *= 16;
      x += (monhexa[nbr] - 'A') + 10;
    }
    else if (monhexa[nbr] >= 'a' && monhexa[nbr] <= 'f') {
      x *= 16;
      x += (monhexa[nbr] - 'a') + 10;
    }
    else break;
  }
  return String(x,DEC);
}
 // ================= fin script =====================================================================================================  
 // ================= fin script =====================================================================================================  
 // ================= fin script =====================================================================================================  
