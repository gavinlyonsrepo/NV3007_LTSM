/*!
	@file   NV3007_LTSM.cpp
	@author Gavin Lyons
	@brief  Source file. Contains driver methods for the NV3007 display driver
*/

#include "NV3007_LTSM.hpp"

/*! @brief Constructor for class NV3007_LTSM */
NV3007_LTSM :: NV3007_LTSM(){}

/*! @brief Init Hardware SPI */
void NV3007_LTSM::TFTHWSPIInitialize(void)
{
	SPI.begin();
}

/*!
	@brief Call when powering down TFT
	@note  Will switch off SPI
*/
void NV3007_LTSM::TFTPowerDown(void)
{
	TFTenableDisplay(false);
	if (_resetPinOn == true) {
		DISPLAY16_RST_SetLow;
	}
	DISPLAY16_DC_SetLow;
	DISPLAY16_CS_SetLow;

	if (_hardwareSPI == true) {
		SPI.end();
	}else{
		DISPLAY16_SCLK_SetLow;
		DISPLAY16_SDATA_SetLow;
	}
}

/*! @brief Method for Hardware Reset pin control */
void NV3007_LTSM ::TFTResetPIN() {
	if (_resetPinOn == false) return;
	DISPLAY16_RST_SetDigitalOutput;
	DISPLAY16_RST_SetHigh;
	MILLISEC_DELAY(5);
	DISPLAY16_RST_SetLow;
	MILLISEC_DELAY(20);
	DISPLAY16_RST_SetHigh;
	MILLISEC_DELAY(150);
}

/*!
	@brief sets up TFT GPIO for software SPI
	@param CommDelay SW SPI GPIO delay
	@param rst reset GPIO, optional pass -1 to disable
	@param dc data or command GPIO.
	@param cs chip select GPIO
	@param sclk Data clock GPIO
	@param din Data to TFT GPIO
	@details Overloaded, one of two. This one is for software SPI.
*/
void NV3007_LTSM::TFTsetupGPIO_SPI(uint16_t CommDelay, int8_t rst, int8_t dc, int8_t cs, int8_t sclk, int8_t din)
{
	TFTSwSpiGpioDelaySet(CommDelay);
	_hardwareSPI = false;
	_display_RST= rst;
	_display_DC = dc;
	_display_CS = cs;
	_display_SDATA = din;
	_display_SCLK = sclk;
	_resetPinOn = (_display_RST != -1); // true when a RST pin is enabled
}

/*!
	@brief sets up TFT GPIO for hardware SPI
	@param speed_hz SPI baudrate in hz
	@param rst reset GPIO, optional pass -1 to disable
	@param dc data or command GPIO.
	@param cs chip select GPIO
	@details Overloaded, one of two. This one is for hardware SPI.
*/
void NV3007_LTSM::TFTsetupGPIO_SPI(uint32_t speed_hz, int8_t rst, int8_t dc, int8_t cs)
{
	_speedSPIHz = speed_hz;
	_hardwareSPI = true;
	_display_RST= rst;
	_display_DC = dc;
	_display_CS = cs;
	_resetPinOn = (_display_RST != -1); // true when a RST pin is enabled
}

/*!
	@brief init routine for NV3007 controller
	@details Call after TFTInitScreen and TFTsetPanelOffset.
*/
void NV3007_LTSM::TFTNV3007Initialize()
{
	if (_resetPinOn == true)
	{
		TFTResetPIN();
	}
	DISPLAY16_DC_SetDigitalOutput;
	DISPLAY16_DC_SetLow;
	DISPLAY16_CS_SetDigitalOutput;
	DISPLAY16_CS_SetHigh;
	if (_hardwareSPI == false)
	{
		DISPLAY16_SCLK_SetDigitalOutput;
		DISPLAY16_SDATA_SetDigitalOutput;
		DISPLAY16_SCLK_SetLow;
		DISPLAY16_SDATA_SetLow;
	}else{
		TFTHWSPIInitialize();
	}
	TFTcheckGeometry();
	TFTInitSequence();
}

/*!
	@brief Toggle the invert mode, inverted colours
	@param invert true = invert off , false = invert on
*/
void NV3007_LTSM ::TFTchangeInvertMode(bool invert) {
	if(invert) {
		writeCommand(NV3007_INVOFF);
	} else {
		writeCommand(NV3007_INVON);
	}
}

