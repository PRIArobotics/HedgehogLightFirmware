#include "hcp_handler.h"
#include "hcp_opcodes.h"
#include "ringbuffer.h"
#include "output.h"
#include "power.h"

void hcp_handler_speaker(hcp_conn_t conn, uint8_t opcode, size_t payloadLength)
{
	uint8_t frequency_h;
	if(ringbuffer_pop(conn.rxBuffer, &frequency_h)) return;
	uint8_t frequency_l;
	if(ringbuffer_pop(conn.rxBuffer, &frequency_l)) return;
	uint16_t frequency = (frequency_h << 8) | frequency_l;

	if(power_getEmergencyStop() && frequency)
	{
		ringbuffer_push(conn.txBuffer, HCP_FAIL_EMERG_ACT);
		return;
	}

	speaker_setFrequency(frequency);

	ringbuffer_push(conn.txBuffer, HCP_OK);
}
