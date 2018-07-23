/*
 * SignalHandler.cpp
 *
 *  Created on: Aug 24, 2016
 */

#include <signal.h>
#include <errno.h>

#include <execinfo.h>
#include <exception>
#include <iostream>

#include "SignalHandler.hpp"

bool SignalHandler::mbGotExitSignal = false;


SignalHandler::SignalHandler() {
	// TODO Auto-generated constructor stub

}

SignalHandler::~SignalHandler() {
	// TODO Auto-generated destructor stub
}



/**
* Returns the bool flag indicating whether we received an exit signal
* @return Flag indicating shutdown of program
*/
bool SignalHandler::gotExitSignal()
{
    return mbGotExitSignal;
}

/**
* Sets the bool flag indicating whether we received an exit signal
*/
void SignalHandler::setExitSignal(bool _bExitSignal)
{
    mbGotExitSignal = _bExitSignal;
}

/**
* Sets exit signal to true.
* @param[in] _ignored Not used but required by function prototype
*                     to match required handler.
*/
void SignalHandler::exitSignalHandler(int _ignored)
{
    mbGotExitSignal = true;
}

/**
* Set up the signal handlers for CTRL-C.
*/
void SignalHandler::setupSignalHandlers()
{
    if (signal((int) SIGINT, SignalHandler::exitSignalHandler) == SIG_ERR)
    {
        throw SignalException("!!!!! Error setting up signal handlers !!!!!");
    }
}



