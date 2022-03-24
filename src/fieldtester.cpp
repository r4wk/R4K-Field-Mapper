/**
 * @file fieldtester.cpp
 * @author r4wk (r4wknet@gmail.com)
 * @brief RAK FieldTester/Mapper
 * @version 0.2a
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * TODO: Detect display so can still work without working/connected display
 *       Battery level indicator (Needs tweaking)
 *       Use proper icons?
 *       Visual for direction of newest line (?)
 *       Add more to MYLOG()
 *       Use TinyGPS to calculate distance between tester and hotspot
 * 
 */

#include <app.h>
#include <U8g2lib.h>
#include <algorithm>
#include <iostream>
#include <sstream>

// Instance for display object (RENDER UPSIDE DOWN)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2);
// Vector string array for display. MAX 9 lines Y. MAX 32 characters X.
std::vector<std::string> displayBuffer;
// Timer to put display to sleep, mostly to save burn in
SoftwareTimer displayTimeoutTimer;
// Bool to display GPS fix once per status change
bool once = true;
// Bool to keep track if display is on/off
bool displayOn = true;
// Vars to keep track of beacons
int32_t txCount = 0;
int32_t rxCount = 0;

/**
 * @brief Redraw info bar and display buffer with up to date info.
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
    u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_micro_mr);

	u8g2.drawStr(0, 5, "R4K v0.2a");

    if(ftester_gps_fix) {u8g2.drawStr(46, 5, "(GPS)");}
    if(g_join_result) {u8g2.drawStr(66, 5, "(H)");}

    std::string rxc = std::to_string(rxCount).c_str();
    std::string txc = std::to_string(txCount).c_str();
    u8g2.drawStr(78, 5,  (rxc + "/" + txc).c_str());

    int battLevel = mv_to_percent(read_batt());
    std::string battString = std::to_string(battLevel) + "%";
    u8g2.drawStr(110, 5,  battString.c_str());

    // This is probably all wrong
    // and highly inaccurate, plz forgive
    // if(battLevel > 370)
    // {
    //     // Batt greater than 3.7v?
    //     u8g2.setFont(u8g2_font_siji_t_6x10);
    //     u8g2.drawGlyph(118, 6, 0xe086);
    // } else if(battLevel < 330) {
    //     // Batt less than 3.3v?
    //     u8g2.setFont(u8g2_font_siji_t_6x10);
    //     u8g2.drawGlyph(118, 6, 0xe085);
    // } else if (battLevel < 290) {
    //     // Batt lass than 2.9v?
    //     u8g2.setFont(u8g2_font_siji_t_6x10);
    //     u8g2.drawGlyph(118, 6, 0xe084);
    // }
    // u8g2.setFont(u8g2_font_micro_mr);

    u8g2.drawLine(0, 6, 128, 6);

    int size = displayBuffer.size();
    for (int y = 0; y < size; y++) 
    {
        u8g2.drawStr(0, 13 + (y*6), displayBuffer[y].c_str());
    }
    u8g2.sendBuffer();
}

/**
 * @brief Sends text to display.
 * Went want to refresh everytime info is added
 * to the display so we have the most up to date
 * info.
 * 
 * @param s String to send.
 * 
 */
void sendToDisplay(std::string s)
{
    int size = displayBuffer.size();
    if(size < 9)
    {
        if(s.length() < 32)
        {
            displayBuffer.push_back(s);
        } else {
            s.resize(30);
            displayBuffer.push_back(s + "..");
        }
    } else {
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
    displayOn = false;
}

/**
 * @brief Initialize Display here
 * 
 */
void display_init(void)
{
    u8g2.begin();
    displayTimeoutTimer.begin(30000, ftester_display_sleep);
    displayTimeoutTimer.start();
    sendToDisplay("Trying to join Helium Netowrk.");
}

/**
 * @brief Field Tester GPS event handler
 * 
 * This needs to be completely reworked again.
 * Info for sat's found is very slow and behind, but does work.
 * 
 */
void ftester_gps_event(void)
{
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
        } else {
            sendToDisplay("Lost Helium Network!");
        }
    }
}

/**
 * @brief Field Testers LoRa Data handler
 * Parse incoming(RX) JSON data from LLIS
 * 
 */
void ftester_lora_data_handler(void)
{
    char log_buff[g_rx_data_len * 3] = {0};
    uint8_t log_idx = 0;
    for (int idx = 0; idx < g_rx_data_len; idx++)
    {
        sprintf(&log_buff[log_idx], "%02X ", g_rx_lora_data[idx]);
        log_idx += 3;
    }

    std::string jsonString;
    std::string hexString = std::string(log_buff);
    std::string::iterator end_pos = std::remove(hexString.begin(), hexString.end(), ' ');
    hexString.erase(end_pos, hexString.end());

    size_t len = hexString.length();
    for(int i = 0; i < len; i+=2)
    {
        std::string byte = hexString.substr(i,2);
        char chr = (char) (int)strtol(byte.c_str(), nullptr, 16);
        jsonString.push_back(chr);
    }

    RSJresource json_Obj (jsonString);

    size_t jsonlen = json_Obj.as_array().size();
    for(int j = 0; j < jsonlen; j++)
    {
        std::string hsNameUP = json_Obj[j]["name"].as_str();
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

            std::string txrssi = json_Obj[j]["rssi"].as_str();
            std::string txsnr = json_Obj[j]["snr"].as_str();
            txsnr.resize(4);

            hsNameUP.erase(
                remove( hsNameUP.begin(), hsNameUP.end(), '\"' ),
                hsNameUP.end()
            );

            std::string rxsnr = std::to_string(g_last_snr);
            rxsnr.resize(4);

            std::string txsnrc = txsnr.c_str();
            std::string rxsnrc = rxsnr.c_str();
            std::string combined = (std::string("RSSI:") + std::to_string(g_last_rssi) + "/" + txrssi) + (" SNR: " + rxsnrc + "/" + txsnrc);

            sendToDisplay(std::to_string(rxCount) + "." + hsNameUP);
            sendToDisplay(combined);
        } else {
            // Bad/Damaged packet. Dropping info, network issues.
            MYLOG("R4K", "Dropped packet: %s", hexString);
            // DEBUG
            sendToDisplay("!EMPTY!");
        }
    }
}

/**
 * @brief Mapper is sending beacon(packet)
 * Keep count of how many beacons sent.
 * Max of 999 because of screen limitations.
 * 
 * TODO: Store GPS co-ords on TX. Wait for RX to send back HS co-ords.
 *       Get distance to hot spot from beacon co-ords.
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
        displayTimeoutTimer.start();
    } else {
        u8g2.setPowerSave(false);
        displayOn = true;
        displayTimeoutTimer.start();
        refreshDisplay();
    }
}