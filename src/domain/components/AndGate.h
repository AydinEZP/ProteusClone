#pragma once
#include "LogicGate.h"

class AndGate : public LogicGate {
public:
    AndGate() : LogicGate("AndGate", 2) { m_gateLabel = "&"; setLabel("AND"); }
    void evaluate() override;
};