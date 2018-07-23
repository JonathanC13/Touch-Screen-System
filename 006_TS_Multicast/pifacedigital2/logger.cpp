/*
 * logger.cpp
 *
 *  Created on: Aug 29, 2016
 */


#include <syslog.h>
#include <stdarg.h>		//for va_*
#include <stdio.h>		//for vsnprintf()

#include <iostream>
using namespace std;


const int log_facility = LOG_LOCAL1;		///var/log/CSysLog.log
const int log_option = (LOG_CONS | LOG_PID);


void log_msg(int prior, char *id, char *fmt, ...)
{
	char buf[256];
	va_list vl;

	va_start(vl, fmt);
	vsnprintf( buf, sizeof( buf), fmt, vl);
	va_end( vl);

	openlog(id, log_option, log_facility);
	syslog(prior, buf);
	closelog();

	printf("%s", buf);
}
