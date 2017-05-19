#pragma once

#include <QWidget>

class Tile;

class QSettings;

class TiledView : public QWidget
{
public:
  explicit TiledView (QWidget *parent = 0);
  ~TiledView ();

  void add (QWidget &widget);
  void remove (QWidget &widget);

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  QSize sizeHint () const override;
  QSize minimumSizeHint () const override;

protected:
  void resizeEvent (QResizeEvent *event) override;

  void mousePressEvent (QMouseEvent *event) override;
  void mouseReleaseEvent (QMouseEvent *event) override;
  void mouseMoveEvent (QMouseEvent *event) override;
  void dragEnterEvent (QDragEnterEvent *event) override;
  void dropEvent (QDropEvent *event) override;

private:
  void reserveTile ();
  void emplace (QWidget *widget);
  void updateTilesGeometry ();
  void updateTilesBorders ();

  void addRow ();
  void addColumn ();
  void removeRow (int index);
  void removeColumn (int index);
  void cleanupDimensions ();

  Tile * findTile (QWidget *widget) const;
  Tile * findTile (const QPoint &pos) const;
  Tile * findTileBorders (const QPoint &pos) const;

  //! Distribute given size over items (rows or cols).
  void adjustSizes (QList<int> &sizes, int sizeToFill) const;
  //! Add new row or col.
  void addDimesion (QList<int> &sizes, const QList<int> &opposite, int fullSize, bool isRow);
  //! Remove given row or col.
  void removeDimesion (QList<int> &sizes, int (Tile::*field), int index);
  //! Size currently occupied by tiles.
  QSize tilesSize () const;

  void setResize (int index, Qt::Orientations dir);
  void handleResizing (const QPoint &current);
  void resizeDimension (int index, QList<int> &sizes, int diff);
  void handleSpanning (const QPoint &current);
  bool spanTile (Tile &tile, const QPoint &diff, bool isRow);


  //! Row sizes,
  QList<int> rows_;
  //! Column sizes.
  QList<int> columns_;
  QList<Tile> tiles_;
  int spacing_;
  int margin_;
  QPoint dragStartPos_;

  int resizeIndex_;
  Qt::Orientations resizeDir_;
};