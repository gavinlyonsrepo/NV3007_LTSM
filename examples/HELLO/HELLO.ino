/*!
	@file    HELLO.ino
	@author  Gavin Lyons
	@brief   Example ino file for nv3007 driver. Test Hello World
	@details Setup for 142x428 display. 0 degrees rotation.
			By default set for 1.65 inch display, if user has
			2.79 inch display select it in NV3007_LTSM.hpp 
			panel selection section.
	@note    See USER OPTIONS in setup function
	@test
		-# Test 101 Print out Hello world
*/

// libraries
#include "NV3007_LTSM.hpp"
// Fonts needed
#include "fonts_LTSM/FontMega_LTSM.hpp"

///@cond

// Section :: Defines
//  Test timing related defines
#define TEST_DELAY1 1000 // mS
#define TEST_DELAY2 2000 // mS
#define TEST_DELAY5 5000 // mS
#ifdef dislib16_ADVANCED_SCREEN_BUFFER_ENABLE
#pragma message("gll: dislib16_ADVANCED_SCREEN_BUFFER_ENABLE is defined. This example is not for that mode")
#endif

// Init display class object
NV3007_LTSM myTFT;
bool bhardwareSPI = true; // true for Hardware SPI , false for software SPI

void setup(void)
{
	Serial.begin(38400);
	delay(1000);
	Serial.println("Start Hello");
	//*** SETUP OPTION 0 SPI GPIO + TYPE ***
	int8_t DC_TFT = 5;
	int8_t RST_TFT = 4;
	int8_t CS_TFT = 15;
	if (bhardwareSPI == true)
	{									  // hw spi
		uint32_t TFT_SCLK_FREQ = 8000000; // Spi freq in Hertz
		myTFT.TFTsetupGPIO_SPI(TFT_SCLK_FREQ, RST_TFT, DC_TFT, CS_TFT);
	}
	else
	{								 // sw spi
		uint16_t SWSPICommDelay = 0; // optional SW SPI GPIO delay in uS
		int8_t SDIN_TFT = 26;
		int8_t SCLK_TFT = 27;
		myTFT.TFTsetupGPIO_SPI(SWSPICommDelay, RST_TFT, DC_TFT, CS_TFT, SCLK_TFT, SDIN_TFT);
	}
	// ***
	// *** SETUP OPTION 1 Screen Setup ***
	// Size of Display (note 142x428 display has VRAM of 168 x 428)
	uint16_t TFT_WIDTH = 142;	 // width  size in pixels
	uint16_t TFT_HEIGHT = 428; // height size in pixels
	myTFT.TFTInitScreen(TFT_WIDTH, TFT_HEIGHT, NV3007_LTSM::MADCTL_FLAGS_t::RGB);
	// *** SETUP OPTION 2 Screen offsets in portrait***
	uint8_t X_Offset = 12;
	uint8_t Y_Offset = 0;
	myTFT.TFTsetPanelOffset(X_Offset, Y_Offset);
	// ***
	myTFT.TFTNV3007Initialize();
	myTFT.TFTsetRotation(myTFT.Degrees_0);
}

void loop(void)
{
	Test101();
	EndTests();
	while (1){};
}
// *** End Main loop **

//   Function Space
void Test101(void)
{
	myTFT.setFont(FontMega);
	myTFT.fillScreen(myTFT.C_RED);
	myTFT.setTextColor(myTFT.C_GREEN, myTFT.C_BLACK);
	myTFT.setCursor(5, 5);
	myTFT.print("Hello!");
	MILLISEC_DELAY(TEST_DELAY5);
	myTFT.fillScreen(myTFT.C_BLUE);
	MILLISEC_DELAY(TEST_DELAY1);
}

void EndTests(void)
{
	myTFT.TFTPowerDown();
	Serial.println("TFT: End");
}
///@endcond
// === EOF ===
