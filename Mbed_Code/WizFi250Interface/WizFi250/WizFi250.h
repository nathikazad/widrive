/* Copyright (C) 2013 gsfan, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* Copyright (C) 2014 Wiznet, MIT License
 *  port to the Wiznet Module WizFi250
 */

#ifndef WIZFI250_H_
#define WIZFI250_H_

#include "WizFi250_conf.h"

#include "mbed.h"
#include "RawSerial.h"
#include "Serial.h"
#include "CBuffer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string>

using namespace std;


#define DEBUG

//Debug is disabled by default
#if 1
#define DBG(x, ...) std::fprintf(stderr,"[DBG]" x "\r\n", ##__VA_ARGS__);
#define WARN(x, ...) std::fprintf(stderr, "[WARN]" x "\r\n", ##__VA_ARGS__);
#define ERR(x, ...) std::fprintf(stderr,"[ERR]" x "\r\n", ##__VA_ARGS__);
#define INFO(x, ...) std::fprintf(stderr,"[INFO]" x "\r\n", ##__VA_ARGS__);
#else
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)
#define INFO(x, ...)
#endif


class WizFi250
{
public:

    enum WiFiMode {
        // kaizen need to change
        WM_STATION = 0,
        WM_AP = 1,
        //WM_ADHOCK,
    };

    /** Wi-Fi security
     */
    enum Security {
        // kaizen need to change
        SEC_AUTO        = 0,
        SEC_OPEN        = 1,
        SEC_WEP         = 2,
        SEC_WPA_TKIP    = 3,
        SEC_WPA_AES     = 4,
        SEC_WPA2_AES    = 5,
        SEC_WPA2_TKIP   = 6,
        SEC_WPA2_MIXED  = 7,
    };

    /** TCP/IP protocol
     */
    enum Protocol {
        PROTO_UDP = 0,
        PROTO_TCP = 1,
    };

    /** Client/Server
     */
    enum Type {
        TYPE_CLIENT = 0,
        TYPE_SERVER = 1,
    };

    enum Response {
        RES_NULL,
        RES_MACADDRESS,
        RES_WJOIN,
        RES_CONNECT,
        RES_SSEND,
        RES_FDNS,
        RES_SMGMT,
    };

    enum Mode {
        MODE_COMMAND,
        MODE_CMDRESP,
        MODE_DATA_RX,
        MODE_DATA_RXUDP,
        MODE_DATA_RXUDP_BULK,
    };

    enum Status {
        STAT_NONE,
        STAT_READY,
        STAT_STANDBY,
        STAT_WAKEUP,
        STAT_DEEPSLEEP,
    };


    WizFi250 (PinName tx, PinName rx, PinName cts, PinName rts, PinName reset, PinName alarm = NC, int baud = 115200);



    // --------- WizFi250_at.cpp ---------
    void clearFlags     ();
    int  sendCommand    (const char * cmd, const char *ACK, char *res = NULL , int timeout = DEFAULT_WAIT_RESP_TIMEOUT, int opt = 2);
    int  sendCommand    (const char * cmd, Response res = RES_NULL, int timeout = DEFAULT_WAIT_RESP_TIMEOUT, int opt = 2);

    int cmdAT       ();
    int cmdMECHO    (bool flg);
    int cmdUSET     (int baud, char *flow);
    int cmdMMAC     (const char *mac = NULL);
    int cmdWSET     (WiFiMode mode, const char *ssid, const char *bssid = NULL, int channel = 6);
    int cmdWSEC     (WiFiMode mode, const char *key, const char *sec = "WPA2");
    int cmdWJOIN    ();
    int cmdWLEAVE   ();
    int cmdSCON     ( const char *openType, const char *socketType, int localPort, const char *dataMode = "0");
    int cmdSCON     ( const char *openType, const char *socketType, const char *remoteIp, int remotePort, int localPort = NULL, const char *dataMode = "0");
    int cmdSSEND    ( const char *data, int cid, int sendSize, const char *remoteIp = NULL, int remotePort = NULL, int Timeout = 10000);
    int cmdCLOSE    ( int cid );
    int cmdFDNS     (const char *host);
    int cmdSMGMT    ( int cid );


