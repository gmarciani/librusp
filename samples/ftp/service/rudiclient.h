#ifndef RUDICLIENT_H_
#define RUDICLIENT_H_

#include "rudicore.h"
#include "rudimenu.h"


/* CONFIGURATION */

#define RUDI_CLIENT_CNF 				"rudic.cnf"
#define RUDI_CLIENT_CNF_REPO 			"client-repo"
#define RUDI_CLIENT_DFL_REPO		 	"rudic-repo"

void rudiLoadClientConfig(void);


// RESPONSE HANDLER ENTRY-POINT

void rudiClientHandleResponse(const message_t inmsg);


// SPECIFIC RESPONSE HANDLERS

void rudiClientHandleResponseGTCWD(const message_t inmsg);

void rudiClientHandleResponseLSDIR(const message_t inmsg);

void rudiClientHandleResponseOPFILE(const message_t inmsg);

void rudiClientHandleResponseDWFILE(const message_t inmsg);

void rudiClientHandleResponseSuccess(const action_t act, const char *obj, const char *descr);

void rudiClientHandleResponseBadRequest(const action_t act, const char *obj, const char *descr);

#endif /* RUDICLIENT_H_ */
