#include "rulerview.h"
#include "extratablemodel.h"


void RuleView::initElements(){
//    auto t = new TestItem();

//    m_scene.addItem(t);
    //   setSceneRect();
    ///1     t->setPos(111,111);
    m_ruler = new Ruler(plt, renderInfo, m_ta);
    m_line = new BookmarksLine(plt, m_bkmngr, renderInfo, m_ta );
    auto model = new ExtraTableModel(m_ta, m_bkmngr, renderInfo.renderStep, this);
    extraTable = new QTableView(this);
    extraTable->setModel(model);
    m_ruler->setPos(0,0);
    m_scene.addItem(m_ruler);
    m_scene.addItem(m_line);
    proxyWtTable = m_scene.addWidget(extraTable);
    extraTable->setVisible(false);
   // proxyWtTable->setVisible(false);
    this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMinimumHeight(100);
    this->setMinimumWidth(100);

}
