#ifndef AXIONET_DEFAULTROUTES_H
#define AXIONET_DEFAULTROUTES_H

#include "axionetd.h"
#include "memory-pool.h"

void route404(AxioResponse *resp, axio_MemoryPool *responsePool); // Not Found
void route405(AxioResponse *resp, axio_MemoryPool *responsePool); // Not Allowed
void route400(AxioResponse *resp, axio_MemoryPool *responsePool); // Bad Request

#endif // AXIONET_DEFAULTROUTES_H
