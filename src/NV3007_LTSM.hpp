/*!
	@file    NV3007_LTSM.hpp
	@author  Gavin Lyons, LionTron Systems
	@brief   Arduino library header file, contains driver methods for the NV3007 display driver
	@note    NV3007: 168 x 428 RGB dot, 262k-colour TFT LCD single-chip driver
	         (New Vision Microelectronics). 4-line SPI (hardware or software).
*/

#pragma once

// Section Libraries
#include "display16_graphics_LTSM.hpp"

/*! @brief Class to control NV3007 TFT basic functionality. */
class NV3007_LTSM : public display16_graphics_LTSM
{

public:
	NV3007_LTSM();
	~NV3007_LTSM() {};

	/*!
	 * @brief NV3007 power states. Combine Normal/Partial, Idle ON/OFF and Sleep IN/OUT.
	 * Listed from highest to lowest power consumption.
	 * See datasheet section 5.9.4 Power Flow Chart.
	 */
	enum class PowerState_e : uint8_t
	{
		NormalIdleOff,       /**< SLPOUT + NORON + IDLEOFF */
		NormalIdleOn,        /**< SLPOUT + NORON + IDLEON */
		PartialIdleOff,      /**< SLPOUT + PTLON + IDLEOFF */
		PartialIdleOn,       /**< SLPOUT + PTLON + IDLEON */
		SleepNormalIdleOff,  /**< SLPIN + NORON + IDLEOFF */
		SleepNormalIdleOn,   /**< SLPIN + NORON + IDLEON */
		SleepPartialIdleOff, /**< SLPIN + PTLON + IDLEOFF */
		SleepPartialIdleOn   /**< SLPIN + PTLON + IDLEON */
	};

	/*! @brief MADCTL bit flags for register MADCTL (0x36). */
	struct MADCTL_FLAGS_t
	{
		static constexpr uint8_t MY = 0x80;  /**< Page address order: bottom to top */
		static constexpr uint8_t MX = 0x40;  /**< Column address order: right to left */
		static constexpr uint8_t MV = 0x20;  /**< Page/Column exchange (transpose) */
		static constexpr uint8_t ML = 0x10;  /**< Line address order: LCD refresh bottom to top */
		static constexpr uint8_t RGB = 0x00; /**< RGB colour order */
		static constexpr uint8_t BGR = 0x08; /**< BGR colour order */
		static constexpr uint8_t MH = 0x04;  /**< Column scan order: LCD refresh right to left */
	};

	/*! @brief Physical GRAM size of the NV3007, fixed by the controller (X=0..167, Y=0..427) */
	static constexpr uint16_t RAM_WIDTH_NV3007  = 168;
	static constexpr uint16_t RAM_HEIGHT_NV3007 = 428;

	virtual void setAddrWindow(uint16_t, uint16_t, uint16_t, uint16_t) override;

	void TFTsetupGPIO_SPI(uint16_t CommDelay, int8_t rst, int8_t dc, int8_t cs, int8_t sclk, int8_t din);
	void TFTsetupGPIO_SPI(uint32_t baudrate, int8_t rst, int8_t dc, int8_t cs);
	void TFTInitScreen(uint16_t w = 142, uint16_t h = 428,
						uint8_t colorOrder = MADCTL_FLAGS_t::RGB);
	void TFTsetPanelOffset(uint8_t colOffset = 12, uint8_t rowOffset = 0);
	void TFTNV3007Initialize(void);
	void TFTPowerDown(void);
	uint16_t TFTSwSpiGpioDelayGet(void);
	void TFTSwSpiGpioDelaySet(uint16_t);

