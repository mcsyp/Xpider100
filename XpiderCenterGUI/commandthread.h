#ifndef COMMANDTHREAD_H
#define COMMANDTHREAD_H

#include <QThread>
#include <QStringList>
#include <QMap>

class CommandParser;
class CommandThread : public QThread
{
  Q_OBJECT
public:
  static constexpr int CMD_MIN_LEN=2;

  explicit CommandThread(QObject *parent = 0);
  ~CommandThread();

  void StartCommandChain(QString &command_text);
  virtual void run();
signals:

public slots:
protected:

protected:
  QMap<QString,CommandParser*> cmd_map_;
  QString cmd_text_;
  int index_;
};


class CommandParser : public QObject
{
  Q_OBJECT
public:
  explicit CommandParser(QObject *parent = 0):QObject(parent){}

  /*purpose: parse the input command line and run
   *input:
   * @argv, the seperated args
   *return:
   * true if the command is already executed
   * false, if the command is invalid or not reconigzed
   */
  virtual bool Exec(QStringList argv)=0;
  virtual const QString & Example() const=0;
  virtual const QString & Key() const=0;//the command word
};
#endif // COMMANDTHREAD_H
