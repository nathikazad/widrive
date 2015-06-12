/* Arduino JPEGCamera Library
 * Copyright 2010 SparkFun Electronics
 * Written by Ryan Owens
 * Modified by arms22
 * Ported to mbed by yamaguch
 */

#include "mbed.h"
#include "Websocket.h"

#ifndef JPEG_CAMERA_H
#define JPEG_CAMERA_H

#if 1
#define DBG(x, ...) std::fprintf(stderr,"[DBG]" x "\r\n", ##__VA_ARGS__);
#define WARN(x, ...) std::fprintf(stderr, "[WARN]" x "\r\n", ##__VA_ARGS__);
#define ERR(x, ...) std::fprintf(stderr,"[ERR]" x "\r\n", ##__VA_ARGS__);
#define INFO(x, ...) std::fprintf(stderr,"[INFO]" x "\r\n", ##__VA_ARGS__);
#endif


/**
 * Interface for LinkSprite JPEG Camera module LS-Y201
 */
class JPEGCamera : public Serial {
public:
    /***/
    enum PictureSize {
        SIZE160x120 = 0x22,
        SIZE320x240 = 0x11,
        SIZE640x480 = 0x00,
    };

    /**
     * Create JPEG Camera
     *
     * @param tx tx pin
     * @param rx rx pin
     */
    JPEGCamera(PinName tx, PinName rx);

    /**
     * Set picture size
     *
     * @param size picture size (available sizes are SIZE160x120, SIZE320x240, SIZE640x480)
     * @param doReset flag to perform reset operation after changing size
     *
     * @returns true if succeeded, false otherwise
     */
    bool setPictureSize(JPEGCamera::PictureSize size, bool doReset = true);

    /**
     * Return whether camera is ready or not
     *
     * @returns true if ready, false otherwise
     */
    bool isReady();

    /**
     * Return whether camera is processing the taken picture or not
     *
     * @returns true if the camera is in processing, false otherwise
     */
    bool isProcessing();

    /**
     * Take a picture
     *
     * @param filename filename to store the picture data
     * @returns true if succeeded, false otherwise
     */
    bool takePicture(char *filename);
    /**
     * Process picture (writing the file)
     *
     * @returns true if no error in processing, false otherwise
     */
    bool processPicture(Websocket *ws);

    /**
     * Perform reset operation (it takes 4 seconds)
     *
     * @returns true if succeeded, false otherwise
     */
    bool reset();

    /**
     * Send a picture command to the camera module
     *
     *  @returns true if succeeded, false otherwise
     */
    bool takePicture(void);

    /**
     * Send a stop pictures command to the camera module
     *
     *  @returns true if succeeded, false otherwise
     */
    bool stopPictures(void);

    /**
     * Get the picture image size
     *
     * @returns the actual image size in bytes
     */
    int getImageSize();
    bool setBaudRate();

    /**
     * Read the picture data to the buffer
     *
     * @param dataBuf data buffer address to store the received data
     * @param size data size to read
     * @param address the address of the picture data to read
     *
     * @returns the size of the data read
     */
    int readData(char *dataBuf, int size, int address);
    
    char * Encode(const char *data, size_t input_length, size_t *output_length);
//private:
    Timer timer;
    FILE *fp;
    int imageSize;
    int address;
    enum State {UNKNOWN, READY, PROCESSING, ERROR = -1} state;
    
    int sendReceive(char *buf, int sendSize, int receiveSize);
    int receive(char *buf, int size, int timeout);
};

#endif
