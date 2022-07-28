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
- RAK1910
- SSD1306 OLED
- mxuteuk Black Self-Lock Push Button Switch DC 30V 1A (OPTIONAL)
  - https://www.amazon.ca/gp/product/B086L2GPGX/ref=ppx_od_dt_b_asin_title_s00
  
## Setting up your device

As this project is a custom version of the RAK mapper firmware you flash it in the same way. If you're not sure on how that's done click the following link:

https://github.com/rakstars/WisBlock-RAK4631-Helium-Mapper/wiki/Make-a-Helium-Mapper-with-the-WisBlock

Once you have the R4K field mapper firmware flashed to the WisBlock and have an OLED soldered onto the device, all that is left is the intergration. 

- To create an HTTP integration in a Helium Console:
  Click on Integrations, select Add New Integration, select HTTP. Then, copy and paste the following Endpoint URL:

  ```http://r4wk.net/lora/ingest.php```

- Finally, give a name to the integration and click on the Add Integration button.

![image](https://user-images.githubusercontent.com/5049300/180680303-f1ae971e-3530-4046-ab12-2a1864c988fd.png)

- Next, click on Flows, grab the device, the decoder (from the RAK mapper wiki), and the integration from the node section tabs. Then use the connection points to specify how the data flows from the device to the integration.

![image](https://user-images.githubusercontent.com/5049300/180680481-daaa9f0d-5440-42de-b955-56d24a4bd4af.png)

- I advise enabling muilti-buy/packet so that you can uplink to multiple hot spots. But with how downlinks work the RX wait time on the router isn't long enough to catch as many hot spots as I wish. Fortunately I've requested this to be adjustable so hopefully we will see that feature soon. 
https://github.com/helium/router/issues/781
