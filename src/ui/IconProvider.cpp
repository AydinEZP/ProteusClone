#include "IconProvider.h"

#include <QPainter>
#include <QPixmap>
#include <QMap>
#include <QColor>
#include <QFont>
#include <QIcon>

namespace {
struct IconSpec {
    QColor bg;
    QColor fg;
    QString text;
};

IconSpec specFor(const QString& name)
{
    static const QMap<QString, IconSpec> specs = {
        {"app",          {QColor("#263238"), QColor("#00e5ff"), "PC"}},
        {"new",          {QColor("#2e7d32"), QColor("#ffffff"), "+"}},
        {"open",         {QColor("#1565c0"), QColor("#ffffff"), "O"}},
        {"save",         {QColor("#455a64"), QColor("#ffffff"), "S"}},
        {"save_as",      {QColor("#455a64"), QColor("#ffeb3b"), "S+"}},
        {"export",       {QColor("#6a1b9a"), QColor("#ffffff"), "PNG"}},
        {"undo",         {QColor("#5d4037"), QColor("#ffffff"), "↶"}},
        {"redo",         {QColor("#5d4037"), QColor("#ffffff"), "↷"}},
        {"run",          {QColor("#1b5e20"), QColor("#ffffff"), "▶"}},
        {"pause",        {QColor("#f9a825"), QColor("#000000"), "Ⅱ"}},
        {"stop",         {QColor("#b71c1c"), QColor("#ffffff"), "■"}},
        {"step",         {QColor("#0277bd"), QColor("#ffffff"), ">|"}},
        {"drc",          {QColor("#4e342e"), QColor("#ffffff"), "✓"}},
        {"wire",         {QColor("#00695c"), QColor("#ffffff"), "W"}},
        {"help",         {QColor("#1565c0"), QColor("#ffffff"), "?"}},
        {"source",       {QColor("#c62828"), QColor("#ffffff"), "V"}},
        {"battery",      {QColor("#ef6c00"), QColor("#ffffff"), "BAT"}},
        {"ground",       {QColor("#424242"), QColor("#ffffff"), "⏚"}},
        {"clock",        {QColor("#00838f"), QColor("#ffffff"), "CLK"}},
        {"passive",      {QColor("#283593"), QColor("#ffffff"), "RLC"}},
        {"resistor",     {QColor("#283593"), QColor("#ffffff"), "R"}},
        {"capacitor",    {QColor("#283593"), QColor("#ffffff"), "C"}},
        {"inductor",     {QColor("#283593"), QColor("#ffffff"), "L"}},
        {"interactive",  {QColor("#00695c"), QColor("#ffffff"), "I"}},
        {"switch",       {QColor("#00695c"), QColor("#ffffff"), "SW"}},
        {"pushbutton",   {QColor("#00695c"), QColor("#ffffff"), "PB"}},
        {"led",          {QColor("#ad1457"), QColor("#ffffff"), "LED"}},
        {"sevensegment", {QColor("#ad1457"), QColor("#ffffff"), "7"}},
        {"digital",      {QColor("#4527a0"), QColor("#ffffff"), "01"}},
        {"gate",         {QColor("#4527a0"), QColor("#ffffff"), "&"}},
        {"dff",          {QColor("#4527a0"), QColor("#ffffff"), "D"}},
        {"advanced",     {QColor("#37474f"), QColor("#ffffff"), "ADV"}},
        {"adc",          {QColor("#37474f"), QColor("#ffffff"), "ADC"}},
        {"dac",          {QColor("#37474f"), QColor("#ffffff"), "DAC"}},
        {"mcu",          {QColor("#37474f"), QColor("#ffffff"), "µC"}},
        {"lcd",          {QColor("#33691e"), QColor("#ffffff"), "LCD"}},
        {"keypad",       {QColor("#33691e"), QColor("#ffffff"), "KEY"}},
        {"measurement",  {QColor("#01579b"), QColor("#ffffff"), "M"}},
        {"probe",        {QColor("#01579b"), QColor("#ffffff"), "P"}},
        {"voltmeter",    {QColor("#01579b"), QColor("#ffffff"), "V"}},
        {"ammeter",      {QColor("#01579b"), QColor("#ffffff"), "A"}},
        {"oscilloscope", {QColor("#01579b"), QColor("#ffffff"), "OSC"}}
    };
    return specs.value(name, {QColor("#607d8b"), QColor("#ffffff"), name.left(2).toUpper()});
}

QIcon fallbackIcon(const QString& name)
{
    const auto s = specFor(name);
    QPixmap pm(64, 64);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(s.bg);
    p.drawRoundedRect(QRectF(4, 4, 56, 56), 12, 12);

    p.setPen(QPen(QColor(255,255,255,70), 2));
    p.drawRoundedRect(QRectF(8, 8, 48, 48), 9, 9);

    p.setPen(s.fg);
    QFont f("Arial");
    f.setBold(true);
    f.setPixelSize(s.text.size() <= 1 ? 32 : (s.text.size() == 2 ? 25 : 18));
    p.setFont(f);
    p.drawText(QRectF(4, 4, 56, 56), Qt::AlignCenter, s.text);
    p.end();

    return QIcon(pm);
}
}

namespace IconProvider {
QIcon icon(const QString& name)
{
    QIcon generated = fallbackIcon(name);

    QIcon res(QString(":/icons/%1.png").arg(name));
    if (!res.isNull()) {
        // Force-load one pixmap. Some broken/cached builds create null pixmaps even
        // when the QIcon object exists; in that case fall back to generated icons.
        QPixmap px = res.pixmap(32, 32);
        if (!px.isNull())
            return res;
    }

    // Guaranteed visible fallback. This is deliberately generated in code, so
    // icons still appear even if the .qrc file is not copied or Qt resource
    // caching is stale after an incremental build.
    return generated;
}
}
