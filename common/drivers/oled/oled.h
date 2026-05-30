#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_SetCursor(uint8_t Y, uint8_t X);
void OLED_WriteData(uint8_t Data);
void OLED_ShowHeart(uint8_t Line, uint8_t Column, uint8_t isSolid);
void OLED_ClearHeart(uint8_t Line, uint8_t Column);
void OLED_ShowFloatNum(uint8_t Line, uint8_t Column, float Number, uint8_t IntegerLength, uint8_t DecimalLength);

#endif
