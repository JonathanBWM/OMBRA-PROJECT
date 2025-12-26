#ifndef OMBRA_TIMING_H
#define OMBRA_TIMING_H
#include "../shared/types.h"
void TimingCalibrate(void);
U64 TimingCompensate(U64 tsc);
#endif
