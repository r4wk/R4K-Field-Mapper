/**
 * @file fieldtester.cpp
 * @author r4wk (r4wknet@gmail.com)
 * @brief RAK FieldTester/Mapper
 * @version 0.2a
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * TODO: Detect display so can still work without working/connected display
 *       Use proper icons?
 *       Add more to MYLOG()
 *       Add counter for how many hot spots heard beacon. Use JSON array to count this
 *       Add RAK12500_GNSS compatibility
 *       Kill some logic when display is off
 * 
 */

#include <app.h>
#include <U8g2lib.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ArduinoJson.h>
#include <vector>

// Instance for display object (RENDER UPSIDE DOWN)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2);
// Vector string array for display. MAX 9 lines Y. MAX 32 characters X
std::vector<std::string> displayBuffer;
// Timer to put display to sleep, mostly to save burn in
SoftwareTimer displayTimeoutTimer;
// Timer to update battery level
SoftwareTimer battTimer;
// Bool to display GPS fix once per status change
bool once = true;
// Bool to keep track if display is on/off
bool displayOn = true;
// Vars to keep track of beacons
int32_t txCount = 0;
int32_t rxCount = 0;
// Var to hold battery level, so I'm not constantly polling
int8_t battLevel = 0;
// Vars for field tester lat/long
double ftester_lat = 0;
double ftester_long = 0;
// Var for GPS sat count
int8_t ftester_satCount = 0;

/**
 * @brief Redraw info bar and display buffer with up to date info
 * TODO: Space out icons better
 * 
 * Firmware version
 * GPS fix status
 * Helium network status
 * Beacon RX/TX Count
 * Battery level indicator
 * Display buffer
 * 
 */
void refreshDisplay(void)
{
    // Clear screen and set font
    u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_micro_mr);

    // Draw firmware version
	u8g2.drawStr(0, 5, "R4K v0.2a");

    // Draw GPS sat fix count
    u8g2.drawStr(38, 5, "(GPS)");
    u8g2.drawStr(58, 5, std::to_string(ftester_satCount).c_str());

    // Draw Helium Join Status
    // RX/TX Count
    if(g_join_result) 
    {
        u8g2.drawStr(68, 5, "(H)");
        std::string rxc = std::to_string(rxCount).c_str();
        std::string txc = std::to_string(txCount).c_str();
        u8g2.drawStr(80, 5,  (rxc + "/" + txc).c_str());
    }

    // Draw battery % based on mv
    std::string battString = std::to_string(battLevel) + "%";
    u8g2.drawStr(110, 5,  battString.c_str());

    u8g2.drawLine(0, 6, 128, 6);

    // Draw our display buffer
    int size = displayBuffer.size();
    for (int y = 0; y < size; y++) 
    {
        u8g2.drawStr(0, 13 + (y*6), displayBuffer[y].c_str());
    }
    u8g2.sendBuffer();
}

/**
 * @brief Sends text to display
 * We want to refresh everytime info is added
 * to the display so we have the most up to date
 * info.
 * 
 * @param s String to send.
 * 
 */
void sendToDisplay(std::string s)
{
    // Get display buffer size
    int size = displayBuffer.size();
    // MAX 9 lines Y
    if(size < 9)
    {
        // MAX 32 characters X
        if(s.length() < 32)
        {
            // Push the newest data to the back of array
            displayBuffer.push_back(s);
        } else {
            // If text too long, resize then push to back
            s.resize(30);
            displayBuffer.push_back(s + "..");
        }
    } else {
        // Erase oldest data and push newest to the back of array
        // Shouldn't I resize X here too? 
        displayBuffer.erase(displayBuffer.begin());
        displayBuffer.push_back(s);
    }
    refreshDisplay();
}

/**
 * @brief Put display into Power Saver mode
 * Mostly to save burn in
 * 
 * @param unused 
 * 
 */
void ftester_display_sleep(TimerHandle_t unused)
{
    u8g2.setPowerSave(true);
    displayTimeoutTimer.stop();
    battTimer.stop();
    displayOn = false;
}

/**
 * @brief Get battery level from API
 * 
 * @return float 
 */
float ftester_getBattLevel()
{
    if(displayOn)
    {
        return mv_to_percent(read_batt());
    }
}

/**
 * @brief Used to update battery level in a timer
 * 
 * @param unused 
 */
void ftester_updateBattLevel(TimerHandle_t unused)
{
    if(displayOn)
    {
        // Batt level bouncies around a lot
        // Smooth this out
        battLevel = ftester_getBattLevel();
        refreshDisplay();
    }
}

/**
 * @brief Initialize Display here
 * 
 */
void display_init(void)
{
    u8g2.begin();
    // Creater timer for display and start it (30 seconds)
    displayTimeoutTimer.begin(30000, ftester_display_sleep);
    displayTimeoutTimer.start();
    battTimer.begin(19500, ftester_updateBattLevel, NULL, true);
    battTimer.start();
    sendToDisplay("Trying to join Helium Netowrk.");
}

/**
 * @brief Field Tester GPS event handler
 * 
 * This needs to be completely reworked again
 * Info for sat's found is very slow and behind, but does work
 * 
 */
void ftester_gps_event(void)
{
    ftester_satCount = my_rak1910_gnss.satellites.value();

    if(ftester_gps_fix) 
    {
        if(once) 
        {
            sendToDisplay("GPS satellites found!");
            once = false;
        }
    } else {
        if(!once)
        {
            sendToDisplay("Lost GPS fix.");
        } else {
            sendToDisplay("Searching for GPS satellites.");
        }
        once = true;
    }
}

