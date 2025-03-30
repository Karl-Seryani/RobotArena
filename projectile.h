#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <QGraphicsObject>
#include <QTimer>
#include <QPointF>
#include <QVector>
#include <QPainter>
#include "robot.h"

/// @brief Projectiles can be normal, missile or bomb
enum class ProjectileType {
    Normal,
    Missile,
    Bomb
};

/// @brief Spawns a projectile that will deal damage against an enemy object
///@author Group 17
class Projectile : public QGraphicsObject {
    Q_OBJECT
public:
    /// @brief Constructor used to spawn the projectile
    /// @param startPos - Spawn position of the projectile
    /// @param endPos - The position which the projectile will naturally disappear
    /// @param direction - Direction of the projectile
    /// @param tileSize - TileSize of the projectile
    /// @param type - Type of projectile that was spawned
    /// @param parent - Parent QGraphicsItem of this object
    Projectile(const QPointF &startPos, const QPointF &endPos, Direction direction, int tileSize, ProjectileType type = ProjectileType::Normal, QGraphicsItem *parent = nullptr);
    /// @brief Object deletion function
    ~Projectile();

    /// @brief Overrides QGraphicsItem
    QRectF boundingRect() const override;
    /// @brief Overrides QGraphicsItem
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    ///@brief Function that displays the starting animation of the projectile (To show the projectile being launch and such)
    void startAnimation();

signals:
    void hitReached(const QPointF &hitPos);

private slots:
    void updateAnimation();

private:
    QPointF m_startPos;
    QPointF m_endPos;
    Direction m_direction;
    ProjectileType m_type;
    int m_tileSize;
    QTimer *m_timer;
    qreal m_progress;
    QVector<QPointF> m_trailPoints;
};

#endif // PROJECTILE_H
