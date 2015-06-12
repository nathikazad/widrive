/* Arduino JPEGCamera Library
 * Copyright 2010 SparkFun Electronic
 * Written by Ryan Owens
 * Modified by arms22
 * Ported to mbed by yamaguch
 */

#include "JPEGCamera.h"

#define min(x, y) ((x) < (y)) ? (x) : (y)

const int RESPONSE_TIMEOUT = 500;
const int DATA_TIMEOUT = 1000;
static const int mod_table[] = {0, 2, 1};

JPEGCamera::JPEGCamera(PinName tx, PinName rx) : Serial(tx, rx) {
    reset();
    baud(115200);
    setBaudRate();
    state = READY;
}

bool JPEGCamera::setPictureSize(JPEGCamera::PictureSize size, bool doReset) {
    char buf[9] = {0x56, 0x00, 0x31, 0x05, 0x04, 0x01, 0x00, 0x19, (char) size};
    int ret = sendReceive(buf, sizeof buf, 5);

    if (ret == 5 && buf[0] == 0x76) {
        if (doReset)
            reset();
        return true;
    } 
    return false;
}

bool JPEGCamera::isReady() {
    return state == READY;
}

bool JPEGCamera::isProcessing() {
    return state == PROCESSING;
}

bool JPEGCamera::takePicture(char *filename) {
    
    if (state == READY) {
        if (1) {
            if (takePicture()) {
                imageSize = getImageSize();
                address = 0;
                state = PROCESSING;
            } else {
                DBG("DID NOT TAKE PIC");
                DBG("takePicture(%s) failed", filename);
                state = ERROR;
            }
        } 
        else {
            DBG("fopen() failed");
            state = ERROR;
        }
    }
    else {
        INFO("takepicture else");
        INFO("NOT READY");
    }
    return state != ERROR;
}

bool JPEGCamera::processPicture(Websocket *ws) {
    if (state == PROCESSING) {
        if (address < imageSize) {
            char data[1024], *encoded, pic[1030];
            size_t _size;
            int size = readData(data, min(sizeof(data), imageSize - address), address);
            encoded = Encode(data, 1024, &_size);
            sprintf(pic, "img:%s", encoded);
            ws->send(pic);
            free(encoded);
            
            int ret = 1;
            if (ret > 0)
                address += size;
            if (ret == 0 || address >= imageSize) {
                stopPictures();
                wait(0.1);
                state = ret > 0 ? READY : ERROR;
            }
        }
    }

    return state == PROCESSING || state == READY;
}

bool JPEGCamera::reset() {
    char buf[4] = {0x56, 0x00, 0x26, 0x00};
    int ret = sendReceive(buf, sizeof buf, 4);
    if (ret == 4 && buf[0] == 0x76) {
        wait(4.0);
        state = READY;
    } else {
        state = ERROR;
    }
    return state == READY;
}

bool JPEGCamera::takePicture() {
    char buf[5] = {0x56, 0x00, 0x36, 0x01, 0x00};
    int ret = sendReceive(buf, sizeof buf, 5);

    return ret == 5 && buf[0] == 0x76;
}

bool JPEGCamera::stopPictures() {
    char buf[5] = {0x56, 0x00, 0x36, 0x01, 0x03};
    int ret = sendReceive(buf, sizeof buf, 5);

    return ret == 4 && buf[0] == 0x76;
}

bool JPEGCamera::setBaudRate() {
    char buf[6] = {0x56, 0x00, 0x24, 0x03, 0x01, 0x0D};
    int ret = sendReceive(buf, sizeof buf, 5);
    
    return true;

    
}

int JPEGCamera::getImageSize() {
    char buf[9] = {0x56, 0x00, 0x34, 0x01, 0x00};
    int ret = sendReceive(buf, sizeof buf, 9);

    //The size is in the last 2 characters of the response.
    return (ret == 9 && buf[0] == 0x76) ? (buf[7] << 8 | buf[8]) : 0;
}

int JPEGCamera::readData(char *dataBuf, int size, int address) {
    char buf[16] = {0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00,
                    address >> 8, address & 255, 0x00, 0x00, size >> 8, size & 255, 0x00, 0x0A
                   };
    int ret = sendReceive(buf, sizeof buf, 5);

    return (ret == 5 && buf[0] == 0x76) ? receive(dataBuf, size, DATA_TIMEOUT) : 0;
}

int JPEGCamera::sendReceive(char *buf, int sendSize, int receiveSize) {
    
    while (readable()) {
         getc();
    }
    
    int i = 0;
    if (writeable()) {
        for (i = 0; i < sendSize; i++) {
            putc(buf[i]);
        }
    }
    return receive(buf, receiveSize, RESPONSE_TIMEOUT);
}

int JPEGCamera::receive(char *buf, int size, int timeout) {
    timer.start();
    timer.reset();

    int i = 0;
    while (i < size && timer.read_ms() < timeout * 5) {
        if (readable()) {
            char lol = getc();
            buf[i++] = lol;
        }
    }

    return i;
}

static const char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

char * JPEGCamera::Encode(const char *data, size_t input_length, size_t *output_length)
{
    *output_length = 4 * ((input_length + 2) / 3);
 
    char *encoded_data = (char *)malloc(*output_length+1);  // often used for text, so add room for NULL
    if (encoded_data == NULL) return NULL;
 
    for (unsigned int i = 0, j = 0; i < input_length;) {
 
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
 
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
 
        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }
 
    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';
 
    encoded_data[*output_length] = '\0';    // as a courtesy to text users
    return encoded_data;
}