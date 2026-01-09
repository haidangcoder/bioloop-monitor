#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

// Send sensor data to Node.js server
void sendDataToServer(float temperature, float moisture, float temp_error, int fan_pwm, bool pump);

#endif
