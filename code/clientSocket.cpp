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

#include <cisstCommon/cmnConstants.h>

#include <sawATINetFT/clientSocket.h>


CMN_IMPLEMENT_SERVICES(clientSocket)

clientSocket::clientSocket(const std::string &taskName, double period):
    mtsTaskContinuous(taskName, 5000)
{    
    ipaddress = "192.168.1.1";      // Default
    port = 49152;

    FTData.SetSize(6);
    RawFTData.SetSize(6);
    initialFTData.SetSize(6);
    programStart = true;

    retryCounter.Data = 2;

    mtsInterfaceProvided *socketInterface = AddInterfaceProvided("ProvidesSocket");

    StateTable.AddData(FTData, "FTData");
    StateTable.AddData(IsConnected, "IsConneted");

    FTData.AutomaticTimestamp() = false;
    IsConnected.AutomaticTimestamp() = false;

    socketInterface->AddCommandReadState(StateTable, FTData, "GetFTData");
    socketInterface->AddCommandReadState(StateTable, IsConnected, "GetSocketStatus");

    socketInterface->AddCommandVoid(&clientSocket::RebiasFTValues, this, "RebiasFTValues");    

    StateTable.Advance();
}

void clientSocket::Startup()
{        
    if(CreateSocket())
        ConnectToSocket();
    else
        CMN_LOG_CLASS_INIT_ERROR << "Socket could not be created " << std::endl;
}

void clientSocket::Cleanup()
{
    CloseSocket();
}

void clientSocket::CloseSocket(void)
{
#ifdef _WIN32
    Closesocket(socketHandle);
#else
    close(socketHandle);
#endif
}

void clientSocket::SetIPAddress(std::string ip)
{
    ipaddress = ip;
}

void clientSocket::Run()
{
    ProcessQueuedCommands();
    CheckSocketConnection();
    if(IsConnected.Data)
        GetReadings();
}

bool clientSocket::CreateSocket(void)
{    
    socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);    
    if(socketHandle == -1)
        return false;

    return true;
}

void clientSocket::CheckSocketConnection(void)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if(setsockopt(socketHandle,SOL_SOCKET,SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        IsConnected.Data = false;
        RetrySocketConnection(retryCounter);
    }
    else
        IsConnected.Data = true;    
}

void clientSocket::ConnectToSocket(void)
{
    *(uint16*)&request[0] = htons(0x1234); /* standard header. */
    *(uint16*)&request[2] = htons(COMMAND); /* per table 9.1 in Net F/T user manual. */
    *(uint32*)&request[4] = htonl(NUM_SAMPLES); /* see section 9.1 in Net F/T user manual. */

    he = gethostbyname(ipaddress.c_str());

    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    err = connect( socketHandle, (struct sockaddr *)&addr, sizeof(addr) );
    CMN_LOG_CLASS_INIT_VERBOSE << "Err " << err << std::endl;
    if (err == -1) {
        exit(2);
    }
}

void clientSocket::GetReadings(void)
{
    send( socketHandle, (const char *)request, 8, 0 );
    recv( socketHandle, (char *)response, 36, 0 );

    this->rdt_sequence = ntohl(*(uint32*)&response[0]);
    this->ft_sequence = ntohl(*(uint32*)&response[4]);
    this->status = ntohl(*(uint32*)&response[8]);
    int temp;
    for( int i = 0; i < 6; i++ ) {
        temp = ntohl(*(int32*)&response[12 + i * 4]);
        if(programStart)
        {
            initialFTData[i] = (double)((double)temp/1000000);
            std::cout << initialFTData[i] << " ";
        }

        RawFTData[i]= (double)((double)temp/1000000);
        FTData[i] = RawFTData[i] - initialFTData[i];
    }

    programStart = false;
}


void clientSocket::RebiasFTValues(void)
{
    initialFTData.Assign(RawFTData);
}

void clientSocket::RetrySocketConnection(const mtsInt &numOfSecs)
{
    CMN_LOG_RUN_WARNING << "Retrying Socket Connection " << std::endl;

    CloseSocket();
    if(CreateSocket())
        ConnectToSocket();

    osaSleep(numOfSecs.Data);
}


void clientSocket::print(mtsDoubleVec &values, int num, std::string name)
{
    std::cout << name;
    for (int i = 0; i < num; ++i)
    {
        std::cout << values[i] << " ";
    }
    std::cout << std::endl;
}
