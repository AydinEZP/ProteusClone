#pragma once
#include <cstdint>
#include "../Component.h"

/**
 * Common-cathode 7-segment display.
 * Pins A..G and DP are independent segment inputs; COM is the common cathode.
 * Bit mapping: bit0=A ... bit6=G, bit7=DP.
 */
class SevenSegment : public Component {
public:
    SevenSegment();
    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;

    void setSegments(uint8_t mask) { m_segMask = mask; }
    uint8_t segments() const { return m_segMask; }

private:
    uint8_t m_segMask {0};
    bool seg(int i) const { return i >= 0 && i < 8 && ((m_segMask >> i) & 1u); }
};
