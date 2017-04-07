#include "proxymodel.h"

#include <QFileSystemModel>

#include <QDebug>

ProxyModel::ProxyModel (QFileSystemModel *model, QObject *parent) :
  QSortFilterProxyModel (parent),
  model_ (model),
  showDirs_ (true),
  nameFilter_ (),
  current_ ()
{
  setSourceModel (model);
}

bool ProxyModel::showDirs () const
{
  return showDirs_;
}

void ProxyModel::setShowDirs (bool showDirs)
{
  showDirs_ = showDirs;
  invalidateFilter ();
}

void ProxyModel::setNameFilter (const QString &name)
{
  nameFilter_ = name;
  invalidateFilter ();
}

void ProxyModel::setCurrent (const QModelIndex &current)
{
  current_ = mapToSource (current);
  invalidateFilter ();
}

bool ProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const
{
  if (sourceParent == current_)
  {
    if (!showDirs_ || !nameFilter_.isEmpty ())
    {
      auto index = model_->index (sourceRow, 0, sourceParent);
      if (!showDirs_ && model_->isDir (index))
      {
        return false;
      }
      if (!nameFilter_.isEmpty () && !QDir::match (nameFilter_, index.data ().toString ()))
      {
        return false;
      }
    }
  }
  return true;
}


QVariant ProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical && role == Qt::DisplayRole)
  {
    return section + 1;
  }
  return QSortFilterProxyModel::headerData (section, orientation, role);
}
