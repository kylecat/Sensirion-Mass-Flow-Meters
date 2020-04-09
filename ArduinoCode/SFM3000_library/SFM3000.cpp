#include "Wire.h"

#include "SFM3000.h"

/* Define*/
#define CRC_POLYNOMIAL 0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001


#define RESET				0x2000
#define START				0x1000 // Start continuous measurement

#define SCALE				0x30DE // Flow scale factor
#define OFFSET				0x30DF // Flow offset

#define ARTICLE_MSB	  0x31E3 // Product type/article number fast two bytes (total 4 bytes)
#define ARTICLE_LSB		0x31E4 // Product type/article number last two bytes (total 4 bytes)
#define SERIAL_MSB		0x31AE // Product serial number last two bytes (total 4 bytes)
#define SERIAL_LSB		0x31AF // Product serial number last two bytes (total 4 bytes)

/*  Public Funciton*/


/**/

SFM3000::SFM3000() {
}

SFM3000::SFM3000(uint8_t _addr){
	_I2CAddr = _addr;

}

SFM3000::~SFM3000(){
}

void SFM3000::init(uint8_t _details){
	Wire.begin();
	Reset();
	getArticleNumber();
	getSerailNumber();
	DebugDetails = _details;
}

uint8_t SFM3000::Start()
{
	_result = false;

	_CMD(START);
	_getRequest(_Data);

	_result = _CRC8(_Data, CRC);
	
	if (_result) {
		if(DebugDetails) showRawData(_Data);
		_Debuger("STRAT continuous measurement\r\n", H_SFM, EOL);
	}

}

void SFM3000::Reset(){
	_CMD(RESET);
	_Debuger("STOP flow measurement\r\n",H_SFM,EOL);
}

void SFM3000::setDebuger(Stream& refSer)
{
	_Debug = true;
	_refSerial = refSer;
	_Debuger("Debuge: ON",H_SFM,EOL);
}

uint16_t SFM3000::getSlop()
{
	_refSerial.println();

	_CMD(SCALE);
	_getRequest(_Data);
	_result = _CRC8(_Data, CRC);

	if (_result) {
		Slop = getValue();
		_Debuger("New Slop: " + String(Slop)+"\r\n" , H_SFM, EOL);
	}

	return Slop;
}

uint16_t SFM3000::getOffset()
{
	_refSerial.println();

	_CMD(OFFSET);
	_getRequest(_Data);
	_result = _CRC8(_Data, CRC);

	if (_result) {
		Offset = getValue();
		_Debuger("New Slop: " + String(Offset) + "\r\n", H_SFM, EOL);
	}

	return Slop;
}

String SFM3000::getArticleNumber()
{
	_CMD(ARTICLE_MSB);
	_getRequest(_Data);
	_result = _CRC8(_Data, CRC);
	if (_result) {
		ArticleNumber = getHEXStr(_Data[0]);
		ArticleNumber += getHEXStr(_Data[1]);
	}
	_CMD(ARTICLE_LSB);
	_getRequest(_Data);
	_result = _CRC8(_Data, CRC);
	if (_result) {
		ArticleNumber = getHEXStr(_Data[0]);
		ArticleNumber += getHEXStr(_Data[1]);
	}

	_Debuger("Article Number: "+ArticleNumber, H_SFM,EOL);

	return  ArticleNumber;
}

String SFM3000::getSerailNumber()
{
	_CMD(SERIAL_MSB);
	_getRequest(_Data);
	_result = _CRC8(_Data, CRC);
	if (_result) {
		SerialNumber = getHEXStr(_Data[0]);
		SerialNumber += getHEXStr(_Data[1]);
	}

	_CMD(SERIAL_LSB);
	_getRequest(_Data);
	_result = _CRC8(_Data, CRC);
	if (_result) {
		SerialNumber = getHEXStr(_Data[0]);
		SerialNumber += getHEXStr(_Data[1]);
	}
	
	_Debuger("Serial Number: " + SerialNumber, H_SFM, EOL);

	return  SerialNumber;
}


void SFM3000::showRawData(uint8_t * _array) {
	_Debuger("RAW: "+ getHEXStr(_array[0]) +" "+ getHEXStr(_array[1]),H_CMD, EOL);
}

uint16_t SFM3000::getValue()
{
	return _Data[0] << 8 | _Data[1];
}

float SFM3000::getFlowRate()
{
	_getRequest(_Data);

	_result = _CRC8(_Data, CRC);
	if (_result) {
		FlowRate = (float)getValue();

		FlowRate = (FlowRate - Offset) / (float)Slop;
	}	

	return FlowRate;
}

/* Private Funciton*/
void SFM3000::_Debuger(String _msg, UART _header, UART _uart){
	if (_Debug) {
		switch (_header)
		{
		case H_SFM:
			_refSerial.print(F("[ SFM ]\t"));
			break;
		case H_CMD:
			_refSerial.print(F("[ CMD ]\t"));
			break;
		default:
			break;
		}

		switch (_uart)
		{
		case NONE:
			_refSerial.print(_msg);
			break;
		case EOL:
			_refSerial.println(_msg);
			break;
		default:
			break;
		}
	}
}

String SFM3000::getHEXStr(uint8_t _d)
{
	if (_d < 16) _strBuffer = "0" + String(_d, HEX);
	else             _strBuffer = String(_d, HEX);
	_strBuffer.toUpperCase();

	return _strBuffer;
}

uint8_t SFM3000::_CRC8(uint8_t* _data, uint8_t _checksum){
	uint8_t _bit;					// bit mask
	uint8_t _byteCtr;			// byte counter
	uint8_t _calcCrc = 0;		// calculated checksum
	_result = false;

	for (uint8_t _byteCtr = 0; _byteCtr < 2; _byteCtr++)
	{
		_calcCrc ^= (_data[_byteCtr]);

		for (_bit = 8; _bit > 0; --_bit)
		{
			if (_calcCrc & 0x80)  _calcCrc = (_calcCrc << 1) ^ CRC_POLYNOMIAL;
			else					      _calcCrc = (_calcCrc << 1);
		}
	}
	
	if (_calcCrc != _checksum) {
		_Debuger("CRC FAIL: C(" + String(_calcCrc) + ") - O(" + String(_checksum) + ")", H_CMD, EOL);
	}
	else {
		//_Debuger("CRC CHECKED", H_CMD, EOL);
		_result = true;
	}

	return _result;
}

void SFM3000::_CMD(uint16_t _cmd){
	_setCMD(_cmd, _Buffer);
	delay(10);
	_sendCMD(_Buffer);
}

void SFM3000::_setCMD(uint16_t _cmd, uint8_t* _buffer){
	_buffer[0] = _cmd >> 8;
	_buffer[1] = _cmd | _buffer[0] << 8;
	_Debuger("SET CMD: 0x"+ getHEXStr(_buffer[0])+ getHEXStr(_buffer[1]), H_CMD, EOL);
}

void SFM3000::_sendCMD(uint8_t* _buffer){
	Wire.beginTransmission(_I2CAddr);
	Wire.write(_buffer[0]);
	Wire.write(_buffer[1]);
	Wire.endTransmission();
}

void SFM3000::_getRequest(uint8_t* _data){
	Wire.requestFrom(_I2CAddr,3);
	_data[0] = Wire.read(); // first received byte stored here
	_data[1] = Wire.read(); // second received byte stored here
	CRC = Wire.read(); // third received byte stored here
	Wire.endTransmission();
}
