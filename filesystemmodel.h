#pragma once

#include <QFileSystemModel>

class FileOperation;

class FileSystemModel : public QFileSystemModel
{
Q_OBJECT
public:
  enum Column : int
  {
    Name, Size, Type, Date
  };

  explicit FileSystemModel (QObject *parent = nullptr);
  bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row,
                     int column, const QModelIndex &parent) override;
  bool setData (const QModelIndex &index, const QVariant &value, int role) override;

signals:
  void fileOperation (QSharedPointer<FileOperation> operation);
};