/*!
	@brief change rotation of display.
	@param mode display_rotate_e enum
		Degrees_0, Degrees_90, Degrees_180, Degrees_270
	@note Uses MX/MY/MV only, the refresh-direction bits (ML, MH) are left clear.
		Datasheet 6.2.20. Memory access control(36h)
*/
void NV3007_LTSM::TFTsetRotation(display_rotate_e mode) {
	uint8_t madctl = _colorOrder; // RGB or BGR colour order bit
	switch (mode) {
		case Degrees_0 :
			_width =_widthStartTFT;
			_height = _heightStartTFT;
			_XStart = _colOffset;
			_YStart = _rowOffset;
			break;
		case Degrees_90:
			madctl |= (MADCTL_FLAGS_t::MX | MADCTL_FLAGS_t::MV);
			_width  = _heightStartTFT;
			_height = _widthStartTFT;
			_XStart = _rowOffset;
			_YStart = _colOffset;
			break;
		case Degrees_180:
			madctl |= (MADCTL_FLAGS_t::MY | MADCTL_FLAGS_t::MX);
			_width  = _widthStartTFT;
			_height = _heightStartTFT;
			_XStart = RAM_WIDTH_NV3007  - _widthStartTFT  - _colOffset;
			_YStart = RAM_HEIGHT_NV3007 - _heightStartTFT - _rowOffset;
			break;
		case Degrees_270:
			madctl |= (MADCTL_FLAGS_t::MY | MADCTL_FLAGS_t::MV);
			_width  = _heightStartTFT;
			_height = _widthStartTFT;
			_XStart = RAM_HEIGHT_NV3007 - _heightStartTFT - _rowOffset;
			_YStart = RAM_WIDTH_NV3007  - _widthStartTFT  - _colOffset;
			break;
	}
	writeCommand(NV3007_MADCTL);
	writeData(madctl);
}

/*!
	@brief initialise the variables that define the size of the screen
	@param width_TFT panel width in pixels (default 142)
	@param height_TFT panel height in pixels (default 428)
	@param colorOrder MADCTL colour order, RGB or BGR, see MADCTL_FLAGS_t struct
	@note The GRAM is fixed at 168x428, panel size must fit inside it.
*/
void NV3007_LTSM::TFTInitScreen(uint16_t width_TFT, uint16_t height_TFT, uint8_t colorOrder)
{
	_width          = width_TFT;
	_height         = height_TFT;
	_widthStartTFT  = width_TFT;
	_heightStartTFT = height_TFT;
	if (colorOrder != MADCTL_FLAGS_t::RGB && colorOrder != MADCTL_FLAGS_t::BGR)
	{
		#ifdef dislib16_DEBUG_MODE_ENABLE
			Serial.println("Warning: TFTInitScreen: Invalid color order, defaulting to RGB");
		#endif
		_colorOrder = MADCTL_FLAGS_t::RGB;
	}else{
		_colorOrder = colorOrder;
	}
}

/*!
	@brief Set the panel RAM offsets for displays where the visible area
		is smaller than the GRAM. Portrait (0 degree) offsets are supplied,
		offsets for other rotations are derived automatically.
	@param colOffset Columns of dead RAM to the left of the panel in portrait (default 12)
	@param rowOffset Rows of dead RAM above the panel in portrait (default 0)
*/
void NV3007_LTSM::TFTsetPanelOffset(uint8_t colOffset, uint8_t rowOffset)
{
	_colOffset = colOffset;
	_rowOffset = rowOffset;
}

/*!
	@brief Sanity check of panel size and offsets against the fixed GRAM size.
	@details On failure a debug warning is printed and size/offsets are clamped
		so the window can never address outside the GRAM.
*/
void NV3007_LTSM::TFTcheckGeometry(void)
{
	if (_widthStartTFT > RAM_WIDTH_NV3007 || _heightStartTFT > RAM_HEIGHT_NV3007)
	{
		#ifdef dislib16_DEBUG_MODE_ENABLE
			Serial.println("Warning: TFTcheckGeometry: panel size exceeds 168x428 GRAM, clamped");
		#endif
		if (_widthStartTFT  > RAM_WIDTH_NV3007)  _widthStartTFT  = RAM_WIDTH_NV3007;
		if (_heightStartTFT > RAM_HEIGHT_NV3007) _heightStartTFT = RAM_HEIGHT_NV3007;
		_width  = _widthStartTFT;
		_height = _heightStartTFT;
	}
	if ((uint16_t)_colOffset + _widthStartTFT > RAM_WIDTH_NV3007 ||
		(uint16_t)_rowOffset + _heightStartTFT > RAM_HEIGHT_NV3007)
	{
		#ifdef dislib16_DEBUG_MODE_ENABLE
			Serial.println("Warning: TFTcheckGeometry: panel offset outside GRAM, offsets set to 0");
		#endif
		_colOffset = 0;
		_rowOffset = 0;
	}
}