	void TFTsetRotation(display_rotate_e r);
	void TFTchangeInvertMode(bool m);
	void TFTsetPowerMode(PowerState_e m);
	void TFTenableDisplay(bool m);
	void TFTsetVerticalScrollArea(uint16_t topFixed, uint16_t scrollArea, uint16_t bottomFixed);
	void TFTsetVerticalScrollStart(uint16_t vsp);
	void TFTScrollModeLeave(void);
	void TFTsetHorizontalScrollArea(uint16_t leftFixed, uint16_t scrollArea, uint16_t rightFixed);
	void TFTsetHorizontalScrollStart(uint16_t hsp);

private:
	void TFTHWSPIInitialize(void);
	void TFTResetPIN(void);
	void TFTInitSequence(void);
	void TFTcheckGeometry(void);
	void writeRegisterRun(uint8_t firstReg, const uint8_t values[], uint8_t count);
	void writeRegisterBlock(uint8_t reg, const uint8_t data[], uint8_t len);
	void writeRegister(uint8_t reg, uint8_t value);
	void writeGammaControl(void);

private:
	// Display
	PowerState_e _currentPowerState = PowerState_e::NormalIdleOff; /**< Current display power mode */
	uint8_t _colorOrder = MADCTL_FLAGS_t::RGB; /**< RGB or BGR bit from MADCTL_FLAGS_t */
	bool _displayOn = false;                   /**< Display on/off status */
	const uint16_t _sleepDelay = 120;          /**< Sleep delay in mS, datasheet 6.2.4 / 6.2.5 */
	// SPI
	bool _resetPinOn = true; /**< true: HW reset pin used, false: no reset pin */
	// Screen size, defaults match the 142x428 panel
	uint16_t _widthStartTFT = 142;  /**< Panel width, never changes after TFTInitScreen */
	uint16_t _heightStartTFT = 428; /**< Panel height, never changes after TFTInitScreen */
	uint8_t _colOffset = 12;        /**< Portrait col (X) dead-RAM offset, user supplied */
	uint8_t _rowOffset = 0;         /**< Portrait row (Y) dead-RAM offset, user supplied */

	// NV3007 public commands (private registers commands  require cmd NV3007_CMDEN to unlock)
	static constexpr uint8_t NV3007_SLPIN    = 0x10; /**< Enter Sleep Mode */
	static constexpr uint8_t NV3007_SLPOUT   = 0x11; /**< Sleep Out */
	static constexpr uint8_t NV3007_PTLON    = 0x12; /**< Partial Mode ON */
	static constexpr uint8_t NV3007_NORON    = 0x13; /**< Normal Display Mode ON */
	static constexpr uint8_t NV3007_INVOFF   = 0x20; /**< Display Inversion OFF */
	static constexpr uint8_t NV3007_INVON    = 0x21; /**< Display Inversion ON */
	static constexpr uint8_t NV3007_DISPOFF  = 0x28; /**< Display OFF */
	static constexpr uint8_t NV3007_DISPON   = 0x29; /**< Display ON */
	static constexpr uint8_t NV3007_CASET    = 0x2A; /**< Column Address Set */
	static constexpr uint8_t NV3007_RASET    = 0x2B; /**< Row Address Set */
	static constexpr uint8_t NV3007_RAMWR    = 0x2C; /**< Memory Write */
	// static constexpr uint8_t NV3007_PTLAR    = 0x30; /**< Partial Area */
	static constexpr uint8_t NV3007_VSCRDEF  = 0x33; /**< Vertical Scrolling Definition */
	// static constexpr uint8_t NV3007_TEOFF    = 0x34; /**< Tearing Effect Line OFF */
	static constexpr uint8_t NV3007_TEON     = 0x35; /**< Tearing Effect Line ON */
	static constexpr uint8_t NV3007_MADCTL   = 0x36; /**< Memory Access Control */
	static constexpr uint8_t NV3007_VSCRSADD = 0x37; /**< Vertical Scrolling Start Address */
	static constexpr uint8_t NV3007_IDLEOFF  = 0x38; /**< Idle mode OFF */
	static constexpr uint8_t NV3007_IDLEON   = 0x39; /**< Idle mode ON */
	static constexpr uint8_t NV3007_COLMOD   = 0x3A; /**< Pixel Format Set: 0x05 = 16bpp, 0x06 = 18bpp */

