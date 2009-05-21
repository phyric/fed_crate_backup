/*****************************************************************************\
* $Id: FIFOParser.h,v 1.1 2009/05/21 15:30:48 paste Exp $
\*****************************************************************************/
#ifndef __EMU_FED_FIFOPARSER_H__
#define __EMU_FED_FIFOPARSER_H__

#include "emu/fed/Parser.h"
#include "emu/fed/Exception.h"

namespace emu {
	namespace fed {

		class FIFO;

		/** @class FIFOParser A parser that builds FIFO objects to be loaded into the DCC.
		*	@sa FIFO
		**/
		class FIFOParser: public Parser
		{
		
		public:

			/** Default constructor.
			*
			*	@param pNode the XML DOM element node to parse.
			**/
			explicit FIFOParser(xercesc::DOMElement *pNode)
			throw (emu::fed::exception::ParseException);

			/** @returns a pointer to the parsed FIFO object. **/
			inline FIFO *getFIFO() { return fifo_; }

			/** @returns whether or not the FIFO is marked to be used at the DCC. **/
			inline const bool isUsed() { return used_; }

			/** @returns the parsed FIFO number. **/
			inline const unsigned int getNumber() { return number_; }
		
		private:

			/// A FIFO object built from the parsed attributes of the DOM node.
			FIFO *fifo_;

			/// Whether or not the fifo is used.
			bool used_;

			/// The FIFO number.
			unsigned int number_;
		};

	}
}

#endif
