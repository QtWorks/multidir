#include "proxymodel.h"
#include "filesystemmodel.h"
#include "constants.h"
#include "backgroundreader.h"

#include <QDateTime>
#include <QPixmapCache>
#include <QImageReader>
#include <QThread>


ProxyModel::ProxyModel (QFileSystemModel *model, QObject *parent) :
  QSortFilterProxyModel (parent),
  model_ (model),
  showDirs_ (true),
  showHidden_ (false),
  showThumbnails_ (false),
  nameFilter_ (),
  current_ (),
  iconReaderThread_ (new QThread (this))
{
  setSourceModel (model);

  auto *reader = new BackgroundReader;
  connect (this, &ProxyModel::iconRequested,
           reader, &BackgroundReader::readIcon);
  connect (reader, &BackgroundReader::iconRead,
           this, &ProxyModel::updateIcon);

  reader->moveToThread (iconReaderThread_);
  connect (iconReaderThread_, &QThread::finished,
           reader, &QObject::deleteLater);
  iconReaderThread_->start ();
}

ProxyModel::~ProxyModel ()
{
  iconReaderThread_->quit ();
  iconReaderThread_->wait (2000);
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

bool ProxyModel::showHidden () const
{
  return showHidden_;
}

void ProxyModel::setShowHidden (bool showHidden)
{
  showHidden_ = showHidden;
  invalidateFilter ();
}


bool ProxyModel::showThumbnails () const
{
  return showThumbnails_;
}

void ProxyModel::setShowThumbnails (bool isOn)
{
  showThumbnails_ = isOn;
  emit dataChanged (index (0, FileSystemModel::Name), index (rowCount () - 1, FileSystemModel::Size),
                    {Qt::DecorationRole});
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
    const auto canFilter = !showDirs_ || !showHidden_ || !nameFilter_.isEmpty ();
    if (canFilter)
    {
      const auto info = model_->fileInfo (model_->index (sourceRow, 0, sourceParent));
      if (!showDirs_ && info.isDir ())
      {
        return false;
      }
      if (!showHidden_ && info.isHidden () && info.fileName () != constants::dotdot)
      {
        return false;
      }
      if (!nameFilter_.isEmpty () && !QDir::match (nameFilter_, info.fileName ()))
      {
        return false;
      }
    }
  }
  return true;
}


QVariant ProxyModel::data (const QModelIndex &index, int role) const
{
  if (role == Qt::BackgroundRole)
  {
    const auto info = model_->fileInfo (mapToSource (index));
    if (info.isDir ())
    {
      if (!info.isExecutable () || !info.isReadable ())
      {
        return QColor (255,153,153);
      }
      return QColor (204,255,255);
    }
    if (info.isExecutable ())
    {
      return QColor (255,204,153);
    }
    else if (!info.isReadable ())
    {
      return QColor (255,153,153);
    }
    return {};
  }

  if (showThumbnails_ && role == Qt::DecorationRole && index.column () == FileSystemModel::Name)
  {
    const auto info = model_->fileInfo (mapToSource (index));
    if (QImageReader::supportedImageFormats ().contains (info.suffix ().toUtf8 ()))
    {
      const auto path = info.absoluteFilePath ();
      if (auto cached = QPixmapCache::find (path))
      {
        return QIcon (*cached);
      }
      emit const_cast<ProxyModel *>(this)->iconRequested (path);
    }
    return QSortFilterProxyModel::data (index, role);
  }

  if (role == Qt::TextAlignmentRole && index.column () == FileSystemModel::Size)
  {
    return int (Qt::AlignVCenter) | Qt::AlignRight;
  }

  return QSortFilterProxyModel::data (index, role);
}


void ProxyModel::updateIcon (const QString &fileName, const QPixmap &pixmap)
{
  auto index = mapFromSource (model_->index (fileName));
  if (index.isValid ())
  {
    QPixmapCache::insert (fileName, pixmap);
    emit dataChanged (index, index, {Qt::DecorationRole});
  }
}


QVariant ProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical && role == Qt::DisplayRole)
  {
    return section + 1;
  }
  return QSortFilterProxyModel::headerData (section, orientation, role);
}


Qt::ItemFlags ProxyModel::flags (const QModelIndex &index) const
{
  auto flags = QSortFilterProxyModel::flags (index);
  if (index.column () != 0 || index.data ().toString () == constants::dotdot)
  {
    flags &= ~Qt::ItemIsEditable;
  }
  return flags;
}

bool ProxyModel::lessThan (const QModelIndex &left, const QModelIndex &right) const
{
  // keep .. on top
  const auto l = model_->fileInfo (left);
  if (l.fileName () == constants::dotdot)
  {
    return sortOrder () == Qt::AscendingOrder;
  }
  const auto r = model_->fileInfo (right);
  if (r.fileName () == constants::dotdot)
  {
    return sortOrder () != Qt::AscendingOrder;
  }

  // keep folders on top
  if (l.isDir () != r.isDir ())
  {
    return (sortOrder () == Qt::AscendingOrder ? l.isDir () : !l.isDir ());
  }


  switch (sortColumn ())
  {
    case FileSystemModel::Column::Name:
      return QString::localeAwareCompare (l.fileName (),r.fileName ()) < 0;

    case FileSystemModel::Column::Size:
      return model_->size (left) < model_->size (right);

    case FileSystemModel::Column::Type:
      return model_->type (left) < model_->type (right);

    case FileSystemModel::Column::Date:
      return model_->lastModified (left) < model_->lastModified (right);
  }
  return QSortFilterProxyModel::lessThan (left, right);
}
