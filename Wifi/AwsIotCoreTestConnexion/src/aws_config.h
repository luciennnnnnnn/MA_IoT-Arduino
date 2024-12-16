// Configuration WiFi
const char* ssid = "vmd-11209";
const char* password = "49vw-87kg-nrcb-zsxv";

// Endpoint AWS IoT Core
const char* aws_endpoint = "a2v2z8f3sbcm46-ats.iot.eu-west-1.amazonaws.com";

WiFiClientSecure net; // Objet pour gérer la connexion sécurisée

// Certificats et clé privée
const char* certificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAPP7vgs8GPa3D4C0bAsJytt3LO9tMA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDEyMTYxNDI2\n" \
"MDVaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDEgHzX0KyeTZ6DS1Ia\n" \
"RFpXtgCIOOEiQRkWm7UiXlJa+a5Dwdrc2eSc1MXQRUAM3/0YfMEtofr+VNUOFXXy\n" \
"im7yA+fowUCPhS7XEwx5acCfbwvcuKp9ziRAIk2/BsFDoPK5gGhK2ME/nERLQYbY\n" \
"/8i3rghrtCyJJw8G/SAVoBq6PRv2Tg6rKfvIW19vR9Yev5L2AP1Z6cauP7Ncccfp\n" \
"jxL+Rk4L4AQC/p4ims6bh8YzEuVPnsYxRmtKhJkQ4IHqjdhpwGlVpy/ylcwHjfsr\n" \
"dVjMjWrFV8KYEah9MBHK22hEeBBVaMGmdSyc/O/X/+ykt+1eKpjAFvek7gvZ7jUV\n" \
"yicFAgMBAAGjYDBeMB8GA1UdIwQYMBaAFK/mKozzzlv9R9hHftk243D9CxfXMB0G\n" \
"A1UdDgQWBBSKD48IzNzanz5479STYydV+vfHaDAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAPMtCJP/PJn74la7nzHLEaOQQ\n" \
"FCwiorAh/aR9sxJbZLQZvR1AW7DopZJkkm8Q2uWpS0kDEjqjq2RFy2G4ExGpG7kv\n" \
"BkwEdzTofa++fQ4PUBB1yLDoF1X9D+NVh2GIqBU3LJ41VgGaZhupvSoNSpT2eiJA\n" \
"Vx0Gix6I6DDTJkMnPe9vf2Zws+6KxgZOJa9WwpDFgm77qVmhVq5nrxeK7XdG0Wqx\n" \
"4CsptDDEvHemKf9uZsN/E5LjMnvi2VpKCGTqvBdOka7wDz3VwAILABgZSqZDbG7F\n" \
"WNglMsHKeXlP+N8WISIQ+QapgmTpO1WKG4tWJywObdDiVE7fQzZWbufxC3VHaQ==\n" \
"-----END CERTIFICATE-----\n";

const char* private_key = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEAxIB819Csnk2eg0tSGkRaV7YAiDjhIkEZFpu1Il5SWvmuQ8Ha\n" \
"3NnknNTF0EVADN/9GHzBLaH6/lTVDhV18opu8gPn6MFAj4Uu1xMMeWnAn28L3Liq\n" \
"fc4kQCJNvwbBQ6DyuYBoStjBP5xES0GG2P/It64Ia7QsiScPBv0gFaAauj0b9k4O\n" \
"qyn7yFtfb0fWHr+S9gD9WenGrj+zXHHH6Y8S/kZOC+AEAv6eIprOm4fGMxLlT57G\n" \
"MUZrSoSZEOCB6o3YacBpVacv8pXMB437K3VYzI1qxVfCmBGofTARyttoRHgQVWjB\n" \
"pnUsnPzv1//spLftXiqYwBb3pO4L2e41FconBQIDAQABAoIBAEGWrZUPpv5z5Avo\n" \
"8S+pTwplyomtip/3YJtmP7CIES8ileMx6o67AA7FCn35D2b8wKsSwRwtK+0GfdLk\n" \
"oOjjAiMv4JGVJ6qABuFtD0XbYZsV4ry6utWhIAJ0MMUpSQr8xufaD3WkIeTMkRMZ\n" \
"fRwjTGja4x6oFhF1gxOPG818Ek/FDeUlXlBcCbob30bthaHiuw2/wmOWe1gGomQC\n" \
"wlXmCP+CIRc9XCU35GKSJb/6N+6Z6v3dfmNo/+b+q0SyqKQwNYcr1Ze8YJ2zj9Eg\n" \
"w5KShi9TCmvlxhsV48RQm4VAzUd0JCEVkhxDZPg4NNxbndGwVuFYAEV+SL4gq+UC\n" \
"VCAwWQUCgYEA7//8cplWr2hTtCBkEmlNW/ubhf3BVNOzYw0EcQmtUxvbzgPPWONe\n" \
"HHLuR0EM1y1vSt8ovuHsWO8WYcbbdTETAmVT3YO+eLDUmTt9kYbj9FQ6hIhrV1Fj\n" \
"vwQzJ1J8F9QE+c25pxx43bhraq1pfuEGPwx+gfFbkRcs/xaqLmfXBHMCgYEA0Zoh\n" \
"3kme2Fns/myYClqpr9/tOr8wZ0qyY9K1P9XrGfF54eLwOT97Uc3uaXx9NIEGKGXC\n" \
"SwkYVUYu44QLGJrO8qBWQUcdDwEDLE5AaNrVPJyimUrb/PpybZhw26fwz0KXxQdX\n" \
"n15N+sz2Mux8lje23s7D9KoedmgB7iPexXoQwKcCgYB9iBCcRHYh41i8w6B0PN34\n" \
"ub8cM6E25FBVA88J7Pzalpy+wiFnJe3oxXi6jlzQA/TNXbIrPXJpBg1X8TKCtrtx\n" \
"qlxWZf1vlVZuG77PKpX7Cz9caRp0Hp8kn2a+L5e2wvAjVSbBMNRLbJXKw4+FjN++\n" \
"hKN7BipV1qBzgIBdK3U83wKBgAgAgGaCHQRZeojEEJHkoDHMtLvXbLukyGazNqQ3\n" \
"QIB/DY4oIc+tssGOwRlZ3VXRzzBfoilHMZyfz050ThAkaN4ubNyVIHDgMwTP//mA\n" \
"0T89I6ACkZ942EGC3lwkbzcabbzh6OB2szdt98A98wuxGNQsxKJklEEk+13iFLEp\n" \
"VdZlAoGAfbKY5zqkXSO95G6sSPVHse9s16Lqo5OK9bDGtJjpo75bvg3IT79/itC+\n" \
"qt+Hb8Z0K4/tvL4N0xv0V3JEIuwlhBgxC/KWDv9Itjv6AtsdrMTMHTa1my7rcqs/\n" \
"b6t5r5H4L8S56dWXc3VNuaKURNMucl3/Bfo4m3V/UqOzJ8YGdA8=\n" \
"-----END PRIVATE KEY-----\n";

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";