/*!
	@brief Freq delay used in SW SPI getter, uS delay used in SW SPI method
	@return The GPIO communications delay in uS
*/
uint16_t NV3007_LTSM::TFTSwSpiGpioDelayGet(void){return _SWSPIGPIODelay;}

/*!
	@brief Freq delay used in SW SPI setter, uS delay used in SW SPI method
	@param CommDelay The GPIO communications delay in uS
*/
void  NV3007_LTSM::TFTSwSpiGpioDelaySet(uint16_t CommDelay){_SWSPIGPIODelay = CommDelay;}

/*!
	@brief SPI displays set an address window rectangle for blitting pixels
	@param x1 Top left corner x coordinate
	@param y1 Top left corner y coordinate
	@param w  Window end x coordinate (inclusive), as passed by display16_LTSM
	@param h  Window end y coordinate (inclusive), as passed by display16_LTSM
*/
void NV3007_LTSM::setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
{
	const bool singlePixel =
		((w == x1 + 1) && (h == y1 + 1));
	if (singlePixel) {
		w = x1;
		h = y1;
	}
	// Translate panel coordinates -> GRAM coordinates
	x1 += _XStart;
	w  += _XStart;
	y1 += _YStart;
	h  += _YStart;

	uint8_t seqCASET[4] = {(uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF), (uint8_t)(w >> 8), (uint8_t)(w & 0xFF)};
	uint8_t seqRASET[4] = {(uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF), (uint8_t)(h >> 8), (uint8_t)(h & 0xFF)};
	writeCommand(NV3007_CASET); // Column address set
	spiWriteDataBuffer(seqCASET, sizeof(seqCASET));
	writeCommand(NV3007_RASET); // Row address set
	spiWriteDataBuffer(seqRASET, sizeof(seqRASET));
	writeCommand(NV3007_RAMWR); // Write to RAM
}

/*!
	@brief Defines the Vertical Scrolling Area of the display.
	@param topFixed Top Fixed Area.
	@param scrollArea Scrolling Area.
	@param bottomFixed Bottom Fixed Area.
*/
void NV3007_LTSM::TFTsetVerticalScrollArea(uint16_t topFixed, uint16_t scrollArea, uint16_t bottomFixed) {
	writeCommand(NV3007_VSCRDEF);
	writeData(topFixed >> 8);
	writeData(topFixed & 0xFF);
	writeData(scrollArea >> 8);
	writeData(scrollArea & 0xFF);
	writeData(bottomFixed >> 8);
	writeData(bottomFixed & 0xFF);
}

/*!
	@brief Used together with TFTverticalsetScrollArea.
	@param vsp scroll start address
*/
void NV3007_LTSM::TFTsetVerticalScrollStart(uint16_t vsp) {
	writeCommand(NV3007_VSCRSADD);
	writeData(vsp >> 8);
	writeData(vsp & 0xFF);
}

/*! @brief Scroll mode can be left by the Normal Display Mode ON command */
void NV3007_LTSM::TFTScrollModeLeave(void) {writeCommand(NV3007_NORON);}

/*!
	@brief Defines the Horizontal Scrolling Area of the display, datasheet 6.2.26 HSDEF (3Dh).
	@param leftFixed Left Fixed Area, in columns from the left of the frame memory
	@param scrollArea Width of the Horizontal Scrolling Area, in frame memory columns
	@param rightFixed Right Fixed Area, in columns from the right of the frame memory
*/
void NV3007_LTSM::TFTsetHorizontalScrollArea(uint16_t leftFixed, uint16_t scrollArea, uint16_t rightFixed) {
	writeCommand(NV3007_HSCRDEF);
	writeData(leftFixed >> 8);
	writeData(leftFixed & 0xFF);
	writeData(scrollArea >> 8);
	writeData(scrollArea & 0xFF);
	writeData(rightFixed >> 8);
	writeData(rightFixed & 0xFF);
}

