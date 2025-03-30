#ifndef HITFEEDBACK_H
#define HITFEEDBACK_H

#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

/// HitFeedback uses property animations to fade out and scale up.
///@author Group 17
class HitFeedback : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
public:
    /// Constructor that will give the feedback for a hit
    ///@param center - The scene coordinate where the effect appears.
    ///@param tileSize - The size of the tile
    ///@param parent - The parent QGraphicsItem that this object will take place in
    explicit HitFeedback(const QPointF &center, int tileSize, QGraphicsItem *parent = nullptr);
    ~HitFeedback();

    /// QGraphicsItem override
    QRectF boundingRect() const override;
    /// QGraphicsItem override
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    int m_tileSize;
    QParallelAnimationGroup *animationGroup;
};

#endif // HITFEEDBACK_H