	// NV3007 public commands, extra (not all used by the driver)
	// static constexpr uint8_t NV3007_NOP      = 0x00; /**< No operation */
	// static constexpr uint8_t NV3007_RDDID    = 0x04; /**< Read display ID */
	// static constexpr uint8_t NV3007_RDDST    = 0x09; /**< Read display status */
	// static constexpr uint8_t NV3007_RAMRD    = 0x2E; /**< Memory read */
	// static constexpr uint8_t NV3007_WRMEMC   = 0x3C; /**< Write memory continue */
	static constexpr uint8_t NV3007_HSCRDEF  = 0x3D; /**< Horizontal Scrolling Definition */
	static constexpr uint8_t NV3007_HSCRSADD = 0x3E; /**< Horizontal Scrolling Start Address */

	// NV3007 private registers. Names are the datasheet mnemonics (section 6.1.2).
	// Private registers (0x40 and up) need the command enable below set first.
	static constexpr uint8_t NV3007_CMDEN         = 0xFF; /**< Private register enable, write 0xA5 to unlock, 0x00 to lock */
	static constexpr uint8_t NV3007_CMDEN_UNLOCK  = 0xA5; /**< Value written to NV3007_CMDEN to unlock private registers */
	static constexpr uint8_t NV3007_CMDEN_LOCK    = 0x00; /**< Value written to NV3007_CMDEN to lock private registers */

	// Private: interface, TE, OTP, timing, RAM control
	// static constexpr uint8_t NV3007_IFCTRL1      = 0x40; /**< Interface control 1 */
	// static constexpr uint8_t NV3007_IFCTRL2      = 0x41; /**< Interface control 2 */
	// static constexpr uint8_t NV3007_IFCTRL3      = 0x42; /**< Interface control 3 */
	// static constexpr uint8_t NV3007_IFCTRL4      = 0x43; /**< Interface control 4 */
	static constexpr uint8_t NV3007_TECTRL1      = 0x44; /**< Tearing effect control 1 */
	// static constexpr uint8_t NV3007_TECTRL2      = 0x45; /**< Tearing effect control 2 */
	static constexpr uint8_t NV3007_TECTRL3      = 0x46; /**< Tearing effect control 3 */
	// static constexpr uint8_t NV3007_TECTRL4      = 0x47; /**< Tearing effect control 4 */
	// static constexpr uint8_t NV3007_TECTRL5      = 0x48; /**< Tearing effect control 5 */
	// static constexpr uint8_t NV3007_SCANCTRL     = 0x49; /**< Scan direction control */
	// static constexpr uint8_t NV3007_OTPCTRL1     = 0x4A; /**< OTP control 1 */
	// static constexpr uint8_t NV3007_OTPCTRL2     = 0x4B; /**< OTP control 2 */
	// static constexpr uint8_t NV3007_OTPCTRL3     = 0x4C; /**< OTP control 3 */
	// static constexpr uint8_t NV3007_OTPCTRL4     = 0x4D; /**< OTP control 4 */
	// static constexpr uint8_t NV3007_USRMAD       = 0x4F; /**< Memory access control */
	static constexpr uint8_t NV3007_ITCTRL1      = 0x53; /**< Internal timing control 1 */
	// static constexpr uint8_t NV3007_ITCTRL2      = 0x54; /**< Internal timing control 2 */
	// static constexpr uint8_t NV3007_ITCTRL3      = 0x55; /**< Internal timing control 3 */
	// static constexpr uint8_t NV3007_ITCTRL4      = 0x56; /**< Internal timing control 4 */
	// static constexpr uint8_t NV3007_IBIASCTRL    = 0x57; /**< Ibias control */
	// static constexpr uint8_t NV3007_LVDCTRL      = 0x59; /**< LVD control */
	// static constexpr uint8_t NV3007_RAMCTRL1     = 0x5C; /**< RAMCTRL 1 */
	// static constexpr uint8_t NV3007_RAMCTRL2     = 0x5D; /**< RAMCTRL 2 */
	// static constexpr uint8_t NV3007_RDBIST       = 0x5E; /**< RDBIST */

