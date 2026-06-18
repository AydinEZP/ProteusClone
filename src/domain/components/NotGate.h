#pragma once
#include "LogicGate.h"

class NotGate : public LogicGate {
public:
    NotGate() : LogicGate("NotGate", 1) { m_gateLabel = "1"; setLabel("NOT"); }
    void evaluate() override;
};