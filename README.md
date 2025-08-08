System Overview
This project is a gas leak detection and monitoring system using:
•	MQ 2 gas sensor → detects gas concentration (LPG, CO, methane, etc.).
•	ESP32 microcontroller → reads the sensor, processes values, drives outputs, and hosts a web dashboard.
•	LED indicators (Red, Yellow, Green) → show safety levels.
•	Buzzer → gives audio alarms for warning/danger.
•	WiFi + Web Server → allows you to monitor gas levels remotely on a web page and control the alarm.
________________________________________
⚡ How It Works
1. Gas Measurement
•	The MQ 2 sensor outputs an analog voltage on pin A0 proportional to the gas concentration.
•	This is connected to ESP32 ADC pin 32.
•	analogRead(mq2Pin) gives values from 0–4095 (ESP32 ADC range).
•	These are mapped to PPM values (map(sensorValue, 0, 4095, 0, 100000)), simulating gas concentration in parts per million.
________________________________________
2. Safety Levels
The system defines three safety thresholds:
•	Safe: gas < 300 ppm
o	✅ Green LED ON, no alarm.
•	Warning: 300 ppm ≤ gas < 800 ppm
o	⚠️ Yellow LED ON, buzzer beeps.
•	Danger: gas ≥ 800 ppm
o	🚨 Red LED ON, buzzer sounds continuously.
This ensures workers, students, or homeowners know the severity of the gas leak at a glance.
________________________________________
3. Alarm Handling
•	In Warning mode, the buzzer toggles ON/OFF every 0.5s to create a beeping sound.
•	In Danger mode, the buzzer stays ON continuously.
•	The web interface includes a "Silence Alarm" button, which allows a user to mute the buzzer temporarily while still seeing alerts.
________________________________________
4. WiFi & Web Dashboard
•	On startup, the ESP32 connects to the configured WiFi (TECNO POP 8).
•	It hosts a web server on port 80.
•	If you visit the ESP32’s IP in a browser, you’ll see a dashboard:
o	✅ Green “Air is Safe”
o	⚠️ Yellow “Warning”
o	🚨 Red “Danger”
o	Current gas level in ppm
o	A button to silence alarm if levels are high.
There’s also a /data endpoint that serves JSON (gas level, status, alarm state), useful for IoT integrations or mobile apps.
________________________________________
5. Continuous Monitoring
•	Every 10 seconds the ESP32 reads the gas sensor and updates LEDs, buzzer, and the web interface.
•	WiFi connection is checked every 5 seconds. If disconnected, it keeps monitoring locally but without web updates.
________________________________________
✅ Summary in Simple Words
•	The MQ 2 sensor senses gases → sends values to ESP32.
•	ESP32 calculates PPM → decides if air is safe, warning, or dangerous.
•	LEDs + buzzer give instant local alerts.
•	If WiFi is connected, ESP32 also provides a web dashboard with live gas readings and alarm control.
So it’s both a real time alarm system and a remote monitoring tool.
THAT IS THE LINK FOR MY GAS MONITORING  SYSTEM
