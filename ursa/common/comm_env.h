// Copyright 2018 H-AXE
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "zmq.hpp"

#include "base/bin_stream.h"
#include "network/mailbox.h"
#include "network/mailbox_adress_book.h"
#include "network/mailbox_eventloop.h"
#include "network/mailbox_recver.h"
#include "network/mailbox_sender.h"
#include "network/mailbox_types.h"
#include "network/threadsafe_queue.h"
#include "network/zmq_helpers.h"

namespace axe {

namespace common {
/* One instance per process.
 *
 * <p>For each process there is a mailbox system:</p>
 * <ul>
 * <li>one mailbox eventloop thread responsible for dispatching messages within process and sending messages to other
 * processes;</li>
 * <li>one mailbox sender to provide sockets to neighbor processes, which is invoked by the eventloop thread; and</li>
 * <li>one mailbox receiver thread to listen for messages and deliver to eventloop through a threadsafe message
 * queue.<li>
* </ul>
 */
class CommEnv {
 public:
  CommEnv(const std::string& hostname, int recv_port);

  inline const auto& GetMailbox() { return mailbox_; }
  inline const auto& GetMailboxAddressBook() const { return mailbox_addr_book_; }
  inline const auto& GetZMQContext() { return zmq_context_; }
  inline const auto& GetMailboxRecver() { return mailbox_recver_; }
  // TODO(tatiana): should wrap internal communication
  inline const auto& GetEventQueue() { return queue_; }
  inline const auto& GetHostName() { return hostname_; }
  inline const auto& GetPort() { return port_; }

  void RegisterChannelActor(int channel_id, const std::string& addr);
  void RegisterChannelActor(int channel_id, network::ThreadsafeQueue<std::shared_ptr<base::BinStream>>* queue);

  void BroadCast(int channel_id, std::shared_ptr<base::BinStream> bin_stream);

  void AddNeighborProcess(uint32_t process_id, const std::string& addr);

  void RemoveNeighborProcess(uint32_t process_id);

 private:
  std::string hostname_;
  int port_;
  std::shared_ptr<zmq::context_t> zmq_context_;
  std::shared_ptr<network::MailboxEventQueue> queue_;
  std::unique_ptr<network::MailboxEventLoop> mailbox_event_loop_;
  std::unique_ptr<network::MailboxSender> mailbox_sender_;
  std::unique_ptr<network::MailboxRecver> mailbox_recver_;
  std::unique_ptr<network::MailboxAddressBook> mailbox_addr_book_;  // not including self
  std::unordered_map<int, zmq::socket_t> handler_connectors_;       // channel id, actor socket
  std::unique_ptr<network::Mailbox> mailbox_;
};

}  // namespace common
}  // namespace axe
