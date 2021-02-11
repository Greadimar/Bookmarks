#include "timeaxis.h"

const msecs TimeInfo::msecsInDay = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(24));
const msecs TimeInfo::msecsIn3hours = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(3));
const msecs TimeInfo::msecsInhour = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(1));

