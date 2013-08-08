/**
 * Author: Rajesh
 * Description: external interfaces
 */

#include "common.hpp"

#ifndef RAD_INTERFACES_HPP
#define RAD_INTERFACES_HPP

/**
 * Initialize OpenNI using an XML file specified by its filename.
 */
void nipub_init(const char* fn);

/**
 * Register the callback sub. The SubscriberCB is a function pointer type
 * where the function signiture is void* (*SubscriberCB)(void*).
 */
void nipub_subscribe(SubscriberCB sub);

/**
 * Make the publisher spin on its own thread so that it can publish data
 * without blocking others.
 */
void* nipub_spin(void* tid);

/**
 * Must be called when the publisher is no longer needed.
 */
void nipub_destroy();

/**
 * Only use it by register it as a callback to the publisher.
 */
void* fba_notify(void* tid);

/**
 * The registered callback will be put on a detached thread upon action
 * completion.
 */
ActComCBID fba_registerCompleteCB(ActionCB hCompleteCB);

/**
 * Unregister an action callback.
 */
void fba_unregisterCompleteCB(ActComCBID id);

/**
 * Return meaningful data after the first call to sd_notify.
 */
void sd_getSceneDims(int* sceneWidth, int* sceneHeight);

/**
 * Only use it by registering it to a publisher.
 */
void* sd_notify(void* tid);

/**
 * Must be called when you are done with the scene drawer.
 */
void sd_destroy();

#endif // RAD_INTERFACES_HPP
