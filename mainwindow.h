#pragma once

#include <QSystemTrayIcon>
#include <QWidget>

class MultiDirWidget;
class FileSystemModel;

class QSettings;
class QAction;
class QMenu;
class QLineEdit;
class QStackedWidget;
class QActionGroup;

class MainWindow : public QWidget
{
Q_OBJECT
public:
  explicit MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

protected:
  void keyPressEvent (QKeyEvent *event) override;

private:
  void updateTrayMenu ();
  void trayClicked (QSystemTrayIcon::ActivationReason reason);
  void toggleVisible ();
  void editSettings ();
  void openConsole (const QString &path);
  void setCheckUpdates (bool isOn);
  void addWidget ();
  void activateFindMode ();
  void showAbout ();

  void updateGroupsMenu ();
  MultiDirWidget * addGroup ();
  void removeGroup ();
  void updateCurrentGroup (QAction *groupAction);
  MultiDirWidget * group (int index) const;
  QAction * groupAction (int index) const;


  FileSystemModel *model_;
  QLineEdit *findEdit_;
  QSystemTrayIcon *tray_;
  QStackedWidget *groups_;
  QAction *toggleAction_;
  QMenu *groupsMenu_;
  QAction *closeGroupAction_;
  QActionGroup *groupsActions_;
  QString consoleCommand_;
  bool checkUpdates_;
};
