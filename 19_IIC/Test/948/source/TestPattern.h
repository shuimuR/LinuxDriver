/*******************************************************************
����948��ʾtest pattern
*******************************************************************/
#ifndef TestPattern_H_
#define TestPattern_H_

#define GroupDataLength 2
#define GroupWaitTime 20

#define DelayBeforeBackLight 10

#define Reg_Reset								0x01
#define Reg_Configuration0						0x02
#define Reg_Configuration1						0x03
#define Reg_I2C_Control1						0x05
#define Reg_I2C_Control2						0x06
#define Reg_General_Status						0x1C
#define Reg_GPIO_REG0							0x1D
#define Reg_GPIO_REG2_REG1						0x1E
#define Reg_GPIO_REG3							0x1F
#define Reg_GPO_REG6_REG5						0x20
#define Reg_GPO_REG8_REG7						0x21
#define Reg_BIST_Control						0x24
#define Reg_BIST_Error							0x25
#define Reg_SCL_High_Time						0x26
#define Reg_SCL_Low_Time						0x27
#define Reg_FRC_Control							0x29
#define Reg_SSCG_Control						0x2C
#define Reg_Link_Error_Count 					0x41
#define Reg_Pattern_Generator_Control 			0x64
#define Reg_Pattern_Generator_Configuration 	0x65

#define Reg_Pattern_Generator_Indirect_Address 	0x66
#define Reg_Pattern_Generator_Indirect_Data 	0x67

#define Reg_Frequency_Counter 					0x1B
#define Reg_GPIO_REG6_REG5						0x20
#define Reg_GPIO_REG8_REG7						0x21
#define Reg_PG_Internal_Clock_Enable			0x39
#define Reg_Map_Select 							0x49

#define Reg_Datapath_Control					0x22
#define Reg_Rx_Mode_Status						0x23
#define Reg_Dual_Rx_Ctl							0x34
#define Reg_Mode_Sel							0x37
#define Reg_Lvds_Control						0x4B

#define Reg_PattGen_IndirectAddr_PGRS 			0x00
#define Reg_PattGen_IndirectAddr_PGGS 			0x01
#define Reg_PattGen_IndirectAddr_PGBS 			0x02
#define Reg_PattGen_IndirectAddr_PGCDC 			0x03
#define Reg_PattGen_IndirectAddr_PGTFS1 		0x04
#define Reg_PattGen_IndirectAddr_PGTFS2			0x05
#define Reg_PattGen_IndirectAddr_PGTFS3 		0x06
#define Reg_PattGen_IndirectAddr_PGAFS1			0x07
#define Reg_PattGen_IndirectAddr_PGAFS2			0x08
#define Reg_PattGen_IndirectAddr_PGAFS3			0x09
#define Reg_PattGen_IndirectAddr_PGHSW 			0x0A
#define Reg_PattGen_IndirectAddr_PGVSW 			0x0B
#define Reg_PattGen_IndirectAddr_PGHBP			0x0C
#define Reg_PattGen_IndirectAddr_PGVBP			0x0D
#define Reg_PattGen_IndirectAddr_PGSC			0x0E
#define Reg_PattGen_IndirectAddr_PGFT			0x0F
#define Reg_PattGen_IndirectAddr_PGTSC			0x10
#define Reg_PattGen_IndirectAddr_PGTSO1 		0x11
#define Reg_PattGen_IndirectAddr_PGTSO2			0x12
#define Reg_PattGen_IndirectAddr_PGTSO3 		0x13
#define Reg_PattGen_IndirectAddr_PGTSO4			0x14
#define Reg_PattGen_IndirectAddr_PGTSO5			0x15
#define Reg_PattGen_IndirectAddr_PGTSO6 		0x16
#define Reg_PattGen_IndirectAddr_PGTSO7			0x17

#define Des_Master_400K						4
#define Des_Master_300K						3
#define Des_Master_200K						2
#define Des_Master_100K						1
#define Des_MasterSpeed_SclHiLoTime			(100/Des_Master_400K - 1)

#define PANEL_RESOLUTION_H 			1920
#define PANEL_RESOLUTION_V 			720
#define PANEL_Total_Horizontal		1980
#define PANEL_Total_Vertical		750
#define PANEL_Horizontal_Sync		8
#define PANEL_Horizontal_BackP		16
#define PANEL_Vertical_Sync			5
#define PANEL_Vertical_BackP		22
#define PANEL_18_24_bit_Mode		24

#define Des_Horizontal_Sync_Negative	1
#define Des_Vertical_Sync_Negative		1

#define Des_ClockDivider 		3	// (200/PANEL_CLK)
	
#define Des_Total_HL 			PANEL_Total_Horizontal&0xFF
#define Des_Total_VL_HH 		((PANEL_Total_Vertical&0x0F)<<4)|((PANEL_Total_Horizontal&0x0F00)>>8)
#define Des_Total_VH			(PANEL_Total_Vertical&0x0FF0)>>4
#define Des_Active_HL			PANEL_RESOLUTION_H&0xFF
#define Des_Active_VL_HH 		((PANEL_RESOLUTION_V&0x0F)<<4)|((PANEL_RESOLUTION_H&0x0F00)>>8)
#define Des_Active_VH			(PANEL_RESOLUTION_V&0x0FF0)>>4
#define Des_Horizontal_Sync		PANEL_Horizontal_Sync
#define Des_Vertical_Sync		PANEL_Vertical_Sync
#define Des_Horizontal_BackP 	PANEL_Horizontal_BackP
#define Des_Vertical_BackP 		PANEL_Vertical_BackP
#define Des_Generator_Sync_Conf	 ((0x01<<Des_Vertical_Sync_Negative)&0x02)|Des_Horizontal_Sync_Negative

#define ICP_RESOLUTION_H 	PANEL_RESOLUTION_H
#define ICP_RESOLUTION_V 	PANEL_RESOLUTION_V

#define DeviceID_Addr   0x00

#define uint8 unsigned char

/*******************************************************************
Test Pattern init
*******************************************************************/
void TestPatternInit();

void Open_DS90UB948();
void DS90UB948_ReadID();

#endif