/*!
	@brief Horizontal scrolling start address, datasheet 6.2.27 HSSAD (3Eh).
	@param hsp Frame memory column shown first after the Left Fixed Area (9 bit)
*/
void NV3007_LTSM::TFTsetHorizontalScrollStart(uint16_t hsp) {
	writeCommand(NV3007_HSCRSADD);
	writeData(hsp >> 8);
	writeData(hsp & 0xFF);
}

/*!
	@brief enable / disable display
	@param enableDisplay true enable on, false disable
	@details Temporarily blank the screen without re-initialising.
*/
void NV3007_LTSM::TFTenableDisplay(bool enableDisplay){
	if(enableDisplay) {
		writeCommand(NV3007_DISPON);
		_displayOn = true;
	} else {
		writeCommand(NV3007_DISPOFF);
		_displayOn = false;
	}
}

/*!
	@brief Set the power mode of the display
	@param mode The power state to set
	@details Based on datasheet section 5.9.4 Power Flow Chart.
*/
void NV3007_LTSM::TFTsetPowerMode(PowerState_e mode) {
	// Always return to a known base state
	writeCommand(NV3007_SLPOUT);
	MILLISEC_DELAY(_sleepDelay);
	switch (mode) {
		case PowerState_e::NormalIdleOff:
			writeCommand(NV3007_NORON);
			writeCommand(NV3007_IDLEOFF);
			break;
		case PowerState_e::NormalIdleOn:
			writeCommand(NV3007_NORON);
			writeCommand(NV3007_IDLEON);
			break;
		case PowerState_e::PartialIdleOff:
			writeCommand(NV3007_PTLON);
			writeCommand(NV3007_IDLEOFF);
			break;
		case PowerState_e::PartialIdleOn:
			writeCommand(NV3007_PTLON);
			writeCommand(NV3007_IDLEON);
			break;
		case PowerState_e::SleepNormalIdleOff:
			writeCommand(NV3007_NORON);
			writeCommand(NV3007_IDLEOFF);
			writeCommand(NV3007_SLPIN);
			MILLISEC_DELAY(_sleepDelay);
			break;
		case PowerState_e::SleepNormalIdleOn:
			writeCommand(NV3007_NORON);
			writeCommand(NV3007_IDLEON);
			writeCommand(NV3007_SLPIN);
			MILLISEC_DELAY(_sleepDelay);
			break;
		case PowerState_e::SleepPartialIdleOff:
			writeCommand(NV3007_PTLON);
			writeCommand(NV3007_IDLEOFF);
			writeCommand(NV3007_SLPIN);
			MILLISEC_DELAY(_sleepDelay);
			break;
		case PowerState_e::SleepPartialIdleOn:
			writeCommand(NV3007_PTLON);
			writeCommand(NV3007_IDLEON);
			writeCommand(NV3007_SLPIN);
			MILLISEC_DELAY(_sleepDelay);
			break;
	}
	_currentPowerState = mode;
}

/*!
	@brief Write a sequence of values to consecutive registers, starting at firstReg
	@param firstReg The first register to write to
	@param values An array of values to write
	@param count The number of values to write
	@details Writes register firstReg + i with values[i], one byte per register.
		Each register is addressed with its own command.
*/
void NV3007_LTSM::writeRegisterRun(uint8_t firstReg, const uint8_t values[], uint8_t count)
{
	for (uint8_t i = 0; i < count; i++) {
		writeCommand(firstReg + i);
		writeData(values[i]);
	}
}

/*!
	@brief Write a block of data to a single register
	@param reg The register to write to
	@param data An array of data to write
	@param len The number of bytes to write
	@details This method is used to write multiple bytes of data after a single register command.
*/
void NV3007_LTSM::writeRegisterBlock(uint8_t reg, const uint8_t data[], uint8_t len)
{
	writeCommand(reg);
	spiWriteDataBuffer((uint8_t *) data, len);
}

/*!
	@brief Write a single byte to a register
	@param reg The register to write to
	@param value The value to write
	@details This method is used to write a single byte of data to a register.
*/
void NV3007_LTSM::writeRegister(uint8_t reg, uint8_t value)
{
	writeCommand(reg);
	writeData(value);
}

