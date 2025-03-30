#include "laserfeedback.h"
#include <QPropertyAnimation>
#include <QPainter>
#include <QGraphicsScene>
#include <algorithm>

LaserFeedback::LaserFeedback(const QPointF &start, const QPointF &end, QGraphicsItem *parent)
    : QGraphicsObject(parent),
      m_start(start),
      m_end(end)
{
    setOpacity(1.0);

    // Fade from 1.0 to 0.0 in 300ms
    auto *opacityAnim = new QPropertyAnimation(this, "opacity");
    opacityAnim->setDuration(500);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);

    animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(opacityAnim);

    connect(animationGroup, &QParallelAnimationGroup::finished, this, [this]() {
        if (scene()) {
            scene()->removeItem(this);
        }
        deleteLater();
    });

    animationGroup->start();
}

LaserFeedback::~LaserFeedback() {
}

QRectF LaserFeedback::boundingRect() const {
    qreal minX = std::min(m_start.x(), m_end.x());
    qreal minY = std::min(m_start.y(), m_end.y());
    qreal maxX = std::max(m_start.x(), m_end.x());
    qreal maxY = std::max(m_start.y(), m_end.y());

    return QRectF(minX - 15, minY - 15, (maxX - minX) + 30, (maxY - minY) + 30);
}

void LaserFeedback::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(Qt::red, 15, Qt::SolidLine, Qt::RoundCap);
    painter->setPen(pen);

    // Draw line from m_start to m_end in scene coords:
    painter->drawLine(m_start, m_end);
}
