/*
 * Copyright (C) 2014 Wiznet, MIT License
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
 
#include "mbed.h"
#include "WizFi250Interface.h"
#include "Websocket.h"
#include "PWMMotor.h"
#include "string.h"
#include "JPEGCamera.h"

#define MAXSPEED .5f 
#define SECURE WizFi250::SEC_WPA2_MIXED
#define SSID "SAMSUNG"
#define PASS "helloeric"

Serial pc(USBTX,USBRX);
WizFi250Interface wizfi250(PTD3,PTD2,PTD1,PTD0,PTD4,NC,115200);
JPEGCamera camera(D1, D0); // TX, RX

int main()
{    
    char msg[20] = {0};
    char filename[32];
    int picNum = 0;
    
    DBG("program restart");
    
    wizfi250.init();
    wizfi250.connect(SECURE, SSID, PASS);
    INFO("IP Address is %s\r\n", wizfi250.getIPAddress());
    
    INFO("before constructor");
    Websocket ws("ws://23.253.239.92:8080/mbed");
    INFO("after constructor");
    wait(5.0);
    ws.connect();
    INFO("after connect");
    INFO("connected");
    
    while(1)
    {
        if (ws.is_connected()) {
            ws.read(msg);
                fprintf(stderr, "msg: %s\n", msg);
                if (!strcmp(msg, "turn_right")) {
                    write_pwm(0.50f, 0.00f, 0.50f, 0.00f,325);
                }
                else if (!strcmp(msg, "move_backward")) {
                    write_pwm(0.00f, 0.50f, 0.50f, 0.00f,500);
                }
                else if (!strcmp(msg, "move_forward")) {
                    write_pwm(0.90f, 0.00f, 0.00f, 0.35f,900);
                }
                else if (!strcmp(msg, "turn_left")) {
                   write_pwm(0.00f, 0.50f, 0.00f, 0.50f,325);
                }
                else if (!strcmp(msg, "move_backward")) {
                   write_pwm(0.00f, 0.50f, 0.50f, 0.00f,500);
                }
                else if (!strcmp(msg, "snap_image")) {
                    camera.isReady();
                    sprintf(filename, "/sd/pict%03d.jpg", picNum++);
                    if (camera.takePicture(filename)) {
                        while (camera.isProcessing()) {    
                            camera.processPicture(&ws);
                        }
                        ws.send("finished_sending");
                    }
                    camera.isReady();
                }
                else {
                    DBG("nothing read");
                }
            
            memset(msg, 0, 20);
            wait(4.0);
            
        }
        else {
            ws.connect();
            wait(4.0);
        }
    }
}