	// Private: gamma control (V0..V63 points, positive/negative pairs)
	static constexpr uint8_t NV3007_GAMCTRL1     = 0x60; /**< Gamma control point 1 */
	static constexpr uint8_t NV3007_GAMCTRL2     = 0x61; /**< Gamma control point 2 */
	static constexpr uint8_t NV3007_GAMCTRL3     = 0x62; /**< Gamma control point 3 */
	static constexpr uint8_t NV3007_GAMCTRL4     = 0x63; /**< Gamma control point 4 */
	static constexpr uint8_t NV3007_GAMCTRL5     = 0x64; /**< Gamma control point 5 */
	static constexpr uint8_t NV3007_GAMCTRL6     = 0x65; /**< Gamma control point 6 */
	static constexpr uint8_t NV3007_GAMCTRL7     = 0x66; /**< Gamma control point 7 */
	static constexpr uint8_t NV3007_GAMCTRL8     = 0x67; /**< Gamma control point 8 */
	static constexpr uint8_t NV3007_GAMCTRL9     = 0x68; /**< Gamma control point 9 */
	static constexpr uint8_t NV3007_GAMCTRL10    = 0x69; /**< Gamma control point 10 */
	static constexpr uint8_t NV3007_GAMCTRL11    = 0x6A; /**< Gamma control point 11 */
	static constexpr uint8_t NV3007_GAMCTRL12    = 0x6B; /**< Gamma control point 12 */
	static constexpr uint8_t NV3007_GAMCTRL13    = 0x6C; /**< Gamma control point 13 */
	static constexpr uint8_t NV3007_GAMCTRL14    = 0x6D; /**< Gamma control point 14 */
	static constexpr uint8_t NV3007_GAMCTRL15    = 0x6E; /**< Gamma control point 15 */
	static constexpr uint8_t NV3007_GAMCTRL16    = 0x6F; /**< Gamma control point 16 */
	static constexpr uint8_t NV3007_GAMCTRL17    = 0x70; /**< Gamma control point 17 */
	static constexpr uint8_t NV3007_GAMCTRL18    = 0x71; /**< Gamma control point 18 */
	static constexpr uint8_t NV3007_GAMCTRL19    = 0x72; /**< Gamma control point 19 */
	static constexpr uint8_t NV3007_GAMCTRL20    = 0x73; /**< Gamma control point 20 */
	static constexpr uint8_t NV3007_GAMCTRL21    = 0x74; /**< Gamma control point 21 */
	static constexpr uint8_t NV3007_GAMCTRL22    = 0x75; /**< Gamma control point 22 */
	static constexpr uint8_t NV3007_GAMCTRL23    = 0x76; /**< Gamma control point 23 */
	static constexpr uint8_t NV3007_GAMCTRL24    = 0x77; /**< Gamma control point 24 */
	static constexpr uint8_t NV3007_GAMCTRL25    = 0x78; /**< Gamma control point 25 */
	static constexpr uint8_t NV3007_GAMCTRL26    = 0x79; /**< Gamma control point 26 */
	static constexpr uint8_t NV3007_GAMCTRL27    = 0x7A; /**< Gamma control point 27 */
	static constexpr uint8_t NV3007_GAMCTRL28    = 0x7B; /**< Gamma control point 28 */
	static constexpr uint8_t NV3007_GAMCTRL29    = 0x7C; /**< Gamma control point 29 */
	static constexpr uint8_t NV3007_GAMCTRL30    = 0x7D; /**< Gamma control point 30 */
	static constexpr uint8_t NV3007_GAMCTRL31    = 0x7E; /**< Gamma control point 31 */
	static constexpr uint8_t NV3007_GAMCTRL32    = 0x7F; /**< Gamma control point 32 */

