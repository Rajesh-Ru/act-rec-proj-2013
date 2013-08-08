/**
 * Author: Rajesh
 * Description: an action object constantly get sensory information from a
 *   SensorDataPub, and invoke registered callback when the action is
 *   detected.
 */

#include "SensorDataSub.hpp"

#ifndef ACTION_HPP
#define ACTION_HPP

namespace rad
{
typedef void (*ActionCallbackHandle)(RADenum type, int size,
				     const void* pParam);

typedef int ActComCBID;
typedef int ActMotCBID;

typedef enum ActionState
{
  UNRECOGNIZED = 0,
  START = 1,
  IN_PROGRESS = 2,
  RECOGNIZED = 3
} ActionState;


class Action : public SensorDataSub
{
public:
  virtual ~Action() {}
  virtual ActMotCBID registerMotionCB(ActionCallbackHandle hMotionCB) = 0;
  virtual ActComCBID registerCompleteCB(ActionCallbackHandle hCompleteCB) = 0;
  virtual void unregisterMotionCB(ActMotCBID id) = 0;
  virtual void unregisterCompleteCB(ActComCBID id) = 0;
};
}

#endif // ACTION_HPP
