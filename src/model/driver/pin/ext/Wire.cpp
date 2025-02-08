/*
  TwoWire.cpp - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
*/
#ifdef NATIVE
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <iostream>
#include <bitset>

#include "Wire.h"
#include "wrapper.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWire::rxBuffer[BUFFER_LENGTH];
uint8_t TwoWire::rxBufferIndex = 0;
uint8_t TwoWire::rxBufferLength = 0;

uint8_t TwoWire::txAddress = 0;
uint8_t TwoWire::txBuffer[BUFFER_LENGTH];
uint8_t TwoWire::txBufferIndex = 0;
uint8_t TwoWire::txBufferLength = 0;

uint8_t TwoWire::transmitting = 0;
void (*TwoWire::user_onRequest)(void);
void (*TwoWire::user_onReceive)(int);

// Constructors ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*!  \fn TwoWire()
*
*   \param void
*   \return void
*   \brief open file descriptor for i2c device
******************************************************************************/
TwoWire::TwoWire()
{
}

bool TwoWire::begin(uint8_t sda, uint8_t scl)
{
  TwoWire::begin();
  return true;
}

// Public Methods //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*!  \fn begin()
*
*   \param void
*   \return void
*   \brief set rx & tx buffers to 0
******************************************************************************/
void TwoWire::begin(void)
{
  rxBufferIndex = 0;
  rxBufferLength = 0;

  txBufferIndex = 0;
  txBufferLength = 0;
}

void TwoWire::begin(uint8_t address)
{
	//TODO
}

void TwoWire::begin(int address)
{
  begin((uint8_t)address);
}

///////////////////////////////////////////////////////////////////////////////
/*!  \fn end()
*
*   \param void
*   \return void
*   \brief close file descriptor of i2c device
******************************************************************************/
void TwoWire::end(void)
{
	//TODO
}
///////////////////////////////////////////////////////////////////////////////
/*!  \fn setClock()
*
*   \param uint32_t clock
*   \return void
*   \brief set i2c clock speed
******************************************************************************/
void TwoWire::setClock(uint32_t clock)
{
  //TODO
}
///////////////////////////////////////////////////////////////////////////////
/*!  \fn requestFrom()
*
*   \param uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop
*   \return number of read bytes
*   \brief request a quantity of bytes from the device at address and write them to rxBuffer
******************************************************************************/
int8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop)
{
	int8_t count = 0;
    fd = open(i2c_name, O_RDWR);

    if (fd < 0)
	{
        perror("Failed to open device");
        return(-1);
    }
    if (ioctl(fd, I2C_SLAVE, address) < 0)
	{
        perror("Failed to select device");
        close(fd);
        return(-1);
    }
    if (wrappedWrite(fd, &iaddress, 1) != 1)
	{
        perror("Failed to write reg");
        close(fd);
        return(-1);
    }
    count = wrappedRead(fd, rxBuffer, quantity);
    if (count < 0)
	{
        perror("Failed to read device");
        close(fd);
        return(-1);
    } else if (count != quantity)
	{
        fprintf(stderr, "Short read  from device, expected %d, got %d\n", quantity, count);
        close(fd);
        return(-1);
    }
    close(fd);
    //print rxBuffer
    for(int i = 0; i< BUFFER_LENGTH; i++)
    {
  		  std::cout<<"rxBuffer["<<i<<"]: "<<std::bitset<8>(rxBuffer[i])<<std::endl;
    }

	rxBufferIndex = 0;
	rxBufferLength = count;

	return count;
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
	return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0, (uint8_t)0, (uint8_t)sendStop);
}

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t TwoWire::requestFrom(int address, int quantity, int sendStop)
{
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}

///////////////////////////////////////////////////////////////////////////////
/*!  \fn beginTransmission()
*
*   \param uint8_t address
*   \return void
*   \brief set the target address and reset the tx buffer
******************************************************************************/
void TwoWire::beginTransmission(uint8_t address)
{
  // indicate that we are transmitting
  transmitting = 1;
  // set address of targeted slave
  txAddress = address;
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
}

void TwoWire::beginTransmission(int address)
{
  beginTransmission((uint8_t)address);
}

