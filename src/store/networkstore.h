#ifndef __KIDS_STORE_NETWORKSTORE_H_
#define __KIDS_STORE_NETWORKSTORE_H_

#include <vector>
#include <list>
#include <string>

#include "store.h"

class NetworkStore : public Store {
 public:
  class Agent;

  NetworkStore(StoreConfig *conf, aeEventLoop *el);
  ~NetworkStore();

  virtual bool Open();
  virtual bool IsOpen();
  virtual void Close();

  virtual StoreState State() { return state_; }

  virtual void Cron();

  void OnMsgSent();
  void OnDisconnect();

  aeEventLoop *eventl_;

 private:
  NetworkStore() = delete;

  virtual bool DoAddMessage(const Message *msg);

  Agent *agent_;

  std::string host_;
  int port_;
  std::string socket_;

  time_t reconnect_interval_;
  time_t last_reconnect_;

  std::list<const Message*> msg_chain_;
  std::list<bool> del_chain_;
  Store::StoreState state_;
};


#endif  // __KIDS_STORE_NETWORKSTORE_H_
