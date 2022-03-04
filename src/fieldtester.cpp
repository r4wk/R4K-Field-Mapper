/**
 * @file fieldtester.cpp
 * @author r4wk (r4wknet@gmail.com)
 * @brief RAK FieldTester/Mapper
 * @version 0.1
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <app.h>
#include <U8g2lib.h>
#include <algorithm>
#include <iostream>
#include <sstream>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2);
// Vector string array for display. MAX 9 lines Y. MAX 32 characters X.
std::vector<std::string> displayBuffer;

/**
 * @brief Redraw display.
 * 
 */
void refreshDisplay(void)
{
    u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_micro_mr);
	u8g2.drawStr(0, 5, "R4K FTester v0.1");
    if(g_join_result) {u8g2.drawStr(98, 5, "(H)");}
    if(ftester_gps_fix) {u8g2.drawStr(110, 5, "(GPS)");}
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
 * @brief Initialize Display here
 * 
 */
void display_init(void)
{
    u8g2.begin();
    refreshDisplay();
    ftester_gps_event();
}

/**
 * @brief Field Tester GPS event handler
 * 
 */
void ftester_gps_event(void)
{
    if(ftester_gps_fix) 
    {
        sendToDisplay("GPS satellites found!");
    } else {
        sendToDisplay("Searching for GPS satellites.");
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

        sendToDisplay(hsNameUP);
        sendToDisplay(combined);
    }
}