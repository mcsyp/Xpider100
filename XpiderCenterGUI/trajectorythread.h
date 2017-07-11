#ifndef TRAJECTORYTHREAD_H
#define TRAJECTORYTHREAD_H
#include <QThread>

#include <vector>

#include "global_xpider.h"
#include "trajectoryplanner.h"

class TrajectoryThread : public QThread
{
  Q_OBJECT
public:
  explicit TrajectoryThread(QObject* parent=NULL);
  virtual void run();

public:
  TrajectoryPlanner planner_;
  std::vector<xpider_opti_t> xpider_queue_;
};

#endif // TRAJECTORYTHREAD_H
