#ifndef RUDISERVER_H_
#define RUDISERVER_H_

#include "rudicore.h"


#define RUDI_SERVER_CNF 				"rudis.cnf"
#define RUDI_SERVER_CNF_REPO 			"server-repo"
#define RUDI_SERVER_DFL_REPO			"rudis-repo"


/* CONFIGURATION */

void rudiLoadServerConfig(void);


// REQUEST HANDLER ENTRY-POINT

void rudiServerHandleRequest(const message_t inmsg, message_t *outmsg);


// SPECIFIC REQUEST HANDLERS

void rudiServerHandleRequestGTCWD(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestCHDIR(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestLSDIR(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestMKDIR(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestRMDIR(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestCPDIR(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestMVDIR(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestOPFILE(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestDWFILE(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestUPFILE(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestRMFILE(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestCPFILE(const message_t inmsg, message_t *outmsg);

void rudiServerHandleRequestMVFILE(const message_t inmsg, message_t *outmsg);

#endif /* RUDISERVER_H_ */
