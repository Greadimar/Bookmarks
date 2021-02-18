#include "rulerview.h"
#include "extratablemodel.h"
#include <QHeaderView>
#include "extratableview.h"
void RuleView::initElements(){
//    auto t = new TestItem();

//    m_scene.addItem(t);
    //   setSceneRect();
    ///1     t->setPos(111,111);
    m_ruler = new Ruler(plt, renderInfo, m_ta);
    m_line = new BookmarksLine(plt, m_bkmngr, renderInfo, m_ta );

    //init table

    auto model = new ExtraTableModel(m_ta, m_bkmngr, renderInfo.renderStep, this);
    extraTable = new ExtraTableView(tableIsHovered, this);
    extraTable->setModel(model);
    extraTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    extraTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    extraTable->resizeRowsToContents();

    //init positions
    m_ruler->setPos(0,0);
    m_scene.addItem(m_ruler);
    m_scene.addItem(m_line);
    proxyWtTable = m_scene.addWidget(extraTable);
    renderInfo.tableSize = extraTable->sizeHint();
    extraTable->setVisible(false);
   // proxyWtTable->setVisible(false);
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMinimumHeight(400);
    this->setMinimumWidth(400);

}

void RuleView::renderTimer(){
    auto cur = std::chrono::system_clock::now();
    auto sinceLastRender = cur - renderInfo.lastRender;
    if (sinceLastRender > msecs(renderInfo.renderStep)) sinceLastRender = msecs(0);
    auto&& nextRender = renderInfo.renderStep - sinceLastRender;
    renderInfo.lastRender = cur;
    QTimer::singleShot(std::chrono::duration_cast<msecs>(nextRender), this, [=](){
        toRender();
        renderTimer();
    });
}

void RuleView::toRender(){
    m_ruler->update();
    m_line->update();
    extraTable->setVisible(m_line->getMouseOnMultiBk() || tableIsHovered);
    proxyWtTable->setPos(m_line->getPosForExtraTable());
    extraTable->move(m_line->getPosForExtraTable().x(), renderInfo.bookmarksBottomY);
}
