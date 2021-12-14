/*
 * https://github.com/m5stack/M5-ProductExampleCodes/blob/master/Hat/RoverC/Arduino/RoverC/RoverC.ino
 */
 
#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "M5AP";
const char *password = "77777777";

int degree = 50; //65536/2;
bool isGRASPING = false;

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
WiFiServer server(80);

WiFiUDP Udp1;

void SetChargingCurrent( uint8_t CurrentLevel )
{
  Wire1.beginTransmission(0x34);
  Wire1.write(0x33);
  Wire1.write(0xC0 | ( CurrentLevel & 0x0f));
  Wire1.endTransmission();
}

void setup()
{
    M5.begin();
    M5.update();
  Wire.begin(0,26,10000);

  uint64_t chipid=ESP.getEfuseMac();
  String str = ssid + String((uint32_t)(chipid>>32),HEX);
  M5.Lcd.setRotation(1);
  M5.Lcd.setSwapBytes(false);
  Disbuff.createSprite(160, 80);
  Disbuff.setSwapBytes(true);
  Disbuff.fillRect(0, 0, 160, 20, Disbuff.color565(50, 50, 50));
  Disbuff.setTextSize(2);
  Disbuff.setTextColor(WHITE);
  Disbuff.setCursor(15, 35);
  Disbuff.print(str);
  Disbuff.pushSprite(0, 0);

  SetChargingCurrent(6);

  Serial.begin(115200);
  //Set device in STA mode to begin with
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
            IPAddress(192, 168, 4, 1),
            IPAddress(255, 255, 255, 0));
  
  WiFi.softAP(str.c_str(), password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Udp1.begin(1003);
}

uint8_t SendBuff[9] = {0xAA, 0x55,
             0x00,
             0x00,
             0x00,
             0x00,
             0x00,
             0x00,
             0xee};

uint8_t I2CWrite1Byte( uint8_t Addr ,  uint8_t Data )
{
  Wire.beginTransmission(0x38);
  Wire.write(Addr);
  Wire.write(Data);
  return Wire.endTransmission();
}

uint8_t I2CWritebuff( uint8_t Addr,  uint8_t* Data, uint16_t Length )
{
  Wire.beginTransmission(0x38);
  Wire.write(Addr);
  for (int i = 0; i < Length; i++)
  {
    Wire.write(Data[i]);
  }
  return Wire.endTransmission();
}


int16_t speed_buff[4] = {0};
int8_t  speed_sendbuff[4] = {0};
uint32_t count = 0;
uint8_t IIC_ReState = I2C_ERROR_NO_BEGIN;

uint8_t Setspeed( int16_t Vtx, int16_t Vty, int16_t Wt)
{
  Wt = ( Wt > 100 )  ? 100 :  Wt;
  Wt = ( Wt < -100 ) ? -100 : Wt;

  Vtx = ( Vtx > 100 )  ? 100 :  Vtx;
  Vtx = ( Vtx < -100 ) ? -100 : Vtx;
  Vty = ( Vty > 100 )  ? 100 :  Vty;
  Vty = ( Vty < -100 ) ? -100 : Vty;

  Vtx = ( Wt != 0 ) ? Vtx *  (100- abs(Wt)) / 100 : Vtx;
  Vty = ( Wt != 0 ) ? Vty *  (100- abs(Wt)) / 100 : Vty;

  speed_buff[0] = Vty - Vtx - Wt ;
  speed_buff[1] = Vty + Vtx + Wt ;
  speed_buff[3] = Vty - Vtx + Wt ;
  speed_buff[2] = Vty + Vtx - Wt ;

  for (int i = 0; i < 4; i++)
  {
    speed_buff[i] = ( speed_buff[i] > 100 )  ? 100 :  speed_buff[i];
    speed_buff[i] = ( speed_buff[i] < -100 ) ? -100 : speed_buff[i];
    speed_sendbuff[i] = speed_buff[i];
  }
  return I2CWritebuff(0x00,(uint8_t*)speed_sendbuff,4);
}

void loop()
{
  int udplength = Udp1.parsePacket();
  if( udplength )
  {
    char udodata[udplength];
    Udp1.read( udodata, udplength);
    IPAddress udp_client = Udp1.remoteIP();
    if(( udodata[0] == 0xAA )&&( udodata[1] == 0x55 )&&( udodata[7] == 0xee ))
    {
      for (int i = 0; i < 8; i++)
      {
        Serial.printf("%02X ",udodata[i]);
      }
      Serial.println();
      //if( udodata[6] == 0x01 )
      if( udodata[6] == 0x65 )
      {
        if( isGRASPING )
        {
          isGRASPING = false;
          // 26を解放
          ledcDetachPin(26);
          delay(100);
          // Wireを再初期化
          Wire.begin(0, 26, 10000);
          delay(100);
        }
        IIC_ReState = Setspeed( udodata[3] - 100  ,udodata[4] - 100 , udodata[5] -100 );
      }
      else
      {
        IIC_ReState = Setspeed( 0  ,0 , 0 );
        
        if( udodata[6] != 0x64 )
        //if( udodata[6] < 20 || udodata[6] > 180 )
        { 
          // WireをM5StickCの無使用ピン36に一時的に割り当て
          // https://qiita.com/coppercele/items/2c4e6c84a7a96e348dc1#%E3%82%B0%E3%83%AA%E3%83%83%E3%83%91%E3%82%92%E5%8B%95%E3%81%8B%E3%81%99
          // 分解能別最大周波数
          // https://lang-ship.com/blog/work/esp32-pwm-max/
          // Catch Unit (SG92Rサーボ搭載 グリッパーユニット)
          // https://docs.m5stack.com/en/unit/catch?id=example
          if( !isGRASPING )
          {
            isGRASPING = true;
            Wire.begin(36, 36, 10000);
            // 26をPWM出力に割り当て
            //ledcSetup(0, 50, 16); // (Ch. ID, PWM freq. [Hz], PWM res. [bit] (16 bit: 0~65535))
            ledcSetup(0, 50, 10); // (Ch. ID, PWM freq. [Hz], PWM res. [bit] (10 bit: 0~1024), 0~90°:25~75)
            ledcAttachPin(26, 0);
          }
          
          // PWM level
          // http://blog.robotakao.jp/blog-entry-379.html
          if ( ( udodata[6] <  20 ) && degree > 25 + 2 ) degree = degree - 2;
          if ( ( udodata[6] > 180 ) && degree < 75 - 2 ) degree = degree + 2;
          
          // サーボを任意角度に設定
          ledcWrite(0, degree);
          //delay(200);
          //isGRASPING = false;
          //// 26を解放
          //ledcDetachPin(26);
          //delay(100);
          //// Wireを再初期化
          //Wire.begin(0, 26, 10000);
          //delay(100);
        }
      }
    }
    else
    {
      IIC_ReState = Setspeed( 0  ,0 , 0 );
    }
    
  }
  count ++; 
  if( count > 100 )
  {
    count = 0;

    Disbuff.fillRect(0, 0, 160, 20, Disbuff.color565(50, 50, 50));
    Disbuff.setTextSize(1);
    Disbuff.setTextColor(WHITE);
    Disbuff.setCursor(5, 5);
    Disbuff.printf("%.2fV,%.2fmA,%d",M5.Axp.GetBatVoltage(),M5.Axp.GetBatCurrent(),IIC_ReState);
    Disbuff.setCursor(130, 5);
    Disbuff.printf("%ddig",(degree-25)*90/50.0);
    Disbuff.pushSprite(0, 0);
  }  
}
