## R4K Field Mapper - Create Helium Field Mapper out of RAK WisBlock modules.🗺️

## About
- Runs as a mapper and field tester.
- Screen time out is 5 minutes. Accelerometer will trigger a screen wake (and also uplink).
- Keeps track of RX/TX beacons. Will reset after one hits 999 (screen size is limited).
- Displays hot spot name that was chosen to handle the downlink. Will also display how many other hot spots heard the beacon. (+#).
- Also displays signal quality of the hot spot chosen to handle the testers downlink and the field tester. (RSSI/SNR).
- Will also displays distance to hot spot in KM.
- Show's how many satellites you have a fix on. Will only send a beacon when you have a good GPS fix (usually 4 or more satellites). 

![r4k_oled_info](https://user-images.githubusercontent.com/5049300/180697646-bcd53024-a1a1-4088-8c95-0b9183925b27.png)

## What's needed
- RAK5005-O
- RAK4631
- RAK1904
- RAK1910/RAK12500
- SSD1306 OLED
- 550mAh 3.7v Battery (can use any size if not using my case)
- mxuteuk Black Self-Lock Push Button Switch DC 30V 1A (OPTIONAL)
  - https://www.amazon.ca/gp/product/B086L2GPGX/ref=ppx_od_dt_b_asin_title_s00
 - DIYmalls 915MHz LoRa Antenna U.FL IPEX to SMA Connector Pigtail (OPTIONAL)
   - https://www.amazon.ca/gp/product/B086ZG5WBR/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
   
## No-Code/Easy setup

If you don't want to make any custom changes to the firmware, follow these very easy instructions to flash your device with the R4K firmware.
- Download the latest release from: https://github.com/r4wk/R4K-Field-Mapper/releases
- Download this flash utility: https://github.com/adafruit/Adafruit_nRF52_nrfutil/releases
- Extract the flash utility and copy the latest R4K firmware to the same folder to make life easier
- Next you want to run the following command (In Windows open CMD app, navigate to the folder where you extracted the flash utility/firmware i.e. 'cd c:\adafruit')
  - adafruit-nrfutil dfu serial --package R4K-Field-Mapper_\<version\>.zip -p \<port\> -b 115200
- Make sure to change \<version\> to the latest version of the R4K firmware that you downloaded from releases (i.e. R4K-Field-Mapper_v0.3a.zip)
- Make sure to change \<port\> to the COM port that your device is connected on: https://www.mathworks.com/help/supportpkg/arduinoio/ug/find-arduino-port-on-windows-mac-and-linux.html

## Code/PlatformIO

If you'd like to make changes to the base firmware (not needed). You can follow this guide:

https://github.com/RAKWireless/WisBlock/tree/master/PlatformIO/RAK4630

## Set up LoRa credentials/settings
- I highly advise using WisBlock-ToolBox app, this allows you do connect your device to Helium right from your phone via Bluetooth (Android Only)
  - https://play.google.com/store/apps/details?id=tk.giesecke.wisblock_toolbox&hl=en&gl=US
- Once you have that installed follow these instructions to get you connected to Helium LoRaWAN network
  - https://github.com/rakstars/WisBlock-RAK4631-Helium-Mapper/wiki/Make-a-Helium-Mapper-with-the-WisBlock#using-the-wisblock-toolbox-lpwan-setup-module
  
- If you don't want to use a phone or only have an iOS device, you can still setup the device with AT commands
- Over Serial/USB to run AT commands
  - https://github.com/rakstars/WisBlock-RAK4631-Helium-Mapper/wiki/Make-a-Helium-Mapper-with-the-WisBlock#setting-up-the-lorawan-parameters
- Over Bluetooth to run AT commands
  - https://github.com/rakstars/WisBlock-RAK4631-Helium-Mapper/wiki/Make-a-Helium-Mapper-with-the-WisBlock#over-ble
  
## Setting up your device

As this project is a custom version of the RAK mapper firmware you build it in the same way. If you're not sure on how that's done click the following link:

https://github.com/rakstars/WisBlock-RAK4631-Helium-Mapper/wiki/Make-a-Helium-Mapper-with-the-WisBlock

Before proceeding make sure you have built the RAK Mapper, as this project uses that as a base and just adds an OLED.

Once you have the R4K field mapper firmware flashed to the WisBlock/RAK Mapper and have an OLED soldered onto the device, all that is left is the intergration. 

- To create an HTTP integration in a Helium Console:
  Click on Integrations, select Add New Integration, select HTTP. Then, copy and paste the following Endpoint URL:

  ```http://r4wk.net/lora/ingest.php```

- Finally, give a name to the integration and click on the Add Integration button.

![image](https://user-images.githubusercontent.com/5049300/180680303-f1ae971e-3530-4046-ab12-2a1864c988fd.png)

- Next, click on Flows, grab the device, the decoder (from the RAK mapper wiki), and the integration from the node section tabs. Then use the connection points to specify how the data flows from the device to the integration.

![image](https://user-images.githubusercontent.com/5049300/180680481-daaa9f0d-5440-42de-b955-56d24a4bd4af.png)

- I advise enabling muilti-buy/packet so that you can uplink to multiple hot spots. But with how downlinks work the RX wait time on the router isn't long enough to catch as many hot spots as I wish. Fortunately I've requested this to be adjustable so hopefully we will see that feature soon. 
https://github.com/helium/router/issues/781

- If you choose to use the RAK12500, you will lose some battery life as it consumes about twice as much as the RAK1910 during sleep. I've also found when using the RAK12500 setting RX window delay to 2 seconds makes downlinks much more reliable vs the RAK1910. 

![R4K_Front](https://user-images.githubusercontent.com/5049300/192627451-c3073dc9-255d-479a-8bbe-1ba905e77733.png)
![R4K_Side](https://user-images.githubusercontent.com/5049300/192627468-149abcb7-0308-4637-87a7-dd6e1e1a7511.png)