	// Private: regulator, VDDS, gamma LDO, ESD, pump control
	// static constexpr uint8_t NV3007_RGLRCTRL1    = 0x80; /**< RGLRCTRL1 */
	// static constexpr uint8_t NV3007_RGLRCTRL2    = 0x81; /**< RGLRCTRL2 */
	// static constexpr uint8_t NV3007_VDDSCTRL     = 0x82; /**< VDDS control */
	static constexpr uint8_t NV3007_GLDOCTRL1    = 0x83; /**< Gamma ldo control 1 */
	static constexpr uint8_t NV3007_GLDOCTRL2    = 0x84; /**< Gamma ldo control2 */
	static constexpr uint8_t NV3007_GLDOCTRL3    = 0x85; /**< Gamma ldo control 3 */
	// static constexpr uint8_t NV3007_ESDCTRL2     = 0x8B; /**< ESD control 2 */
	// static constexpr uint8_t NV3007_ESDCTRL3     = 0x8C; /**< ESD control 3 */
	// static constexpr uint8_t NV3007_GLDOCTRL4    = 0x8D; /**< Gamma Shift */
	// static constexpr uint8_t NV3007_RDOTPLD      = 0x8E; /**< RDOTPLD */
	static constexpr uint8_t NV3007_PWRCTRL1     = 0x8F; /**< Pump control 1 */
	// static constexpr uint8_t NV3007_PWRCTRL2     = 0x90; /**< Pump control 2 */
	// static constexpr uint8_t NV3007_PWRCTRL3     = 0x91; /**< Pump control 3 */
	// static constexpr uint8_t NV3007_PWRCTRL4     = 0x92; /**< Pump control 4 */
	// static constexpr uint8_t NV3007_PWRCTRL5     = 0x93; /**< Pump control 5 */
	// static constexpr uint8_t NV3007_PWRCTRL6     = 0x94; /**< Pump control 6 */
	// static constexpr uint8_t NV3007_PWRCTRL7     = 0x95; /**< Pump control 7 */
	// static constexpr uint8_t NV3007_PWRCTRL8     = 0x96; /**< Pump control 8 */
	// static constexpr uint8_t NV3007_PWRCTRL9     = 0x97; /**< Pump control 9 */
	// static constexpr uint8_t NV3007_PWRCTRL10    = 0x98; /**< Pump control 10 */
	// static constexpr uint8_t NV3007_PWRCTRL11    = 0x99; /**< Pump control 11 */
	static constexpr uint8_t NV3007_PWRCTRL12    = 0x9A; /**< Pump control 12 */
	// static constexpr uint8_t NV3007_PWRCTRL13    = 0x9B; /**< Pump control 13 */
	// static constexpr uint8_t NV3007_PWRCTRL14    = 0x9C; /**< Pump control 14 */
	// static constexpr uint8_t NV3007_PWRCTRL15    = 0x9D; /**< Pump control 15 */
	// static constexpr uint8_t NV3007_PWRCTRL16    = 0x9E; /**< Pump control 16 */
	// static constexpr uint8_t NV3007_PWRCTRL17    = 0x9F; /**< Pump control 17 */

	// Private: GOA control and VST
	static constexpr uint8_t NV3007_GOACTRL      = 0xA0; /**< GOA control */
	static constexpr uint8_t NV3007_VSTCTRL1     = 0xA1; /**< GOA VST control 1 */
	// static constexpr uint8_t NV3007_VSTCTRL2     = 0xA2; /**< GOA VST control 2 */
	// static constexpr uint8_t NV3007_VSTCTRL3     = 0xA3; /**< GOA VST control 3 */
	// static constexpr uint8_t NV3007_VSTCTRL4     = 0xA4; /**< GOA VST control 4 */
	// static constexpr uint8_t NV3007_VSTCTRL5     = 0xA5; /**< GOA VST control 5 */
	// static constexpr uint8_t NV3007_VSTCTRL6     = 0xA6; /**< GOA VST control 6 */
	// static constexpr uint8_t NV3007_VSTCTRL7     = 0xA7; /**< GOA VST control 7 */
	static constexpr uint8_t NV3007_VSTCTRL8     = 0xA8; /**< GOA VST control 8 */
	// static constexpr uint8_t NV3007_VSTCTRL9     = 0xA9; /**< GOA VST control 9 */
	// static constexpr uint8_t NV3007_VSTCTRL10    = 0xAA; /**< GOA VST control 10 */

