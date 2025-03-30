#include "projectile.h"
#include <QGraphicsScene>
#include <QGraphicsDropShadowEffect>
#include <QRadialGradient>

Projectile::Projectile(const QPointF &startPos, const QPointF &endPos, Direction direction, int tileSize, ProjectileType type, QGraphicsItem *parent)
    : QGraphicsObject(parent),
      m_startPos(startPos),
      m_endPos(endPos),
      m_direction(direction),
      m_type(type),
      m_tileSize(tileSize),
      m_progress(0.0)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Projectile::updateAnimation);
    setPos(m_startPos);

    // Add a drop shadow for glowing effect
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(10);
    effect->setOffset(0, 0);
    effect->setColor(Qt::gray);
    setGraphicsEffect(effect);
}

Projectile::~Projectile() {
}

QRectF Projectile::boundingRect() const {
    // Extend the bounding rectangle to accommodate the trail
    return QRectF(-10, -10, 20, 20);
}


void Projectile::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Draw differently based on m_type:
    if (m_type == ProjectileType::Normal) {
        // A simple black dot
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), 3, 3);
    }
    else if (m_type == ProjectileType::Missile) {
        // red circle
        painter->setBrush(Qt::red);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0,0), 4, 4);
    }
    else if (m_type == ProjectileType::Bomb) {
        // bomb icon
        QPixmap bombIcon(":/sprites/Sprite/Icons/bomb.png");
        if (!bombIcon.isNull()) {
            bombIcon = bombIcon.scaled(m_tileSize*0.5, m_tileSize*0.5,
                                       Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPointF topLeft(-bombIcon.width()/2.0, -bombIcon.height()/2.0);
            painter->drawPixmap(topLeft, bombIcon);
        }
    }

    // draw a fading trai
    int n = m_trailPoints.size();
    for (int i = 0; i < n; i++) {
        qreal factor = (qreal)(i + 1) / n; // fade older points
        QColor trailColor = Qt::black;
        trailColor.setAlphaF(factor * 0.3);
        painter->setBrush(trailColor);
        painter->setPen(Qt::NoPen);

        // Convert the stored scene coords to local coords:
        QPointF localPt = m_trailPoints[i] - pos();
        painter->drawEllipse(localPt, 2, 2);
    }
}

void Projectile::startAnimation() {
    m_timer->start(30); // Update every 30ms.
}

void Projectile::updateAnimation() {
    // Record current position for the trail.
    m_trailPoints.append(pos());
    const int maxTrailPoints = 10;
    if (m_trailPoints.size() > maxTrailPoints) {
        m_trailPoints.removeFirst();
    }

    m_progress += 0.25; // Adjust for speed.
    if (m_progress >= 1.0) {
        m_timer->stop();
        setPos(m_endPos);
        emit hitReached(m_endPos);
        deleteLater();
    } else {
        qreal newX = m_startPos.x() + (m_endPos.x() - m_startPos.x()) * m_progress;
        qreal newY = m_startPos.y() + (m_endPos.y() - m_startPos.y()) * m_progress;
        setPos(newX, newY);
    }
}
