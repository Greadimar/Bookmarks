#include "bookmark.h"


void Bookmark::applyPainter(BookmarkPainter &bp)
{
    bp.paint(*this);
}

void MultiBookmark::applyPainter(BookmarkPainter &bp)
{
    bp.paint(*this);
}
