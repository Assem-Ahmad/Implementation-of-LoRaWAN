#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "LoRa_Gateway"                        
 
const char WIFI_SSID[] = "Enter your network";               
const char WIFI_PASSWORD[] = "Enter your password";          
const char AWS_IOT_ENDPOINT[] = "Enter your-aws-iot-endpoint.amazonaws.com";      
 //change this with the actual AWS IoT Core endpoint associated with your AWS account
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
 
 
)KEY";
 
// Device Private Key                                              
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
 
 
)KEY";
