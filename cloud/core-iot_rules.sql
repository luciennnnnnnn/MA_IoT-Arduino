-- Rule for LoRaWan message data processing

SELECT uplink_message.decoded_payload.digital_in_1 AS is_moving, 
uplink_message.decoded_payload.digital_in_2 AS door_state,
uplink_message.decoded_payload.temperature_3 AS temperature, 
uplink_message.decoded_payload.relative_humidity_4 AS humidity,
uplink_message.decoded_payload.gps_5.latitude AS latitude, 
uplink_message.decoded_payload.gps_5.longitude AS longitude, 
FROM 'lorawan/+/uplink'

-- Rule for WiFi message data processing
SELECT is_moving AS is_moving, 
door AS door_state, 
temperature AS temperature, 
humidity AS humidity, 
latitude AS latitude, 
longitude AS longitude, 
FROM 'publish/+/uplink'