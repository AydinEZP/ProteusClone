#include "XorGate.h"
void XorGate::evaluate() { bool r=false; for (bool b : m_inputs) r = r ^ b; setRawOutput(r); }
