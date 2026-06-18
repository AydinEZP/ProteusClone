#pragma once
#include "LogicGate.h"

class NandGate : public LogicGate {
public:
    NandGate() : LogicGate("NandGate", 2) { m_gateLabel = "&\u0305"; setLabel("NAND"); }
    void evaluate() override;
};