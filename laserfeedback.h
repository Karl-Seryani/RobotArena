#include <QGraphicsObject>
#include <QParallelAnimationGroup>

/**
 * @brief This class uses property animation to create feedback for lasers
 */
class LaserFeedback : public QGraphicsObject {
    Q_OBJECT
public:
    /// @brief Constructor that sends a feedback when a laser is created
    /// @param start - The starting point of the laser
    /// @param end - The end point of the laser
    /// @param parent - The parent QGraphicsItem of this object
    LaserFeedback(const QPointF &start, const QPointF &end, QGraphicsItem *parent = nullptr);
    ~LaserFeedback();

    /// @brief QGraphicsItem override
    QRectF boundingRect() const override;
    /// @brief QGraphicsItem override
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    QPointF m_start;
    QPointF m_end;
    QParallelAnimationGroup* animationGroup;
};
