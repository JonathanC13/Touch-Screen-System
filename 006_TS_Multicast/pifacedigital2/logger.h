/*
 * logger.h
 *
 *  Created on: Aug 29, 2016
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <syslog.h>

/*
--------------
Logging conf
--------------
sudo nano /etc/rsyslog.conf
	Add this line to the end to log all LOCAL1 logging to /var/log/CSysLog.log
		local1.* /var/log/CSysLog.log

Restart rsyslogd
		/etc/init.d/rsyslog restart
*/

void log_msg(int prior, char *id, char *fmt, ...);

#ifdef DEBUG
#define LOG_DEBUG_MSG(priority, ident, ...) log_msg(priority, ident, ##__VA_ARGS__);
#else
#define LOG_DEBUG_MSG(priority, ident, ...)
#endif

#define LOG_MSG(priority, ident, ...) log_msg(priority, ident, ##__VA_ARGS__);

struct attrib {
	int priority;
	char *ident;
};

const int LOG_INFO_INDEX = 0;
const int LOG_NOTICE_INDEX = 1;
const int LOG_ERROR_INDEX = 2;
const int LOG_CRIT_INDEX = 3;
const int LOG_DEBUG_INDEX = 4;


struct attrib log_attribs[5] = {
		{LOG_INFO, 		(char*)"CSysIOPF_INFO"},
		{LOG_NOTICE, 	(char*)"CSysIOPF_NOTI"},
		{LOG_ERR,		(char*)"CSysIOPF_ERRR"},
		{LOG_CRIT,		(char*)"CSysIOPF_CRIT"},
		{LOG_DEBUG,		(char*)"CSysIOPF_DBUG"}
};



#endif /* LOGGER_H_ */

