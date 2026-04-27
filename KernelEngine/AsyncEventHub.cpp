#include "AsyncEventHub.h"

std::unique_ptr<AsyncEventBus> GlobalEventBus::inst_ = nullptr;