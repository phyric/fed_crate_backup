/*****************************************************************************\
* $Id: DBAgent.h,v 1.1 2009/05/16 18:55:20 paste Exp $
\*****************************************************************************/
#ifndef __EMU_FED_DBAGENT_H__
#define __EMU_FED_DBAGENT_H__

#include <string>
#include <iostream>
#include <sstream>

#include "xdaq/WebApplication.h"
#include "xoap/MessageReference.h"
#include "emu/fed/Exception.h"

namespace emu {
	namespace fed {

		/** @class DBAgent A utility class that allows for easy communication with a TStore database **/
		class DBAgent
		{
		public:

			/** Default constructor.
			**/
			DBAgent(xdaq::WebApplication *application);
			
			/** Connect to the database and store the connectionID for later use.
			*	@param username the username to use for connecting to the database.
			*	@param password the password to use for connecting to the database.
			**/
			void connect(const std::string &username, const std::string &password)
			throw (emu::fed::exception::DBException);

		protected:
			
			/** Send a SOAP message to the given application **/
			xoap::MessageReference sendSOAPMessage(xoap::MessageReference message, std::string klass, int instance = -1)
			throw (emu::fed::exception::SOAPException);

			/** Send a SOAP message to the given application **/
			xoap::MessageReference sendSOAPMessage(xoap::MessageReference message, xdaq::ApplicationDescriptor *app)
			throw (emu::fed::exception::SOAPException);
			
			/// The application context from which to send the SOAP messages
			xdaq::WebApplication *application_;

			/// The ID recieved from the database after connecting for future requests
			std::string connectionID_;

		private:
		
		};

	}
}

#endif
