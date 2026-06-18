#pragma once
#include "LogicGate.h"

class XorGate : public LogicGate {
public:
    XorGate() : LogicGate("XorGate", 2) { m_gateLabel = "=1"; setLabel("XOR"); }
    void evaluate() override;
};