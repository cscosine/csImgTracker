#pragma once
#include <QTreeWidgetItem>

class TreeWidgetItem : public QTreeWidgetItem {
public:
  TreeWidgetItem(QTreeWidget *parent);
  virtual ~TreeWidgetItem();

private:
  bool operator<(const QTreeWidgetItem &other) const override;
};
