/* Tandem Diabetes Care, Inc. 2023. All rights reserved.

   Sample Take-Home Coding Exercise - Transmitting and Decoding Messages
   Original Author: Mike Herbst
   
   This module simulates a legacy system that packages a string into a serial 
      message, which is then "transmitted" to a receiving module. 
   The receiver then decodes the message and parses it for output.
   
   This entire file can be cut and pasted into the following Online IDE:
   https://www.onlinegdb.com/online_c_compiler
   
   As provided, this code should compile and correctly run through one 
      iteration of the "stringMsgArray" test inputs.
   
   For this exercise, we would like to expand the functionality of the transmit
   and receive modules to also allow transmission of binary packets.  Please
      make whatever additions or modifications you deem appropriate so that 
      both the legacy string inputs and sample binary inputs can be transmitted 
      and decoded correctly.  Create and use whatever tools or helper Functions
      you need to validate the encoded/decoded output.
   
    Sample test data has been provided below.  
   
   There may also be additional opportunities for optimization of the existing
      implementation. We encourage you to make changes, but we recommend adding 
      comments or notations so they can be discussed later.

 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#define MAX_MSG 110
#define NUM_TEST_MSG 4

// Test Data
char stringMsgArray[NUM_TEST_MSG][MAX_MSG] = {
    "Now is the winter of our discontent.",
    "The quick brown fox jumps over the lazy dog.",
    "We are the music makers, and we are the dreamers of dreams...",
    "Two roads diverged in a wood, and I, I took the one less traveled by, And that has made all the difference..",
    };

uint8_t binaryMsgArray[NUM_TEST_MSG][MAX_MSG] = {
    { 0xDE, 0xAD, 0xBE, 0xEF }, // 4 bytes
    { 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A }, // 6 bytes
    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }, // 9 bytes
    { 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF } // 32 bytes
    };
// End Test Data

typedef enum {
   MSG_TYPE_UNKNOWN,
   MSG_TYPE_STRING,
   MSG_TYPE_BINARY,
   NUM_MSG_TYPE,
} MessageType;

// Local Data Types
typedef struct {
    uint16_t messageID;
    uint8_t messageType;
    uint16_t messageLen;
    char msg[MAX_MSG];
    uint32_t crc;
} Message;

// Local Functions
uint32_t crc32(uint8_t *s,size_t n);
void buildMessage(Message *msgToSend, char *payload);
void simTx(Message *msgToSend);
bool decodeMessage(Message *msgRcv, uint8_t *rxBuf);
void printBuf(void);

// Rx Buffer - Simulate a hardware buffer to decode from
static uint8_t rxBuffer[sizeof(Message)];


/* Test Harness - Simulates building a message and transmitting it to a receiver,
   then decoding the message at the receiver */
int main()
{
    Message messageTx, messageRx;
    uint8_t index;
    uint32_t errorCount = 0;
    
    // Test each input
    for (index = 0; index < NUM_TEST_MSG; index++)
    {
        // ################## TRANSMITTER MODULE ###############################
        // Prepare for transmission
        buildMessage(&messageTx, stringMsgArray[index%NUM_TEST_MSG], MSG_TYPE_STRING);
    
        // Simulate transmission 
        simTx(&messageTx);
        // ################## END TRANSMITTER MODULE ###########################
        
    
        // ################## RECEIVER MODULE ##################################
        if (true == decodeMessage(&messageRx, rxBuffer))
        {
            // Print the decoded message
            printMessage(messageRx);
        }
        else
        {
            errorCount++;
        }
        // ################# END RECEIVER MODULE ###############################
    }

    return errorCount;
}

// printMessage
// Prints message to command line output
void printMessage(Message msg)
{
    // Print the decoded message
    printf("Message #%d (CRC: 0x%08X, %3lu bytes): ",
    msg.messageID, msg.crc,
    strlen(msg.msg));
    if(msg.messageType == MSG_TYPE_STRING)
    {
        printf("%s\n", msg.msg);
    }
    else
    {
        int i;
        for(i = 0; i < msg.messageLen; i++)
        {
            printf("0x%x ", msg.msg[i]);
        }
    }
}

// buildMessage 
// Create a message structure for transmission via simulated modem
void buildMessage(Message *msgToSend, char *payload, MessageType msgType)
{
    
    static uint16_t messageCount = 0;
    uint32_t crc;

    msgToSend->messageID = ++messageCount;
    msgToSend->messageLen = strlen(payload);
    msgToSend->messageType = msgType;
    memcpy(msgToSend->msg, payload, msgToSend->messageLen);
    
    crc = crc32((uint8_t *)msgToSend, (MAX_MSG + 2));
    msgToSend->crc = crc;

}

// simTx 
// Simulate transmission via a generic modem
// Places the prepared message into the receive buffer
void simTx(Message *msgToSend)
{
    memcpy(rxBuffer, msgToSend, sizeof(Message));
}
// simTx


// decodeMessage
// Parses serial buffer rxBuf for a message from the simulated modem
// Returns a decoded message in struct msgRcv
bool decodeMessage(Message *msgRcv, uint8_t *rxBuf)
{
    bool result = true;
    uint8_t length = strlen((char *)&rxBuf[2]);
    uint32_t *msgCrc = (uint32_t *)&rxBuf[MAX_MSG+2];
    
    msgRcv->crc = crc32(rxBuf, MAX_MSG+2);
    
    msgRcv->messageID = (uint16_t)rxBuf[0];
    memcpy(msgRcv->msg, &rxBuf[2], length);
    
    if (msgRcv->crc != *msgCrc)
    {
        result = false;
        printf("CRC Mismatch on received message #%d (0x%08X), (0x%08X)", msgRcv->messageID, *msgCrc, msgRcv->crc);
    }
    return result;
}
// decodeMessage()


// Standard CRC32 Implementation
// Please leave this module untouched to ensure consistency of generated CRCs
uint32_t crc32(uint8_t *s,size_t n) {
	uint32_t crc=0xFFFFFFFF;
	
	for(size_t i=0;i<n;i++) {
		uint8_t ch=s[i];
		for(size_t j=0;j<8;j++) {
			uint32_t b=(ch^crc)&1;
			crc>>=1;
			if(b) crc=crc^0xEDB88320;
			ch>>=1;
		}
	}
	
	return ~crc;
}
// crc32()
