#include "TreeWidgetItem.h"
#include <QCollator>

TreeWidgetItem::TreeWidgetItem(QTreeWidget *parent) : QTreeWidgetItem(parent) {}

TreeWidgetItem::~TreeWidgetItem() {}

bool TreeWidgetItem::operator<(const QTreeWidgetItem &other) const {
  int column = treeWidget()->sortColumn();
  QCollator collator;
  collator.setNumericMode(true);
  return collator.compare(text(column), other.text(column)) < 0;
}