//
//	Originally, 'endTransmission' was an f(void) function.
//	It has been modified to take one parameter indicating
//	whether or not a STOP should be performed on the bus.
//	Calling endTransmission(false) allows a sketch to 
//	perform a repeated start. 
//
//	WARNING: Nothing in the library keeps track of whether
//	the bus tenure has been properly ended with a STOP. It
//	is very possible to leave the bus in a hung state if
//	no call to endTransmission(true) is made. Some I2C
//	devices will behave oddly if they do not see a STOP.
//
///////////////////////////////////////////////////////////////////////////////
/*!  \fn endTransmission()
*
*   \param uint8_t sendStop
*   \return number of sent bytes
*   \brief send the tx buffer data bytewise to the target
******************************************************************************/
uint8_t TwoWire::endTransmission(uint8_t sendStop)
{
	int8_t count = 0;
	// transmit buffer (blocking)
	fd = open(i2c_name, O_RDWR);
    if (fd < 0)
	{
        perror("Failed to open device");
        return(-1);
    }
    if (ioctl(fd, I2C_SLAVE, txAddress) < 0)
	{
        perror("Failed to select device");
        close(fd);
        return(-1);
    }
	count = wrappedWrite(fd, txBuffer, txBufferLength);
	if (count < 0)
	{
        perror("Failed to write reg");
		close(fd);
		return(-1);
	}
	else if (count != txBufferLength)
	{
		fprintf(stderr, "Short write to device, expected %d, got %d\n", txBufferLength, count);
        close(fd);
		return(-1);
	}
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  transmitting = 0;
  return (uint8_t)count;
}

//	This provides backwards compatibility with the original
//	definition, and expected behaviour, of endTransmission
//
uint8_t TwoWire::endTransmission(void)
{
  return endTransmission(true);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t TwoWire::write(uint8_t data)
{
  if(transmitting){
  // in master transmitter mode
    // don't bother if buffer is full
    if(txBufferLength >= BUFFER_LENGTH){
      //setWriteError();
      return 0;
    }
    // put byte in tx buffer
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    // update amount in buffer   
    txBufferLength = txBufferIndex;
  }else{
  // in slave send mode
    // reply to master
  }
  return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
///////////////////////////////////////////////////////////////////////////////
/*!  \fn write()
*
*   \param uint8_t data, size_t quantity
*   \return number of written bytes
*   \brief write a quantity of data bytes to tx buffer
******************************************************************************/
size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
  if(transmitting){
  // in master transmitter mode
    for(size_t i = 0; i < quantity; ++i){
      write(data[i]);
    }
  }else{
  // in slave send mode
    // reply to master
  }
  return quantity;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
///////////////////////////////////////////////////////////////////////////////
/*!  \fn available()
*
*   \param void
*   \return number of bytes in rx buffer
*   \brief return the number of bytes in rx buffer
******************************************************************************/
int TwoWire::available(void)
{
  return rxBufferLength - rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
///////////////////////////////////////////////////////////////////////////////
/*!  \fn read()
*
*   \param void
*   \return data byte from rx buffer
*   \brief read and remove one byte from the rx buffer
******************************************************************************/
int TwoWire::read(void)
{
  int value = -1;
  
  // get each successive byte on each call
  if(rxBufferIndex < rxBufferLength){
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }
  return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
///////////////////////////////////////////////////////////////////////////////
/*!  \fn peek()
*
*   \param void
*   \return data byte from rx buffer
*   \brief read but not remove the last received byte from the rxBuffer
******************************************************************************/
int TwoWire::peek(void)
{
	int value = -1;
  
	if(rxBufferIndex < rxBufferLength){
	value = rxBuffer[rxBufferIndex];
}

  return value;
}

void TwoWire::flush(void)
{
  // XXX: to be implemented.
}

// behind the scenes function that is called when data is received
void TwoWire::onReceiveService(uint8_t* inBytes, int numBytes)
{
  // don't bother if user hasn't registered a callback
  if(!user_onReceive){
    return;
  }
  // don't bother if rx buffer is in use by a master requestFrom() op
  // i know this drops data, but it allows for slight stupidity
  // meaning, they may not have read all the master requestFrom() data yet
  if(rxBufferIndex < rxBufferLength){
    return;
  }
  // copy twi rx buffer into local read buffer
  // this enables new reads to happen in parallel
  for(uint8_t i = 0; i < numBytes; ++i){
    rxBuffer[i] = inBytes[i];    
  }
  // set rx iterator vars
  rxBufferIndex = 0;
  rxBufferLength = numBytes;
  // alert user program
  user_onReceive(numBytes);
}

// behind the scenes function that is called when data is requested
void TwoWire::onRequestService(void)
{
  // don't bother if user hasn't registered a callback
  if(!user_onRequest){
    return;
  }
  // reset tx buffer iterator vars
  // !!! this will kill any pending pre-master sendTo() activity
  txBufferIndex = 0;
  txBufferLength = 0;
  // alert user program
  user_onRequest();
}

// sets function called on slave write
void TwoWire::onReceive( void (*function)(int) )
{
  user_onReceive = function;
}

// sets function called on slave read
void TwoWire::onRequest( void (*function)(void) )
{
  user_onRequest = function;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

TwoWire Wire = TwoWire();
#endif
