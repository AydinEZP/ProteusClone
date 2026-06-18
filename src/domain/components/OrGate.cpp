#include "OrGate.h"
void OrGate::evaluate() { bool r=false; for (bool b : m_inputs) r = r || b; setRawOutput(r); }
