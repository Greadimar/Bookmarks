#ifndef COMMON_H
#define COMMON_H
#include <chrono>
#include <QSharedPointer>

class Bookmark;
using hours = std::chrono::hours;
template <typename T, typename U>
auto duration_cast(U const& u) -> decltype(std::chrono::duration_cast<T>(u)){
    return std::chrono::duration_cast<T>(u);
}
using msecs = std::chrono::milliseconds;
using ShpBookmark = QSharedPointer<Bookmark>;
#endif // COMMON_H