	// Private: GOA VEND
	static constexpr uint8_t NV3007_VENDCTRL1    = 0xAB; /**< GOA VEND control 1 */
	// static constexpr uint8_t NV3007_VENDCTRL2    = 0xAC; /**< GOA VEND control 2 */
	// static constexpr uint8_t NV3007_VENDCTRL3    = 0xAD; /**< GOA VEND control 3 */
	// static constexpr uint8_t NV3007_VENDCTRL4    = 0xAE; /**< GOA VEND control 4 */
	// static constexpr uint8_t NV3007_VENDCTRL5    = 0xAF; /**< GOA VEND control 5 */
	static constexpr uint8_t NV3007_VENDCTRL6    = 0xB0; /**< GOA VEND control 6 */
	// static constexpr uint8_t NV3007_VENDCTRL9    = 0xB3; /**< GOA VEND control 9 */
	// static constexpr uint8_t NV3007_VENDCTRL10   = 0xB4; /**< GOA VEND control 10 */
	// static constexpr uint8_t NV3007_VENDCTRL11   = 0xB5; /**< GOA VEND control 11 */
	static constexpr uint8_t NV3007_VENDCTRL12   = 0xB6; /**< GOA VEND control 12 */
	// static constexpr uint8_t NV3007_VENDCTRL13   = 0xB7; /**< GOA VEND control 13 */
	// static constexpr uint8_t NV3007_VENDCTRL14   = 0xB8; /**< GOA VEND control 14 */

	// Private: GOA CLK
	static constexpr uint8_t NV3007_CLKCTRL1     = 0xB9; /**< GOA CLK control 1 */
	// static constexpr uint8_t NV3007_CLKCTRL2     = 0xBA; /**< GOA CLK control 2 */
	// static constexpr uint8_t NV3007_CLKCTRL3     = 0xBB; /**< GOA CLK control 3 */
	// static constexpr uint8_t NV3007_CLKCTRL4     = 0xBC; /**< GOA CLK control 4 */
	// static constexpr uint8_t NV3007_CLKCTRL5     = 0xBD; /**< GOA CLK control 5 */
	// static constexpr uint8_t NV3007_CLKCTRL6     = 0xBE; /**< GOA CLK control 6 */
	// static constexpr uint8_t NV3007_CLKCTRL7     = 0xBF; /**< GOA CLK control 7 */
	static constexpr uint8_t NV3007_CLKCTRL8     = 0xC0; /**< GOA CLK control 8 */
	// static constexpr uint8_t NV3007_CLKCTRL9     = 0xC1; /**< GOA CLK control 9 */
	// static constexpr uint8_t NV3007_CLKCTRL10    = 0xC2; /**< GOA CLK control 10 */
	// static constexpr uint8_t NV3007_CLKCTRL11    = 0xC3; /**< GOA CLK control 11 */
	static constexpr uint8_t NV3007_CLKCTRL12    = 0xC4; /**< GOA CLK control 12 */
	// static constexpr uint8_t NV3007_CLKCTRL13    = 0xC5; /**< GOA CLK control 13 */
	// static constexpr uint8_t NV3007_CLKCTRL14    = 0xC6; /**< GOA CLK control 14 */
	// static constexpr uint8_t NV3007_CLKCTRL15    = 0xC7; /**< GOA CLK control 15 */
	static constexpr uint8_t NV3007_CLKCTRL16    = 0xC8; /**< GOA CLK control 16 */
	// static constexpr uint8_t NV3007_CLKCTRL17    = 0xC9; /**< GOA CLK control 17 */
	// static constexpr uint8_t NV3007_CLKCTRL18    = 0xCA; /**< GOA CLK control 18 */
	// static constexpr uint8_t NV3007_CLKCTRL19    = 0xCB; /**< GOA CLK control 19 */
	// static constexpr uint8_t NV3007_CLKCTRL20    = 0xCC; /**< GOA CLK control 20 */
	static constexpr uint8_t NV3007_CLKCTRL21    = 0xCD; /**< GOA CLK control 21 */
	// static constexpr uint8_t NV3007_CLKCTRL22    = 0xCE; /**< GOA CLK control 22 */
	// static constexpr uint8_t NV3007_CLKCTRL23    = 0xCF; /**< GOA CLK control 23 */
	// static constexpr uint8_t NV3007_CLKCTRL24    = 0xD0; /**< GOA CLK control 24 */
	// static constexpr uint8_t NV3007_CLKCTRL25    = 0xD1; /**< GOA CLK control 25 */