/*!
	@brief NV3007 init sequence.
	@details Private registers (0x40 and up) are unlocked with NV3007_CMDEN.
		The register values (gamma, GOA, source, pump) are the buydisplay.com
		vendor values for the 1.65" 142x428 panel (TFTM1.65-2), they are panel
		specific tuning. Register names are the datasheet mnemonics.
		The sequence then sets COLMOD (RGB565), MADCTL, sleep out and display on.
		Note the vendor sequence enables the tearing effect output (NV3007_TEON).
*/
void NV3007_LTSM::TFTInitSequence(void)
{
	// === Private register enable ===
	writeRegister(NV3007_CMDEN, NV3007_CMDEN_UNLOCK);
	// === pump control ===
	writeRegisterRun(NV3007_PWRCTRL12, (const uint8_t[]){0x08, 0x08, 0xB0, 0x17, 0xC2}, 5);
	writeRegisterBlock(NV3007_PWRCTRL1, (const uint8_t[]){0x22, 0x04}, 2);
	// === gamma control + gamma LDO ===
	writeGammaControl();
	// === ? ===
	writeRegister(0x50, 0x00);  // Undocumented in datasheet, vendor value
	writeRegister(0x52, 0xD6);  // Undocumented in datasheet, vendor value
	// === Internal timing control ===
	writeRegisterRun(NV3007_ITCTRL1, (const uint8_t[]){0x04, 0x04, 0x1B, 0x1B}, 4);
	// === GOA control and VST ===
	writeRegisterBlock(NV3007_GOACTRL, (const uint8_t[]){0x2A, 0x24, 0x00}, 3);
	writeRegisterRun(NV3007_VSTCTRL1, (const uint8_t[]){0x84, 0x85}, 2);
	writeRegisterRun(NV3007_VSTCTRL8, (const uint8_t[]){0x34, 0x80, 0x73}, 3);
	// === GOA VEND control 1 ===
	const uint8_t vendData[] = {
	0x03, 0x61, 0x03, 0x65, 0x03, 0x60, 0x03, 0x64 
	};
	for (uint8_t i = 0; i < 4; ++i) {
		writeRegisterBlock(NV3007_VENDCTRL1 + i, &vendData[i * 2], 2);
	}
	// === GOA CLK control 25 registers ===
	// Single byte write for 1-8, 12-15, 17-20, 22-25
	// Skip 9-11
	// Double byte write for 16, 21
	const uint8_t clkSingle[] = {
	0x82, 0x83, 0x80, 0x81, 0x02, 0x01, 0x04, 0x03,
	0x33, 0x80, 0x73, 0x00,0x5B, 0x5A, 0x5D, 0x5C, 0x5F, 0x5E, 0x61, 0x60
	};
	const uint8_t clkDouble[] = {0x33, 0x33};
	uint8_t clkSingle_index = 0;
	uint8_t reg = NV3007_CLKCTRL1;
	for (uint8_t i = 0; i < 22; ++i) {
		if (reg == NV3007_CLKCTRL16 || reg == NV3007_CLKCTRL21) {
			writeRegisterBlock(reg, clkDouble, 2);
		} else {
			writeRegister(reg, clkSingle[clkSingle_index]);
			clkSingle_index ++;
		}
		if (reg == NV3007_CLKCTRL8) { // skip the unused gap CLKCTRL9..11
			reg = NV3007_CLKCTRL12;
		} else {
			++reg;
		}
	}
	// === GOA VEND control 2 ===
	writeRegisterBlock(NV3007_VENDCTRL6, (const uint8_t[]){0x3A, 0x3A, 0x00, 0x00}, 4);
	writeRegisterRun(NV3007_VENDCTRL12, (const uint8_t[]){0x32, 0x80, 0x73}, 3);
	// === Source + FSM control ===
	writeRegister(NV3007_SOUCTRL1, 0x00);
	writeRegisterBlock(NV3007_SOUCTRL2, (const uint8_t[]){0x03, 0x0F}, 2);
	writeRegisterRun(NV3007_SOUCTRL3, (const uint8_t[]){0x04, 0x01, 0x0E, 0x01, 0x19, 0x10, 0x10, 0x21, 0x12, 0xD0, 0x04, 0x07, 0x07, 0x09, 0xD0, 0x0E}, 16);
	writeRegister(NV3007_FSMCTRL, 0x56);
	writeRegisterBlock(NV3007_SOUCTRL19, (const uint8_t[]){0x26, 0x1B, 0x0B, 0x20}, 4);
	writeRegister(NV3007_SOUCTRL13, 0x04);
	// === Tearing Effect ===
	writeRegister(NV3007_TEON, 0x00);
	writeRegisterBlock(NV3007_TECTRL1, (const uint8_t[]){0x00, 0x10}, 2);
	writeRegister(NV3007_TECTRL3, 0x10);
	// === Private register enable lock ===
	writeRegister(NV3007_CMDEN, NV3007_CMDEN_LOCK);

	// === Public registers ===
	writeRegister(NV3007_COLMOD, 0x05);
	writeRegister(NV3007_MADCTL, _colorOrder);
	writeCommand(NV3007_SLPOUT);
	MILLISEC_DELAY(120);
	writeCommand(NV3007_DISPON);
	MILLISEC_DELAY(10);
	_displayOn = true;
	TFTsetRotation(Degrees_0);
}

