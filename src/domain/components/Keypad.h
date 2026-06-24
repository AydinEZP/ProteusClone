#pragma once
#include "../Component.h"
#include <QVector>

/**
 * Live 4x4 matrix keypad.
 *
 * - Row pins R1..R4 are normally scanned by the MCU.
 * - Column pins C1..C4 are pulled through the pressed key to the active row.
 * - Mouse interaction is momentary: press on a visual key to close it, release
 *   the mouse to open it again.
 */
class Keypad : public Component {
public:
    Keypad();

    int pressedRow() const { return m_pressedRow; }
    int pressedCol() const { return m_pressedCol; }

    void setPressedKey(const QString& key);
    QString pressedKey() const;
    void releaseKey();

    /** Return the key under a world-space mouse position, or an empty string. */
    QString keyAtWorldPos(QPointF worldPos) const;

    /** Press the key under worldPos. Returns true only when a key cell was hit. */
    bool pressAtWorldPos(QPointF worldPos);

    /** True when the selected column is connected to a row currently driven LOW. */
    bool columnActive(int col, const QVector<bool>& rowDriveLow) const;

    QRectF boundingBox() const override;
    void draw(QPainter& painter, bool selected) const override;
    QJsonObject serialize() const override;
    void deserialize(const QJsonObject& obj) override;
    QMap<QString,QString> properties() const override;
    void setProperty(const QString& key, const QString& value) override;

private:
    int m_pressedRow {-1};
    int m_pressedCol {-1};
};