	// Private: GOA RST
	// static constexpr uint8_t NV3007_RSTCTRL1     = 0xD2; /**< GOA RST control 1 */
	// static constexpr uint8_t NV3007_RSTCTRL2     = 0xD3; /**< GOA RST control 2 */
	// static constexpr uint8_t NV3007_RSTCTRL3     = 0xD4; /**< GOA RST control 3 */
	// static constexpr uint8_t NV3007_RSTCTRL4     = 0xD5; /**< GOA RST control 4 */
	// static constexpr uint8_t NV3007_RSTCTRL5     = 0xD6; /**< GOA RST control 5 */

	// Private: read/write ID
	// static constexpr uint8_t NV3007_RDID1        = 0xDA; /**< Read ID1 */
	// static constexpr uint8_t NV3007_RDID2        = 0xDB; /**< Read ID2 */
	// static constexpr uint8_t NV3007_RDID3        = 0xDC; /**< Read ID3 */
	// static constexpr uint8_t NV3007_WRID1        = 0xDD; /**< Write ID1 */
	// static constexpr uint8_t NV3007_WRID2        = 0xDE; /**< Write ID2 */
	// static constexpr uint8_t NV3007_WRID3        = 0xDF; /**< Write ID3 */

	// Private: source control
	static constexpr uint8_t NV3007_SOUCTRL1     = 0xE0; /**< Source control 1 */
	static constexpr uint8_t NV3007_SOUCTRL2     = 0xE1; /**< Source control 2 */
	static constexpr uint8_t NV3007_SOUCTRL3     = 0xE2; /**< Source control 3 */
	// static constexpr uint8_t NV3007_SOUCTRL4     = 0xE3; /**< Source control 4 */
	// static constexpr uint8_t NV3007_SOUCTRL5     = 0xE4; /**< Source control 5 */
	// static constexpr uint8_t NV3007_SOUCTRL6     = 0xE5; /**< Source control 6 */
	// static constexpr uint8_t NV3007_SOUCTRL7     = 0xE6; /**< Source control 7 */
	// static constexpr uint8_t NV3007_SOUCTRL8     = 0xE7; /**< Source control 8 */
	// static constexpr uint8_t NV3007_SOUCTRL9     = 0xE8; /**< Source control 9 */
	// static constexpr uint8_t NV3007_SOUCTRL10    = 0xE9; /**< Source control 10 */
	// static constexpr uint8_t NV3007_SOUCTRL11    = 0xEA; /**< Source control 11 */
	// static constexpr uint8_t NV3007_SOUCTRL12    = 0xEB; /**< Source control 12 */
	static constexpr uint8_t NV3007_SOUCTRL13    = 0xEC; /**< Source control 13 */
	// static constexpr uint8_t NV3007_SOUCTRL14    = 0xED; /**< Source control 14 */
	// static constexpr uint8_t NV3007_SOUCTRL15    = 0xEE; /**< Source control 15 */
	// static constexpr uint8_t NV3007_SOUCTRL16    = 0xEF; /**< Source control 16 */
	// static constexpr uint8_t NV3007_SOUCTRL17    = 0xF0; /**< Source control 17 */
	// static constexpr uint8_t NV3007_SOUCTRL18    = 0xF1; /**< Source control 18 */
	static constexpr uint8_t NV3007_SOUCTRL19    = 0xF2; /**< Source control 19 */

	// Private: test, FSM, pad, state
	// static constexpr uint8_t NV3007_CPTEST1      = 0xF4; /**< CP test 1 */
	// static constexpr uint8_t NV3007_CPTEST2      = 0xF5; /**< CP test 2 */
	static constexpr uint8_t NV3007_FSMCTRL      = 0xF9; /**< FSM Control */
	// static constexpr uint8_t NV3007_PADCTRL      = 0xFB; /**< Pad control */
	// static constexpr uint8_t NV3007_RDSTATE      = 0xFC; /**< RDSTATE */
}; // end of class

//  === EOF ===