    static WizFi250 * getInstance() {
        return _inst;
    };

    Serial * getDebug() {
        return &_debug;
    }



    // --------- WizFi250_sock.cpp ---------
    int getHostByName   (const char * host, char *ip);
    int open            (Protocol proto, const char *ip, int remotePort, int localPort = 0, void(*func)(int) = NULL);
    int listen          (Protocol proto, int port, void(*func)(int)=NULL);
    int close           (int cid);
    void initCon        (int cid, bool connected);
    int send            (int cid, const char *buf, int len);
    int sendto          (int cid, const char *buf, int len, const char *ip, int port);
    int recv            (int cid, char *buf, int len);
    int recvfrom        (int cid, char *buf, int len, char *ip, int *port);
    int readable        (int cid);
    bool isConnected    (int cid);
    int accept          (int cid);
    int getRemote       (int cid, char **ip, int *port);



protected:
    static WizFi250 * _inst;


    // Serial _wizfi
    RawSerial _wizfi;
    int _baud;
    DigitalIn *_cts;
    DigitalOut *_rts;
    int _flow;
    Serial _debug;


#if defined(TARGET_FRDM_KL25Z)
    UART_Type * _uart;
#endif

    DigitalInOut _reset;

    struct STATE {
        WiFiMode wm;
        Security sec;
        char ssid[35];
        char pass[66];
        char ip[16];
        char netmask[16];
        char gateway[16];
        char nameserver[16];
        char mac[18];
        char resolv[16];
        char name[32];
        int rssi;
        bool dhcp;
        time_t time;

        bool initialized;
        bool associated;
        volatile Mode mode;
        volatile Status status;
        bool escape;
        volatile bool ok, failure;
        volatile Response res;
        int cid;
        int n;
        CircBuffer<char> *buf;
        char dbgRespBuf[128];
        char dummyBuf[128];
    } _state;


public:
    struct CONNECTION {
        Protocol protocol;
        Type type;
        bool connected;
        char ip[16];
        int port;
        int send_length;
        int recv_length;
        CircBuffer<char> *buf;
        volatile bool received;
        volatile int parent;
        volatile bool accept;
        void(*func)(int);
    } _con[8];


    // --------- WizFi250.cpp ---------

    int join ();
    bool isAssociated();

    int limitedap ();
    int dissociate ();
    int disconnect () {
        return dissociate();
    }

    Status getStatus ();

    int setMacAddress   (const char *mac);
    int getMacAddress   (char *mac);
    int setAddress      (const char *name = NULL);
    int setAddress      (const char *ip, const char *netmask, const char *gateway, const char *dns = NULL, const char *name = NULL);
    int getAddress      (char *ip, char *netmask, char *gateway);
    int setSsid         (const char *ssid);
    int setSec          (Security sec, const char *phrase);


    // --------- WizFi250_msg.cpp ---------
    void recvData       ( char c );
    int  parseMessage   ();
    void msgOk          (const char *buf);
    void msgError       (const char *buf);
    void msgConnect     (const char *buf);
    void msgDisconnect  (const char *buf);
    void msgListen      (const char *buf);

    void resMacAddress  (const char *buf);
    void resWJOIN       (const char *buf);
    void resConnect     (const char *buf);
    void resSSEND       (const char *buf);
    void resFDNS        (const char *buf);
    void resSMGMT       (const char *buf);



    // --------- WizFi250_hal.cpp ---------
    void setReset   (bool flg);
    void isrUart    ();
    int  getUart    ();
    void putUart    (char c);
    void setRts     (bool flg);
    int  lockUart   (int ms);
    void unlockUart ();
    void initUart   (PinName cts, PinName rts, PinName alarm, int baud);


    // --------- WizFi2550_util.cpp ---------
    int x2i     (char c);
    int i2x     (int i);



};

#endif /* WIZFI250_H_ */