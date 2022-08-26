// GPLv2 2022 e@richiardone.eu

#include <Arduino.h>
#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_TIMEOUT 1000
#define PM_QUERY_LEN 5
#define RX_BUF_LEN 20


class PM1006 {

  private:
    uint8_t PM_QUERY[PM_QUERY_LEN] = {0x11,0x02,0x0b,0x01,0xe1};

    Stream *_serial;
    uint8_t _rxbuf[RX_BUF_LEN];

    bool request();
    bool check_rx();


  public:
    static const int BIT_RATE = 9600;
    
    explicit PM1006(Stream *serial);
    
    bool read_pm25(uint16_t *pm);

};
