#ifndef _SFM3000_H_
#define _SFM3000_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class SFM3000
{
public:
  SFM3000();
  SFM3000(uint8_t _addr);
  ~SFM3000();

  void     init(uint8_t _details = false);      // 初始化I2C並取得儀器資料
  uint8_t Start();                              // 開始連續分析
  void      Reset();                            // 停止並重新設定系統

  void       setDebuger(Stream& refSer);        // 設定debug用的輸出
  uint16_t getSlop(); 
  uint16_t getOffset();

  String getArticleNumber();
  String getSerailNumber();

  void       showRawData(uint8_t* _array);
  uint16_t getValue();
  float       getFlowRate();


  uint16_t Offset = 32000;
  uint16_t Slop = 140;
  float FlowRate;
  String ArticleNumber;
  String SerialNumber;

  uint8_t DebugDetails = false;
  
private:

  enum UART
  {
    NONE,
    EOL,
    H_SFM,
    H_CMD
  };

  uint8_t _I2CAddr = 0x40;

  uint8_t _Debug = false;
  Stream& _refSerial = Serial;
  String _DebugString;
  void _Debuger(String _msg, UART _header, UART _uart = NONE);

  uint8_t _Buffer[2];  // [MSB, LSB]
  uint8_t _Data[2];   // [MSB, LSB]
  uint8_t _result;

  String getHEXStr(uint8_t _d);
  String _strBuffer;

  uint8_t _CRC8(uint8_t* _data, uint8_t _checksum);
  uint8_t CRC = 0;

  void _CMD(uint16_t _cmd);
  void _setCMD(uint16_t _cmd, uint8_t* _buffer);
  void _sendCMD(uint8_t* _buffer);
  void _getRequest(uint8_t *_data);
};

#endif // !SFM3000_H_
