#include "timing.h"
static U64 g_Overhead = 2000;
void TimingCalibrate(void) { /* TODO */ }
U64 TimingCompensate(U64 tsc) { return tsc > g_Overhead ? tsc - g_Overhead : tsc; }