/*!
	@brief Write the gamma control and gammo LDO registers
	@details The values are vendor specific tuning for the 1.65" 142x428 panel (TFTM1.65-2).
*/
void NV3007_LTSM::writeGammaControl(void)
{
	writeCommand(NV3007_GLDOCTRL2);
	writeData(0x90);
	writeCommand(NV3007_GLDOCTRL1);
	writeData(0x7B);
	writeCommand(NV3007_GLDOCTRL3);
	writeData(0x4F);

	writeCommand(NV3007_GAMCTRL15);
	writeData(0x0F);
	writeCommand(NV3007_GAMCTRL31);
	writeData(0x0F);
	writeCommand(NV3007_GAMCTRL1);
	writeData(0x00);
	writeCommand(NV3007_GAMCTRL17);
	writeData(0x00);
	writeCommand(NV3007_GAMCTRL14);
	writeData(0x39);
	writeCommand(NV3007_GAMCTRL30);
	writeData(0x31);
	writeCommand(NV3007_GAMCTRL2);
	writeData(0x0A);
	writeCommand(NV3007_GAMCTRL18);
	writeData(0x0A);
	writeCommand(NV3007_GAMCTRL13);
	writeData(0x35);
	writeCommand(NV3007_GAMCTRL29);
	writeData(0x29);
	writeCommand(NV3007_GAMCTRL3);
	writeData(0x0F);
	writeCommand(NV3007_GAMCTRL19);
	writeData(0x0F);
	writeCommand(NV3007_GAMCTRL9);
	writeData(0x4F);
	writeCommand(NV3007_GAMCTRL25);
	writeData(0x45);
	writeCommand(NV3007_GAMCTRL7);
	writeData(0x33);
	writeCommand(NV3007_GAMCTRL23);
	writeData(0x33);
	writeCommand(NV3007_GAMCTRL12);
	writeData(0x14);
	writeCommand(NV3007_GAMCTRL28);
	writeData(0x14);
	writeCommand(NV3007_GAMCTRL4);
	writeData(0x09);
	writeCommand(NV3007_GAMCTRL20);
	writeData(0x09);
	writeCommand(NV3007_GAMCTRL11);
	writeData(0x13);
	writeCommand(NV3007_GAMCTRL27);
	writeData(0x16);
	writeCommand(NV3007_GAMCTRL5);
	writeData(0x08);
	writeCommand(NV3007_GAMCTRL21);
	writeData(0x08);
	writeCommand(NV3007_GAMCTRL10);
	writeData(0x07);
	writeCommand(NV3007_GAMCTRL26);
	writeData(0x0D);
	writeCommand(NV3007_GAMCTRL6);
	writeData(0x05);
	writeCommand(NV3007_GAMCTRL22);
	writeData(0x05);
	writeCommand(NV3007_GAMCTRL8);
	writeData(0x33);
	writeCommand(NV3007_GAMCTRL24);
	writeData(0x33);
	writeCommand(NV3007_GAMCTRL16);
	writeData(0x00);
	writeCommand(NV3007_GAMCTRL32);
	writeData(0x00);
}
// === EOF ===
