-- This file contains the queries used in Grafana to display the data from the database

-- Query for the temperature data
SELECT measure_value::bigint as Temperature, time
FROM "hivesData"."table" 
WHERE measure_name = 'temperature' AND deviceID = '$deviceID'
ORDER BY time

-- Query for last temperature data
SELECT measure_value::bigint as Temperature, time, deviceID
FROM "hivesData"."table" 
WHERE measure_name = 'temperature' AND deviceID = '$deviceID'
ORDER BY time DESC 
limit 1

-- Query for the humidity data
SELECT measure_value::bigint as Humidity, time, deviceID
FROM "hivesData"."table" 
WHERE measure_name = 'humidity' AND deviceID = '$deviceID'
ORDER BY time

-- Query for the last humidity data
SELECT measure_value::bigint as Humidity, time, deviceID
FROM "hivesData"."table" 
WHERE measure_name = 'humidity' AND deviceID = '$deviceID'
ORDER BY time DESC
limit 1

-- Query for the moving status
SELECT measure_value::bigint, time,
  CASE 
    WHEN measure_value::bigint = 0 THEN 'Not moving'
    WHEN measure_value::bigint = 1 THEN 'Moving'
    ELSE 'No data'
  END AS moving_status
FROM "hivesData"."table" 
WHERE measure_name = 'is_moving' AND deviceID = '$deviceID'
ORDER BY time DESC 
limit 1

-- Query for the door status
SELECT
  CASE 
    WHEN measure_value::bigint = 0 THEN 'Closed'
    WHEN measure_value::bigint = 1 THEN 'Open'
    ELSE 'No data'
  END AS door_status
FROM "hivesData"."table" 
WHERE measure_name = 'door_state' AND deviceID = '$deviceID'
ORDER BY time DESC 
limit 1