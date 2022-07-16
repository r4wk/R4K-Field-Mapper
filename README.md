R4K Field Mapper - Create Helium Field Tester out of RAK WisBlock modules.🗺️

About
- Runs as a mapper and field tester.
- Screen time out is 5 minutes. Accelerometer will trigger a screen wake.
- Keeps track of RX/TX beacons. Will reset after one hits 999 (screen size is limited).
- Displays hot spot name that was chosen to handle the downlink. Will also display how many other hot spots heard the beacon. (+#).
- Also displays signal quality of the hot spot chosen to handle the testers downlink and the field tester. (RSSI/SNR).
- Will also displays distance to hot spot in KM.
- Show's how many satellites you have a fix on. Will only send a beacon when you have a good GPS fix (usually 4 or more satellites). 

What's needed
- RAK5005-O
- RAK4631
- RAK1904
- RAK1910
- SSD1306 OLED
- mxuteuk 50pcs Black Self-Lock Push Button Switch DC 30V 1A (OPTIONAL)
  - https://www.amazon.ca/gp/product/B086L2GPGX/ref=ppx_od_dt_b_asin_title_s00

Create an HTTP integration in the Helium Console and point it to the following endpoint 
http://r4wk.net/lora/ingest.php