/**
 * @brief Field Testers event handler
 * 
 */
void ftester_event_handler(void)
{
    if((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
    {
        g_task_event_type &= N_LORA_JOIN_FIN;
        if(g_join_result)
        {
            sendToDisplay("Joined Helium Network!");
            // DEBUG TO MAKE SURE SCREEN TURNS ON
            if(displayOn)
            {
                displayTimeoutTimer.reset();
            } else {
                u8g2.setPowerSave(false);
                displayOn = true;
                displayTimeoutTimer.reset();
                battTimer.reset();
            }
        }
    }
}

/**
 * @brief Field Testers LoRa Data handler
 * Parse incoming(RX) JSON data from LLIS
 * Convert from HEX to string
 * Build JSON obj
 * Iterate through JSON obj
 * 
 */
void ftester_lora_data_handler(void)
{
    // Convert RX data buffer to char
    char log_buff[g_rx_data_len * 3] = {0};
    uint8_t log_idx = 0;
    for (int idx = 0; idx < g_rx_data_len; idx++)
    {
        sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
        log_idx += 3;
    }

    std::string jsonString;
    std::string hexString = std::string(log_buff);
    // Remove spaces
    std::string::iterator end_pos = std::remove(hexString.begin(), hexString.end(), ' ');
    hexString.erase(end_pos, hexString.end());

    // Convert from HEX to "text" to build JSON string
    size_t len = hexString.length();
    for(int i = 0; i < len; i+=2)
    {
        std::string byte = hexString.substr(i,2);
        char chr = (char) (int)strtol(byte.c_str(), nullptr, 16);
        jsonString.push_back(chr);
    }

    DynamicJsonDocument jsonObj(256);
    DeserializationError error = deserializeJson(jsonObj, jsonString);

    if(error)
    {
        MYLOG("R4K", "Deserialize error, bad data?");
    } else {
        // If there's no name, the packet got damaged
        std::string hsNameUP = jsonObj["name"];
        if(!hsNameUP.empty())
        {
            // RX Counter
            if(rxCount < 999)
            {
                rxCount++;
                refreshDisplay();
            } else {
                rxCount = 0;
                txCount = 0;
                sendToDisplay("Resetting RX/TX Beacon Count!");
            }

            // Get our TX signal quality
            std::string txrssi = jsonObj["rssi"];
            std::string txsnr = jsonObj["snr"];

            // Get hot spot lat/long
            std::string hsLat = jsonObj["lat"];
            std::string hsLong = jsonObj["long"];
            double hsLatD = std::stod(hsLat);
            double hsLongD = std::stod(hsLong);
            double distM = my_rak1910_gnss.distanceBetween(ftester_lat, ftester_long, hsLatD, hsLongD);
            // Round and set precision to display 1 decimal place only
            double distKM = round((distM / 1000.0) * 10.0) / 10.0;
            std::ostringstream ossDist;
            ossDist << std::setprecision(2) << std::noshowpoint << distKM;
            std::string distKMS = "";
            if(distKM <= 0)
            {
                distKMS = "<0.1km";
            } else {
                distKMS = " " + ossDist.str() + "km";
            }

            // Clean up hot spot name for human readable
            hsNameUP.erase(
                remove( hsNameUP.begin(), hsNameUP.end(), '\"' ),
                hsNameUP.end()
            );

            // Get our RX signal quality
            // Round and set precision to display 1 decimal place only
            std::ostringstream ossSnr;
            double rxsnrd = round(g_last_snr * 10.0) / 10.0;
            ossSnr << std::setprecision(2) << std::noshowpoint << rxsnrd;
            std::string rxsnr = ossSnr.str();
            std::string rxrssi = std::to_string(g_last_rssi);

            // Build signal info/string for display
            std::string txsnrc = txsnr.c_str();
            std::string rxsnrc = rxsnr.c_str();
            std::string combined = (std::string("RSSI:") + rxrssi + "/" + txrssi) + (" SNR: " + rxsnrc + "/" + txsnrc);

            // Send everything to the display
            sendToDisplay(std::to_string(rxCount) + "." + hsNameUP  + distKMS);
            sendToDisplay(combined);
        } else {
            // Bad/Damaged packet. Dropping info, network issues
            MYLOG("R4K", "Damaged/Bad JSON Data");
        }
    }
}

/**
 * @brief Mapper is sending beacon(packet)
 * Keep count of how many beacons sent
 * Max of 999 because of screen limitations
 * 
 */
void ftester_tx_beacon(void)
{
    if(txCount < 999)
    {
        txCount++;
        refreshDisplay();
    } else {
        rxCount = 0;
        txCount = 0;
        sendToDisplay("Resetting RX/TX Beacon Count!");
    }
}

/**
 * @brief Field Tester accelerometer event
 * Turn off power saver mode and start timeout again
 * 
 */
void ftester_acc_event(void)
{
    if(displayOn)
    {
        // Keep display on
        displayTimeoutTimer.reset();
    } else {
        // Screen is off, wake up
        u8g2.setPowerSave(false);
        displayOn = true;
        displayTimeoutTimer.reset();
        battTimer.reset();
    }
}

/**
 * @brief Set GPS data for device
 * 
 * @param lat 
 * @param lon 
 */
void ftester_setGPSData(int64_t lat, int64_t lon)
{
    ftester_lat = lat / 100000.0;
    ftester_long = lon / 100000.0;
}