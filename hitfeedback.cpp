#include "hitfeedback.h"
#include <QGraphicsScene>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPainter>


HitFeedback::HitFeedback(const QPointF &center, int tileSize, QGraphicsItem *parent)
    : QGraphicsObject(parent), m_tileSize(tileSize)
{
    // Position the effect at the given center.
    setPos(center);
    setOpacity(1.0);
    setScale(1.0);

    // Animate opacity from 1.0 to 0.0 and scale from 1.0 to 1.5.
    QPropertyAnimation *opacityAnim = new QPropertyAnimation(this, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);

    QPropertyAnimation *scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(300);
    scaleAnim->setStartValue(1.0);
    scaleAnim->setEndValue(1.5);

    animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(opacityAnim);
    animationGroup->addAnimation(scaleAnim);

    connect(animationGroup, &QParallelAnimationGroup::finished, this, [this]() {
        if (scene()) {
            scene()->removeItem(this);
        }
        deleteLater();
    });
    animationGroup->start();
}

HitFeedback::~HitFeedback() {
    // animationGroup is a child and will be deleted automatically.
}

QRectF HitFeedback::boundingRect() const {
    // The hit effect is drawn as a circle with diameter = 0.75 * tile size.
    qreal diameter = m_tileSize * 0.75;
    return QRectF(-diameter/2, -diameter/2, diameter, diameter);
}

void HitFeedback::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::red);
    painter->drawEllipse(boundingRect());
}
