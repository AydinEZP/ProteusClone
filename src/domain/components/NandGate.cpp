#include "NandGate.h"
void NandGate::evaluate() { bool r=true; for (bool b : m_inputs) r = r && b; setRawOutput(!r); }
