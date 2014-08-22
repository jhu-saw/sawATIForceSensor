/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
$Id: $

Author(s):  Preetham Chalasani
Created on: 2013

(C) Copyright 2006-2013 Johns Hopkins University (JHU), All Rights
Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---

*/

#ifndef _CLIENTSOCKET_H
#define _CLIENTSOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include <cisstMultiTask.h>
#include <cisstCommon.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstOSAbstraction/osaSocket.h>

//#define PORT 49152 /* Port the Net F/T always uses */
#define COMMAND 2 /* Command code 2 starts streaming */
#define NUM_SAMPLES 1 /* Will send 1 sample `before stopping */

typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char byte;

class clientSocket: public mtsTaskContinuous {
    CMN_DECLARE_SERVICES(CMN_NO_DYNAMIC_CREATION, CMN_LOG_LOD_NONE );

public:
    clientSocket(const std::string & taskName, double period);
    ~clientSocket(){};

    void Startup(void);
    void Run(void);
    void Cleanup(void);
    void CloseSocket(void);
    void SetIPAddress(std::string ip);

protected:   
    bool CreateSocket(void);
    void CheckSocketConnection(void);
    void ConnectToSocket(void);
    void GetReadings(void);
    void RebiasFTValues(void);
    void RetrySocketConnection(const mtsInt &numOfSecs);        

private:

    bool            programStart;
    mtsBool         IsConnected;
    mtsDoubleVec    FTData;
    mtsDoubleVec    RawFTData;
    mtsDoubleVec    initialFTData;    
    mtsInt          retryCounter;

    std::string     ipaddress;
    uint16          port;
    uint32          rdt_sequence;
    uint32          ft_sequence;
    uint32          status;


#ifdef _WIN32
    SOCKET          socketHandle;		/* Handle to UDP socket used to communicate with Net F/T. */
    WSADATA         wsaData;
    WORD            wVersionRequested;
#else
    int             socketHandle;			/* Handle to UDP socket used to communicate with Net F/T. */
#endif

    struct          sockaddr_in addr;	/* Address of Net F/T. */
    struct          hostent *he;			/* Host entry for Net F/T. */
    byte            request[8];			/* The request data sent to the Net F/T. */
    byte            response[36];			/* The raw response data received from the Net F/T. */
    int             err;					/* Error status of operations. */
};

CMN_DECLARE_SERVICES_INSTANTIATION(clientSocket);

#endif
