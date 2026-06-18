#pragma once
#include "../Component.h"

class Ground : public Component {
public:
    Ground();
    QRectF boundingBox() const override;
    void   draw(QPainter& painter, bool selected) const override;
};