#include <digitalIO.h>
#include "hcp_handler.h"
#include "hcp_opcodes.h"
#include "ringbuffer.h"


void hcp_handler_digitalReq(hcp_conn_t conn, uint8_t opcode, size_t payloadLength)
{
	uint8_t port;
	if(ringbuffer_pop(conn.rxBuffer, &port)) return;

	if(port >= DIGITAL_COUNT)
	{
		ringbuffer_push(conn.txBuffer, HCP_INVALID_PORT);
		return;
	}

	ringbuffer_push(conn.txBuffer, HCP_DIGITAL_REP);
	ringbuffer_push(conn.txBuffer, port);
	ringbuffer_push(conn.txBuffer, (uint8_t)digitalIO_getState(port));
}
