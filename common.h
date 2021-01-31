#ifndef COMMON_H
#define COMMON_H
#include <chrono>
#include <QSharedPointer>

class Bookmark;

using msecs = std::chrono::milliseconds;
using ShpBookmark = QSharedPointer<Bookmark>;
#endif // COMMON_H
