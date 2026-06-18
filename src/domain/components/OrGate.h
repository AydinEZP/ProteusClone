#pragma once
#include "LogicGate.h"

class OrGate : public LogicGate {
public:
    OrGate() : LogicGate("OrGate", 2) { m_gateLabel = "≥1"; setLabel("OR"); }
    void evaluate() override;
};