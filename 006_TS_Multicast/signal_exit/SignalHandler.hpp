/*
 * SignalHandler.hpp
 *
 *  Created on: Aug 24, 2016
 */

#ifndef SIGNALHANDLER_HPP_
#define SIGNALHANDLER_HPP_


#include <string>      // for string
#include <stdexcept>   // for exception, runtime_error, out_of_range
#include <iostream>    // for cout


using std::runtime_error;

class SignalException : public runtime_error
{
public:
   SignalException(const std::string& _message)
      : std::runtime_error(_message)
   {}
};


class SignalHandler {
protected:
    static bool mbGotExitSignal;

public:
    SignalHandler();
    ~SignalHandler();

    static bool gotExitSignal();
    static void setExitSignal(bool _bExitSignal);

    void        setupSignalHandlers();
    static void exitSignalHandler(int _ignored);
};

#endif /* SIGNALHANDLER_HPP_ */
