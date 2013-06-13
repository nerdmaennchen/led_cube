/*
 * msgProxy.h
 *
 *  Created on: Jan 2, 2013
 *      Author: lutz
 */

#ifndef MSGPROXY_CONFIG_H_
#define MSGPROXY_CONFIG_H_


#include "msgIDs.h"


/*
 * a map from external messages (msgIDs of messages transmitted on external busses)
 * to internal message IDs
 */

typedef struct tag_extIDtoIntIDMap
{
	uint16_t externalID;
	msgID_t internalID;
} extIDtoIntIDPair_t;

#define MSG_PROXY_EXT_2_INT_MSG_ID_MAP { \
	}


#endif /* MSGPROXY_CONFIG_H_ */
